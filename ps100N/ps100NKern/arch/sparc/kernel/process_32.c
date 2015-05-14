/*  linux/arch/sparc/kernel/process.c
 *
 *  Copyright (C) 1995, 2008 David S. Miller (davem@davemloft.net)
 *  Copyright (C) 1996 Eddie C. Dost   (ecd@skynet.be)
 */

/*
 * This file handles the architecture-dependent parts of process handling..
 */

#include <stdarg.h>

#include <linux/errno.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/stddef.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/user.h>
#include <linux/smp.h>
#include <linux/reboot.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/init.h>

#include <asm/auxio.h>
#include <asm/oplib.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/page.h>
#include <asm/pgalloc.h>
#include <asm/pgtable.h>
#include <asm/delay.h>
#include <asm/processor.h>
#include <asm/psr.h>
#include <asm/elf.h>
#include <asm/prom.h>
#include <asm/unistd.h>

/* 
 * Power management idle function 
 * Set in pm platform drivers (apc.c and pmc.c)
 */
void (*pm_idle)(void);
EXPORT_SYMBOL(pm_idle);

/* 
 * Power-off handler instantiation for pm.h compliance
 * This is done via auxio, but could be used as a fallback
 * handler when auxio is not present-- unused for now...
 */
void (*pm_power_off)(void) = machine_power_off;
EXPORT_SYMBOL(pm_power_off);

/*
 * sysctl - toggle power-off restriction for serial console 
 * systems in machine_power_off()
 */
int scons_pwroff = 1;

extern void fpsave(unsigned long *, unsigned long *, void *, unsigned long *);

struct task_struct *last_task_used_math = NULL;
struct thread_info *current_set[NR_CPUS];

#ifndef CONFIG_SMP

#define SUN4C_FAULT_HIGH 100

/*
 * the idle loop on a Sparc... ;)
 */
void cpu_idle(void)
{
	/* endless idle loop with no priority at all */
	for (;;) {
		if (ARCH_SUN4C) {
			static int count = HZ;
			static unsigned long last_jiffies;
			static unsigned long last_faults;
			static unsigned long fps;
			unsigned long now;
			unsigned long faults;

			extern unsigned long sun4c_kernel_faults;
			extern void sun4c_grow_kernel_ring(void);

			local_irq_disable();
			now = jiffies;
			count -= (now - last_jiffies);
			last_jiffies = now;
			if (count < 0) {
				count += HZ;
				faults = sun4c_kernel_faults;
				fps = (fps + (faults - last_faults)) >> 1;
				last_faults = faults;
#if 0
				printk("kernel faults / second = %ld\n", fps);
#endif
				if (fps >= SUN4C_FAULT_HIGH) {
					sun4c_grow_kernel_ring();
				}
			}
			local_irq_enable();
		}

		if (pm_idle) {
			while (!need_resched())
				(*pm_idle)();
		} else {
			while (!need_resched())
				cpu_relax();
		}
		preempt_enable_no_resched();
		schedule();
		preempt_disable();
		check_pgt_cache();
	}
}

#else

/* This is being executed in task 0 'user space'. */
void cpu_idle(void)
{
        set_thread_flag(TIF_POLLING_NRFLAG);
	/* endless idle loop with no priority at all */
	while(1) {
		while (!need_resched())
			cpu_relax();
		preempt_enable_no_resched();
		schedule();
		preempt_disable();
		check_pgt_cache();
	}
}

#endif

/* XXX cli/sti -> local_irq_xxx here, check this works once SMP is fixed. */
void machine_halt(void)
{
	local_irq_enable();
	mdelay(8);
	local_irq_disable();
	prom_halt();
	panic("Halt failed!");
}

void machine_restart(char * cmd)
{
	char *p;
	
	local_irq_enable();
	mdelay(8);
	local_irq_disable();

	p = strchr (reboot_command, '\n');
	if (p) *p = 0;
	if (cmd)
		prom_reboot(cmd);
	if (*reboot_command)
		prom_reboot(reboot_command);
	prom_feval ("reset");
	panic("Reboot failed!");
}

void machine_power_off(void)
{
	if (auxio_power_register &&
	    (strcmp(of_console_device->type, "serial") || scons_pwroff))
		*auxio_power_register |= AUXIO_POWER_OFF;
	machine_halt();
}

