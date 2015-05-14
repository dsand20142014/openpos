/*
 * linux/kernel/time/clocksource.c
 *
 * This file contains the functions which manage clocksource drivers.
 *
 * Copyright (C) 2004, 2005 IBM, John Stultz (johnstul@us.ibm.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * TODO WishList:
 *   o Allow clocksource drivers to be unregistered
 */

#include <linux/device.h>
#include <linux/clocksource2.h>
#include <linux/clocksource.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h> /* for spin_unlock_irq() using preempt_count() m68k */
#include <linux/tick.h>
#include <linux/kthread.h>

void timecounter_init35(struct timecounter *tc,
		      const struct cyclecounter *cc,
		      u64 start_tstamp)
{
	tc->cc = cc;
	tc->cycle_last = cc->read(cc);
	tc->nsec = start_tstamp;
}
EXPORT_SYMBOL_GPL(timecounter_init35);

/**
 * timecounter_read_delta - get nanoseconds since last call of this function
 * @tc:         Pointer to time counter
 *
 * When the underlying cycle counter runs over, this will be handled
 * correctly as long as it does not run over more than once between
 * calls.
 *
 * The first call to this function for a new time counter initializes
 * the time tracking and returns an undefined result.
 */
static u64 timecounter_read_delta(struct timecounter *tc)
{
	cycle_t cycle_now, cycle_delta;
	u64 ns_offset;

	/* read cycle counter: */
	cycle_now = tc->cc->read(tc->cc);

	/* calculate the delta since the last timecounter_read_delta(): */
	cycle_delta = (cycle_now - tc->cycle_last) & tc->cc->mask;

	/* convert to nanoseconds: */
	ns_offset = cyclecounter_cyc2ns(tc->cc, cycle_delta);

	/* update time stamp of timecounter_read_delta() call: */
	tc->cycle_last = cycle_now;

	return ns_offset;
}

u64 timecounter_read35(struct timecounter *tc)
{
	u64 nsec;

	/* increment time by nanoseconds since last call */
	nsec = timecounter_read_delta(tc);
	nsec += tc->nsec;
	tc->nsec = nsec;

	return nsec;
}
EXPORT_SYMBOL_GPL(timecounter_read35);

u64 timecounter_cyc2time35(struct timecounter *tc,
			 cycle_t cycle_tstamp)
{
	u64 cycle_delta = (cycle_tstamp - tc->cycle_last) & tc->cc->mask;
	u64 nsec;

	/*
	 * Instead of always treating cycle_tstamp as more recent
	 * than tc->cycle_last, detect when it is too far in the
	 * future and treat it as old time stamp instead.
	 */
	if (cycle_delta > tc->cc->mask / 2) {
		cycle_delta = (tc->cycle_last - cycle_tstamp) & tc->cc->mask;
		nsec = tc->nsec - cyclecounter_cyc2ns(tc->cc, cycle_delta);
	} else {
		nsec = cyclecounter_cyc2ns(tc->cc, cycle_delta) + tc->nsec;
	}

	return nsec;
}
EXPORT_SYMBOL_GPL(timecounter_cyc2time35);

