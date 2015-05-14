/*
 * Copyright (c) 2007-2008 Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*                                                                      */
/*  Module Name : func_extr.c                                           */
/*                                                                      */
/*  Abstract                                                            */
/*      This module contains function prototype.                        */
/*                                                                      */
/*  NOTES                                                               */
/*      None                                                            */
/*                                                                      */
/************************************************************************/

#ifndef _CFUNC_H
#define _CFUNC_H

#include "queue.h"

/* amsdu.c */
void zfDeAmsdu(zdev_t* dev, zbuf_t* buf, u16_t vap, u8_t encryMode);

/* cscanmgr.c */
void zfScanMgrInit(zdev_t* dev);
u8_t zfScanMgrScanStart(zdev_t* dev, u8_t scanType);
void zfScanMgrScanStop(zdev_t* dev, u8_t scanType);
void zfScanMgrScanAck(zdev_t* dev);

/* cpsmgr.c */
void zfPowerSavingMgrInit(zdev_t* dev);
void zfPowerSavingMgrSetMode(zdev_t* dev, u8_t mode);
void zfPowerSavingMgrMain(zdev_t* dev);
void zfPowerSavingMgrWakeup(zdev_t* dev);
u8_t zfPowerSavingMgrIsSleeping(zdev_t *dev);
void zfPowerSavingMgrProcessBeacon(zdev_t* dev, zbuf_t* buf);
void zfPowerSavingMgrAtimWinExpired(zdev_t* dev);
void zfPowerSavingMgrConnectNotify(zdev_t *dev);
void zfPowerSavingMgrPreTBTTInterrupt(zdev_t *dev);

/* ccmd.c */
u16_t zfWlanEnable(zdev_t* dev);

/* cfunc.c */
u8_t zfQueryOppositeRate(zdev_t* dev, u8_t dst_mac[6], u8_t frameType);
void zfCopyToIntTxBuffer(zdev_t* dev, zbuf_t* buf, u8_t* src,
                         u16_t offset, u16_t length);
void zfCopyToRxBuffer(zdev_t* dev, zbuf_t* buf, u8_t* src,
                      u16_t offset, u16_t length);
void zfCopyFromIntTxBuffer(zdev_t* dev, zbuf_t* buf, u8_t* dst,
                           u16_t offset, u16_t length);
void zfCopyFromRxBuffer(zdev_t* dev, zbuf_t* buf, u8_t* dst,
                        u16_t offset, u16_t length);
void zfMemoryCopy(u8_t* dst, u8_t* src, u16_t length);
void zfMemoryMove(u8_t* dst, u8_t* src, u16_t length);
void zfZeroMemory(u8_t* va, u16_t length);
u8_t zfMemoryIsEqual(u8_t* m1, u8_t* m2, u16_t length);
u8_t zfRxBufferEqualToStr(zdev_t* dev, zbuf_t* buf, const u8_t* str,
                          u16_t offset, u16_t length);
void zfTxBufferCopy(zdev_t*dev, zbuf_t* dst, zbuf_t* src,
                    u16_t dstOffset, u16_t srcOffset, u16_t length);
void zfRxBufferCopy(zdev_t*dev, zbuf_t* dst, zbuf_t* src,
                    u16_t dstOffset, u16_t srcOffset, u16_t length);

void zfCollectHWTally(zdev_t*dev, u32_t* rsp, u8_t id);
void zfTimerInit(zdev_t* dev);
u16_t zfTimerSchedule(zdev_t* dev, u16_t event, u32_t tick);
u16_t zfTimerCancel(zdev_t* dev, u16_t event);
void zfTimerClear(zdev_t* dev);
u16_t zfTimerCheckAndHandle(zdev_t* dev);
void zfProcessEvent(zdev_t* dev, u16_t* eventArray, u8_t eventCount);

void zfBssInfoCreate(zdev_t* dev);
void zfBssInfoDestroy(zdev_t* dev);

