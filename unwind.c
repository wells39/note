#include <string.h>

/* Dummy functions to avoid linker complaints */
void __aeabi_unwind_cpp_pr0(void)
{
};

void __aeabi_unwind_cpp_pr1(void)
{
};

void __aeabi_unwind_cpp_pr2(void)
{
};

enum unwind_reason_code {
	URC_OK = 0,			/* operation completed successfully */
	URC_CONTINUE_UNWIND = 8,
	URC_FAILURE = 9			/* unspecified failure of some kind */
};

struct unwind_idx {
	unsigned long addr_offset;
	unsigned long insn;
};

struct unw_info
{
  unsigned long pc;
  unsigned long so_addr;
  const char* name;
};

struct unwind_ctrl_block {
	unsigned long vrs[16];		/* virtual register set */
	const unsigned long *insn;	/* pointer to the current instructions word */
	unsigned long sp_high;		/* highest value of sp allowed */
	/*
	 * 1 : check for stack overflow for each register pop.
	 * 0 : save overhead if there is plenty of stack remaining.
	 */
	int check_each_pop;
	int entries;			/* number of entries left to interpret */
	int byte;			/* current byte number in the instructions word */
};

struct stackframe {
    /*
     * FP member should hold R7 when CONFIG_THUMB2_KERNEL is enabled
     * and R11 otherwise.
     */
    unsigned long fp;
    unsigned long sp;
    unsigned long lr;
    unsigned long pc;
};

typedef struct backtrace
{
	void *function;
	void *address;
	const char *name;
} backtrace_t;

enum regs {
#ifdef _THUMB2_
	FP = 7,
#else
	FP = 11,
#endif
	SP = 13,
	LR = 14,
	PC = 15
};

/* Convert a prel31 symbol to an absolute address */
#define prel31_to_addr(ptr)				\
({							\
	/* sign-extend to 32 bits */			\
	long offset = (((long)*(ptr)) << 1) >> 1;	\
	(unsigned long)(ptr) + offset;			\
})

static struct unwind_idx* _exidx_start;
static struct unwind_idx* _exidx_end;
register unsigned long current_stack_pointer asm ("sp");

/*
 * Binary search in the unwind index. The entries are
 * guaranteed to be sorted in ascending order by the linker.
 *
 * start = first entry
 * stop - 1 = last entry
 */
#if defined __ARMEL__ || defined __ARMEB__
static const struct unwind_idx *search_index(unsigned long addr,
				       const struct unwind_idx *start,
				       const struct unwind_idx *stop)
{
	unsigned long addr_prel31;

	/* prel31 for address relavive to start */
	addr_prel31 = (addr - (unsigned long)start) & 0x7fffffff;

	while (start < stop - 1) {
		const struct unwind_idx *mid = start + ((stop - start) >> 1);

		/*
		 * As addr_prel31 is relative to start an offset is needed to
		 * make it relative to mid.
		 */
		if (addr_prel31 - ((unsigned long)mid - (unsigned long)start) <
				mid->addr_offset)
			stop = mid;
		else {
			/* keep addr_prel31 relative to start */
			addr_prel31 -= ((unsigned long)mid -
					(unsigned long)start);
			start = mid;
		}
	}

	if (start->addr_offset <= addr_prel31)
		return start;
	else {
		return 0;
	}
}

static const struct unwind_idx *unwind_find_idx(unsigned long addr)
{
	const struct unwind_idx *idx = 0;

	/* main unwind table */
	idx = search_index(addr, _exidx_start, _exidx_end);
	return idx;
}

static unsigned long unwind_get_byte(struct unwind_ctrl_block *ctrl)
{
	unsigned long ret;

	if (ctrl->entries <= 0) {
		return 0;
	}

	ret = (*ctrl->insn >> (ctrl->byte * 8)) & 0xff;

	if (ctrl->byte == 0) {
		ctrl->insn++;
		ctrl->entries--;
		ctrl->byte = 3;
	} else
		ctrl->byte--;

	return ret;
}

/* Before poping a register check whether it is feasible or not */
static int unwind_pop_register(struct unwind_ctrl_block *ctrl,
				unsigned long **vsp, unsigned int reg)
{
	if (ctrl->check_each_pop)
		if (*vsp >= (unsigned long *)ctrl->sp_high)
			return -URC_FAILURE;

	ctrl->vrs[reg] = *(*vsp);
	(*vsp)++;
	return URC_OK;
}