/**
 * clocks_calc_mult_shift - calculate mult/shift factors for scaled math of clocks
 * @mult:	pointer to mult variable
 * @shift:	pointer to shift variable
 * @from:	frequency to convert from
 * @to:		frequency to convert to
 * @maxsec:	guaranteed runtime conversion range in seconds
 *
 * The function evaluates the shift/mult pair for the scaled math
 * operations of clocksources and clockevents.
 *
 * @to and @from are frequency values in HZ. For clock sources @to is
 * NSEC_PER_SEC == 1GHz and @from is the counter frequency. For clock
 * event @to is the counter frequency and @from is NSEC_PER_SEC.
 *
 * The @maxsec conversion range argument controls the time frame in
 * seconds which must be covered by the runtime conversion with the
 * calculated mult and shift factors. This guarantees that no 64bit
 * overflow happens when the input vaORT_INPUT_REFERENCE_SIGNAL,1,
	 "Task A: X port input reference signal definition"},
	{R_93_A_I_PORT_OUTPUT_FORMATS_AND_CONF,1,
	 "Task A: I port output formats and configuration"},
	{R_94_A_HORIZ_INPUT_WINDOW_START,2,
	 "Task A: Horizontal input window start"},
	{R_96_A_HORIZ_INPUT_WINDOW_LENGTH,2,
	 "Task A: Horizontal input window length"},
	{R_98_A_VERT_INPUT_WINDOW_START,2,
	 "Task A: Vertical input window start"},
	{R_9A_A_VERT_INPUT_WINDOW_LENGTH,2,
	 "Task A: Vertical input window length"},
	{R_9C_A_HORIZ_OUTPUT_WINDOW_LENGTH,2,
	 "Task A: Horizontal output window length"},
	{R_9E_A_VERT_OUTPUT_WINDOW_LENGTH,2,
	 "Task A: Vertical output window length"},

	/* Task A: FIR filtering and prescaling */
	{R_A0_A_HORIZ_PRESCALING,1,
	 "Task A: Horizontal prescaling"},
	{R_A1_A_ACCUMULATION_LENGTH,1,
	 "Task A: Accumulation length"},
	{R_A2_A_PRESCALER_DC_GAIN_AND_FIR_PREFILTER,1,
	 "Task A: Prescaler DC gain and FIR prefilter"},
	/* 0xa3 - Reserved */
	{R_A4_A_LUMA_BRIGHTNESS_CNTL,1,
	 "Task A: Luminance brightness control"},
	{R_A5_A_LUMA_CONTRAST_CNTL,1,
	 "Task A: Luminance contrast control"},
	{R_A6_A_CHROMA_SATURATION_CNTL,1,
	 "Task A: Chrominance saturation control"},
	/* 0xa7 - Reserved */

	/* Task A: Horizontal phase scaling */
	{R_A8_A_HORIZ_LUMA_SCALING_INC,2,
	 "Task A: Horizontal luminance scaling increment"},
	{R_AA_A_HORIZ_LUMA_PHASE_OFF,1,
	 "Task A: Horizontal luminance phase offset"},
	/* 0xab - Reserved */
	{R_AC_A_HORIZ_CHROMA_SCALING_INC,2,
	 "Task A: Horizontal chrominance scaling increment"},
	{R_AE_A_HORIZ_CHROMA_PHASE_OFF,1,
	 "Task A: Horizontal chrominance phase offset"},
	/* 0xaf - Reserved */

	/* Task A: Vertical scaling */
	{R_B0_A_VERT_LUMA_SCALING_INC,2,
	 "Task A: Vertical luminance scaling increment"},
	{R_B2_A_VERT_CHROMA_SCALING_INC,2,
	 "Task A: Vertical chrominance scaling increment"},
	{R_B4_A_VERT_SCALING_MODE_CNTL,1,
	 "Task A: Vertical scaling mode control"},
	/* 0xb5-0xb7 - Reserved */
	{R_B8_A_VERT_CHROMA_PHASE_OFF_00,1,
	 "Task A: Vertical chrominance phase offset '00'"},
	{R_B9_A_VERT_CHROMA_PHASE_OFF_01,1,
	 "Task A: Vertical chrominance phase offset '01'"},
	{R_BA_A_VERT_CHROMA_PHASE_OFF_10,1,
	 "Task A: Vertical chrominance phase offset '10'"},
	{R_BB_A_VERT_CHROMA_PHASE_OFF_11,1,
	 "Task A: Vertical chrominance phase offset '11'"},
	{R_BC_A_VERT_LUMA_PHASE_OFF_00,1,
	 "Task A: Vertical luminance phase offset '00'"},
	{R_BD_A_VERT_LUMA_PHASE_OFF_01,1,
	 "Task A: Vertical luminance phase offset '01'"},
	{R_BE_A_VERT_LUMA_PHASE_OFF_10,1,
	 "Task A: Vertical luminance phase offset '10'"},
	{R_BF_A_VERT_LUMA_PHASE_OFF_11,1,
	 "Task A: Vertical luminance phase offset '11'"},

	/* Task B definition: R_C0_B_TASK_HANDLING_CNTL to R_EF_B_VERT_LUMA_PHASE_OFF_11 */
	/* Task B: Basic settings and acquisition window definition */
	{R_C0_B_TASK_HANDLING_CNTL,1,
	 "Task B: Task handling control"},
	{R_C1_B_X_PORT_FORMATS_AND_CONF,1,
	 "Task B: X port formats and configuration"},
	{R_C2_B_INPUT_REFERENCE_SIGNAL_DEFINITION,1,
	 "Task B: Input reference signal definition"},
	{R_C3_B_I_PORT_FORMATS_AND_CONF,1,
	 "Task B: I port formats and configuration"},
	{R_C4_B_HORIZ_INPUT_WINDOW_START,2,
	 "Task B: Horizontal input window start"},
	{R_C6_B_HORIZ_INPUT_WINDOW_LENGTH,2,
	 "Task B: Horizontal input window length"},
	{R_C8_B_VERT_INPUT_WINDOW_START,2,
	 "Task B: Vertical input window start"},
	{R_CA_B_VERT_INPUT_WINDOW_LENGTH,2,
	 "Task B: Vertical input window length"},
	{R_CC_B_HORIZ_OUTPUT_WINDOW_LENGTH,2,
	 "Task B: Horizontal output window length"},
	{R_CE_B_VERT_OUTPUT_WINDOW_LENGTH,2,
	 "Task B: Vertical output window length"},

	/* Task B: FIR filtering and prescaling */
	{R_D0_B_HORIZ_PRESCALING,1,
	 "Task B: Horizontal prescaling"},
	{R_D1_B_ACCUMULATION_LENGTH,1,
	 "Task B: Accumulation length"},
	{R_D2_B_PRESCALER_DC_GAIN_AND_FIR_PREFILTER,1,
	 "Task B: Prescaler DC gain and FIR prefilter"},
	/* 0xd3 - Reserved */
	{R_D4_B_LUMA_BRIGHTNESS_CNTL,1,
	 "Task B: Luminance brightness control"},
	{R_D5_B_LUMA_CONTRAST_CNTL,1,
	 "Task B: Luminance contrast control"},
	{R_D6_B_CHROMA_SATURATION_CNTL,1,
	 "Task B: Chrominance saturation control"},
	/* 0xd7 - Reserved */

	/* Task B: Horizontal phase scaling */
	{R_D8_B_HORIZ_LUMA_SCALING_INC,2,
	 "Task B: Horizontal luminance scaling increment"},
	{R_DA_B_HORIZ_LUMA_PHASE_OFF,1,
	 "Task B: Horizontal luminance phase offset"},
	/* 0xdb - Reserved */
	{R_DC_B_HORIZ_CHROMA_SCALING,2,
	 "Task B: Horizontal chrominance scaling"},
	{R_DE_B_HORIZ_PHASE_OFFSET_CRHOMA,1,
	 "Task B: Horizontal Phase Offset Chroma"},
	/* 0xdf - Reserved */

	/* Task B: Vertical scaling */
	{R_E0_B_VERT_LUMA_SCALING_INC,2,
	 "Task B: Vertical luminance scaling increment"},
	{R_E2_B_VERT_CHROMA_SCALING_INC,2,
	 "Task B: Vertical chrominance scaling increment"},
	{R_E4_B_VERT_SCALING_MODE_CNTL,1,
	 "Task B: Vertical scaling mode control"},
	/* 0xe5-0xe7 - Reserved */
	{R_E8_B_VERT_CHROMA_PHASE_OFF_00,1,
	 "Task B: Vertical chrominance phase offset '00'"},
	{R_E9_B_VERT_CHROMA_PHASE_OFF_01,1,
	 "Task B: Vertical chrominance phase offset '01'"},
	{R_EA_B_VERT_CHROMA_PHASE_OFF_10,1,
	 "Task B: Vertical chrominance phase offset '10'"},
	{R_EB_B_VERT_CHROMA_PHASE_OFF_11,1,
	 "Task B: Vertical chrominance phase offset '11'"},
	{R_EC_B_VERT_LUMA_PHASE_OFF_00,1,
	 "Task B: Vertical luminance phase offset '00'"},
	{R_ED_B_VERT_LUMA_PHASE_OFF_01,1,
	 "Task B: Vertical luminance phase offset '01'"},
	{R_EE_B_VERT_LUMA_PHASE_OFF_10,1,
	 "Task B: Vertical luminance phase offset '10'"},
	{R_EF_B_VERT_LUMA_PHASE_OFF_11,1,
	 "Task B: Vertical luminance phase offset '11'"},

	/* second PLL (PLL2) and Pulsegenerator Programming */
	{ R_F0_LFCO_PER_LINE, 1,
	  "LFCO's per line"},
	{ R_F1_P_I_PARAM_SELECT,1,
	  "P-/I- Param. Select., PLL Mode, PLL H-Src., LFCO's per line"},
	{ R_F2_NOMINAL_PLL2_DTO,1,
	 "Nominal PLL2 DTO"},
	{R_F3_PLL_INCREMENT,1,
	 "PLL2 Increment"},
	{R_F4_PLL2_STATUS,1,
	 "PLL2 Status"},
	{R_F5_PULSGEN_LINE_LENGTH,1,
	 "Pulsgen. line length"},
	{R_F6_PULSE_A_POS_LSB_AND_PULSEGEN_CONFIG,1,
	 "Pulse A Position, Pulsgen Resync., Pulsgen. H-Src., Pulsgen. line length"},
	{R_F7_PULSE_A_POS_MSB,1,
	 "Pulse A Position"},
	{R_F8_PULSE_B_POS,2,
	 "Pulse B Position"},
	{R_FA_PULSE_C_POS,2,
	 "Pulse C Position"},
	/* 0xfc to 0xfe - Reserved */
	{R_FF_S_PLL_MAX_PHASE_ERR_THRESH_NUM_LINES,1,
	 "S_PLL max. phase, error threshold, PLL2 no. of lines, threshold"},
};
#endif
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             /*
 *  Copyright (c) 1998-2001 Vojtech Pavlik
 *
 *  Based on the work of:
 *	Steffen Schwenke
 */