struct zsBssInfo* zfBssInfoAllocate(zdev_t* dev);
void zfBssInfoFree(zdev_t* dev, struct zsBssInfo* pBssInfo);
void zfBssInfoReorderList(zdev_t* dev);
void zfBssInfoInsertToLisevice_accumulate_irb(cdev, irb);
	/* Remember to clear irb to avoid residuals. */
	memset(&cdev->private->irb, 0, sizeof(struct irb));
	/* Try to start delayed device verification. */
	ccw_device_online_verify(cdev, 0);
	/* Note: Don't call handler for cio initiated clear! */
}

static void
ccw_device_killing_irq(struct ccw_device *cdev, enum dev_event dev_event)
{
	struct subchannel *sch;

	sch = to_subchannel(cdev->dev.parent);
	ccw_device_set_timeout(cdev, 0);
	/* Start delayed path verification. */
	ccw_device_online_verify(cdev, 0);
	/* OK, i/o is dead now. Call interrupt handler. */
	if (cdev->handler)
		cdev->handler(cdev, cdev->private->intparm,
			      ERR_PTR(-EIO));
}

static void
ccw_device_killing_timeout(struct ccw_device *cdev, enum dev_event dev_event)
{
	int ret;

	ret = ccw_device_cancel_halt_clear(cdev);
	if (ret == -EBUSY) {
		ccw_device_set_timeout(cdev, 3*HZ);
		return;
	}
	/* Start delayed path verification. */
	ccw_device_online_verify(cdev, 0);
	if (cdev->handler)
		cdev->handler(cdev, cdev->private->intparm,
			      ERR_PTR(-EIO));
}

void ccw_device_kill_io(struct ccw_device *cdev)
{
	int ret;

	ret = ccw_device_cancel_halt_clear(cdev);
	if (ret == -EBUSY) {
		ccw_device_set_timeout(cdev, 3*HZ);
		cdev->private->state = DEV_STATE_TIMEOUT_KILL;
		return;
	}
	/* Start delayed path verification. */
	ccw_device_online_verify(cdev, 0);
	if (cdev->handler)
		cdev->handler(cdev, cdev->private->intparm,
			      ERR_PTR(-EIO));
}

static void
ccw_device_delay_verify(struct ccw_device *cdev, enum dev_event dev_event)
{
	/* Start verification after current task finished. */
	cdev->private->flags.doverify = 1;
}

static void
ccw_device_stlck_done(struct ccw_device *cdev, enum dev_event dev_event)
{
	struct irb *irb;

	switch (dev_event) {
	case DEV_EVENT_INTERRUPT:
		irb = (struct irb *) __LC_IRB;
		/* Check for unsolicited interrupt. */
		if ((scsw_stctl(&irb->scsw) ==
		     (SCSW_STCTL_STATUS_PEND | SCSW_STCTL_ALERT_STATUS)) &&
		    (!scsw_cc(&irb->scsw)))
			/* FIXME: we should restart stlck here, but this
			 * is extremely unlikely ... */
			goto out_wakeup;

		ccw_device_accumulate_irb(cdev, irb);
		/* We don't care about basic sense etc. */
		break;
	default: /* timeout */
		break;
	}
out_wakeup:
	wake_up(&cdev->private->wait_q);
}

static void
ccw_device_start_id(struct ccw_device *cdev, enum dev_event dev_event)
{
	struct subchannel *sch;

	sch = to_subchannel(cdev->dev.parent);
	if (cio_enable_subchannel(sch, (u32)(addr_t)sch) != 0)
		/* Couldn't enable the subchannel for i/o. Sick device. */
		return;

	/* After 60s the device recognition is considered to have failed. */
	ccw_device_set_timeout(cdev, 60*HZ);

	cdev->private->state = DEV_STATE_DISCONNECTED_SENSE_ID;
	ccw_device_sense_id_start(cdev);
}

