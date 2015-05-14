/*
 * wof.S: Sparc window overflow handler.
 *
 * Copyright (C) 1995 David S. Miller (davem@caip.rutgers.edu)
 */

#include <asm/contregs.h>
#include <asm/page.h>
#include <asm/ptrace.h>
#include <asm/psr.h>
#include <asm/smp.h>
#include <asm/asi.h>
#include <asm/winmacro.h>
#include <asm/asmmacro.h>
#include <asm/thread_info.h>

/* WARNING: This routine is hairy and _very_ complicated, but it
 *          must be as fast as possible as it handles the allocation
 *          of register windows to the user and kernel.  If you touch
 *          this code be _very_ careful as many other pieces of the
 *          kernel depend upon how this code behaves.  You have been
 *          duly warned...
 */

/* We define macro's for registers which have a fixed
 * meaning throughout this entire routine.  The 'T' in
 * the comments mean that the register can only be
 * accessed when in the 'trap' window, 'G' means
 * accessible in any window.  Do not change these registers
 * after they have been set, until you are ready to return
 * from the trap.
 */
#define t_psr       l0 /* %psr at trap time                     T */
#define t_pc        l1 /* PC for trap return                    T */
#define t_npc       l2 /* NPC for trap return                   T */
#define t_wim       l3 /* %wim at trap time                     T */
#define saved_g5    l5 /* Global save register                  T */
#define saved_g6    l6 /* Global save register                  T */
#define curptr      g6 /* Gets set to 'current' then stays      G */

/* Now registers whose values can change within the handler.      */
#define twin_tmp    l4 /* Temp reg, only usable in trap window  T */
#define glob_tmp    g5 /* Global temporary reg, usable anywhere G */

	.text
	.align	4
	/* BEGINNING OF PATCH INSTRUCTIONS */
	/* On a 7-window Sparc the boot code patches spnwin_*
	 * instructions with the following ones.
	 */
	.globl	spnwin_patch1_7win, spnwin_patch2_7win, spnwin_patch3_7win
spnwin_patch1_7win:	sll	%t_wim, 6, %glob_tmp
spnwin_patch2_7win:	and	%glob_tmp, 0x7f, %glob_tmp
spnwin_patch3_7win:	and	%twin_tmp, 0x7f, %twin_tmp
	/* END OF PATCH INSTRUCTIONS */

	/* The trap entry point has done the following:
	 *
	 * rd    %psr, %l0
	 * rd    %wim, %l3
	 * b     spill_window_entry
	 * andcc %l0, PSR_PS, %g0
	 */

	/* Datum current_thread_info->uwinmask contains at all times a bitmask
	 * where if any user windows are active, at least one bit will
	 * be set in to mask.  If no user windows are active, the bitmask
	 * will be all zeroes.
	 */
	.globl	spill_window_entry 
	.globl	spnwin_patch1, spnwin_patch2, spnwin_patch3
spill_window_entry:
	/* LOCATION: Trap Window */

	mov	%g5, %saved_g5		! save away global temp register
	mov	%g6, %saved_g6		! save away 'current' ptr register

	/* Compute what the new %wim will be if we save the
	 * window properly in this trap handler.
	 *
	 * newwim = ((%wim>>1) | (%wim<<(nwindows - 1)));
	 */
		srl	%t_wim, 0x1, %twin_tmp
spnwin_patch1:	sll	%t_wim, 7, %glob_tmp
		or	%glob_tmp, %twin_tmp, %glob_tmp
spnwin_patch2:	and	%glob_tmp, 0xff, %glob_tmp

	/* The trap entry point has set the condition codes
	 * up for us to see if this is from user or kernel.
	 * Get the load of 'curptr' out of the way.
	 */
	LOAD_CURRENT(curptr, twin_tmp)

	andcc	%t_psr, PSR_PS, %g0
	be,a	spwin_fromuser				! all user wins, branch
	 save	%g0, %g0, %g0				! Go where saving will occur
	
	/* See if any user windows are active in the set. */
	ld	[%curptr + TI_UWINMASK], %twin_tmp	! grab win mask
	orcc	%g0, %twin_tmp, %g0			! check for set bits
	bne	spwin_exist_uwins			! yep, there are some
	 andn	%twin_tmp, %glob_tmp, %twin_tmp		! compute new uwinmask

	/* Save into the window which must be saved and do it.
	 * Basically if we are here, this means that we trapped
	 * from kernel mode with only kernel windows in the register
	 * file.
	 */
	save	%g0, %g0, %g0		! save into the window to stash away
	wr	%glob_tmp, 0x0, %wim	! set new %wim, this is safe now