/*
 * TurboGraFX parallel port interface driver for Linux.
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Should you need to contact me, the author, you can do so either by
 * e-mail - mail your message to <vojtech@ucw.cz>, or by paper mail:
 * Vojtech Pavlik, Simunkova 1594, Prague 8, 182 00 Czech Republic
 */

#include <linux/kernel.h>
#include <linux/parport.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mutex.h>

MODULE_AUTHOR("Vojtech Pavlik <vojtech@ucw.cz>");
MODULE_DESCRIPTION("TurboGraFX parallel port interface driver");
MODULE_LICENSE("GPL");

#define TGFX_MAX_PORTS		3
#define TGFX_MAX_DEVICES	7

struct tgfx_config {
	int args[TGFX_MAX_DEVICES + 1];
	unsigned int nargs;
};

static struct tgfx_config tgfx_cfg[TGFX_MAX_PORTS] __initdata;

module_param_array_named(map, tgfx_cfg[0].args, int, &tgfx_cfg[0].nargs, 0);
MODULE_PARM_DESC(map, "Describes first set of devices (<parport#>,<js1>,<js2>,..<js7>");
module_param_array_named(map2, tgfx_cfg[1].args, int, &tgfx_cfg[1].nargs, 0);
MODULE_PARM_DESC(map2, "Describes second set of devices");
module_param_array_named(map3, tgfx_cfg[2].args, int, &tgfx_cfg[2].nargs, 0);
MODULE_PARM_DESC(map3, "Describes third set of devices");