void ccw_device_trigger_reprobe(struct ccw_device *cdev)
{
	struct subchannel *sch;

	if (cdev->private->state != DEV_STATE_DISCONNECTED)
		return;

	sch = to_subchannel(cdev->dev.parent);
	/* Update some values. */
	if (cio_update_schib(sch))
		return;
	/*
	 * The pim, pam, pom values may not be accurate, but they are the best
	 * we have before performing device selection :/
	 */
	sch->lpm = sch->schib.pmcw.pam & sch->opm;
	/*
	 * Use the initial configuration since we can't be shure that the old
	 * paths are valid.
	 */
	io_subchannel_init_config(sch);
	if (cio_commit_config(sch))
		return;

	/* We should also udate ssd info, but this has to wait. */
	/* Check if this is another device which appeared on the same sch. */
	if (sch->schib.pmcw.dev != cdev->private->dev_id.devno) {
		PREPARE_WORK(&cdev->private->kick_work,
			     ccw_device_move_to_orphanage);
		queue_work(slow_path_wq, &cdev->private->kick_work);
	} else
		ccw_device_start_id(cdev, 0);
}

static void
ccw_device_offline_irq(struct ccw_device *cdev, enum dev_event dev_event)
{
	struct subchannel *sch;

	sch = to_subchannel(cdev->dev.parent);
	/*
	 * An interrupt in state offline means a previous disable was not
	 * successful - should not happen, but we try to disable again.
	 */
	cio_disable_subchannel(sch);
}

static void
ccw_device_change_cmfstate(struct ccw_device *cdev, enum dev_event dev_event)
{
	retry_set_schib(cdev);
	cdev->private->state = DEV_STATE_ONLINE;
	dev_fsm_event(cdev, dev_event);
}

static void ccw_device_update_cmfblock(struct ccw_device *cdev,
				       enum dev_event dev_event)
{
	cmf_retry_copy_block(cdev);
	cdev->private->state = DEV_STATE_ONLINE;
	dev_fsm_event(cdev, dev_event);
}

static void
ccw_device_quiesce_done(struct ccw_device *cdev, enum dev_event dev_event)
{
	ccw_device_set_timeout(cdev, 0);
	if (dev_event == DEV_EVENT_NOTOPER)
		cdev->private->state = DEV_STATE_NOT_OPER;
	else
		cdev->private->state = DEV_STATE_OFFLINE;
	wake_up(&cdev->private->wait_q);
}

static void
ccw_device_quiesce_timeout(struct ccw_device *cdev, enum dev_event dev_event)
{
	int ret;

	ret = ccw_device_cancel_halt_clear(cdev);
	switch (ret) {
	case 0:
		cdev->private->state = DEV_STATE_OFFLINE;
		wake_up(&cdev->private->wait_q);
		break;
	case -ENODEV:
		cdev->private->state = DEV_STATE_NOT_OPER;
		wake_up(&cdev->private->wait_q);
		break;
	default:
		ccw_device_set_timeout(cdev, HZ/10);
	}
}

/*
 * No operation action. This is used e.g. to ignore a timeout event in
 * state offline.
 */
static void
ccw_device_nop(struct ccw_device *cdev, enum dev_event dev_event)
{
}

/*
 * Bug operation action. 
 */
static void
ccw_device_bug(struct ccw_device *cdev, enum dev_event dev_event)
{
	CIO_MSG_EVENT(0, "Internal state [%i][%i] not handled for device "
		      "0.%x.%04x\n", cdev->private->state, dev_event,
		      cdev->private->dev_id.ssid,
		      cdev->private->dev_id.devno);
	BUG();
}

/*
 * device statemachine
 */