/* Helper functions to execute the instructions */
static int unwind_exec_pop_subset_r4_to_r13(struct unwind_ctrl_block *ctrl,
						unsigned long mask)
{
	unsigned long *vsp = (unsigned long *)ctrl->vrs[SP];
	int load_sp, reg = 4;

	load_sp = mask & (1 << (13 - 4));
	while (mask) {
		if (mask & 1)
			if (unwind_pop_register(ctrl, &vsp, reg))
				return -URC_FAILURE;
		mask >>= 1;
		reg++;
	}
	if (!load_sp)
		ctrl->vrs[SP] = (unsigned long)vsp;

	return URC_OK;
}

static int unwind_exec_pop_r4_to_rN(struct unwind_ctrl_block *ctrl,
					unsigned long insn)
{
	unsigned long *vsp = (unsigned long *)ctrl->vrs[SP];
	int reg;

	/* pop R4-R[4+bbb] */
	for (reg = 4; reg <= 4 + (insn & 7); reg++)
		if (unwind_pop_register(ctrl, &vsp, reg))
				return -URC_FAILURE;

	if (insn & 0x8)
		if (unwind_pop_register(ctrl, &vsp, 14))
				return -URC_FAILURE;

	ctrl->vrs[SP] = (unsigned long)vsp;

	return URC_OK;
}

static int unwind_exec_pop_subset_r0_to_r3(struct unwind_ctrl_block *ctrl,
						unsigned long mask)
{
	unsigned long *vsp = (unsigned long *)ctrl->vrs[SP];
	int reg = 0;

	/* pop R0-R3 according to mask */
	while (mask) {
		if (mask & 1)
			if (unwind_pop_register(ctrl, &vsp, reg))
				return -URC_FAILURE;
		mask >>= 1;
		reg++;
	}
	ctrl->vrs[SP] = (unsigned long)vsp;

	return URC_OK;
}

/*
 * Execute the current unwind instruction.
 */
static int unwind_exec_insn(struct unwind_ctrl_block *ctrl)
{
	unsigned long insn = unwind_get_byte(ctrl);
	int ret = URC_OK;

	if ((insn & 0xc0) == 0x00)
		ctrl->vrs[SP] += ((insn & 0x3f) << 2) + 4;
	else if ((insn & 0xc0) == 0x40)
		ctrl->vrs[SP] -= ((insn & 0x3f) << 2) + 4;
	else if ((insn & 0xf0) == 0x80) {
		unsigned long mask;

		insn = (insn << 8) | unwind_get_byte(ctrl);
		mask = insn & 0x0fff;
		if (mask == 0) {
			return -URC_FAILURE;
		}

		ret = unwind_exec_pop_subset_r4_to_r13(ctrl, mask);
		if (ret)
			goto error;
	} else if ((insn & 0xf0) == 0x90 &&
		   (insn & 0x0d) != 0x0d)
		ctrl->vrs[SP] = ctrl->vrs[insn & 0x0f];
	else if ((insn & 0xf0) == 0xa0) {
		ret = unwind_exec_pop_r4_to_rN(ctrl, insn);
		if (ret)
			goto error;
	} else if (insn == 0xb0) {
		if (ctrl->vrs[PC] == 0)
			ctrl->vrs[PC] = ctrl->vrs[LR];
		/* no further processing */
		ctrl->entries = 0;
	} else if (insn == 0xb1) {
		unsigned long mask = unwind_get_byte(ctrl);

		if (mask == 0 || mask & 0xf0) {
			return -URC_FAILURE;
		}

		ret = unwind_exec_pop_subset_r0_to_r3(ctrl, mask);
		if (ret)
			goto error;
	} else if (insn == 0xb2) {
		unsigned long uleb128 = unwind_get_byte(ctrl);

		ctrl->vrs[SP] += 0x204 + (uleb128 << 2);
	} else {
		return -URC_FAILURE;
	}

error:
	return ret;
}

/*
 * Unwind a single frame starting with *sp for the symbol at *pc. It
 * updates the *pc and *sp with the new values.
 */