#define TGFX_REFRESH_TIME	HZ/100	/* 10 ms */

#define TGFX_TRIGGER		0x08
#define TGFX_UP			0x10
#define TGFX_DOWN		0x20
#define TGFX_LEFT		0x40
#define TGFX_RIGHT		0x80

#define TGFX_THUMB		0x02
#define TGFX_THUMB2		0x04
#define TGFX_TOP		0x01
#define TGFX_TOP2		0x08

static int tgfx_buttons[] = { BTN_TRIGGER, BTN_THUMB, BTN_THUMB2, BTN_TOP, BTN_TOP2 };

static struct tgfx {
	struct pardevice *pd;
	struct timer_list timer;
	struct input_dev *dev[TGFX_MAX_DEVICES];
	char name[TGFX_MAX_DEVICES][64];
	char phys[TGFX_MAX_DEVICES][32];
	int sticks;
	int used;
	struct mutex sem;
} *tgfx_base[TGFX_MAX_PORTS];

/*
 * tgfx_timer() reads and analyzes TurboGraFX joystick data.
 */

static void tgfx_timer(unsigned long private)
{
	struct tgfx *tgfx = (void *) private;
	struct input_dev *dev;
	int data1, data2, i;

	for (i = 0; i < 7; i++)
		if (tgfx->sticks & (1 << i)) {

			dev = tgfx->dev[i];

			parport_write_data(tgfx->pd->port, ~(1 << i));
			data1 = parport_read_status(tgfx->pd->port) ^ 0x7f;
			data2 = parport_read_control(tgfx->pd->port) ^ 0x04;	/* CAVEAT parport */

			input_report_abs(dev, ABS_X, !!(data1 & TGFX_RIGHT) - !!(data1 & TGFX_LEFT));
			input_report_abs(dev, ABS_Y, !!(data1 & TGFX_DOWN ) - !!(data1 & TGFX_UP  ));

			input_report_key(dev, BTN_TRIGGER, (data1 & TGFX_TRIGGER));
			input_report_key(dev, BTN_THUMB,   (data2 & TGFX_THUMB  ));
			input_report_key(dev, BTN_THUMB2,  (data2 & TGFX_THUMB2 ));
			input_report_key(dev, BTN_TOP,     (data2 & TGFX_TOP    ));
			input_report_key(dev, BTN_TOP2,    (data2 & TGFX_TOP2   ));

			input_sync(dev);
		}

	mod_timer(&tgfx->timer, jiffies + TGFX_REFRESH_TIME);
}

