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


struct perf_event * __percpu *sample_hbp;
struct perf_event * __percpu *brkp;

static volatile atomic_t test;
static unsigned long kaddr = 0;
module_param(kaddr, ulong, 0644);
MODULE_PARM_DESC(kaddr, "Kernel addr to monitor; this module will report any"
			" read/write operations on the kernel symbol");

static void sample_hbp_handler(struct perf_event *bp,
			       struct perf_sample_data *data,
			       struct pt_regs *regs);

static void brk_handler(struct perf_event *bp,
			       struct perf_sample_data *data,
			       struct pt_regs *regs)
{
	struct perf_event_attr attr;
	/* watchpoint is prohibit, so can read the val */
	printk("[watchpoint]============ new val:%x ==========  \n", *(int *)kaddr);
	unregister_wide_hw_breakpoint(brkp);
	hw_breakpoint_init(&attr);
	attr.bp_addr = kaddr; 
	attr.bp_len = HW_BREAKPOINT_LEN_4;
	attr.bp_type = HW_BREAKPOINT_W | HW_BREAKPOINT_R;
	sample_hbp = register_wide_hw_breakpoint(&attr, sample_hbp_handler, NULL);
	if (IS_ERR((void __force *)sample_hbp)) {
		printk("register hw watchponit fail.\n");
	}
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
	if(( *(unsigned int *)ret_addr & 0xf00f) == 0x8001) {
	#else  
	if(( *(unsigned int *)ret_addr & 0xff00000) == 0x1800000) {
	#endif
		ret_addr += 12; //arm32 architecture user ldrex and strex
	} else {
#else
	ret_addr = regs->pc; // arm64 lr/x30
	#if defined __ARM_BIG_ENDIAN
	/* big endian strx/ldrx instruct */
	/* armv8 atomic implenment according to strx/ldrx*/
	if(( *(unsigned int *)ret_addr & 0x3f) == 0x8) {
	#else  
	if(( *(unsigned int *)ret_addr & 0x3f000000) == 0x8000000) {
	#endif
		ret_addr += 8;
	} else {
#endif
		ret_addr += 4;
	}
	need_sing_step = 1;
#endif
	if(need_sing_step) {
		struct perf_event_attr attr;
		hw_breakpoint_init(&attr);
		attr.bp_addr = ret_addr; 
		attr.bp_len = HW_BREAKPOINT_LEN_4;
		attr.bp_type = HW_BREAKPOINT_X;
		brkp = register_wide_hw_breakpoint(&attr, brk_handler, NULL);
		if (IS_ERR((void __force *)brkp)) {
			printk("register brk fail.\n");
		}
		/* watchpoint is prohibit, so can read the val */
		unregister_wide_hw_breakpoint(sample_hbp);
		printk(KERN_INFO "[watchpoint]%p value is changed old val: %x\n", kaddr, *(int *)kaddr);
	}
	/* to do */
	dump_stack();
	printk(KERN_INFO "Dump stack from sample_hbp_handler\n");
}

static int __init hw_break_module_init(void)
{
	int ret;
	struct perf_event_attr attr;

	if(!kaddr)
		kaddr = &test;

	hw_breakpoint_init(&attr);
	attr.bp_addr = kaddr; 
	attr.bp_len = HW_BREAKPOINT_LEN_4; /* monitor data size */
	attr.bp_type = HW_BREAKPOINT_W | HW_BREAKPOINT_R;

	sample_hbp = register_wide_hw_breakpoint(&attr, sample_hbp_handler, NULL);
	if (IS_ERR((void __force *)sample_hbp)) {
		ret = PTR_ERR((void __force *)sample_hbp);
		goto fail;
	}
	printk(KERN_INFO "HW Breakpoint for %p installed\n", kaddr);

	if(kaddr == &test) {
		*(int*)&test = 0xfe;
		printk(KERN_INFO "HW Breakpoint for %p [%x] test\n", kaddr, test.counter);

		atomic_inc(&test);
		printk(KERN_INFO "HW Breakpoint for %p [%x] test\n", kaddr, *(int *)kaddr);
	}

	return 0;

fail:
	printk(KERN_INFO "Breakpoint <%p> registration failed: %d\n", kaddr, ret);

	return ret;
}

static void __exit hw_break_module_exit(void)
{
	if (!IS_ERR((void __force *)sample_hbp)) {
		unregister_wide_hw_breakpoint(sample_hbp);
	}
	printk(KERN_INFO "HW Breakpoint for %p write uninstalled\n", kaddr);
}

module_init(hw_break_module_init);
module_exit(hw_break_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("K.Prasad");
MODULE_DESCRIPTION("ksym breakpoint");
