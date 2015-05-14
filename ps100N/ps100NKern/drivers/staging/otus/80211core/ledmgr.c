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

#include "cprecomp.h"

/************************************************************************/
/*                                                                      */
/*    FUNCTION DESCRIPTION                  zfLedCtrlType1              */
/*      Traditional single-LED state                                    */
/*                                                                      */
/*    INPUTS                                                            */
/*      dev : device pointer                                            */
/*                                                                      */
/*    OUTPUTS                                                           */
/*      None                                                            */
/*                                                                      */
/*    AUTHOR                                                            */
/*      Stephen Chen        Atheros Communications, INC.    2007.6      */
/*                                                                      */
/************************************************************************/
// bit 15-12 : Toff for Scan state
//     11-8 : Ton for Scan state
//     7 : Reserved
//     6 : mode
//--------------------------------------
//     bit 6 = 0
//     5-4 : Connect state
//           00 => always off
//           01 => always on
//           10 => Idle off, acitve on
//           11 => Idle on, active off
//--------------------------------------
//     bit 6 = 1
//     5-4 : freq
//           00 => 1Hz
//           01 => 0.5Hz
//           10 => 0.25Hz
//           11 => 0.125Hz
//--------------------------------------
//     3 : Power save state
//         0 => always off in power save state
//         1 => works as connect state
//     2 : Disable state
//     1 : Reserved
//     0 : Power-on state
void zfLedCtrlType1(zdev_t* dev)
{
    u16_t i;
    u32_t ton, toff, tmp, period;
    zmw_get_wlan_dev(dev);

    for (i=0; i<ZM_MAX_LED_NUMBER; i++)
    {
        if (zfStaIsConnected(dev) != TRUE)
        {
            //Scan state
            ton = ((wd->ledStruct.ledMode[i] & 0xf00) >> 8) * 5;
            toff = ((wd->ledStruct.ledMode[i] & 0xf000) >> 12) * 5;

            if ((ton + toff) != 0)
            {
                tmp = wd->ledStruct.counter / (ton+toff);
                tmp = wd->ledStruct.counter - (tmp * (ton+toff));
                if (tmp < ton)
                {
                    zfHpLedCtrl(dev, i, 1);
                }
                else
                {
                    zfHpLedCtrl(dev, i, 0);
                }
            }
        }
        else
        {
            if ((zfPowerSavingMgrIsSleeping(dev)) && ((wd->ledStruct.ledMode[i] & 0x8) == 0))
            {
                zfHpLedCtrl(dev, i, 0);
            }
            else
            {
                //Connect state
                if ((wd->ledStruct.ledMode[i] & 0x40) == 0)
                {
                    if ((wd->ledStruct.counter & 1) == 0)
                    {
                        zfHpLedCtrl(dev, i, (wd->ledStruct.ledMode[i] & 0x10) >> 4);
                    }
                    else
                    {
                        if ((wd->ledStruct.txTraffic > 0) || (wd->ledStruct.rxTraffic >page;

	/*
	 * check ECC mode, default to software if 3byte/512byte hardware ECC is
	 * selected and we have 256 byte pagesize fallback to software ECC
	 */

	switch (chip->ecc.mode) {
	case NAND_ECC_HW:
		/* Use standard hwecc read page function ? */
		if (!chip->ecc.read_page)
			chip->ecc.read_page = nand_read_page_hwecc;
		if (!chip->ecc.write_page)
			chip->ecc.write_page = nand_write_page_hwecc;
		if (!chip->ecc.read_page_raw)
			chip->ecc.read_page_raw = nand_read_page_raw;
		if (!chip->ecc.write_page_raw)
			chip->ecc.write_page_raw = nand_write_page_raw;
		if (!chip->ecc.read_oob)
			chip->ecc.read_oob = nand_read_oob_std;
		if (!chip->ecc.write_oob)
			chip->ecc.write_oob = nand_write_oob_std;

	case NAND_ECC_HW_SYNDROME:
		if ((!chip->ecc.calculate || !chip->ecc.correct ||
		     !chip->ecc.hwctl) &&
		    (!chip->ecc.read_page ||
		     chip->ecc.read_page == nand_read_page_hwecc ||
		     !chip->ecc.write_page ||
		     chip->ecc.write_page == nand_write_page_hwecc)) {
			printk(KERN_WARNING "No ECC functions supplied, "
			       "Hardware ECC not possible\n");
			BUG();
		}
		/* Use standard syndrome read/write page function ? */
		if (!chip->ecc.read_page)
			chip->ecc.read_page = nand_read_page_syndrome;
		if (!chip->ecc.write_page)
			chip->ecc.write_page = nand_write_page_syndrome;
		if (!chip->ecc.read_page_raw)
			chip->ecc.read_page_raw = nand_read_page_raw_syndrome;
		if (!chip->ecc.write_page_raw)
			chip->ecc.write_page_raw = nand_write_page_raw_syndrome;
		if (!chip->ecc.read_oob)
			chip->ecc.read_oob = nand_read_oob_syndrome;
		if (!chip->ecc.write_oob)
			chip->ecc.write_oob = nand_write_oob_syndrome;

		if (mtd->writesize >= chip->ecc.size)
			break;
		printk(KERN_WARNING "%d byte HW ECC not possible on "
		       "%d byte page size, fallback to SW ECC\n",
		       chip->ecc.size, mtd->writesize);
		chip->ecc.mode = NAND_ECC_SOFT;

	case NAND_ECC_SOFT:
		chip->ecc.calculate = nand_calculate_ecc;
		chip->ecc.correct = nand_correct_data;
		chip->ecc.read_page = nand_read_page_swecc;
		chip->ecc.read_subpage = nand_read_subpage;
		chip->ecc.write_page = nand_write_page_swecc;
		chip->ecc.read_page_raw = nand_read_page_raw;
		chip->ecc.write_page_raw = nand_write_page_raw;
		chip->ecc.read_oob = nand_read_oob_std;
		chip->ecc.write_oob = nand_write_oob_std;
		chip->ecc.size = 256;
		chip->ecc.bytes = 3;
		break;

	case NAND_ECC_NONE:
		printk(KERN_WARNING "NAND_ECC_NONE selected by board driver. "
		       "This is not recommended !!\n");
		chip->ecc.read_page = nand_read_page_raw;
		chip->ecc.write_page = nand_write_page_raw;
		chip->ecc.read_oob = nand_read_oob_std;
		chip->ecc.read_page_raw = nand_read_page_raw;
		chip->ecc.write_page_raw = nand_write_page_raw;
		chip->ecc.write_oob = nand_write_oob_std;
		chip->ecc.size = mtd->writesize;
		chip->ecc.bytes = 0;
		break;

	default:
		printk(KERN_WARNING "Invalid NAND_ECC_MODE %d\n",
		       chip->ecc.mode);
		BUG();
	}

	/*
	 * The number of bytes available for a client to place data into
	 * the out of band area
	 */
	chip->ecc.layout->oobavail = 0;
	for (i = 0; chip->ecc.layout->oobfree[i].length
			&& i < ARRAY_SIZE(chip->ecc.layout->oobfree); i++)
		chip->ecc.layout->oobavail +=
			chip->ecc.layout->oobfree[i].length;
	mtd->oobavail = chip->ecc.layout->oobavail;

	/*
	 * Set the number of read / write steps for one page depending on ECC
	 * mode
	 */
	chip->ecc.steps = mtd->writesize / chip->ecc.size;
	if(chip->ecc.steps * chip->ecc.size != mtd->writesize) {
		printk(KERN_WARNING "Invalid ecc parameters\n");
		BUG();
	}
	chip->ecc.total = chip->ecc.steps * chip->ecc.bytes;

	/*
	 * Allow subpage writes up to ecc.steps. Not possible for MLC
	 * FLASH.
	 */
	if (!(chip->options & NAND_NO_SUBPAGE_WRITE) &&
	    !(chip->cellinfo & NAND_CI_CELLTYPE_MSK)) {
		switch(chip->ecc.steps) {
		case 2:
			mtd->subpage_sft = 1;
			break;
		case 4:
		case 8:
		case 16:
			mtd->subpage_sft = 2;
			break;
		}
	}
	chip->subpagesize = mtd->writesize >> mtd->subpage_sft;

	/* Initialize state */
	chip->state = FL_READY;

	/* De-select the device */
	chip->select_chip(mtd, -1);

	/* Invalidate the pagebuffer reference */
	chip->pagebuf = -1;

	/* Fill in remaining MTD driver data */
	mtd->type = MTD_NANDFLASH;
	mtd->flags = MTD_CAP_NANDFLASH;
	mtd->erase = nand_erase;
	mtd->point = NULL;
	mtd->unpoint = NULL;
	mtd->read = nand_read;
	mtd->write = nand_write;
	mtd->read_oob = nand_read_oob;
	mtd->write_oob = nand_write_oob;
	mtd->sync = nand_sync;
	mtd->lock = NULL;
	mtd->unlock = NULL;
	mtd->suspend = nand_suspend;
	mtd->resume = nand_resume;
	mtd->block_isbad = nand_block_isbad;
	mtd->block_markbad = nand_block_markbad;

	/* propagate ecc.layout to mtd_info */
	mtd->ecclayout = chip->ecc.layout;

	/* Check, if we should skip the bad block table scan */
	if (chip->options & NAND_SKIP_BBTSCAN)
		return 0;

	/* Build bad block table */
	return chip->scan_bbt(mtd);
}

/* is_module_text_address() isn't exported, and it's mostly a pointless
   test if this is a module _anyway_ -- they'd have to try _really_ hard
   to call us from in-kernel code if the core NAND support is modular. */
#ifdef MODULE
#define caller_is_module() (1)
#else
#define caller_is_module() \
	is_module_text_address((unsigned long)__builtin_return_address(0))
#endif

/**
 * nand_scan - [NAND Interface] Scan for the NAND device
 * @mtd:	MTD device structure
 * @maxchips:	Number of chips to scan for
 *
 * This fills out all the uninitialized function pointers
 * with the defaults.
 * The flash ID is read and the mtd/chip structures are
 * filled with the appropriate values.
 * The mtd->owner field must be set to the module of the caller
 *
 */
int nand_scan(struct mtd_info *mtd, int maxchips)
{
	int ret;

	/* Many callers got this wrong, so check for it for a while... */
	if (!mtd->owner && caller_is_module()) {
		printk(KERN_CRIT "nand_scan() called with NULL mtd->owner!\n");
		BUG();
	}

	ret = nand_scan_ident(mtd, maxchips);
	if (!ret)
		ret = nand_scan_tail(mtd);
	return ret;
}

/**
 * nand_release - [NAND Interface] Free resources held by the NAND device
 * @mtd:	MTD device structure
*/
void nand_release(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;

#ifdef CONFIG_MTD_PARTITIONS
	/* Deregister partitions */
	del_mtd_partitions(mtd);
#endif
	/* Deregister the device */
	del_mtd_device(mtd);

	/* Free bad block table memory */
	kfree(chip->bbt);
	if (!(chip->options & NAND_OWN_BUFFERS))
		kfree(chip->buffers);
}

EXPORT_SYMBOL_GPL(nand_scan);
EXPORT_SYMBOL_GPL(nand_scan_ident);
EXPORT_SYMBOL_GPL(nand_scan_tail);
EXPORT_SYMBOL_GPL(nand_release);

static int __init nand_base_init(void)
{
	led_trigger_register_simple("nand-disk", &nand_led_trigger);
	return 0;
}

static void __exit nand_base_exit(void)
{
	led_trigger_unregister_simple(nand_led_trigger);
}

module_init(nand_base_init);
module_exit(nand_base_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Steven J. Hill <sjhill@realitydiluted.com>, Thomas Gleixner <tglx@linutronix.de>");
MODULE_DESCRIPTION("Generic NAND flash driver code");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            /*
 * random.c -- A strong random number generator
 *
 * Copyright Matt Mackall <mpm@selenic.com>, 2003, 2004, 2005
 *
 * Copyright Theodore Ts'o, 1994, 1995, 1996, 1997, 1998, 1999.  All
 * rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, and the entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * ALTERNATIVELY, this product may be distributed under the terms of
 * the GNU General Public License, in which case the provisions of the GPL are
 * required INSTEAD OF the above restrictions.  (This clause is
 * necessary due to a potential bad interaction between the GPL and
 * the restrictions contained in a BSD-style copyright.)
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ALL OF
 * WHICH ARE HEREBY DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF NOT ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/*
 * (now, with legal B.S. out of the way.....)
 *
 * This routine gathers environmental noise from device drivers, etc.,
 * and returns good random numbers, suitable for cryptographic use.
 * Besides the obvious cryptographic uses, these numbers are also good
 * for seeding TCP sequence numbers, and other places where it is
 * desirable to have numbers which are not only random, but hard to
 * predict by an attacker.
 *
 * Theory of operation
 * ===================
 *
 * Computers are very predictable devices.  Hence it is extremely hard
 * to produce truly random numbers on a computer --- as opposed to
 * pseudo-random numbers, which can easily generated by using a
 * algorithm.  Unfortunately, it is very easy for attackers to guess
 * the sequence of pseudo-random number generators, and for some
 * applications this is not acceptable.  So instead, we must try to
 * gather "environmental noise" from the computer's environment, which
 * must be hard for outside attackers to observe, and use that to
 * generate random numbers.  In a Unix environment, this is best done
 * from inside the kernel.
 *
 * Sources of randomness from the environment include inter-keyboard
 * timings, inter-interrupt timings from some interrupts, and other
 * events which are both (a) non-deterministic and (b) hard for an
 * outside observer to measure.  Randomness from these sources are
 * added to an "entropy pool", which is mixed using a CRC-like function.
 * This is not cryptographically strong, but it is adequate assuming
 * the randomness is not chosen maliciously, and it is fast enough that
 * the overhead of doing it on every interrupt is very reasonable.
 * As random bytes are mixed into the entropy pool, the routines keep
 * an *estimate* of how many bits of randomness have been stored into
 * the random number generator's internal state.
 *
 * When random bytes are desired, they are obtained by taking the SHA
 * hash of the contents of the "entropy pool".  The SHA hash avoids
 * exposing the internal state of the entropy pool.  It is believed to
 * be computationally infeasible to derive any useful information
 * about the input of SHA from its output.  Even if it is possible to
 * analyze SHA in some clever way, as long as the amount of data
 * returned from the generator is less than the inherent entropy in
 * the pool, the output data is totally unpredictable.  For this
 * reason, the routine decreases its internal estimate of how many
 * bits of "true randomness" are contained in the entropy pool as it
 * outputs random numbers.
 *
 * If this estimate goes to zero, the routine can still generate
 * random numbers; however, an attacker may (at least in theory) be
 * able to infer the future output of the generator from prior
 * outputs.  This requires successful cryptanalysis of SHA, which is
 * not believed to be feasible, but there is a remote possibility.
 * Nonetheless, these numbers should be useful for the vast majority
 * of purposes.
 *
 * Exported interfaces ---- output
 * ===============================
 *
 * There are three exported interfaces; the first is one designed to
 * be used from within the kernel:
 *
 * 	void get_random_bytes(void *buf, int nbytes);
 *
 * This interface will return the requested number of random bytes,
 * and place it in the requested buffer.
 *
 * The two other interfaces are two character devices /dev/random and
 * /dev/urandom.  /dev/random is suitable for use when very high
 * quality randomness is desired (for example, for key generation or
 * one-time pads), as it will only return a maximum of the number of
 * bits of randomness (as estimated by the random number generator)
 * contained in the entropy pool.
 *
 * The /dev/urandom device does not have this limit, and will return
 * as many bytes as are requested.  As more and more random bytes are
 * requested without giving time for the entropy pool to recharge,
 * this will result in random numbers that are merely cryptographically
 * strong.  For many applications, however, this is acceptable.
 *
 * Exported interfaces ---- input
 * ==============================
 *
 * The current exported interfaces for gathering environmental noise
 * from the devices are:
 *
 * 	void add_input_randomness(unsigned int type, unsigned int code,
 *                                unsigned int value);
 * 	void add_interrupt_randomness(int irq);
 *
 * add_input_randomness() uses the input layer interrupt timing, as well as
 * the event type information from the hardware.
 *
 * add_interrupt_randomness() uses the inter-interrupt timing as random
 * inputs to the entropy pool.  Note that not all interrupts are good
 * sources of randomness!  For example, the timer interrupts is not a
 * good choice, because the periodicity of the interrupts is too
 * regular, and hence predictable to an attacker.  Disk interrupts are
 * a better measure, since the timing of the disk interrupts are more
 * unpredictable.
 *
 * All of these routines try to estimate how many bits of randomness a
 * particular randomness source.  They do this by keeping track of the
 * first and second order deltas of the event timings.
 *
 * Ensuring unpredictability at system startup
 * ============================================
 *
 * When any operating system starts up, it will go through a sequence
 * of actions that are fairly predictable by an adversary, especially
 * if the start-up does not involve interaction with a human operator.
 * This reduces the actual number of bits of unpredictability in the
 * entropy pool below the value in entropy_count.  In order to
 * counteract this effect, it helps to carry information in the
 * entropy pool across shut-downs and start-ups.  To do this, put the
 * following lines an appropriate script which is run during the boot
 * sequence:
 *
 *	echo "Initializing random number generator..."
 *	random_seed=/var/run/random-seed
 *	# Carry a random seed from start-up to start-up
 *	# Load and then save the whole entropy pool
 *	if [ -f $random_seed ]; then
 *		cat $random_seed >/dev/urandom
 *	else
 *		touch $random_seed
 *	fi
 *	chmod 600 $random_seed
 *	dd if=/dev/urandom of=$random_seed count=1 bs=512
 *
 * and the following lines in an appropriate script which is run as
 * the system is shutdown:
 *
 *	# Carry a random seed from shut-down to start-up
 *	# Save the whole entropy pool
 *	echo "Saving random seed..."
 *	random_seed=/var/run/random-seed
 *	touch $random_seed
 *	chmod 600 $random_seed
 *	dd if=/dev/urandom of=$random_seed count=1 bs=512
 *
 * For example, on most modern systems using the System V init
 * scripts, such code fragments would be found in
 * /etc/rc.d/init.d/random.  On older Linux systems, the correct script
 * location might be in /etc/rcb.d/rc.local or /etc/rc.d/rc.0.
 *
 * Effectively, these commands cause the contents of the entropy pool
 * to be saved at shut-down time and reloaded into the entropy pool at
 * start-up.  (The 'dd' in the addition to the bootup script is to
 * make sure that /etc/random-seed is different for every start-up,
 * even if the system crashes without executing rc.0.)  Even with
 * complete knowledge of the start-up activities, predicting the state
 * of the entropy pool requires knowledge of the previous history of
 * the system.
 *
 * Configuring the /dev/random driver under Linux
 * ==============================================
 *
 * The /dev/random driver under Linux uses minor numbers 8 and 9 of
 * the /dev/mem major number (#1).  So if your system does not have
 * /dev/random and /dev/urandom created already, they can be created
 * by using the commands:
 *
 * 	mknod /dev/random c 1 8
 * 	mknod /dev/urandom c 1 9
 *
 * Acknowledgements:
 * =================
 *
 * Ideas for constructing this random number generator were derived
 * from Pretty Good Privacy's random number generator, and from private
 * discussions with Phil Karn.  Colin Plumb provided a faster random
 * number generator, which speed up the mixing function of the entropy
 * pool, taken from PGPfone.  Dale Worley has also contributed many
 * useful ideas and suggestions to improve this driver.
 *
 * Any flaws in the design are s