static int tgfx_open(struct input_dev *dev)
{
	struct tgfx *tgfx = input_get_drvdata(dev);
	int err;

	err = mutex_lock_interruptible(&tgfx->sem);
	if (err)
		return err;

	if (!tgfx->used++) {
		parport_claim(tgfx->pd);
		parport_write_control(tgfx->pd->port, 0x04);
		mod_timer(&tgfx->timer, jiffies + TGFX_REFRESH_TIME);
	}

	mutex_unlock(&tgfx->sem);
	return 0;
}

static void tgfx_close(struct input_dev *dev)
{
	struct tgfx *tgfx = input_get_drvdata(dev);

	mutex_lock(&tgfx->sem);
	if (!--tgfx->used) {
		del_timer_sync(&tgfx->timer);
		parport_write_control(tgfx->pd->port, 0x00);
		parport_release(tgfx->pd);
	}
	mutex_unlock(&tgfx->sem);
}



/*
 * tgfx_probe() probes for tg gamepads.
 */

static struct tgfx __init *tgfx_probe(int parport, int *n_buttons, int n_devs)
{
	struct tgfx *tgfx;
	struct input_dev *input_dev;
	struct parport *pp;
	struct pardevice *pd;
	int i, j;
	int err;

	pp = parport_find_number(parport);
	if (!pp) {
		printk(KERN_ERR "turbografx.c: no such parport\n");
		err = -EINVAL;
		goto err_out;
	}