fsm_func_t *dev_jumptable[NR_DEV_STATES][NR_DEV_EVENTS] = {
	[DEV_STATE_NOT_OPER] = {
		[DEV_EVENT_NOTOPER]	= ccw_device_nop,
		[DEV_EVENT_INTERRUPT]	= ccw_device_bug,
		[DEV_EVENT_TIMEOUT]	= ccw_device_nop,
		[DEV_EVENT_VERIFY]	= ccw_device_nop,
	},
	[DEV_STATE_SENSE_PGID] = {
		[DEV_EVENT_NOTOPER]	= ccw_device_generic_notoper,
		[DEV_EVENT_INTERRUPT]	= ccw_device_sense_pgid_irq,
		[DEV_EVENT_TIMEOUT]	= ccw_device_onoff_timeout,
		[DEV_EVENT_VERIFY]	= ccw_device_nop,
	},
	[DEV_STATE_SENSE_ID] = {
		[DEV_EVENT_NOTOPER]	= ccw_device_recog_notoper,
		[DEV_EVENT_INTERRUPT]	= ccw_device_sense_id_irq,
		[DEV_EVENT_TIMEOUT]	= ccw_device_recog_timeout,
		[DEV_EVENT_VERIFY]	= ccw_device_nop,
	},
	[DEV_STATE_OFFLINE] = {
		[DEV_EVENT_NOTOPER]	= ccw_device_generic_notoper,
		[DEV_EVENT_INTERRUPT]	= ccw_device_offline_irq,
		[DEV_EVENT_TIMEOUT]	= ccw_device_nop,
		[DEV_EVENT_VERIFY]	= ccw_device_nop,
	},
	[DEV_STATE_VERIFY] = {
		[DEV_EVENT_NOTOPER]	= ccw_device_generic_notoper,
		[DEV_EVENT_INTERRUPT]	= ccw_device_verify_irq,
		[DEV_EVENT_TIMEOUT]	= ccw_device_onoff_timeout,
		[DEV_EVENT_VERIFY]	= ccw_device_delay_verify,
	},
	[DEV_STATE_ONLINE] = {
		[DEV_EVENT_NOTOPER]	= ccw_device_generic_notoper,
		[DEV_EVENT_INTERRUPT]	= ccw_device_irq,
		[DEV_EVENT_TIMEOUT]	= ccw_device_online_timeout,
		[DEV_EVENT_VERIFY]	= ccw_device_online_verify,
	},
	[DEV_STATE_W4SENSE] = {
		[DEV_EVENT_NOTOPER]	= ccw_device_generic_notoper,
		[DEV_EVENT_INTERRUPT]	= ccw_device_w4sense,
		[DEV_EVENT_TIMEOUT]	= ccw_device_nop,
		[DEV_EVENT_VERIFY]	= ccw_device_online_verify,
	},
	[DEV_STATE_DISBAND_PGID] = {
		[DEV_EVENT_NOTOPER]	= ccw_device_generic_notoper,
		[DEV_EVENT_INTERRUPT]	= ccw_device_disband_irq,
		[DEV_EVENT_TIMEOUT]	= ccw_device_onoff_timeout,
		[DEV_EVENT_VERIFY]	= ccw_device_nop,
	},
	[DEV_STATE_BOXED] = {
		[DEV_EVENT_NOTOPER]	= ccw_device_generic_notoper,
		[DEV_EVENT_INTERRUPT]	= ccw_device_stlck_done,
		[DEV_EVENT_TIMEOUT]	= ccw_device_stlck_done,
		[DEV_EVENT_VERIFY]	= ccw_device_nop,
	},
	/* states to wait for i/o completion before doing something */
	[DEV_STATE_CLEAR_VERIFY] = {
		[DEV_EVENT_NOTOPER]	= ccw_device_generic_notoper,
		[DEV_EVENT_INTERRUPT]   = ccw_device_clear_verify,
		[DEV_EVENT_TIMEOUT]	= ccw_device_nop,
		[DEV_EVENT_VERIFY]	= ccw_device_nop,
	},
	[DEV_STATE_TIMEOUT_KILL] = {
		[DEV_EVENT_NOTOPER]	= ccw_device_generic_notoper,
		[DEV_EVENT_INTERRUPT]	= ccw_device_killing_irq,
		[DEV_EVENT_TIMEOUT]	= ccw_device_killing_timeout,
		[DEV_EVENT_VERIFY]	= ccw_device_nop, //FIXME
	},
	[DEV_STATE_QUIESCE] = {
		[DEV_EVENT_NOTOPER]	= ccw_device_quiesce_done,
		[DEV_EVENT_INTERRUPT]	= ccw_device_quiesce_done,
		[DEV_EVENT_TIMEOUT]	= ccw_device_quiesce_timeout,
		[DEV_EVENT_VERIFY]	= ccw_device_nop,
	},
	/* special states for devices gone not operational */
	[DEV_STATE_DISCONNECTED] = {
		[DEV_EVENT_NOTOPER]	= ccw_device_nop,
		[DEV_EVENT_INTERRUPT]	= ccw_device_start_id,
		[DEV_EVENT_TIMEOUT]	= ccw_device_bug,
		[DEV_EVENT_VERIFY]	= ccw_device_start_id,
	},
	[DEV_STATE_DISCONNECTED_SENSE_ID] = {
		[DEV_EVENT_NOTOPER]	= ccw_device_recog_notoper,
		[DEV_EVENT_INTERRUPT]	= ccw_device_sense_id_irq,
		[DEV_EVENT_TIMEOUT]	= ccw_device_recog_timeout,
		[DEV_EVENT_VERIFY]	= ccw_device_nop,
	},
	[DEV_STATE_CMFCHANGE] = {
		[DEV_EVENT_NOTOPER]	= ccw_device_change_cmfstate,
		[DEV_EVENT_INTERRUPT]	= ccw_device_change_cmfstate,
		[DEV_EVENT_TIMEOUT]	= ccw_device_change_cmfstate,
		[DEV_EVENT_VERIFY]	= ccw_device_change_cmfstate,
	},
	[DEV_STATE_CMFUPDATE] = {
		[DEV_EVENT_NOTOPER]	= ccw_device_update_cmfblock,
		[DEV_EVENT_INTERRUPT]	= ccw_device_update_cmfblock,
		[DEV_EVENT_TIMEOUT]	= ccw_device_update_cmfblock,
		[DEV_EVENT_VERIFY]	= ccw_device_update_cmfblock,
	},
};