int unwind_frame(struct stackframe *frame)
{
	unsigned long low, high;
	const struct unwind_idx *idx;
	struct unwind_ctrl_block ctrl;

	/* store the highest address on the stack to avoid crossing it*/
	low = frame->sp;
	high = (((long long)low+128*1024) & (~(128*1024-1))) + 4*1024;
	ctrl.sp_high = high < 0xffffffff? high : 0xffffffff;

	idx = unwind_find_idx(frame->pc);
	if (!idx) {
		return -URC_FAILURE;
	}

	ctrl.vrs[FP] = frame->fp;
	ctrl.vrs[SP] = frame->sp;
	ctrl.vrs[LR] = frame->lr;
	ctrl.vrs[PC] = 0;

	if (idx->insn == 1)
		/* can't unwind */
		return -URC_FAILURE;
	else if ((idx->insn & 0x80000000) == 0)
		/* prel31 to the unwind table */
		ctrl.insn = (unsigned long *)prel31_to_addr(&idx->insn);
	else if ((idx->insn & 0xff000000) == 0x80000000)
		/* only personality routine 0 supported in the index */
		ctrl.insn = &idx->insn;
	else {
		return -URC_FAILURE;
	}

	/* check the personality routine */
	if ((*ctrl.insn & 0xff000000) == 0x80000000) {
		ctrl.byte = 2;
		ctrl.entries = 1;
	} else if ((*ctrl.insn & 0xff000000) == 0x81000000) {
		ctrl.byte = 1;
		ctrl.entries = 1 + ((*ctrl.insn & 0x00ff0000) >> 16);
	} else {
		return -URC_FAILURE;
	}

	ctrl.check_each_pop = 0;

	while (ctrl.entries > 0) {
		int urc;
		if ((ctrl.sp_high - ctrl.vrs[SP]) < sizeof(ctrl.vrs))
			ctrl.check_each_pop = 1;
		urc = unwind_exec_insn(&ctrl);
		if (urc < 0)
			return urc;
		if (ctrl.vrs[SP] < low || ctrl.vrs[SP] >= ctrl.sp_high) {
			printf("stack overhead.\n");
			return -URC_FAILURE;
		}
	}

	if (ctrl.vrs[PC] == 0)
		ctrl.vrs[PC] = ctrl.vrs[LR];

	/* check for infinite loop */
	if (frame->pc == ctrl.vrs[PC])
		return -URC_FAILURE;

	frame->fp = ctrl.vrs[FP];
	frame->sp = ctrl.vrs[SP];
	frame->lr = ctrl.vrs[LR];
	frame->pc = ctrl.vrs[PC];

	return URC_OK;
}
#endif

#if defined __AARCH64EL__ || defined __AARCH64EB__
#ifdef __ILP32__
int unwind_frame(struct stackframe *frame)
{
    unsigned long high, low;
    unsigned long fp = frame->fp;

    low  = frame->sp;
	// Only walk stack 132K, ilp32 address space is 4G
    high = (((long long)low+128*1024) & (~(128*1024-1))) + 4*1024 < 0xffffffff? high :0xffffffff;

    if (fp < low || fp > high - 0x18 || fp & 0xf)
        return -URC_FAILURE;

    frame->sp = fp + 0x10;
    frame->fp = *(unsigned long long *)(fp);
    frame->pc = *(unsigned long long *)(fp + 8);
    frame->lr = frame->pc; //compat arm32

    return URC_OK;
}
#endif
#endif

#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <link.h>

int callbk (struct dl_phdr_info *info, size_t size, void *data)
{
	const ElfW(Phdr) *phdr;
	int i;
	int match;
	struct unw_info * uinfo = (struct unw_info *)data;
	phdr = info->dlpi_phdr;
	match = 0;
	for(i = info->dlpi_phnum; i > 0; i--, phdr++) {
		if (phdr->p_type == PT_LOAD){
			long vaddr = phdr->p_vaddr + info->dlpi_addr;
			if (uinfo->pc >= vaddr && uinfo->pc < vaddr + phdr->p_memsz){
				match = 1;
				uinfo->so_addr = info->dlpi_addr;
				uinfo->name = info->dlpi_name;
			}
		}
		else if (phdr->p_type == PT_ARM_EXIDX) {
			_exidx_start = (struct unwind_idx *)(phdr->p_vaddr + info->dlpi_addr);
			_exidx_end = (struct unwind_idx *)(phdr->p_vaddr + info->dlpi_addr + phdr->p_memsz);
		}
	}
	return match;
}

int unwind_backtrace(void **buffer, int size)
{
	
	struct stackframe frame;
	struct unw_info uinfo;
	int count = 0;
	int rsize = size/sizeof(struct unw_info);
	struct unw_info *buf = (struct unw_info *)buffer;
	memset(buffer, 0, size);

	frame.fp = (unsigned long)__builtin_frame_address(0);
	frame.sp = current_stack_pointer;
	frame.lr = (unsigned long)__builtin_return_address(0);
	frame.pc = (unsigned long)unwind_backtrace;
	while (1) {
		int urc = 0;
		memset(&uinfo, 0, sizeof(struct unw_info));
		uinfo.pc = frame.lr;
		urc = dl_iterate_phdr(callbk, &uinfo);
		if (!urc)
			break;
		urc = unwind_frame(&frame);
		if (urc < 0)
			break;
		buf[count].pc = uinfo.pc;
		buf[count].so_addr = uinfo.so_addr;
		buf[count].name = uinfo.name;
		++count;
		if(count >= rsize)
			break;
	}
	return count;
}