	pd = parport_register_device(pp, "turbografx", NULL, NULL, NULL, PARPORT_DEV_EXCL, NULL);
	if (!pd) {
		printk(KERN_ERR "turbografx.c: parport busy already - lp.o loaded?\n");
		err = -EBUSY;
		goto err_put_pp;
	}

	tgfx = kzalloc(sizeof(struct tgfx), GFP_KERNEL);
	if (!tgfx) {
		printk(KERN_ERR "turbografx.c: Not enough memory\n");
		err = -ENOMEM;
		goto err_unreg_pardev;
	}

	mutex_init(&tgfx->sem);
	tgfx->pd = pd;
	init_timer(&tgfx->timer);
	tgfx->timer.data = (long) tgfx;
	tgfx->timer.function = tgfx_timer;

	for (i = 0; i < n_devs; i++) {
		if (n_buttons[i] < 1)
			continue;

		if (n_buttons[i] > 6) {
			printk(KERN_ERR "turbografx.c: Invalid number of buttons %d\n", n_buttons[i]);
			err = -EINVAL;
			goto err_unreg_devs;
		}

		tgfx->dev[i] = input_dev = input_allocate_device();
		if (!input_dev) {
			printk(KERN_ERR "turbografx.c: Not enough memory for input device\n");
			err = -ENOMEM;
			goto err_unreg_devs;
		}

		tgfx->sticks |= (1 << i);
		snprintf(tgfx->name[i], sizeof(tgfx->name[i]),
			 "TurboGraFX %d-button Multisystem joystick", n_buttons[i]);
		snprintf(tgfx->phys[i], sizeof(tgfx->phys[i]),
			 "%s/input%d", tgfx->pd->port->name, i);

		input_dev->name = tgfx->name[i];
		input_dev->phys = tgfx->phys[i];
		input_dev->id.bustype = BUS_PARPORT;
		input_dev->id.vendor = 0x0003;
		input_dev->id.product = n_buttons[i];
		input_dev->id.version = 0x0100;

		input_set_drvdata(input_dev, tgfx);

		input_dev->open = tgfx_open;
		input_dev->close = tgfx_close;

		input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
		input_set_abs_params(input_dev, ABS_X, -1, 1, 0, 0);
		input_set_abs_params(input_dev, ABS_Y, -1, 1, 0, 0);

		for (j = 0; j < n_buttons[i]; j++)
			set_bit(tgfx_buttons[j], input_dev->keybit);

		err = input_register_device(tgfx->dev[i]);
		if (err)
			goto err_free_dev;
	}

        if (!tgfx->sticks) {
		printk(KERN_ERR "turbografx.c: No valid devices specified\n");
		err = -EINVAL;
		goto err_free_tgfx;
        }

	return tgfx;

 err_free_dev:
	input_free_device(tgfx->dev[i]);
 err_unreg_devs:
	while (--i >= 0)
		if (tgfx->dev[i])
			input_unregister_device(tgfx->dev[i]);
 err_free_tgfx:
	kfree(tgfx);
 err_unreg_pardev:
	parport_unregister_device(pd);
 err_put_pp:
	parport_put_port(pp);
 err_out:
	return ERR_PTR(err);
}

static void tgfx_remove(struct tgfx *tgfx)
{
	int i;

	for (i = 0; i < TGFX_MAX_DEVICES; i++)
		if (tgfx->dev[i])
			input_unregister_device(tgfx->dev[i]);
	parport_unregister_device(tgfx->pd);
	kfree(tgfx);
}