#if 0

static DEFINE_SPINLOCK(sparc_backtrace_lock);

void __show_backtrace(unsigned long fp)
{
	struct reg_window32 *rw;
	unsigned long flags;
	int cpu = smp_processor_id();

	spin_lock_irqsave(&sparc_backtrace_lock, flags);

	rw = (struct reg_window32 *)fp;
        while(rw && (((unsigned long) rw) >= PAGE_OFFSET) &&
            !(((unsigned long) rw) & 0x7)) {
		printk("CPU[%d]: ARGS[%08lx,%08lx,%08lx,%08lx,%08lx,%08lx] "
		       "FP[%08lx] CALLER[%08lx]: ", cpu,
		       rw->ins[0], rw->ins[1], rw->ins[2], rw->ins[3],
		       rw->ins[4], rw->ins[5],
		       rw->ins[6],
		       rw->ins[7]);
		printk("%pS\n", (void *) rw->ins[7]);
		rw = (struct reg_window32 *) rw->ins[6];
	}
	spin_unlock_irqrestore(&sparc_backtrace_lock, flags);
}

#define __SAVE __asm__ __volatile__("save %sp, -0x40, %sp\n\t")
#define __RESTORE __asm__ __volatile__("restore %g0, %g0, %g0\n\t")
#define __GET_FP(fp) __asm__ __volatile__("mov %%i6, %0" : "=r" (fp))

void show_backtrace(void)
{
	unsigned long fp;

	__SAVE; __SAVE; __SAVE; __SAVE;
	__SAVE; __SAVE; __SAVE; __SAVE;
	__RESTORE; __RESTORE; __RESTORE; __RESTORE;
	__RESTORE; __RESTORE; __RESTORE; __RESTORE;

	__GET_FP(fp);

	__show_backtrace(fp);
}

#ifdef CONFIG_SMP
void smp_show_backtrace_all_cpus(void)
{
	xc0((smpfunc_t) show_backtrace);
	show_backtrace();
}
#endif

void show_stackframe(struct sparc_stackf *sf)
{
	unsigned long size;
	unsigned long *stk;
	int i;

	printk("l0: %08lx l1: %08lx l2: %08lx l3: %08lx "
	       "l4: %08lx l5: %08lx l6: %08lx l7: %08lx\n",
	       sf->locals[0], sf->locals[1], sf->locals[2], sf->locals[3],
	       sf->locals[4], sf->locals[5], sf->locals[6], sf->locals[7]);
	printk("i0: %08lx i1: %08lx i2: %08lx i3: %08lx "
	       "i4: %08lx i5: %08lx fp: %08lx i7: %08lx\n",
	       sf->ins[0], sf->ins[1], sf->ins[2], sf->ins[3],
	       sf->ins[4], sf->ins[5], (unsigned long)sf->fp, sf->callers_pc);
	printk("sp: %08lx x0: %08lx x1: %08lx x2: %08lx "
	       "x3: %08lx x4: %08lx x5: %08lx xx: %08lx\n",
	       (unsigned long)sf->structptr, sf->xargs[0], sf->xargs[1],
	       sf->xargs[2], sf->xargs[3], sf->xargs[4], sf->xargs[5],
	       sf->xxargs[0]);
	size = ((unsigned long)sf->fp) - ((unsigned long)sf);
	size -= STACKFRAME_SZ;
	stk = (unsigned long *)((unsigned long)sf + STACKFRAME_SZ);
	i = 0;
	do {
		printk("s%d: %08lx\n", i++, *stk++);
	} while ((size -= sizeof(unsigned long)));
}
#endif