spwin_no_userwins_from_kernel:
	/* LOCATION: Window to be saved */

	STORE_WINDOW(sp)		! stash the window
	restos & CFSYNCH))
			spi_max_offset(starget) = 0;
		spi_min_period(starget) = 
			ahc_find_period(ahc, scsirate, maxsync);

		tinfo = ahc_fetch_transinfo(ahc, channel, ahc->our_id,
					    starget->id, &tstate);
	}
	ahc_compile_devinfo(&devinfo, our_id, starget->id,
			    CAM_LUN_WILDCARD, channel,
			    ROLE_INITIATOR);
	ahc_set_syncrate(ahc, &devinfo, NULL, 0, 0, 0,
			 AHC_TRANS_GOAL, /*paused*/FALSE);
	ahc_set_width(ahc, &devinfo, MSG_EXT_WDTR_BUS_8_BIT,
		      AHC_TRANS_GOAL, /*paused*/FALSE);
	ahc_unlock(ahc, &flags);

	return 0;
}

static void
ahc_linux_target_destroy(struct scsi_target *starget)
{
	struct scsi_target **ahc_targp = ahc_linux_target_in_softc(starget);

	*ahc_targp = NULL;
}

static int
ahc_linux_slave_alloc(struct scsi_device *sdev)
{
	struct	ahc_softc *ahc =
		*((struct ahc_softc **)sdev->host->hostdata);
	struct scsi_target *starget = sdev->sdev_target;
	struct ahc_linux_device *dev;

	if (bootverbose)
		printf("%s: Slave Alloc %d\n", ahc_name(ahc), sdev->id);

	dev = scsi_transport_device_data(sdev);
	memset(dev, 0, sizeof(*dev));

	/*
	 * We start out life using untagged
	 * transactions of which we allow one.
	 */
	dev->openings = 1;

	/*
	 * Set maxtags to 0.  This will be changed if we
	 * later determine that we are dealing with
	 * a tagged queuing capable device.
	 */
	dev->maxtags = 0;
	
	spi_period(starget) = 0;

	return 0;
}

static int
ahc_linux_slave_configure(struct scsi_device *sdev)
{
	struct	ahc_softc *ahc;

	ahc = *((struct ahc_softc **)sdev->host->hostdata);

	if (bootverbose)
		sdev_printk(KERN_INFO, sdev, "Slave Configure\n");

	ahc_linux_device_queue_depth(sdev);

	/* Initial Domain Validation */
	if (!spi_initial_dv(sdev->sdev_target))
		spi_dv_device(sdev);

	return 0;
}

#if defined(__i386__)
/*
 * Return the disk geometry for the given SCSI device.
 */
static int
ahc_linux_biosparam(struct scsi_device *sdev, struct block_device *bdev,
		    sector_t capacity, int geom[])
{
	uint8_t *bh;
	int	 heads;
	int	 sectors;
	int	 cylinders;
	int	 ret;
	int	 extended;
	struct	 ahc_softc *ahc;
	u_int	 channel;

	ahc = *((struct ahc_softc **)sdev->host->hostdata);
	channel = sdev_channel(sdev);

	bh = scsi_bios_ptable(bdev);
	if (bh) {
		ret = scsi_partsize(bh, capacity,
				    &geom[2], &geom[0], &geom[1]);
		kfree(bh);
		if (ret != -1)
			return (ret);
	}
	heads = 64;
	sectors = 32;
	cylinders = aic_sector_div(capacity, heads, sectors);

	if (aic7xxx_extended != 0)
		extended = 1;
	else if (channel == 0)
		extended = (ahc->flags & AHC_EXTENDED_TRANS_A) != 0;
	else
		extended = (ahc->flags & AHC_EXTENDED_TRANS_B) != 0;
	if (extended && cylinders >= 1024) {
		heads = 255;
		sectors = 63;
		cylinders = aic_sector_div(capacity, heads, sectors);
	}
	geom[0] = heads;
	geom[1] = sectors;
	geom[2] = cylinders;
	return (0);
}
#endif