static int __init tgfx_init(void)
{
	int i;
	int have_dev = 0;
	int err = 0;

	for (i = 0; i < TGFX_MAX_PORTS; i++) {
		if (tgfx_cfg[i].nargs == 0 || tgfx_cfg[i].args[0] < 0)
			continue;

		if (tgfx_cfg[i].nargs < 2) {
			printk(KERN_ERR "turbografx.c: at least one joystick must be specified\n");
			err = -EINVAL;
			break;
		}

		tgfx_base[i] = tgfx_probe(tgfx_cfg[i].args[0],
					  tgfx_cfg[i].args + 1,
					  tgfx_cfg[i].nargs - 1);
		if (IS_ERR(tgfx_base[i])) {
			err = PTR_ERR(tgfx_base[i]);
			break;
		}

		have_dev = 1;
	}

	if (err) {
		while (--i >= 0)
			if (tgfx_base[i])
				tgfx_remove(tgfx_base[i]);
		return err;
	}

	return have_dev ? 0 : -ENODEV;
}

static void __exit tgfx_exit(void)
{
	int i;

	for (i = 0; i < TGFX_MAX_PORTS; i++)
		if (tgfx_base[i])
			tgfx_remove(tgfx_base[i]);
}

module_init(tgfx_init);
module_exit(tgfx_exit);
                                                                                                                                #ifndef __RIVAFB_H
#define __RIVAFB_H

#include <linux/fb.h>
#include <video/vga.h>
#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>

#include "riva_hw.h"

/* GGI compatibility macros */
#define NUM_SEQ_REGS		0x05
#define NUM_CRT_REGS		0x41
#define NUM_GRC_REGS		0x09
#define NUM_ATC_REGS		0x15

/* I2C */
#define DDC_SCL_READ_MASK       (1 << 2)
#define DDC_SCL_WRITE_MASK      (1 << 5)
#define DDC_SDA_READ_MASK       (1 << 3)
#define DDC_SDA_WRITE_MASK      (1 << 4)

/* holds the state of the VGA core and extended Riva hw state from riva_hw.c.
 * From KGI originally. */
struct riva_regs {
	u8 attr[NUM_ATC_REGS];
	u8 crtc[NUM_CRT_REGS];
	u8 gra[NUM_GRC_REGS];
	u8 seq[NUM_SEQ_REGS];
	u8 misc_output;
	RIVA_HW_STATE ext;
};

struct riva_par;

struct riva_i2c_chan {
	struct riva_par *par;
	unsigned long   ddc_base;
	struct i2c_adapter adapter;
	struct i2c_algo_bit_data algo;
};

struct riva_par {
	RIVA_HW_INST riva;	/* interface to riva_hw.c */
	u32 pseudo_palette[16]; /* default palette */
	u32 palette[16];        /* for Riva128 */
	u8 __iomem *ctrl_base;	/* virtual control register base addr */
	unsigned dclk_max;	/* max DCLK */

	struct riva_regs initial_state;	/* initial startup video mode */
	struct riva_regs current_state;
#ifdef CONFIG_X86
	struct vgastate state;
#endif
	struct mutex open_lock;
	unsigned int ref_count;
	unsigned char *EDID;
	unsigned int Chipset;
	int forceCRTC;
	Bool SecondCRTC;
	int FlatPanel;
	struct pci_dev *pdev;
	int cursor_reset;
#ifdef CONFIG_MTRR
	struct { int vram; int vram_valid; } mtrr;
#endif
	struct riva_i2c_chan chan[3];
};

void riva_common_setup(struct riva_par *);
unsigned long riva_get_memlen(struct riva_par *);
unsigned long riva_get_maxdclk(struct riva_par *);
void riva_delete_i2c_busses(struct riva_par *par);
void riva_create_i2c_busses(struct riva_par *par);
int riva_probe_i2c_connector(struct riva_par *par, int conn, u8 **out_edid);

