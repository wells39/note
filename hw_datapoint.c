/*
 * Modified from kernel source sample/hw_breakpoint
 * resolved watchpoint trigger infinitely and gives rise to a sofelockup finally
 */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */
#include <linux/kallsyms.h>

#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <asm/hw_breakpoint.h>


static struct perf_event * __percpu *sample_hbp;
static struct perf_event * __percpu *brkp;
static void (*m_arch_install_hw_breakpoint)(struct perf_event *bp);
static void (*m_arch_uninstall_hw_breakpoint)(struct perf_event *bp);

static volatile atomic_t test;
static unsigned long kaddr = 0;
static unsigned int size = 0;
static unsigned int type = 0;
module_param(type, uint, 0644);
MODULE_PARM_DESC(type, "\tOperation type to monitor; this param must be"
			" 1(read)  2(write)  3(read and write)");
module_param(size, uint, 0644);
MODULE_PARM_DESC(size, "\tKernel or User space addr size to monitor;"
			"this param must be 1 2 4 8, unit Byte");
module_param(kaddr, ulong, 0644);
MODULE_PARM_DESC(kaddr, "\tKernel or User space addr to monitor; this module will report"
			"any specified operations on the address");

/*********** to do **************/
static void pre_handler(void)
{
	printk(KERN_INFO "[watchpoint]%p value is changed old val: 0x%x\n", kaddr, *(int *)kaddr);
	dump_stack();
	printk(KERN_INFO "Dump stack from sample_hbp_handler\n");
}

static void post_handler(void)
{
	printk("[watchpoint]============ new val:0x%x ==========  \n", *(int *)kaddr);
}
/****************** **************/

static void brk_handler(struct perf_event *bp,
			       struct perf_sample_data *data,
			       struct pt_regs *regs)
{
	struct perf_event *event;
	event = *this_cpu_ptr(sample_hbp);
	/* watchpoint is prohibit, so can read the val */
	post_handler();
	m_arch_uninstall_hw_breakpoint(bp);
	m_arch_install_hw_breakpoint(event);
	preempt_enable();
}

static void sample_hbp_handler(struct perf_event *bp,
			       struct perf_sample_data *data,
			       struct pt_regs *regs)
{
	long volatile ret_addr = 0;
	int need_sing_step = 0;
#if defined CONFIG_ARM || CONFIG_ARM64
#ifdef CONFIG_ARM
	/* get exception info  */
	ret_addr = regs->ARM_pc; /* saved LR_abt */
	#if defined __ARM_BIG_ENDIAN
	/* big endian strx/ldrx instruct */
	/* armv8 atomic implenment according to strx/ldrx*/
	if((*(unsigned int *)ret_addr & 0xf00f) == 0x8001) {
	#else  
	if((*(unsigned int *)ret_addr & 0xff00000) == 0x1800000) {
	#endif
#else
	ret_addr = regs->pc; // arm64 lr/x30
	#if defined __ARM_BIG_ENDIAN
	/* big endian strx/ldrx instruct */
	/* armv8 atomic implenment according to strx/ldrx*/
	if((*(unsigned int *)ret_addr & 0x3f) == 0x8) {
	#else  
	if((*(unsigned int *)ret_addr & 0x3f000000) == 0x8000000) {
	#endif
#endif
		ret_addr += 12; //arm32 architecture user ldrex and strex
	} else {
		ret_addr += 4;
	}
	need_sing_step = 1;
	preempt_disable();
#endif
	if(need_sing_step) {
		int cpu;
		struct perf_event *bbp = per_cpu(*brkp, smp_processor_id());
		struct arch_hw_breakpoint *info = counter_arch_bp(bbp);
		info->address = ret_addr;
		m_arch_uninstall_hw_breakpoint(bp);
		m_arch_uninstall_hw_breakpoint(bbp); //uninstall the brk slot firstly.
		m_arch_install_hw_breakpoint(bbp);
		/* watchpoint is prohibit, so can read the val */
	}
	/* to do */
	pre_handler();
}

static int __init hw_break_module_init(void)
{
	int cpu, ret;
	struct perf_event_attr attr;

	if(!kaddr)
		kaddr = &test;

	m_arch_install_hw_breakpoint = kallsyms_lookup_name("arch_install_hw_breakpoint");
	m_arch_uninstall_hw_breakpoint = kallsyms_lookup_name("arch_uninstall_hw_breakpoint");
	if(!m_arch_install_hw_breakpoint || !m_arch_uninstall_hw_breakpoint){
		printk("__perf_event_disable/__perf_event_enable not found.\n");
		return -ENOSYS;
	}

	hw_breakpoint_init(&attr);
	attr.bp_addr = kaddr; 
	attr.bp_len = size; /* monitor data size */
	attr.bp_type = type;

	sample_hbp = register_wide_hw_breakpoint(&attr, sample_hbp_handler, NULL);
	if (IS_ERR((void __force *)sample_hbp)) {
		ret = PTR_ERR((void __force *)sample_hbp);
		goto fail;
	}

	attr.bp_addr = kaddr; /* why here must kaddr? or will miss the point */
	attr.bp_len = HW_BREAKPOINT_LEN_4;
	attr.bp_type = HW_BREAKPOINT_X;

	brkp = register_wide_hw_breakpoint(&attr, brk_handler, NULL);
	if (IS_ERR((void __force *)brkp)) {
		printk("register brk fail.\n");
		goto brk_fail;
	}
	printk(KERN_INFO "HW Breakpoint for %p installed\n", kaddr);

	if(kaddr == &test) {
		*(int*)&test = 0xfe;
		printk(KERN_INFO "HW Breakpoint for %p [%x] test\n", kaddr, test.counter);

		atomic_inc(&test);
		printk(KERN_INFO "HW Breakpoint for %p [%x] test\n", kaddr, *(int *)kaddr);
	}

	return 0;

brk_fail:
	unregister_wide_hw_breakpoint(sample_hbp);
fail:
	printk(KERN_INFO "Breakpoint <%p> registration failed: %d\n", kaddr, ret);

	return ret;
}

static void __exit hw_break_module_exit(void)
{
	if (!IS_ERR((void __force *)brkp)) {
		unregister_wide_hw_breakpoint(brkp);
		printk(KERN_INFO "HW Breakpoint for %p uninstalled\n",
			per_cpu(*brkp, smp_processor_id())->attr.bp_addr);
	}
	if (!IS_ERR((void __force *)sample_hbp)) {
		unregister_wide_hw_breakpoint(sample_hbp);
		printk(KERN_INFO "HW Watchpoint for %p uninstalled\n", kaddr);
	}
}

module_init(hw_break_module_init);
module_exit(hw_break_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("K.Prasad");
MODULE_DESCRIPTION("ksym breakpoint");