/*
 * Abort the current SCSI command(s).
 */
static int
ahc_linux_abort(struct scsi_cmnd *cmd)
{
	int error;

	error = ahc_linux_queue_recovery_cmd(cmd, SCB_ABORT);
	if (error != 0)
		printf("aic7xxx_abort returns 0x%x\n", error);
	return (error);
}

/*
 * Attempt to send a target reset message to the device that timed out.
 */
static int
ahc_linux_dev_reset(struct scsi_cmnd *cmd)
{
	int error;

	error = ahc_linux_queue_recovery_cmd(cmd, SCB_DEVICE_RESET);
	if (error != 0)
		printf("aic7xxx_dev_reset returns 0x%x\n", error);
	return (error);
}

/*
 * Reset the SCSI bus.
 */
static int
ahc_linux_bus_reset(struct scsi_cmnd *cmd)
{
	struct ahc_softc *ahc;
	int    found;
	unsigned long flags;

	ahc = *(struct ahc_softc **)cmd->device->host->hostdata;

	ahc_lock(ahc, &flags);
	found = ahc_reset_channel(ahc, scmd_channel(cmd) + 'A',
				  /*initiate reset*/TRUE);
	ahc_unlock(ahc, &flags);

	if (bootverbose)
		printf("%s: SCSI bus reset delivered. "
		       "%d SCBs aborted.\n", ahc_name(ahc), found);

	return SUCCESS;
}

struct scsi_host_template aic7xxx_driver_template = {
	.module			= THIS_MODULE,
	.name			= "aic7xxx",
	.proc_name		= "aic7xxx",
	.proc_info		= ahc_linux_proc_info,
	.info			= ahc_linux_info,
	.queuecommand		= ahc_linux_queue,
	.eh_abort_handler	= aSTACK_OFFSET), %sp
	add	%curptr, %sp, %sp

	/* Restore the saved globals and build a pt_regs frame. */
	mov	%saved_g5, %g5
	mov	%saved_g6, %g6
	STORE_PT_ALL(sp, t_psr, t_pc, t_npc, g1)

	sethi	%hi(STACK_OFFSET), %g6
	or	%g6, %lo(STACK_OFFSET), %g6
	sub	%sp, %g6, %g6		! curptr

	/* Turn on traps and call c-code to deal with it. */
	wr	%t_psr, PSR_ET, %psr
	nop
	call	window_overflow_fault
	 nop

	/* Return from trap if C-code actually fixes things, if it
	 * doesn't then we never get this far as the process will
	 * be given the look of death from Commander Peanut.
	 */
	b	ret_trap_entry
	 clr	%l6

spwin_bad_ustack_from_kernel:
	/* LOCATION: Window to be saved */

	/* The kernel provoked a spill window trap, but the window we
	 * need to save is a user one and the process has trashed its
	 * stack pointer.  We need to be quick, so we throw it into
	 * a per-process window buffer until we can properly handle
	 * this later on.
	 */
	SAVE_BOLIXED_USER_STACK(curptr, glob_tmp)
	restore	%g0, %g0, %g0

	/* LOCATION: Trap window */

	/* Restore globals, condition codes in the %psr and
	 * return from trap.  Note, restoring %g6 when returning
	 * to kernel mode is not necessarily these days. ;-)
	 */
	mov	%saved_g5, %g5
	mov	%saved_g6, %g6

	wr	%t_psr, 0x0, %psr
	WRITE_PAUSE

	jmp	%t_pc
	rett	%t_npc

/* Undefine the register macros which would only cause trouble
 * if used below.  This helps find 'stupid' coding errors that
 * produce 'odd' behavior.  The routines below are allowed to
 * make usage of glob_tmp and t_psr so we leave them defined.
 */
#undef twin_tmp
#undef curptr
#undef t_pc
#undef t_npc
#undef t_wim
#undef saved_g5
#undef saved_g6

/* Now come the per-architecture window overflow stack checking routines.
 * As noted above %curptr cannot be touched by this routine at all.
 */