#endif /* __RIVAFB_H */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              /*
 * QLogic Fibre Channel HBA Driver
 * Copyright (c)  2003-2008 QLogic Corporation
 *
 * See LICENSE.qla2xxx for copyright and licensing details.
 */
#include "qla_def.h"

#include <linux/delay.h>
#include <scsi/scsi_tcq.h>

static void qla2x00_mbx_completion(scsi_qla_host_t *, uint16_t);
static void qla2x00_process_completed_request(struct scsi_qla_host *,
	struct req_que *, uint32_t);
static void qla2x00_status_entry(scsi_qla_host_t *, struct rsp_que *, void *);
static void qla2x00_status_cont_entry(struct rsp_que *, sts_cont_entry_t *);
static void qla2x00_error_entry(scsi_qla_host_t *, struct rsp_que *,
	sts_entry_t *);

/**
 * qla2100_intr_handler() - Process interrupts for the ISP2100 and ISP2200.
 * @irq:
 * @dev_id: SCSI driver HA context
 *
 * Called by system whenever the host adapter generates an interrupt.
 *
 * Returns handled flag.
 */
irqreturn_t
qla2100_intr_handler(int irq, void *dev_id)
{
	scsi_qla_host_t	*vha;
	struct qla_hw_data *ha;
	struct device_reg_2xxx __iomem *reg;
	int		status;
	unsigned long	iter;
	uint16_t	hccr;
	uint16_t	mb[4];
	struct rsp_que *rsp;
	unsigned long	flags;

	rsp = (struct rsp_que *) dev_id;
	if (!rsp) {
		printk(KERN_INFO
		    "%s(): NULL response queue pointer\n", __func__);
		return (IRQ_NONE);
	}

	ha = rsp->hw;
	reg = &ha->iobase->isp;
	status = 0;

	spin_lock_irqsave(&ha->hardware_lock, flags);
	vha = pci_get_drvdata(ha->pdev);
	for (iter = 50; iter--; ) {
		hccr = RD_REG_WORD(&reg->hccr);
		if (hccr & HCCR_RISC_PAUSE) {
			if (pci_channel_offline(ha->pdev))
				break;

			/*
			 * Issue a "HARD" reset in order for the RISC interrupt
			 * bit to be cleared.  Schedule a big hammmer to get
			 * out of the RISC PAUSED state.
			 */
			WRT_REG_WORD(&reg->hccr, HCCR_RESET_RISC);
			RD_REG_WORD(&reg->hccr);

			ha->isp_ops->fw_dump(vha, 1);
			set_bit(ISP_ABORT_NEEDED, &vha->dpc_flags);
			break;
		} else if ((RD_REG_WORD(&reg->istatus) & ISR_RISC_INT) == 0)
			break;

		if (RD_REG_WORD(&reg->semaphore) & BIT_0) {
			WRT_REG_WORD(&reg->hccr, HCCR_CLR_RISC_INT);
			RD_REG_WORD(&reg->hccr);

			/* Get mailbox data. */
			mb[0] = RD_MAILBOX_REG(ha, reg, 0);
			if (mb[0] > 0x3fff && mb[0] < 0x8000) {
				qla2x00_mbx_completion(vha, mb[0]);
				status |= MBX_INTERRUPT;
			} else if (mb[0] > 0x7fff && mb[0] < 0xc000) {
				mb[1] = RD_MAILBOX_REG(ha, reg, 1);
				mb[2] = RD_MAILBOX_REG(ha, reg, 2);
				mb[3] = RD_MAILBOX_REG(ha, reg, 3);
				qla2x00_async_event(vha, rsp, mb);
			} else {
				/*EMPTY*/
				DEBUG2(printk("scsi(%ld): Unrecognized "
				    "interrupt type (%d).\n",
				    vha->host_no, mb[