EXPORT_SYMBOL_GPL(ccw_device_set_timeout);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         K 25
svn:wc:ra_dav:version-url
V 75
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/video/bt8xx
END
Kconfig
K 25
svn:wc:ra_dav:version-url
V 83
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/video/bt8xx/Kconfig
END
bttvp.h
K 25
svn:wc:ra_dav:version-url
V 83
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/video/bt8xx/bttvp.h
END
bttv-driver.c
K 25
svn:wc:ra_dav:version-url
V 89
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/video/bt8xx/bttv-driver.c
END
bttv-cards.c
K 25
svn:wc:ra_dav:version-url
V 88
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/video/bt8xx/bttv-cards.c
END
bttv-if.c
K 25
svn:wc:ra_dav:version-url
V 85
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/video/bt8xx/bttv-if.c
END
bttv-gpio.c
K 25
svn:wc:ra_dav:version-url
V 87
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/video/bt8xx/bttv-gpio.c
END
bttv-input.c
K 25
svn:wc:ra_dav:version-url
V 88
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/video/bt8xx/bttv-input.c
END
bttv-audio-hook.c
K 25
svn:wc:ra_dav:version-url
V 93
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/video/bt8xx/bttv-audio-hook.c
END
bttv-risc.c
K 25
svn:wc:ra_dav:version-url
V 87
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/video/bt8xx/bttv-risc.c
END
bt848.h
K 25
svn:wc:ra_dav:version-url
V 83
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/video/bt8xx/bt848.h
END
bttv-audio-hook.h
K 25
svn:wc:ra_dav:version-url
V 93
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/video/bt8xx/bttv-audio-hook.h
END
bttv.h
K 25
svn:wc:ra_dav:version-url
V 82
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/video/bt8xx/bttv.h
END
bttv-i2c.c
K 25
svn:wc:ra_dav:version-url
V 86
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/video/bt8xx/bttv-i2c.c
END
Makefile
K 25
svn:wc:ra_dav:version-url
V 84
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/video/bt8xx/Makefile
END
bttv-vbi.c
K 25
svn:wc:ra_dav:version-url
V 86
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/media/video/bt8xx/bttv-vbi.c
END
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        /* ibmmpeg2.h - IBM MPEGCD21 definitions */

#ifndef __IBM_MPEG2__
#define __IBM_MPEG2__