spwin_sun4c_stackchk:
	/* LOCATION: Window to be saved on the stack */

	/* See if the stack is in the address space hole but first,
	 * check results of callers andcc %sp, 0x7, %g0
	 */
	be	1f
	 sra	%sp, 29, %glob_tmp

	rd	%psr, %glob_tmp
	b	spwin_user_stack_is_bolixed + 0x4
	 nop

1:
	add	%glob_tmp, 0x1, %glob_tmp
	andncc	%glob_tmp, 0x1, %g0
	be	1f
	 and	%sp, 0xfff, %glob_tmp		! delay slot

	rd	%psr, %glob_tmp
	b	spwin_user_stack_is_bolixed + 0x4
	 nop

	/* See if our dump area will be on more than one
	 * page.
	 */
1:
	add	%glob_tmp, 0x38, %glob_tmp
	andncc	%glob_tmp, 0xff8, %g0
	be	spwin_sun4c_onepage		! only one page to check
	 lda	[%sp] ASI_PTE, %glob_tmp	! have to check first page anyways

spwin_sun4c_twopages:
	/* Is first page ok permission wise? */
	srl	%glob_tmp, 29, %glob_tmp
	cmp	%glob_tmp, 0x6
	be	1f
	 add	%sp, 0x38, %glob_tmp	/* Is second page in vma hole? */

	rd	%psr, %glob_tmp
	b	spwin_user_stack_is_bolixed + 0x4
	 nop

1:
	sra	%glob_tmp, 29, %glob_tmp
	add	%glob_tmp, 0x1, %glob_tmp
	andncc	%glob_tmp, 0x1, %g0
	be	1f
	 add	%sp, 0x38, %glob_tmp

	rd	%psr, %glob_tmp
	b	spwin_user_stack_is_bolixed + 0x4
	 nop

1:
	lda	[%glob_tmp] ASI_PTE, %glob_tmp

spwin_sun4c_onepage:
	srl	%glob_tmp, 29, %glob_tmp
	cmp	%glob_tmp, 0x6				! can user write to it?
	be	spwin_good_ustack			! success
	 nop

	rd	%psr, %glob_tmp
	b	spwin_user_stack_is_bolixed + 0x4
	 nop

	/* This is a generic SRMMU routine.  As far as I know this
	 * works for all current v8/srmmu implementations, we'll
	 * see...
	 */
	.globl	spwin_srmmu_stackchk
spwin_srmmu_stackchk:
	/* LOCATION: Window to be saved on the stack */

	/* Because of SMP concerns and speed we play a trick.
	 * We disable fault traps in the MMU control register,
	 * Execute the stores, then check the fault registers
	 * to see what happens.  I can hear Linus now
	 * "disgusting... broken hardware...".
	 *
	 * But first, check to see if the users stack has ended
	 * up in kernel vma, then we would succeed for the 'wrong'
	 * reason... ;(  Note that the 'sethi' below assumes the
	 * kernel is page aligned, which should always be the case.
	 */
	/* Check results of callers andcc %sp, 0x7, %g0 */
	bne	spwin_user_stack_is_bolixed
	 sethi   %hi(PAGE_OFFSET), %glob_tmp
	cmp	%glob_tmp, %sp
	bleu	spwin_user_stthat may be in
	 * the middle of our option argument.
	 */
	tok_end = strchr(opt_arg, '\0');
	if (tok_end < end)
		*tok_end = ',';
	while (!done) {
		switch (*opt_arg) {
		case '{':
			if (instance == -1) {
				instance = 0;
			} else {
				if (depth > 1) {
					if (targ == -1)
						targ = 0;
				} else {
					printf("Malformed Option %s\n",
					       opt_name);
					done = TRUE;
				}
			}
			opt_arg++;
			break;
		case '}':
			if (targ != -1)
				targ = -1;
			else if (instance != -1)
				instance = -1;
			opt_arg++;
			break;
		case ',':
		case '.':
			if (instance == -1)
				done = TRUE;
			else if (targ >= 0)
				targ++;
			else if (instance >= 0)
				instance++;
			opt_arg++;
			break;
		case '\0':
			done = TRUE;
			break;
		default:
			tok_end = end;
			for (i = 0; tok_list[i]; i++) {
				tok_end2 = strc