void show_regs(struct pt_regs *r)
{
	struct reg_window32 *rw = (struct reg_window32 *) r->u_regs[14];

        printk("PSR: %08lx PC: %08lx NPC: %08lx Y: %08lx    %s\n",
	       r->psr, r->pc, r->npc, r->y, print_tainted());
	printk("PC: <%pS>\n", (void *) r->pc);
	printk("%%G: %08lx %08lx  %08lx %08lx  %08lx %08lx  %08lx %08lx\n",
	       r->u_regs[0], r->u_regs[1], r->u_regs[2], r->u_regs[3],
	       r->u_regs[4], r->u_regs[5], r->u_regs[6], r->u_regs[7]);
	printk("%%O: %08lx %08lx  %08lx %08lx  %08lx %08lx  %08lx %08lx\n",
	       r->u_regs[8], r->u_regs[9], r->u_regs[10], r->u_regs[11],
	       r->u_regs[12], r->u_regs[13], r->u_regs[14], r->u_regs[15]);
	printk("RPC: <%pS>\n", (void *) r->u_regs[15]);

	printk("%%L: %08lx %08lx  %08lx %08lx  %08lx %08lx  %08lx %08lx\n",
	       rw->locals[0], rw->locals[1], rw->locals[2], rw->locals[3],
	       rw->locals[4], rw->locals[5], rw->locals[6], rw->locals[7]);
	printk("%%I: %08lx %08lx  %08lx %08lx  %08lx %08lx  %08lx %08lx\n",
	       rw->ins[0], rw->ins[1], rw->ins[2], rw->ins[3],
	       rw->ins[4], rw->ins[5], rw->ins[6], rw->ins[7]);
}

/*
 * The show_stack is an external API which we do not use ourselves.
 * The oops is printed in die_if_kernel.
 */
void show_stack(struct task_struct *tsk, unsigned long *_ksp)
{
	unsigned long pc, fp;
	unsigned long task_base;
	struct reg_window32 *rw;
	int count = 0;

	if (tsk != NULL)
		task_base = (unsigned long) task_stack_page(tsk);
	else
		task_base = (unsigned long) current_thread_info();

	fp = (unsigned long) _ksp;
	do {
		/* Bogus frame pointer? */
		if (fp < (task_base + sizeof(struct thread_info)) ||
		    fp >= (task_base + (PAGE_SIZE << 1)))
			break;
		rw = (struct reg_window32 *) fp;
		pc = rw->ins[7];
		printk("[%08lx : ", pc);
		printk("%pS ] ", (void *) pc);
		fp = rw->ins[6];
	} while (++count < 16);
	printk("\n");
}

void dump_stack(void)
{
	unsigned long *ksp;

	__asm__ __volatile__("mov	%%fp, %0"
			     : "=r" (ksp));
	show_stack(current, ksp);
}

EXPORT_SYMBOL(dump_stack);

/*
 * Note: sparc64 has a pretty intricated thread_saved_pc, check it out.
 */
unsigned long thread_saved_pc(struct task_struct *tsk)
{
	return task_thread_info(tsk)->kpc;
}

/*
 * Free current thread data structures etc..
 */