/* Define all MPEG Decoder registers */
/* Chip Control and Status */
#define IBM_MP2_CHIP_CONTROL	0x200*2
#define IBM_MP2_CHIP_MODE	0x201*2
/* Timer Control and Status */
#define IBM_MP2_SYNC_STC2	0x202*2
#define IBM_MP2_SYNC_STC1	0x203*2
#define IBM_MP2_SYNC_STC0	0x204*2
#define IBM_MP2_SYNC_PTS2	0x205*2
#define IBM_MP2_SYNC_PTS1	0x206*2
#define IBM_MP2_SYNC_PTS0	0x207*2
/* Video FIFO Control */
#define IBM_MP2_FIFO		0x208*2
#define IBM_MP2_FIFOW		0x100*2
#define IBM_MP2_FIFO_STAT	0x209*2
#define IBM_MP2_RB_THRESHOLD	0x22b*2
/* Command buffer */
#define IBM_MP2_COMMAND		0x20a*2
#define IBM_MP2_CMD_DATA	0x20b*2
#define IBM_MP2_CMD_STAT	0x20c*2
#define IBM_MP2_CMD_ADDR	0x20d*2
/* Internal Processor Control and Status */
#define IBM_MP2_PROC_IADDR	0x20e*2
#define IBM_MP2_PROC_IDATA	0x20f*2
#define IBM_MP2_WR_PROT		0x235*2
/* DRAM Access */
#define IBM_MP2_DRAM_ADDR	0x210*2
#define IBM_MP2_DRAM_DATA	0x212*2
#define IBM_MP2_DRAM_CMD_STAT	0x213*2
#define IBM_MP2_BLOCK_SIZE	0x23b*2
#define IBM_MP2_SRC_ADDR	0x23c*2
/* Onscreen Display */
#define IBM_MP2_OSD_ADDR	0x214*2
#define IBM_MP2_OSD_DATA	0x215*2
#define IBM_MP2_OSD_MODE	0x217*2
#define IBM_MP2_OSD_LINK_ADDR	0x229*2
#define IBM_MP2_OSD_SIZE	0x22a*2
/* Interrupt Control */
#define IBM_MP2_HOST_INT	0x218*2
#define IBM_MP2_MASK0		0x219*2
#define IBM_MP2_HOST_INT1	0x23e*2
#define IBM_MP2_MASK1		0x23f*2
/* Audio Control */
#define IBM_MP2_AUD_IADDR	0x21a*2
#define IBM_MP2_AUD_IDATA	0x21b*2
#define IBM_MP2_AUD_FIFO	0x21c*2
#define IBM_MP2_AUD_FIFOW	0x101*2
#define IBM_MP2_AUD_CTL		0x21d*2
#define IBM_MP2_BEEP_CTL	0x21e*2
#define IBM_MP2_FRNT_ATTEN	0x22d*2
/* Display Control */
#define IBM_MP2_DISP_MODE	0x220*2
#define IBM_MP2_DISP_DLY	0x221*2
#define IBM_MP2_VBI_CTL		0x222*2
#define IBM_MP2_DISP_LBOR	0x223*2
#define IBM_MP2_DISP_TBOR	0x224*2
/* Polarity Control */
#define IBM_MP2_INFC_CTL	0x22c*2

/* control commands */
#define IBM_MP2_PLAY		0
#define IBM_MP2_PAUSE		1
#define IBM_MP2_SINGLE_FRAME	2
#define IBM_MP2_FAST_FORWARD	3
#define IBM_MP2_SLOW_MOTION	4
#define IBM_MP2_IMED_NORM_PLAY	5
#define IBM_MP2_RESET_WINDOW	6
#define IBM_MP2_FREEZE_FRAME	7
#define IBM_MP2_RESET_VID_RATE	8
#define IBM_MP2_CONFIG_DECODER	9
#define IBM_MP2_CHANNEL_SWITCH	10
#define IBM_MP2_RESET_AUD_RATE	11
#define IBM_MP2_PRE_OP_CHN_SW	12
#define IBM_MP2_SET_STILL_MODE	14

/* Define Xilinx FPGA Internal Registers */

/* general control register 0 */
#define XILINX_CTL0		0x600
/* genlock 