void exit_thread(void)
{
#ifndef CONFIG_SMP
	if(last_task_used_math == current) {
#else
	if (test_thread_flag(TIF_USEDFPU)) {
#endif
		/* Keep process from leaving FPU in a bogon state. */
		put_psr(get_psr() | PSR_EF);
		fpsave(&current->thread.float_regs[0], &current->thread.fsr,
		       &current->thread.fpqueue[0], &current->thread.fpqdepth);
#ifndef CONFIG_SMP
		last_task_used_math = NULL;
#else
		clear_thread_flag(TIF_USEDFPU);
#endif
	}
}

void flush_thread(void)
{
	current_thread_info()->w_saved = 0;

#ifndef CONFIG_SMP
	if(last_task_used_math == current) {
#else
	if (test_thread_flag(TIF_USEDFPU)) {
#endif
		/* Clean the fpu. */
		put_psr(get_psr() | PSR_EF);
		fpsave(&current->thread.float_regs[0], &current->thread.fsr,
		       &current->thread.fpqueue[0], &current->thread.fpqdepth);
#ifndef CONFIG_SMP
		last_task_used_math = NULL;
#else
		clear_thread_flag(TIF_USEDFPU);
#endif
	}

	/* Now, this task is no longer a kernel thread. */
	current->thread.current_ds = USER_DS;
	if (current->thread.flags & SPARC_FLAG_KTHREAD) {
		current->thread.flags &= ~SPARC_FLAG_KTHREAD;

		/* We must fixup kregs as well. */
		/* XXX This was not fixed for ti for a while, worked. Unused? */
		current->thread.kregs = (struct pt_regs *)
		    (task_stack_page(current) + (THREAD_SIZE - TRACEREG_SZ));
	}
}

static inline struct sparc_stackf __user *
clone_stackframe(struct sparc_stackf __user *dst,
		 struct sparc_stackf __user *src)
{
	unsigned long size, fp;
	struct sparc_stackf *tmp;
	struct sparc_stackf __user *sp;

	if (get_user(tmp, &src->fp))
		return NULL;

	fp = (unsigned long) tmp;
	size = (fp - ((unsigned long) src));
	fp = (unsigned long) dst;
	sp = (struct sparc_stackf __user *)(fp - size); 

	/* do_fork() grabs the parent semaphore, we must release it
	 * temporarily so we can build the child clone stack frame
	 * without deadlocking.
	 */
	if (__copy_user(sp, src, size))
		sp = NULL;
	else if (put_user(fp, &sp->fp))
		sp = NULL;

	return sp;
}

asmlinkage int sparc_do_fork(unsigned long clone_flags,
                             unsigned long stack_start,
                             struct pt_regs *regs,
                             unsigned long stack_size)
{
	unsigned long parent_tid_ptr, child_tid_ptr;
	unsigned long orig_i1 = regs->u_regs[UREG_I1];
	long ret;

	parent_tid_ptr = regs->u_regs[UREG_I2];
	child_tid_ptr = regs->u_regs[UREG_I4];

	ret = do_fork(clone_flags, stack_start,
		      regs, stack_size,
		      (int __user *) parent_tid_ptr,
		      (int __user *) child_tid_ptr);

	/* If we get an error and potentially restart the system
	 * call, we're screwed because copy_thread() clobbered
	 * the parent's %o1.  So detect that case and restore it
	 * here.
	 */
	if ((unsigned long)ret >= -ERESTART_RESTARTBLOCK)
		regs->u_regs[UREG_I1] = orig_i1;

	return ret;
}

/* Copy a Sparc thread.  The fork() return value conventions
 * under SunOS are nothing short of bletcherous:
 * Parent -->  %o0 == childs  pid, %o1 == 0
 * Child  -->  %o0 == parents pid, %o1 == 1
 *
 * NOTE: We have a separate fork kpsr/kwim because
 *       the parent could change these values between
 *       sys_fork invocation and when we reach here
 *       if the parent should sleep while trying to
 *       allocate the task_struct and kernel stack in
 *       do_fork().
 * XXX See comment above sys_vfork in sparc64. todo.
 */
extern void ret_from_fork(void);

int copy_thread(unsigned long clone_flags, unsigned long sp,
		unsigned long unused,
		struct task_struct *p, struct pt_regs *regs)
{
	struct thread_info *ti = task_thread_info(p);
	struct pt_regs *childregs;
	char *new_stack;

#ifndef CONFIG_SMP
	if(last_task_used_math == current) {
#else
	if (test_thread_flag(TIF_USEDFPU)) {
#endif
		put_psr(get_psr() | PSR_EF);
		fpsave(&p->thread.float_regs[0], &p->thread.fsr,
		       &p->thread.fpqueue[0], &p->thread.fpqdepth);
#ifdef CONFIG_SMP
		clear_thread_flag(TIF_USEDFPU);
#e10)) ? true : false;
	sensor_bits->sense_chgcurrs = (sense_0 & (1 << 11)) ? true : false;
	sensor_bits->sense_bpons = (sense_0 & (1 << 12)) ? true : false;
	sensor_bits->sense_lobatls = (sense_0 & (1 << 13)) ? true : false;
	sensor_bits->sense_lobaths = (sense_0 & (1 << 14)) ? true : false;
	sensor_bits->sense_usb4v4s = (sense_0 & (1 << 16)) ? true : false;
	sensor_bits->sense_usb2v0s = (sense_0 & (1 << 17)) ? true : false;
	sensor_bits->sense_usb0v8s = (sense_0 & (1 << 18)) ? true : false;
	sensor_bits->sense_id_floats = (sense_0 & (1 << 19)) ? true : false;
	sensor_bits->sense_id_gnds = (sense_0 & (1 << 20)) ? true : false;
	sensor_bits->sense_se1s = (sense_0 & (1 << 21)) ? true : false;
	sensor_bits->sense_ckdets = (sense_0 & (1 << 22)) ? true : false;

	sensor_bits->sense_onofd1s = (sense_1 & (1 << 3)) ? true : false;
	sensor_bits->sense_onofd2s = (sense_1 & (1 << 4)) ? true : false;
	sensor_bits->sense_onofd3s = (sense_1 & (1 << 5)) ? true : false;
	sensor_bits->sense_pwrrdys = (sense_1 & (1 << 11)) ? true : false;
	sensor_bits->sense_thwarnhs = (sense_1 & (1 << 12)) ? true : false;
	sensor_bits->sense_thwarnls = (sense_1 & (1 << 13)) ? true : false;
	sensor_bits->sense_clks = (sense_1 & (1 << 14)) ? true : false;
	sensor_bits->sense_mc2bs = (sense_1 & (1 << 17)) ? true : false;
	sensor_bits->sense_hsdets = (sense_1 & (1 << 18)) ? true : false;
	sensor_bits->sense_hsls = (sense_1 & (1 << 19)) ? true : false;
	sensor_bits->sense_alspths = (sense_1 & (1 << 20)) ? true : false;
	sensor_bits->sense_ahsshorts = (sense_1 & (1 << 21)) ? true : false;
	return PMIC_SUCCESS;
}

EXPORT_SYMBOL(pmic_check_sensor);
EXPORT_SYMBOL(pmic_get_sensors);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             /*
 * Copyright (C) 2003-2008 Takahiro Hirofuchi
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 */

#include "usbip_common.h"
#include "stub.h"
#include "../../usb/core/hcd.h"


static int is_clear_halt_cmd(struct urb *urb)
{
	struct usb_ctrlrequest *req;

	req = (struct usb_ctrlrequest *) urb->setup_packet;

	 return (req->bRequest == USB_REQ_CLEAR_FEATURE) &&
		 (req->bRequestType == USB_RECIP_ENDPOINT) &&
		 (req->wValue == USB_ENDPOINT_HALT);
}

static int is_set_interface_cmd(struct urb *urb)
{
	struct usb_ctrlrequest *req;

	req = (struct usb_ctrlrequest *) urb->setup_packet;

	return (req->bRequest == USB_REQ_SET_INTERFACE) &&
		   (req->bRequestType == USB_RECIP_INTERFACE);
}

static int is_set_configuration_cmd(struct urb *urb)
{
	struct usb_ctrlrequest *req;

	req = (struct usb_ctrlrequest *) urb->setup_packet;

	return (req->bRequest == USB_REQ_SET_CONFIGURATION) &&
		   (req->bRequestType == USB_RECIP_DEVICE);
}

static int is_reset_device_cmd(struct urb *urb)
{
	struct usb_ctrlrequest *req;
	__u16 value;
	__u16 index;

	req = (struct usb_ctrlrequest *) urb->setup_packet;
	value = le16_to_cpu(req->wValue);
	index = le16_to_cpu(req->wIndex);

	if ((req->bRequest == USB_REQ_SET_FEATURE) &&
			(req->bRequestType == USB_RT_PORT) &&
			(value = USB_PORT_FEAT_RESET)) {
		dbg_stub_rx("reset_device_cmd, port %u\n", index);
		return 1;
	} else
		return 0;
}

static int tweak_clear_halt_cmd(struct urb *urb)
{
	struct usb_ctrlrequest *req;
	int target_endp;
	int target_dir;
	int target_pipe;
	int ret;

	req = (struct usb_ctrlrequest *) urb->setup_packet;

	/*
	 * The stalled endpoint is specified in the wIndex value. The endpoint
	 * of the urb is the target of this clear_halt request (i.e., control
	 * endpoint).
	 */
	target_endp = le16_to_cpu(req->wIndex) & 0x000f;

	/* the stalled endpoint direction is IN or OUT?. USB_DIR_IN is 0x80.  */
	target_dir = le16_to_cpu(req->wIndex) & 0x0080;

	if (target_dir)
		target_pipe = usb_rcvctrlpipe(urb->dev, target_endp);
	else
		tar