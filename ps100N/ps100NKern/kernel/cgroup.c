
{
	reg0 |= state->reg0 & 0x63;

	tda10023_writereg (state, 0x00, reg0 & 0xfe);
	tda10023_writereg (state, 0x00, reg0 | 0x01);

	state->reg0 = reg0;
	return 0;
}

static int tda10023_set_symbolrate (struct tda10023_state* state, u32 sr)
{
	s32 BDR;
	s32 BDRI;
	s16 SFIL=0;
	u16 NDEC = 0;

	/* avoid floating point operations multiplying syscloc and divider
	   by 10 */
	u32 sysclk_x_10 = state->sysclk * 10;

	if (sr < (u32)(sysclk_x_10/984)) {
		NDEC=3;
		SFIL=1;
	} else if (sr < (u32)(sysclk_x_10/640)) {
		NDEC=3;
		SFIL=0;
	} else if (sr < (u32)(sysclk_x_10/492)) {
		NDEC=2;
		SFIL=1;
	} else if (sr < (u32)(sysclk_x_10/320)) {
		NDEC=2;
		SFIL=0;
	} else if (sr < (u32)(sysclk_x_10/246)) {
		NDEC=1;
		SFIL=1;
	} else if (sr < (u32)(sysclk_x_10/160)) {
		NDEC=1;
		SFIL=0;
	} else if (sr < (u32)(sysclk_x_10/123)) {
		NDEC=0;
		SFIL=1;
	}

	BDRI = (state->sysclk)*16;
	BDRI>>=NDEC;
	BDRI +=sr/2;
	BDRI /=sr;

	if (BDRI>255)
		BDRI=255;

	{
		u64 BDRX;

		BDRX=1<<(24+NDEC);
		BDRX*=sr;
		do_div(BDRX, state->sysclk); 	/* BDRX/=SYSCLK; */

		BDR=(s32)BDRX;
	}
	dprintk("Symbolrate %i, BDR %i BDRI %i, NDEC %i\n",
		sr, BDR, BDRI, NDEC);
	tda10023_writebit (state, 0x03, 0xc0, NDEC<<6);
	tda10023_writereg (state, 0x0a, BDR&255);
	tda10023_writereg (state, 0x0b, (BDR>>8)&255);
	tda10023_writereg (state, 0x0c, (BDR>>16)&31);
	tda10023_writereg (state, 0x0d, BDRI);
	tda10023_writereg (state, 0x3d, (SFIL<<7));
	return 0;
}

static int tda10023_init (struct dvb_frontend *fe)
{
	struct tda10023_state* state = fe->demodulator_priv;
	u8 tda10023_inittab[] = {
/*        reg  mask val */
/* 000 */ 0x2a, 0xff, 0x02,  /* PLL3, Bypass, Power Down */
/* 003 */ 0xff, 0x64, 0x00,  /* Sleep 100ms */
/* 006 */ 0x2a, 0xff, 0x03,  /* PLL3, Bypass, Power Down */
/* 009 */ 0xff, 0x64, 0x00,  /* Sleep 100ms */
			   /* PLL1 */
/* 012 */ 0x28, 0xff, (state->pll_m-1),
			   /* PLL2 */
/* 015 */ 0x29, 0xff, ((state->pll_p-1)<<6)|(state->pll_n-1),
			   /* GPR FSAMPLING=1 */
/* 018 */ 0x00, 0xff, REG0_INIT_VAL,
/* 021 */ 0x2a, 0xff, 0x08,  /* PLL3 PSACLK=1 */
/* 024 */ 0xff, 0x64, 0x00,  /* Sleep 100ms */
/* 027 */ 0x1f, 0xff, 0x00,  /* RESET */
/* 030 */ 0xff, 0x64, 0x00,  /* Sleep 100ms */
/* 033 */ 0xe6, 0x0c, 0x04,  /* RSCFG_IND */
/* 036 */ 0x10, 0xc0, 0x80,  /* DECDVBCFG1 PBER=1 */

/* 039 */ 0x0e, 0xff, 0x82,  /* GAIN1 */
/* 042 */ 0x03, 0x08, 0x08,  /* CLKCONF DYN=1 */
/* 045 */ 0x2e, 0xbf, 0x30,  /* AGCCONF2 TRIAGC=0,POSAGC=ENAGCIF=1
				       PPWMTUN=0 PPWMIF=0 */
/* 048 */ 0x01, 0xff, 0x30,  /* AGCREF */
/* 051 */ 0x1e, 0x84, 0x84,  /* CONTROL SACLK_ON=1 */
/* 054 */ 0x1b, 0xff, 0xc8,  /* ADC TWOS=1 */
/* 057 */ 0x3b, 0xff, 0xff,  /* IFMAX */
/* 060 */ 0x3c, 0xff, 0x00,  /* IFMIN */
/* 063 */ 0x34, 0xff, 0x00,  /* PWMREF */
/* 066 */ 0x35, 0xff, 0xff,  /* TUNMAX */
/* 069 */ 0x36, 0xff, 0x00,  /* TUNMIN */
/* 072 */ 0x06, 0xff, 0x7f,  /* EQCONF1 POSI=7 ENADAPT=ENEQUAL=DFE=1 */
/* 075 */ 0x1c, 0x30, 0x30,  /* EQCONF2 STEPALGO=SGNALGO=1 */
/* 078 */ 0x37, 0xff, 0xf6,  /* DELTAF_LSB */
/* 081 */ 0x38, 0xff, 0xff,  /* DELTAF_MSB */
/* 084 */ 0x02, 0xff, 0x93,  /* AGCCONF1  IFS=1 KAGCIF=2 KAGCTUN=3 */
/* 087 */ 0x2d, 0xff, 0xf6,  /* SWEEP SWPOS=1 SWDYN=7 SWSTEP=1 SWLEN=2 */
/* 090 */ 0x04, 0x10, 0x00,  /* SWRAMP=1 */
/* 093 */ 0x12, 0xff, TDA10023_OUTPUT_MODE_PARALLEL_B, /*
				INTP1 POCLKP=1 FEL=1 MFS=0 */
/* 096 */ 0x2b, 0x01, 0xa1,  /* INTS1 */
/* 099 */ 0x20, 0xff, 0x04,  /* INTP2 SWAPP=? MSBFIRSTP=? INTPSEL=? */
/* 102 */ 0x2c, 0xff, 0x0d,  /* INTP/S TRIP=0 TRIS=0 */
/* 105 */ 0xc4, 0xff, 0x00,
/* 108 */ 0xc3, 0x30, 0x00,
/* 111 */ 0xb5, 0xff, 0x19,  /* ERAGC_THD */
/* 114 */ 0x00, 0x03, 0x01,  /* GPR, CLBS soft reset */
/* 117 */ 0x00, 0x03, 0x03,  /* GPR, CLBS soft reset */
/* 120 */ 0xff, 0x64, 0x00,  /* Sleep 100ms */
/* 123 */ 0xff, 0xff, 0xff
};
	dprintk("DVB: TDA10023(%d): init chip\n", fe->dvb->num);

	/* override default values if set in config */
	if (state->config->deltaf) {
		tda10023_inittab[80] = (state->config->deltaf & 0xff);
		tda10023_inittab[83] = (state->config->deltaf >> 8);
	}

	if (state->config->output_mode)
		tda10023_inittab[95] = state->config->output_mode;

	tda10023_writetab(state, tda10023_inittab);

	return 0;
}

static int tda10023_set_parameters (struct dvb_frontend *fe,
			    struct dvb_frontend_parameters *p)
{
	struct tda10023_state* state = fe->demodulator_priv;

	static int qamvals[6][6] = {
		//  QAM   LOCKTHR  MSETH   AREF AGCREFNYQ  ERAGCNYQ_THD
		{ (5<<2),  0x78,    0x8c,   0x96,   0x78,   0x4c  },  // 4 QAM
		{ (0<<2),  0x87,    0xa2,   0x91,   0x8c,   0x57  },  // 16 QAM
		{ (1<<2),  0x64,    0x74,   0x96,   0x8c,   0x57  },  // 32 QAM
		{ (2<<2),  0x46,    0x43,   0x6a,   0x6a,   0x44  },  // 64 QAM
		{ (3<<2),  0x36,    0x34,   0x7e,   0x78,   0x4c  },  // 128 QAM
		{ (4<<2),  0x26,    0x23,   0x6c,   0x5c,   0x3c  },  // 256 QAM
	};

	int qam = p->u.qam.modulation;

	if (qam < 0 || qam > 5)
		return -EINVAL;

	if (fe->ops.tuner_ops.set_params) {
		fe->ops.tuner_ops.set_params(fe, p);
		if (fe->ops.i2c_gate_ctrl) fe->ops.i2c_gate_ctrl(fe, 0);
	}

	tda10023_set_symbolrate (state, p->u.qam.symbol_rate);
	tda10023_writereg (state, 0x05, qamvals[qam][1]);
	tda10023_writereg (state, 0x08, qamvals[qam][2]);
	tda10023_writereg (state, 0x09, qamvals[qam][3]);
	tda10023_writereg (state, 0xb4, qamvals[qam][4]);
	tda10023_writereg (state, 0xb6, qamvals[qam][5]);

//	tda10023_writereg (state, 0x04, (p->inversion?0x12:0x32));
//	tda10023_writebit (state, 0x04, 0x60, (p->inversion?0:0x20));
	tda10023_writebit (state, 0x04, 0x40, 0x40);
	tda10023_setup_reg0 (state, qamvals[qam][0]);

	return 0;
}

static int tda10023_read_status(struct dvb_frontend* fe, fe_status_t* status)
{
	struct tda10023_state* state = fe->demodulator_priv;
	int sync;

	*status = 0;

	//0x11[1] == CARLOCK -> Carrier locked
	//0x11[2] == FSYNC -> Frame synchronisation
	//0x11[3] == FEL -> Front End locked
	//0x11[6] == NODVB -> DVB Mode Information
	sync = tda10023_readreg (state, 0x11);

	if (sync & 2)
		*status |= FE_HAS_SIGNAL|FE_HAS_CARRIER;

	if (sync & 4)
		*status |= FE_HAS_SYNC|FE_HAS_VITERBI;

	if (sync & 8)
		*status |= FE_HAS_LOCK;

	return 0;
}

static int tda10023_read_ber(struct dvb_frontend* fe, u32* ber)
{
	struct tda10023_state* state = fe->demodulator_priv;
	u8 a,b,c;
	a=tda10023_readreg(state, 0x14);
	b=tda10023_readreg(state, 0x15);
	c=tda10023_readreg(state, 0x16)&0xf;
	tda10023_writebit (state, 0x10, 0xc0, 0x00);

	*ber = a | (b<<8)| (c<<16);
	return 0;
}

static int tda10023_read_signal_strength(struct dvb_frontend* fe, u16* strength)
{
	struct tda10023_state* state = fe->demodulator_priv;
	u8 ifgain=tda10023_readreg(state, 0x2f);

	u16 gain = ((255-tda10023_readreg(state, 0x17))) + (255-ifgain)/16;
	// Max raw value is about 0xb0 -> Normalize to >0xf0 after 0x90
	if (gain>0x90)
		gain=gain+2*(gain-0x90);
	if (gain>255)
		gain=255;

	*strength = (gain<<8)|gain;
	return 0;
}

static int tda10023_read_snr(struct dvb_frontend* fe, u16* snr)
{
	struct tda10023_state* state = fe->demodulator_priv;

	u8 quality = ~tda10023_readreg(state, 0x18);
	*snr = (quality << 8) | quality;
	return 0;
}

static int tda10023_read_ucblocks(struct dvb_frontend* fe, u32* ucblocks)
{
	struct tda10023_state* state = fe->demodulator_priv;
	u8 a,b,c,d;
	a= tda10023_readreg (state, 0x74);
	b= tda10023_readreg (state, 0x75);
	c= tda10023_readreg (state, 0x76);
	d= tda10023_readreg (state, 0x77);
	*ucblocks = a | (b<<8)|(c<<16)|(d<<24);

	tda10023_writebit (state, 0x10, 0x20,0x00);
	tda10023_writebit (state, 0x10, 0x20,0x20);
	tda10023_writebit (state, 0x13, 0x01, 0x00);

	return 0;
}

static int tda10023_get_frontend(struct dvb_frontend* fe, struct dvb_frontend_parameters *p)
{
	struct tda10023_state* state = fe->demodulator_priv;
	int sync,inv;
	s8 afc = 0;

	sync = tda10023_readreg(state, 0x11);
	afc = tda10023_readreg(state, 0x19);
	inv = tda10023_readreg(state, 0x04);

	if (verbose) {
		/* AFC only valid when carrier has been recovered */
		printk(sync & 2 ? "DVB: TDA10023(%d): AFC (%d) %dHz\n" :
				  "DVB: TDA10023(%d): [AFC (%d) %dHz]\n",
			state->frontend.dvb->num, afc,
		       -((s32)p->u.qam.symbol_rate * afc) >> 10);
	}

	p->inversion = (inv&0x20?0:1);
	p->u.qam.modulation = ((state->reg0 >> 2) & 7) + QAM_16;

	p->u.qam.fec_inner = FEC_NONE;
	p->frequency = ((p->frequency + 31250) / 62500) * 62500;

	if (sync & 2)
		p->frequency -= ((s32)p->u.qam.symbol_rate * afc) >> 10;

	return 0;
}

static int tda10023_sleep(struct dvb_frontend* fe)
{
	struct tda10023_state* state = fe->demodulator_priv;

	tda10023_writereg (state, 0x1b, 0x02);  /* pdown ADC */
	tda10023_writereg (state, 0x00, 0x80);  /* standby */

	return 0;
}

static int tda10023_i2c_gate_ctrl(struct dvb_frontend* fe, int enable)
{
	struct tda10023_state* state = fe->demodulator_priv;

	if (enable) {
		lock_tuner(state);
	} else {
		unlock_tuner(state);
	}
	return 0;
}

static void tda10023_release(struct dvb_frontend* fe)
{
	struct tda10023_state* state = fe->demodulator_priv;
	kfree(state);
}

static struct dvb_frontend_ops tda10023_ops;

struct dvb_frontend *tda10023_attach(const struct tda10023_config *config,
				     struct i2c_adapter *i2c,
				     u8 pwm)
{
	struct tda10023_state* state = NULL;

	/* allocate memory for the internal state */
	state = kzalloc(sizeof(struct tda10023_state), GFP_KERNEL);
	if (state == NULL) goto error;

	/* setup the state */
	state->config = config;
	state->i2c = i2c;

	/* wakeup if in standby */
	tda10023_writereg (state, 0x00, 0x33);
	/* check if the demod is there */
	if ((tda10023_readreg(state, 0x1a) & 0xf0) != 0x70) goto error;

	/* create dvb_frontend */
	memcpy(&state->frontend.ops, &tda10023_ops, sizeof(struct dvb_frontend_ops));
	state->pwm = pwm;
	state->reg0 = REG0_INIT_VAL;
	if (state->config->xtal) {
		state->xtal  = state->config->xtal;
		state->pll_m = state->config->pll_m;
		state->pll_p = state->config->pll_p;
		state->pll_n = state->config->pll_n;
	} else {
		/* set default values if not defined in config */
		state->xtal  = 28920000;
		state->pll_m = 8;
		state->pll_p = 4;
		state->pll_n = 1;
	}

	/* calc sysclk */
	state->sysclk = (state->xtal * state->pll_m / \
			(state->pll_n * state->pll_p));

	state->frontend.ops.info.symbol_rate_min = (state->sysclk/2)/64;
	state->frontend.ops.info.symbol_rate_max = (state->sysclk/2)/4;

	dprintk("DVB: TDA10023 %s: xtal:%d pll_m:%d pll_p:%d pll_n:%d\n",
		__func__, state->xtal, state->pll_m, state->pll_p,
		state->pll_n);

	state->frontend.demodulator_priv = state;
	return &state->frontend;

error:
	kfree(state);
	return NULL;
}

static struct dvb_frontend_ops tda10023_ops = {

	.info = {
		.name = "Philips TDA10023 DVB-C",
		.type = FE_QAM,
		.frequency_stepsize = 62500,
		.frequency_min =  47000000,
		.frequency_max = 862000000,
		.symbol_rate_min = 0,  /* set in tda10023_attach */
		.symbol_rate_max = 0,  /* set in tda10023_attach */
		.caps = 0x400 | //FE_CAN_QAM_4
			FE_CAN_QAM_16 | FE_CAN_QAM_32 | FE_CAN_QAM_64 |
			FE_CAN_QAM_128 | FE_CAN_QAM_256 |
			FE_CAN_FEC_AUTO
	},

	.release = tda10023_release,

	.init = tda10023_init,
	.sleep = tda10023_sleep,
	.i2c_gate_ctrl = tda10023_i2c_gate_ctrl,

	.set_frontend = tda10023_set_parameters,
	.get_frontend = tda10023_get_frontend,

	.read_status = tda10023_read_status,
	.read_ber = tda10023_read_ber,
	.read_signal_strength = tda10023_read_signal_strength,
	.read_snr = tda10023_read_snr,
	.read_ucblocks = tda10023_read_ucblocks,
};


MODULE_DESCRIPTION("Philips TDA10023 DVB-C demodulator driver");
MODULE_AUTHOR("Georg Acher, Hartmut Birr");
MODULE_LICENSE("GPL");

EXPORT_SYMBOL(tda10023_attach);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           /*
    bt848.h - Bt848 register offsets

    Copyright (C) 1996,97,98 Ralph Metzler (rjkm@thp.uni-koeln.de)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _BT848_H_
#define _BT848_H_

#ifndef PCI_VENDOR_ID_BROOKTREE
#define PCI_VENDOR_ID_BROOKTREE 0x109e
#endif
#ifndef PCI_DEVICE_ID_BT848
#define PCI_DEVICE_ID_BT848     0x350
#endif
#ifndef PCI_DEVICE_ID_BT849
#define PCI_DEVICE_ID_BT849     0x351
#endif
#ifndef PCI_DEVICE_ID_BT878
#define PCI_DEVICE_ID_BT878     0x36e
#endif
#ifndef PCI_DEVICE_ID_BT879
#define PCI_DEVICE_ID_BT879     0x36f
#endif


/* Brooktree 848 registers */

#define BT848_DSTATUS          0x000
#define BT848_DSTATUS_PRES     (1<<7)
#define BT848_DSTATUS_HLOC     (1<<6)
#define BT848_DSTATUS_FIELD    (1<<5)
#define BT848_DSTATUS_NUML     (1<<4)
#define BT848_DSTATUS_CSEL     (1<<3)
#define BT848_DSTATUS_PLOCK    (1<<2)
#define BT848_DSTATUS_LOF      (1<<1)
#define BT848_DSTATUS_COF      (1<<0)

#define BT848_IFORM            0x004
#define BT848_IFORM_HACTIVE    (1<<7)
#define BT848_IFORM_MUXSEL     (3<<5)
#define BT848_IFORM_MUX0       (2<<5)
#define BT848_IFORM_MUX1       (3<<5)
#define BT848_IFORM_MUX2       (1<<5)
#define BT848_IFORM_XTSEL      (3<<3)
#define BT848_IFORM_XT0        (1<<3)
#define BT848_IFORM_XT1        (2<<3)
#define BT848_IFORM_XTAUTO     (3<<3)
#define BT848_IFORM_XTBOTH     (3<<3)
#define BT848_IFORM_NTSC       1
#define BT848_IFORM_NTSC_J     2
#define BT848_IFORM_PAL_BDGHI  3
#define BT848_IFORM_PAL_M      4
#define BT848_IFORM_PAL_N      5
#define BT848_IFORM_SECAM      6
#define BT848_IFORM_PAL_NC     7
#define BT848_IFORM_AUTO       0
#define BT848_IFORM_NORM       7

#define BT848_TDEC             0x008
#define BT848_TDEC_DEC_FIELD   (1<<7)
#define BT848_TDEC_FLDALIGN    (1<<6)
#define BT848_TDEC_DEC_RAT     (0x1f)

#define BT848_E_CROP           0x00C
#define BT848_O_CROP           0x08C

#define BT848_E_VDELAY_LO      0x010
#define BT848_O_VDELAY_LO      0x090

#define BT848_E_VACTIVE_LO     0x014
#define BT848_O_VACTIVE_LO     0x094

#define BT848_E_HDELAY_LO      0x018
#define BT848_O_HDELAY_LO      0x098

#define BT848_E_HACTIVE_LO     0x01C
#define BT848_O_HACTIVE_LO     0x09C

#define BT848_E_HSCALE_HI      0x020
#define BT848_O_HSCALE_HI      0x0A0

#define BT848_E_HSCALE_LO      0x024
#define BT848_O_HSCALE_LO      0x0A4

#define BT848_BRIGHT           0x028

#define BT848_E_CONTROL        0x02C
#define BT848_O_CONTROL        0x0AC
#define BT848_CONTROL_LNOTCH    (1<<7)
#define BT848_CONTROL_COMP      (1<<6)
#define BT848_CONTROL_LDEC      (1<<5)
#define BT848_CONTROL_CBSENSE   (1<<4)
#define BT848_CONTROL_CON_MSB   (1<<2)
#define BT848_CONTROL_SAT_U_MSB (1<<1)
#define BT848_CONTROL_SAT_V_MSB (1<<0)

#define BT848_CONTRAST_LO      0x030
#define BT848_SAT_U_LO         0x034
#define BT848_SAT_V_LO         0x038
#define BT848_HUE              0x03C

#define BT848_E_SCLOOP         0x040
#define BT848_O_SCLOOP         0x0C0
#define BT848_SCLOOP_CAGC       (1<<6)
#define BT848_SCLOOP_CKILL      (1<<5)
#define BT848_SCLOOP_HFILT_AUTO (0<<3)
#define BT848_SCLOOP_HFILT_CIF  (1<<3)
#define BT848_SCLOOP_HFILT_QCIF (2<<3)
#define BT848_SCLOOP_HFILT_ICON (3<<3)

#define BT848_SCLOOP_PEAK       (1<<7)
#define BT848_SCLOOP_HFILT_MINP (1<<3)
#define BT848_SCLOOP_HFILT_MEDP (2<<3)
#define BT848_SCLOOP_HFILT_MAXP (3<<3)


#define BT848_OFORM            0x048
#define BT848_OFORM_RANGE      (1<<7)
#define BT848_OFORM_CORE0   e
 * task->cgroup without the expense of grabbing a system global
 * mutex.  Therefore except as noted below, when dereferencing or, as
 * in cgroup_attach_task(), modifying a task'ss cgroup pointer we use
 * task_lock(), which acts on a spinlock (task->alloc_lock) already in
 * the task_struct routinely used for such matters.
 *
 * P.S.  One more locking exception.  RCU is used to guard the
 * update of a tasks cgroup pointer by cgroup_attach_task()
 */

/**
 * cgroup_lock - lock out any changes to cgroup structures
 *
 */
void cgroup_lock(void)
{
	mutex_lock(&cgroup_mutex);
}

/**
 * cgroup_unlock - release lock on cgroup changes
 *
 * Undo the lock taken in a previous cgroup_lock() call.
 */
void cgroup_unlock(void)
{
	mutex_unlock(&cgroup_mutex);
}

/*
 * A couple of forward declarations required, due to cyclic reference loop:
 * cgroup_mkdir -> cgroup_create -> cgroup_populate_dir ->
 * cgroup_add_file -> cgroup_create_file -> cgroup_dir_inode_operations
 * -> cgroup_mkdir.
 */

static int cgroup_mkdir(struct inode *dir, struct dentry *dentry, int mode);
static int cgroup_rmdir(struct inode *unused_dir, struct dentry *dentry);
static int cgroup_populate_dir(struct cgroup *cgrp);
static struct inode_operations cgroup_dir_inode_operations;
static struct file_operations proc_cgroupstats_operations;

static struct backing_dev_info cgroup_backing_dev_info = {
	.capabilities	= BDI_CAP_NO_ACCT_AND_WRITEBACK,
};

static int alloc_css_id(struct cgroup_subsys *ss,
			struct cgroup *parent, struct cgroup *child);

static struct inode *cgroup_new_inode(mode_t mode, struct super_block *sb)
{
	struct inode *inode = new_inode(sb);

	if (inode) {
		inode->i_mode = mode;
		inode->i_uid = current_fsuid();
		inode->i_gid = current_fsgid();
		inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
		inode->i_mapping->backing_dev_info = &cgroup_backing_dev_info;
	}
	return inode;
}

/*
 * Call subsys's pre_destroy handler.
 * This is called before css refcnt check.
 */
static int cgroup_call_pre_destroy(struct cgroup *cgrp)
{
	struct cgroup_subsys *ss;
	int ret = 0;

	for_each_subsys(cgrp->root, ss)
		if (ss->pre_destroy) {
			ret = ss->pre_destroy(ss, cgrp);
			if (ret)
				break;
		}
	return ret;
}

static void free_cgroup_rcu(struct rcu_head *obj)
{
	struct cgroup *cgrp = container_of(obj, struct cgroup, rcu_head);

	kfree(cgrp);
}

static void cgroup_diput(struct dentry *dentry, struct inode *inode)
{
	/* is dentry a directory ? if so, kfree() associated cgroup */
	if (S_ISDIR(inode->i_mode)) {
		struct cgroup *cgrp = dentry->d_fsdata;
		struct cgroup_subsys *ss;
		BUG_ON(!(cgroup_is_removed(cgrp)));
		/* It's possible for external users to be holding css
		 * reference counts on a cgroup; css_put() needs to
		 * be able to access the cgroup after decrementing
		 * the reference count in order to know if it needs to
		 * queue the cgroup to be handled by the release
		 * agent */
		synchronize_rcu();

		mutex_lock(&cgroup_mutex);
		/*
		 * Release the subsystem state objects.
		 */
		for_each_subsys(cgrp->root, ss)
			ss->destroy(ss, cgrp);

		cgrp->root->number_of_cgroups--;
		mutex_unlock(&cgroup_mutex);

		/*
		 * Drop the active superblock reference that we took when we
		 * created the cgroup
		 */
		deactivate_super(cgrp->root->sb);

		call_rcu(&cgrp->rcu_head, free_cgroup_rcu);
	}
	iput(inode);
}

static void remove_dir(struct dentry *d)
{
	struct dentry *parent = dget(d->d_parent);

	d_delete(d);
	simple_rmdir(parent->d_inode, d);
	dput(parent);
}

static void cgroup_clear_directory(struct dentry *dentry)
{
	struct list_head *node;

	BUG_ON(!mutex_is_locked(&dentry->d_inode->i_mutex));
	spin_lock(&dcache_lock);
	node = dentry->d_subdirs.next;
	while (node != &dentry->d_subdirs) {
		struct dentry *d = list_entry(node, struct dentry, d_u.d_child);
		list_del_init(node);
		if (d->d_inode) {
			/* This should never be called on a cgroup
			 * directory with child cgroups */
			BUG_ON(d->d_inode->i_mode & S_IFDIR);
			d = dget_locked(d);
			spin_unlock(&dcache_lock);
			d_delete(d);
			simple_unlink(dentry->d_inode, d);
INT_RISCI   (1<<11)
#define BT848_INT_GPINT   (1<<9)
#define BT848_INT_I2CDONE (1<<8)
#define BT848_INT_VPRES   (1<<5)
#define BT848_INT_HLOCK   (1<<4)
#define BT848_INT_OFLOW   (1<<3)
#define BT848_INT_HSYNC   (1<<2)
#define BT848_INT_VSYNC   (1<<1)
#define BT848_INT_FMTCHG  (1<<0)


#define BT848_GPIO_DMA_CTL             0x10C
#define BT848_GPIO_DMA_CTL_GPINTC      (1<<15)
#define BT848_GPIO_DMA_CTL_GPINTI      (1<<14)
#define BT848_GPIO_DMA_CTL_GPWEC       (1<<13)
#define BT848_GPIO_DMA_CTL_GPIOMODE    (3<<11)
#define BT848_GPIO_DMA_CTL_GPCLKMODE   (1<<10)
#define BT848_GPIO_DMA_CTL_PLTP23_4    (0<<6)
#define BT848_GPIO_DMA_CTL_PLTP23_8    (1<<6)
#define BT848_GPIO_DMA_CTL_PLTP23_16   (2<<6)
#define BT848_GPIO_DMA_CTL_PLTP23_32   (3<<6)
#define BT848_GPIO_DMA_CTL_PLTP1_4     (0<<4)
#define BT848_GPIO_DMA_CTL_PLTP1_8     (1<<4)
#define BT848_GPIO_DMA_CTL_PLTP1_16    (2<<4)
#define BT848_GPIO_DMA_CTL_PLTP1_32    (3<<4)
#define BT848_GPIO_DMA_CTL_PKTP_4      (0<<2)
#define BT848_GPIO_DMA_CTL_PKTP_8      (1<<2)
#define BT848_GPIO_DMA_CTL_PKTP_16     (2<<2)
#define BT848_GPIO_DMA_CTL_PKTP_32     (3<<2)
#define BT848_GPIO_DMA_CTL_RISC_ENABLE (1<<1)
#define BT848_GPIO_DMA_CTL_FIFO_ENABLE (1<<0)

#define BT848_I2C              0x110
#define BT878_I2C_MODE         (1<<7)
#define BT878_I2C_RATE         (1<<6)
#define BT878_I2C_NOSTOP       (1<<5)
#define BT878_I2C_NOSTART      (1<<4)
#define BT848_I2C_DIV          (0xf<<4)
#define BT848_I2C_SYNC         (1<<3)
#define BT848_I2C_W3B	       (1<<2)
#define BT848_I2C_SCL          (1<<1)
#define BT848_I2C_SDA          (1<<0)

#define BT848_RISC_STRT_ADD    0x114
#define BT848_GPIO_OUT_EN      0x118
#define BT848_GPIO_REG_INP     0x11C
#define BT848_RISC_COUNT       0x120
#define BT848_GPIO_DATA        0x200


/* Bt848 RISC commands */

/* only for the SYNC RISC command */
#define BT848_FIFO_STATUS_FM1  0x06
#define BT848_FIFO_STATUS_FM3  0x0e
#define BT848_FIFO_STATUS_SOL  0x02
#define BT848_FIFO_STATUS_EOL4 0x01
#define BT848_FIFO_STATUS_EOL3 0x0d
#define BT848_FIFO_STATUS_EOL2 0x09
#define BT848_FIFO_STATUS_EOL1 0x05
#define BT848_FIFO_STATUS_VRE  0x04
#define BT848_FIFO_STATUS_VRO  0x0c
#define BT848_FIFO_STATUS_PXV  0x00

#define BT848_RISC_RESYNC      (1<<15)

/* WRITE and SKIP */
/* disable which bytes of each DWORD */
#define BT848_RISC_BYTE0       (1U<<12)
#define BT848_RISC_BYTE1       (1U<<13)
#define BT848_RISC_BYTE2       (1U<<14)
#define BT848_RISC_BYTE3       (1U<<15)
#define BT848_RISC_BYTE_ALL    (0x0fU<<12)
#define BT848_RISC_BYTE_NONE   0
/* cause RISCI */
#define BT848_RISC_IRQ         (1U<<24)
/* RISC command is last one in this line */
#define BT848_RISC_EOL         (1U<<26)
/* RISC command is first one in this line */
#define BT848_RISC_SOL         (1U<<27)

#define BT848_RISC_WRITE       (0x01U<<28)
#define BT848_RISC_SKIP        (0x02U<<28)
#define BT848_RISC_WRITEC      (0x05U<<28)
#define BT848_RISC_JUMP        (0x07U<<28)
#define BT848_RISC_SYNC        (0x08U<<28)

#define BT848_RISC_WRITE123    (0x09U<<28)
#define BT848_RISC_SKIP123     (0x0aU<<28)
#define BT848_RISC_WRITE1S23   (0x0bU<<28)


/* Bt848A and higher only !! */
#define BT848_TGLB             0x080
#define BT848_TGCTRL           0x084
#define BT848_FCAP             0x0E8
#define BT848_PLL_F_LO         0x0F0
#define BT848_PLL_F_HI         0x0F4

#define BT848_PLL_XCI          0x0F8
#define BT848_PLL_X            (1<<7)
#define BT848_PLL_C            (1<<6)

#define BT848_DVSIF            0x0FC

/* Bt878 register */

#define BT878_DEVCTRL 0x40
#define BT878_EN_TBFX 0x02
#define BT878_EN_VSFX 0x04

#endif
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    = (unsigned long)-1;

#ifdef CONFIG_CPUSETS
	mask = ~(1UL << cpuset_subsys_id);
#endif

	opts->subsys_bits = 0;
	opts->flags = 0;
	opts->release_agent = NULL;

	while ((token = strsep(&o, ",")) != NULL) {
		if (!*token)
			return -EINVAL;
		if (!strcmp(token, "all")) {
			/* Add all non-disabled subsystems */
			int i;
			opts->subsys_bits = 0;
			for (i = 0; i < CGROUP_SUBSYS_COUNT; i++) {
				struct cgroup_subsys *ss = subsys[i];
				if (!ss->disabled)
					opts->subsys_bits |= 1ul << i;
			}
		} else if (!strcmp(token, "noprefix")) {
			set_bit(ROOT_NOPREFIX, &opts->flags);
		} else if (!strncmp(token, "release_agent=", 14)) {
			/* Specifying two release agents is forbidden */
			if (opts->release_agent)
				return -EINVAL;
			opts->release_agent = kzalloc(PATH_MAX, GFP_KERNEL);
			if (!opts->release_agent)
				return -ENOMEM;
			strncpy(opts->release_agent, token + 14, PATH_MAX - 1);
			opts->release_agent[PATH_MAX - 1] = 0;
		} else {
			struct cgroup_subsys *ss;
			int i;
			for (i = 0; i < CGROUP_SUBSYS_COUNT; i++) {
				ss = subsys[i];
				if (!strcmp(token, ss->name)) {
					if (!ss->disabled)
						set_bit(i, &opts->subsys_bits);
					break;
				}
			}
			if (i == CGROUP_SUBSYS_COUNT)
				return -ENOENT;
		}
	}

	/*
	 * Option noprefix was introduced just for backward compatibility
	 * with the old cpuset, so we allow noprefix only if mounting just
	 * the cpuset subsystem.
	 */
	if (test_bit(ROOT_NOPREFIX, &opts->flags) &&
	    (opts->subsys_bits & mask))
		return -EINVAL;

	/* We can't have an empty hierarchy */
	if (!opts->subsys_bits)
		return -EINVAL;

	return 0;
}

static int cgroup_remount(struct super_block *sb, int *flags, char *data)
{
	int ret = 0;
	struct cgroupfs_root *root = sb->s_fs_info;
	struct cgroup *cgrp = &root->top_cgroup;
	struct cgroup_sb_opts opts;

	lock_kernel();
	mutex_lock(&cgrp->dentry->d_inode->i_mutex);
	mutex_lock(&cgroup_mutex);

	/* See what subsystems are wanted */
	ret = parse_cgroupfs_options(data, &opts);
	if (ret)
		goto out_unlock;

	/* Don't allow flags to change at remount */
	if (opts.flags != root->flags) {
		ret = -EINVAL;
		goto out_unlock;
	}

	ret = rebind_subsystems(root, opts.subsys_bits);
	if (ret)
		goto out_unlock;

	/* (re)populate subsystem files */
	cgroup_populate_dir(cgrp);

	if (opts.release_agent)
		strcpy(root->release_agent_path, opts.release_agent);
 out_unlock:
	kfree(opts.release_agent);
	mutex_unlock(&cgroup_mutex);
	mutex_unlock(&cgrp->dentry->d_inode->i_mutex);
	unlock_kernel();
	return ret;
}

static struct super_operations cgroup_ops = {
	.statfs = simple_statfs,
	.drop_inode = generic_delete_inode,
	.show_options = cgroup_show_options,
	.remount_fs = cgroup_remount,
};

static void init_cgroup_housekeeping(struct cgroup *cgrp)
{
	INIT_LIST_HEAD(&cgrp->sibling);
	INIT_LIST_HEAD(&cgrp->children);
	INIT_LIST_HEAD(&cgrp->css_sets);
	INIT_LIST_HEAD(&cgrp->release_list);
	INIT_LIST_HEAD(&cgrp->pids_list);
	init_rwsem(&cgrp->pids_mutex);
}
static void init_cgroup_root(struct cgroupfs_root *root)
{
	struct cgroup *cgrp = &root->top_cgroup;
	INIT_LIST_HEAD(&root->subsys_list);
	INIT_LIST_HEAD(&root->root_list);
	root->number_of_cgroups = 1;
	cgrp->root = root;
	cgrp->top_cgroup = cgrp;
	init_cgroup_housekeeping(cgrp);
}

static int cgroup_test_super(struct super_block *sb, void *data)
{
	struct cgroupfs_root *new = data;
	struct cgroupfs_root *root = sb->s_fs_info;

	/* First check subsystems */
	if (new->subsys_bits != root->subsys_bits)
	    return 0;

	/* Next check flags */
	if (new->flags != root->flags)
		return 0;

	return 1;
}

static int cgroup_set_super(struct super_block *sb, void *data)
{
	int ret;
	struct cgroupfs_root *root = data;

	ret = set_anon_super(sb, NULL);
	if (ret)
		return ret;

	sb->s_fs_info = root;
	root->sb = sb;

	sb->s_blocksize = PAGE_CACHE_SIZE;
	sb->s_blocksize_bits = PAGE_CACHE_SHIFT;
	sb->s_magic = CGROUP_SUPER_MAGIC;
	sb->s_op = &cgroup_ops;

	return 0;
}

static int cgroup_get_rootdir(struct super_block *sb)
{
	struct inode *inode =
		cgroup_new_inode(S_IFDIR | S_IRUGO | S_IXUGO | S_IWUSR, sb);
	struct dentry *dentry;

	if (!inode)
		return -ENOMEM;

	inode->i_fop = &simple_dir_operations;
	inode->i_op = &cgroup_dir_inode_operations;
	/* directories start off with i_nlink == 2 (for "." entry) */
	inc_nlink(inode);
	dentry = d_alloc_root(inode);
	if (!dentry) {
		iput(inode);
		return -ENOMEM;
	}
	sb->s_root = dentry;
	return 0;
}

static int cgroup_get_sb(struct file_system_type *fs_type,
			 int flags, const char *unused_dev_name,
			 void *data, struct vfsmount *mnt)
{
	struct cgroup_sb_opts opts;
	int ret = 0;
	struct super_block *sb;
	struct cgroupfs_root *root;
	struct list_head tmp_cg_links;

	/* First find the desired set of subsystems */
	ret = parse_cgroupfs_options(data, &opts);
	if (ret) {
		kfree(opts.release_agent);
		return ret;
	}

	root = kzalloc(sizeof(*root), GFP_KERNEL);
	if (!root) {
		kfree(opts.release_agent);
		return -ENOMEM;
	}

	init_cgroup_root(root);
	root->subsys_bits = opts.subsys_bits;
	root->flags = opts.flags;
	if (opts.release_agent) {
		strcpy(root->release_agent_path, opts.release_agent);
		kfree(opts.release_agent);
	}

	sb = sget(fs_type, cgroup_test_super, cgroup_set_super, root);

	if (IS_ERR(sb)) {
		kfree(root);
		return PTR_ERR(sb);
	}

	if (sb->s_fs_info != root) {
		/* Reusing an existing superblock */
		BUG_ON(sb->s_root == NULL);
		kfree(root);
		root = NULL;
	} else {
		/* New superblock */
		struct cgroup *root_cgrp = &root->top_cgroup;
		struct inode *inode;
		int i;

		BUG_ON(sb->s_root != NULL);

		ret = cgroup_get_rootdir(sb);
		if (ret)
			goto drop_new_super;
		inode = sb->s_root->d_inode;

		mutex_lock(&inode->i_mutex);
		mutex_lock(&cgroup_mutex);

		/*
		 * We're accessing css_set_count without locking
		 * css_set_lock here, but that's OK - it can only be
		 * increased by someone holding cgroup_lock, and
		 * that's us. The worst that can happen is that we
		 * have some link structures left over
		 */
		ret = allocate_cg_links(css_set_count, &tmp_cg_links);
		if (ret) {
			mutex_unlock(&cgroup_mutex);
			mutex_unlock(&inode->i_mutex);
			goto drop_new_super;
		}

		ret = rebind_subsystems(root, root->subsys_bits);
		if (ret == -EBUSY) {
			mutex_unlock(&cgroup_mutex);
			mutex_unlock(&inode->i_mutex);
			goto free_cg_links;
		}

		/* EBUSY should be the only error here */
		BUG_ON(ret);

		list_add(&root->root_list, &roots);
		root_count++;

		sb->s_root->d_fsdata = root_cgrp;
		root->top_cgroup.dentry = sb->s_root;

		/* Link the top cgroup in this hierarchy into all
		 * the css_set objects */
		write_lock(&css_set_lock);
		for (i = 0; i < CSS_SET_TABLE_SIZE; i++) {
			struct hlist_head *hhead = &css_set_table[i];
			struct hlist_node *node;
			struct css_set *cg;

			hlist_for_each_entry(cg, node, hhead, hlist)
				link_css_set(&tmp_cg_links, cg, root_cgrp);
		}
		write_unlock(&css_set_lock);

		free_cg_links(&tmp_cg_links);

		BUG_ON(!list_empty(&root_cgrp->sibling));
		BUG_ON(!list_empty(&root_cgrp->children));
		BUG_ON(root->number_of_cgroups != 1);

		cgroup_populate_dir(root_cgrp);
		mutex_unlock(&inode->i_mutex);
		mutex_unlock(&cgroup_mutex);
	}

	simple_set_mnt(mnt, sb);
	return 0;

 free_cg_links:
	free_cg_links(&tmp_cg_links);
 drop_new_super:
	deactivate_locked_super(sb);
	return ret;
}

static void cgroup_kill_sb(struct super_block *sb) {
	struct cgroupfs_root *root = sb->s_fs_info;
	struct cgroup *cgrp = &root->top_cgroup;
	int ret;
	struct cg_cgroup_link *link;
	struct cg_cgroup_link *saved_link;

	BUG_ON(!root);

	BUG_ON(root->number_of_cgroups != 1);
	BUG_ON(!list_empty(&cgrp->children));
	BUG_ON(!list_empty(&cgrp->sibling));

	mutex_lock(&cgroup_mutex);

	/* Rebind all subsystems back to the default hierarchy */
	ret = rebind_subsystems(root, 0);
	/* Shouldn't be able to fail ... */
	BUG_ON(ret);

	/*
	 * Release all the links from css_sets to this hierarchy's
	 * root cgroup
	 */
	write_lock(&css_set_lock);

	list_for_each_entry_safe(link, saved_link, &cgrp->css_sets,
				 cgrp_link_list) {
		list_del(&link->cg_link_list);
		list_del(&link->cgrp_link_list);
		kfree(link);
	}
	write_unlock(&css_set_lock);

	if (!list_empty(&root->root_list)) {
		list_del(&root->root_list);
		root_count--;
	}

	mutex_unlock(&cgroup_mutex);

	kill_litter_super(sb);
	kfree(root);
}

static struct file_system_type cgroup_fs_type = {
	.name = "cgroup",
	.get_sb = cgroup_get_sb,
	.kill_sb = cgroup_kill_sb,
};

static inline struct cgroup *__d_cgrp(struct dentry *dentry)
{
	return dentry->d_fsdata;
}

static inline struct cftype *__d_cft(struct dentry *dentry)
{
	return dentry->d_fsdata;
}

/**
 * cgroup_path - generate the path of a cgroup
 * @cgrp: the cgroup in question
 * @buf: the buffer to write the path into
 * @buflen: the length of the buffer
 *
 * Called with cgroup_mutex held or else with an RCU-protected cgroup
 * reference.  Writes path of cgroup into buf.  Returns 0 on success,
 * -errno on error.
 */
int cgroup_path(const struct cgroup *cgrp, char *buf, int buflen)
{
	char *start;
	struct dentry *dentry = rcu_dereference(cgrp->dentry);

	if (!dentry || cgrp == dummytop) {
		/*
		 * Inactive subsystems have no dentry for their root
		 * cgroup
		 */
		strcpy(buf, "/");
		return 0;
	}

	start = buf + buflen;

	*--start = '\0';
	for (;;) {
		int len = dentry->d_name.len;
		if ((start -= len) < buf)
			return -ENAMETOOLONG;
		memcpy(start, cgrp->dentry->d_name.name, len);
		cgrp = cgrp->parent;
		if (!cgrp)
			break;
		dentry = rcu_dereference(cgrp->dentry);
		if (!cgrp->parent)
			continue;
		if (--start < buf)
			return -ENAMETOOLONG;
		*start = '/';
	}
	memmove(buf, start, buf + buflen - start);
	return 0;
}

/*
 * Return the first subsystem attached to a cgroup's hierarchy, and
 * its subsystem id.
 */

static void get_first_subsys(const struct cgroup *cgrp,
			struct cgroup_subsys_state **css, int *subsys_id)
{
	const struct cgroupfs_root *root = cgrp->root;
	const struct cgroup_subsys *test_ss;
	BUG_ON(list_empty(&root->subsys_list));
	test_ss = list_entry(root->subsys_list.next,
			     struct cgroup_subsys, sibling);
	if (css) {
		*css = cgrp->subsys[test_ss->subsys_id];
		BUG_ON(!*css);
	}
	if (subsys_id)
		*subsys_id = test_ss->subsys_id;
}

/**
 * cgroup_attach_task - attach task 'tsk' to cgroup 'cgrp'
 * @cgrp: the cgroup the task is attaching to
 * @tsk: the task to be attached
 *
 * Call holding cgroup_mutex. May take task_lock of
 * the task 'tsk' during call.
 */
int cgroup_attach_task(struct cgroup *cgrp, struct task_struct *tsk)
{
	int retval = 0;
	struct cgroup_subsys *ss;
	struct cgroup *oldcgrp;
	struct css_set *cg;
	struct css_set *newcg;
	struct cgroupfs_root *root = cgrp->root;
	int subsys_id;

	get_first_subsys(cgrp, NULL, &subsys_id);

	/* Nothing to do if the task is already in that cgroup */
	oldcgrp = task_cgroup(tsk, subsys_id);
	if (cgrp == oldcgrp)
		return 0;

	for_each_subsys(root, ss) {
		if (ss->can_attach) {
			retval = ss->can_attach(ss, cgrp, tsk);
			if (retval)
				return retval;
		}
	}

	task_lock(tsk);
	cg = tsk->cgroups;
	get_css_set(cg);
	task_unlock(tsk);
	/*
	 * Locate or allocate a new css_set for this task,
	 * based on its final set of cgroups
	 */
	newcg = find_css_set(cg, cgrp);
	put_css_set(cg);
	if (!newcg)
		return -ENOMEM;

	task_lock(tsk);
	if (tsk->flags & PF_EXITING) {
		task_unlock(tsk);
		put_css_set(newcg);
		return -ESRCH;
	}
	rcu_assign_pointer(tsk->cgroups, newcg);
	task_unlock(tsk);

	/* Update the css_set linked lists if we're using them */
	write_lock(&css_set_lock);
	if (!list_empty(&tsk->cg_list)) {
		list_del(&tsk->cg_list);
		list_add(&tsk->cg_list, &newcg->tasks);
	}
	write_unlock(&css_set_lock);

	for_each_subsys(root, ss) {
		if (ss->attach)
			ss->attach(ss, cgrp, oldcgrp, tsk);
	}
	set_bit(CGRP_RELEASABLE, &oldcgrp->flags);
	synchronize_rcu();
	put_css_set(cg);

	/*
	 * wake up rmdir() waiter. the rmdir should fail since the cgroup
	 * is no longer empty.
	 */
	cgroup_wakeup_rmdir_waiter(cgrp);
	return 0;
}

/*
 * Attach task with pid 'pid' to cgroup 'cgrp'. Call with cgroup_mutex
 * held. May take task_lock of task
 */
static int attach_task_by_pid(struct cgroup *cgrp, u64 pid)
{
	struct task_struct *tsk;
	const struct cred *cred = current_cred(), *tcred;
	int ret;

	if (pid) {
		rcu_read_lock();
		#ifndef __RIVAFB_H
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
 * DO NOT EDIT - This file is automatically generated
 *		 from the following source files:
 *
 * $Id: //depot/aic7xxx/aic7xxx/aic7xxx.seq#58 $
 * $Id: //depot/aic7xxx/aic7xxx/aic7xxx.reg#40 $
 */
typedef int (ahc_reg_print_t)(u_int, u_int *, u_int);
typedef struct ahc_reg_parse_entry {
	char	*name;
	uint8_t	 value;
	uint8_t	 mask;
} ahc_reg_parse_entry_t;

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_scsiseq_print;
#else
#define ahc_scsiseq_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SCSISEQ", 0x00, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_sxfrctl0_print;
#else
#define ahc_sxfrctl0_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SXFRCTL0", 0x01, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_scsisigi_print;
#else
#define ahc_scsisigi_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SCSISIGI", 0x03, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_scsirate_print;
#else
#define ahc_scsirate_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SCSIRATE", 0x04, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_sstat0_print;
#else
#define ahc_sstat0_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SSTAT0", 0x0b, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_sstat1_print;
#else
#define ahc_sstat1_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SSTAT1", 0x0c, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_sstat2_print;
#else
#define ahc_sstat2_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SSTAT2", 0x0d, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_sstat3_print;
#else
#define ahc_sstat3_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SSTAT3", 0x0e, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_simode0_print;
#else
#define ahc_simode0_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SIMODE0", 0x10, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_simode1_print;
#else
#define ahc_simode1_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SIMODE1", 0x11, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_scsibusl_print;
#else
#define ahc_scsibusl_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SCSIBUSL", 0x12, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_sblkctl_print;
#else
#define ahc_sblkctl_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SBLKCTL", 0x1f, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_seq_flags_print;
#else
#define ahc_seq_flags_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SEQ_FLAGS", 0x3c, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_lastphase_print;
#else
#define ahc_lastphase_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "LASTPHASE", 0x3f, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_seqctl_print;
#else
#define ahc_seqctl_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SEQCTL", 0x60, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_sram_base_print;
#else
#define ahc_sram_base_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SRAM_BASE", 0x70, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_error_print;
#else
#define ahc_error_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "ERROR", 0x92, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_dfcntrl_print;
#else
#define ahc_dfcntrl_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "DFCNTRL", 0x93, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_dfstatus_print;
#else
#define ahc_dfstatus_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "DFSTATUS", 0x94, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_scsiphase_print;
#else
#define ahc_scsiphase_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SCSIPHASE", 0x9e, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_scb_base_print;
#else
#define ahc_scb_base_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SCB_BASE", 0xa0, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_scb_control_print;
#else
#define ahc_scb_control_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SCB_CONTROL", 0xb8, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_scb_scsiid_print;
#else
#define ahc_scb_scsiid_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SCB_SCSIID", 0xb9, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_scb_lun_print;
#else
#define ahc_scb_lun_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SCB_LUN", 0xba, regvalue, cur_col, wrap)
#endif

#if AIC_DEBUG_REGISTERS
ahc_reg_print_t ahc_scb_tag_print;
#else
#define ahc_scb_tag_print(regvalue, cur_col, wrap) \
    ahc_print_register(NULL, 0, "SCB_TAG", 0xbb, regvalue, cur_col, wrap)
#endif


#define	SCSISEQ         		0x00
#define		TEMODE          	0x80
#define		SCSIRSTO        	0x01

#define	SXFRCTL0        		0x01
#define		DFON            	0x80
#define		DFPEXP          	0x40
#define		FAST20          	0x20
#define		CLRSTCNT        	0x10
#define		SPIOEN          	0x08
#define		SCAMEN          	0x04
#define		CLRCHN          	0x02

#define	SXFRCTL1        		0x02
#define		STIMESEL        	0x18
#define		BITBUCKET       	0x80
#define		SWRAPEN         	0x40
#define		ENSTIMER        	0x04
#define		ACTNEGEN        	0x02
#define		STPWEN          	0x01

#define	SCSISIGO        		0x03
#define		CDO             	0x80
#define		IOO             	0x40
#define		MSGO            	0x20
#define		ATNO            	0x10
#define		SELO            	0x08
#define		BSYO            	0x04
#define		REQO            	0x02
#define		ACKO            	0x01

#define	SCSISIGI        		0x03
#define		P_DATAIN_DT     	0x60
#define		P_DATAOUT_DT    	0x20
#define		ATNI            	0x10
#define		SELI            	0x08
#define		BSYI            	0x04
#define		REQI            	0x02
#define		ACKI            	0x01

#define	SCSIRATE        		0x04
#define		SXFR            	0x70
#define		SOFS            	0x0f
#define		SXFR_ULTRA2     	0x0f
#define		WIDEXFER        	0x80
#define		ENABLE_CRC      	0x40
#define		SINGLE_EDGE     	0x10

#define	SCSIID          		0x05
#define	SCSIOFFSET      		0x05
#define		SOFS_ULTRA2     	0x7f

#define	SCSIDATL        		0x06

#define	SCSIDATH        		0x07

#define	STCNT           		0x08

#define	OPTIONMODE      		0x08
#define		OPTIONMODE_DEFAULTS	0x03
#define		AUTORATEEN      	0x80
#define		AUTOACKEN       	0x40
#define		ATNMGMNTEN      	0x20
#define		BUSFREEREV      	0x10
#define		EXPPHASEDIS     	0x08
#define		SCSIDATL_IMGEN  	0x04
#define		AUTO_MSGOUT_DE  	0x02
#define		DIS_MSGIN_DUALEDGE	0x01

#define	TARGCRCCNT      		0x0a

#define	CLRSINT0        		0x0b
#define		CLRSELDO        	0x40
#define		CLRSELDI        	0x20
#define		CLRSELINGO      	0x10
#define		CLRIOERR        	0x08
#define		CLRSWRAP        	0x08
#define		CLRSPIORDY      	0x02

#define	SSTAT0          		0x0b
#define		TARGET          	0x80
#define		SELDO           	0x40
#define		SELDI           	0x20
#define		SELINGO         	0x10
#define		SWRAP           	0x08
#define		IOERR           	0x08
#define		SDONE           	0x04
#define		SPIORDY         	0x02
#define		DMADONE         	0x01

#define	CLRSINT1        		0x0c
#define		CLRSELTIMEO     	0x80
#define		CLRATNO         	0x40
#define		CLRSCSIRSTI     	0x20
#define		CLRBUSFREE      	0x08
#define		CLRSCSIPERR     	0x04
#define		CLRPHASECHG     	0x02
#define		CLRREQINIT      	0x01

#define	SSTAT1          		0x0c
#define		SELTO           	0x80
#define		ATNTARG         	0x40
#define		SCSIRSTI        	0x20
#define		PHASEMIS        	0x10
#define		BUSFREE         	0x08
#define		SCSIPERR        	0x04
#define		PHASECHG        	0x02
#define		REQINIT         	0x01

#define	SSTAT2          		0x0d
#define		SFCNT           	0x1f
#define		OVERRUN         	0x80
#define		SHVALID         	0x40
#define		EXP_ACTIVE      	0x10
#define		CRCVALERR       	0x08
#define		CRCENDERR       	0x04
#define		CRCREQERR       	0x02
#define		DUAL_EDGE_ERR   	0x01

#define	SSTAT3          		0x0e
#define		SCSICNT         	0xf0
#define		U2OFFCNT        	0x7f
#define		OFFCNT          	0x0f

#define	SCSIID_ULTRA2   		0x0f

#define	SIMODE0         		0x10
#define		ENSELDO         	0x40
#define		ENSELDI         	0x20
#define		ENSELINGO       	0x10
#define		ENIOERR         	0x08
#define		ENSWRAP         	0x08
#define		ENSDONE         	0x04
#define		ENSPIORDY       	0x02
#define		ENDMADONE       	0x01

#define	SIMODE1         		0x11
#define		ENSELTIMO       	0x80
#define		ENATNTARG       	0x40
#define		ENSCSIRST       	0x20
#define		ENPHASEMIS      	0x10
#define		ENBUSFREE       	0x08
#define		ENSCSIPERR      	0x04
#define		ENPHASECHG      	0x02
#define		ENREQINIT       	0x01

#define	SCSIBUSL        		0x12

#define	SCSIBUSH        		0x13

#define	SXFRCTL2        		0x13
#define		ASYNC_SETUP     	0x07
#define		AUTORSTDIS      	0x10
#define		CMDDMAEN        	0x08

#define	SHADDR          		0x14

#define	SELTIMER        		0x18
#define	TARGIDIN        		0x18
#define		STAGE6          	0x20
#define		STAGE5          	0x10
#define		STAGE4          	0x08
#define		STAGE3          	0x04
#define		STAGE2          	0x02
#define		STAGE1          	0x01

#define	SELID           		0x19
#define		SELID_MASK      	0xf0
#define		ONEBIT          	0x08

#define	SCAMCTL         		0x1a
#define		SCAMLVL         	0x03
#define		ENSCAMSELO      	0x80
#define		CLRSCAMSELID    	0x40
#define		ALTSTIM         	0x20
#define		DFLTTID         	0x10

#define	TARGID          		0x1b

#define	SPIOCAP         		0x1b
#define		SOFT1           	0x80
#define		SOFT0           	0x40
#define		SOFTCMDEN       	0x20
#define		EXT_BRDCTL      	0x10
#define		SEEPROM         	0x08
#define		EEPROM          	0x04
#define		ROM             	0x02
#define		SSPIOCPS        	0x01

#define	BRDCTL          		0x1d
#define		BRDDAT7         	0x80
#define		BRDDAT6         	0x40
#define		BRDDAT5         	0x20
#define		BRDDAT4         	0x10
#define		BRDSTB          	0x10
#define		BRDDAT3         	0x08
#define		BRDCS           	0x08
#define		BRDDAT2         	0x04
#define		BRDRW           	0x04
#define		BRDRW_ULTRA2    	0x02
#define		BRDCTL1         	0x02
#define		BRDCTL0         	0x01
#define		BRDSTB_ULTRA2   	0x01

#define	SEECTL          		0x1e
#define		EXTARBACK       	0x80
#define		EXTARBREQ       	0x40
#define		SEEMS           	0x20
#define		SEERDY          	0x10
#define		SEECS           	0x08
#define		SEECK           	0x04
#define		SEEDO           	0x02
#define		SEEDI           	0x01

#define	SBLKCTL         		0x1f
#define		DIAGLEDEN       	0x80
#define		DIAGLEDON       	0x40
#define		AUTOFLUSHDIS    	0x20
#define		ENAB40          	0x08
#define		SELBUSB         	0x08
#define		ENAB20          	0x04
#define		SELWIDE         	0x02
#define		XCVR            	0x01

#define	BUSY_TARGETS    		0x20
#define	TARG_SCSIRATE   		0x20

#define	ULTRA_ENB       		0x30
#define	CMDSIZE_TABLE   		0x30

#define	DISC_DSB        		0x32

#define	CMDSIZE_TABLE_TAIL		0x34

#define	MWI_RESIDUAL    		0x38

#define	NEXT_QUEUED_SCB 		0x39

#define	MSG_OUT         		0x3a

#define	DMAPARAMS       		0x3b
#define		PRELOADEN       	0x80
#define		WIDEODD         	0x40
#define		SCSIEN          	0x20
#define		SDMAEN          	0x10
#define		SDMAENACK       	0x10
#define		HDMAEN          	0x08
#define		HDMAENACK       	0x08
#define		DIRECTION       	0x04
#define		FIFOFLUSH       	0x02
#define		FIFORESET       	0x01

#define	SEQ_FLAGS       		0x3c
#define		NOT_IDENTIFIED  	0x80
#define		NO_CDB_SENT     	0x40
#define		TARGET_CMD_IS_TAGGED	0x40
#define		DPHASE          	0x20
#define		TARG_CMD_PENDING	0x10
#define		CMDPHASE_PENDING	0x08
#define		DPHASE_PENDING  	0x04
#define		SPHASE_PENDING  	0x02
#define		NO_DISCONNECT   	0x01

#define	SAVED_SCSIID    		0x3d

#define	SAVED_LUN       		0x3e

#define	LASTPHASE       		0x3f
#define		P_MESGIN        	0xe0
#define		PHASE_MASK      	0xe0
#define		P_STATUS        	0xc0
#define		P_MESGOUT       	0xa0
#define		P_COMMAND       	0x80
#define		P_DATAIN        	0x40
#define		P_BUSFREE       	0x01
#define		P_DATAOUT       	0x00
#define		CDI             	0x80
#define		IOI             	0x40
#define		MSGI            	0x20

#define	WAITING_SCBH    		0x40

#define	DISCONNECTED_SCBH		0x41

#define	FREE_SCBH       		0x42

#define	COMPLETE_SCBH   		0x43

#define	HSCB_ADDR       		0x44

#define	SHARED_DATA_ADDR		0x48

#define	KERNEL_QINPOS   		0x4c

#define	QINPOS          		0x4d

#define	QOUTPOS         		0x4e

#define	KERNEL_TQINPOS  		0x4f

#define	TQINPOS         		0x50

#define	ARG_1           		0x51
#define	RETURN_1        		0x51
#define		SEND_MSG        	0x80
#define		SEND_SENSE      	0x40
#define		SEND_REJ        	0x20
#define		MSGOUT_PHASEMIS 	0x10
#define		EXIT_MSG_LOOP   	0x08
#define		CONT_MSG_LOOP   	0x04
#define		CONT_TARG_SESSION	0x02

#define	ARG_2           		0x52
#define	RETURN_2        		0x52

#define	LAST_MSG        		0x53
#define	TARG_IMMEDIATE_SCB		0x53

#define	SCSISEQ_TEMPLATE		0x54
#define		ENSELO          	0x40
#define		ENSELI          	0x20
#define		ENRSELI         	0x10
#define		ENAUTOATNO      	0x08
#define		ENAUTOATNI      	0x04
#define		ENAUTOATNP      	0x02

#define	HA_274_BIOSGLOBAL		0x56
#define	INITIATOR_TAG   		0x56
#define		HA_274_EXTENDED_TRANS	0x01

#define	SEQ_FLAGS2      		0x57
#define		TARGET_MSG_PENDING	0x02
#define		SCB_DMA         	0x01

#define	SCSICONF        		0x5a
#define		HWSCSIID        	0x0f
#define		HSCSIID         	0x07
#define		TERM_ENB        	0x80
#define		RESET_SCSI      	0x40
#define		ENSPCHK         	0x20

#define	INTDEF          		0x5c
#define		VECTOR          	0x0f
#define		EDGE_TRIG       	0x80

#define	HOSTCONF        		0x5d

#define	HA_274_BIOSCTRL 		0x5f
#define		BIOSDISABLED    	0x30
#define		BIOSMODE        	0x30
#define		CHANNEL_B_PRIMARY	0x08

#define	SEQCTL          		0x60
#define		PERRORDIS       	0x80
#define		PAUSEDIS        	0x40
#define		FAILDIS         	0x20
#define		FASTMODE        	0x10
#define		BRKADRINTEN     	0x08
#define		STEP            	0x04
#define		SEQRESET        	0x02
#define		LOADRAM         	0x01

#define	SEQRAM          		0x61

#define	SEQADDR0        		0x62

#define	SEQADDR1        		0x63
#define		SEQADDR1_MASK   	0x01

#define	ACCUM           		0x64

#define	SINDEX          		0x65

#define	DINDEX          		0x66

#define	ALLONES         		0x69

#define	ALLZEROS        		0x6a

#define	NONE            		0x6a

#define	FLAGS           		0x6b
#define		ZERO            	0x02
#define		CARRY           	0x01

#define	SINDIR          		0x6c

#define	DINDIR          		0x6d

#define	FUNCTION1       		0x6e

#define	STACK           		0x6f

#define	TARG_OFFSET     		0x70

#define	SRAM_BASE       		0x70

#define	BCTL            		0x84
#define		ACE             	0x08
#define		ENABLE          	0x01

#define	DSCOMMAND0      		0x84
#define		CACHETHEN       	0x80
#define		DPARCKEN        	0x40
#define		MPARCKEN        	0x20
#define		EXTREQLCK       	0x10
#define		INTSCBRAMSEL    	0x08
#define		RAMPS           	0x04
#define		USCBSIZE32      	0x02
#define		CIOPARCKEN      	0x01

#define	BUSTIME         		0x85
#define		BOFF            	0xf0
#define		BON             	0x0f

#define	DSCOMMAND1      		0x85
#define		DSLATT          	0xfc
#define		HADDLDSEL1      	0x02
#define		HADDLDSEL0      	0x01

#define	BUSSPD          		0x86
#define		DFTHRSH         	0xc0
#define		DFTHRSH_75      	0x80
#define		STBOFF          	0x38
#define		STBON           	0x07

#define	HS_MAILBOX      		0x86
#define		HOST_MAILBOX    	0xf0
#define		HOST_TQINPOS    	0x80
#define		SEQ_MAILBOX     	0x0f

#define	DSPCISTATUS     		0x86
#define		DFTHRSH_100     	0xc0

#define	HCNTRL          		0x87
#define		POWRDN          	0x40
#define		SWINT           	0x10
#define		IRQMS           	0x08
#define		PAUSE           	0x04
#define		INTEN           	0x02
#define		CHIPRST         	0x01
#define		CHIPRSTACK      	0x01

#define	HADDR           		0x88

#define	HCNT            		0x8c

#define	SCBPTR          		0x90

#define	INTSTAT         		0x91
#define		SEQINT_MASK     	0xf1
#define		OUT_OF_RANGE    	0xe1
#define		NO_FREE_SCB     	0xd1
#define		SCB_MISMATCH    	0xc1
#define		MISSED_BUSFREE  	0xb1
#define		MKMSG_FAILED    	0xa1
#define		DATA_OVERRUN    	0x91
#define		PERR_DETECTED   	0x81
#define		BAD_STATUS      	0x71
#define		HOST_MSG_LOOP   	0x61
#define		PDATA_REINIT    	0x51
#define		IGN_WIDE_RES    	0x41
#define		NO_MATCH        	0x31
#define		PROTO_VIOLATION 	0x21
#define		SEND_REJECT     	0x11
#define		INT_PEND        	0x0f
#define		BAD_PHASE       	0x01
#define		BRKADRINT       	0x08
#define		SCSIINT         	0x04
#define		CMDCMPLT        	0x02
#define		SEQINT          	0x01

#define	CLRINT          		0x92
#define		CLRPARERR       	0x10
#define		CLRBRKADRINT    	0x08
#define		CLRSCSIINT      	0x04
#define		CLRCMDINT       	0x02
#define		CLRSEQINT       	0x01

#define	ERROR           		0x92
#define		CIOPARERR       	0x80
#define		PCIERRSTAT      	0x40
#define		MPARERR         	0x20
#define		DPARERR         	0x10
#define		SQPARERR        	0x08
#define		ILLOPCODE       	0x04
#define		ILLSADDR        	0x02
#define		ILLHADDR        	0x01

#define	DFCNTRL         		0x93

#define	DFSTATUS        		0x94
#define		PRELOAD_AVAIL   	0x80
#define		DFCACHETH       	0x40
#define		FIFOQWDEMP      	0x20
#define		MREQPEND        	0x10
#define		HDONE           	0x08
#define		DFTHRESH        	0x04
#define		FIFOFULL        	0x02
#define		FIFOEMP         	0x01

#define	DFWADDR         		0x95

#define	DFRADDR         		0x97

#define	DFDAT           		0x99

#define	SCBCNT          		0x9a
#define		SCBCNT_MASK     	0x1f
#define		SCBAUTO         	0x80

#define	QINFIFO         		0x9b

#define	QINCNT          		0x9c

#define	QOUTFIFO        		0x9d

#define	CRCCONTROL1     		0x9d
#define		CRCONSEEN       	0x80
#define		CRCVALCHKEN     	0x40
#define		CRCENDCHKEN     	0x20
#define		CRCREQCHKEN     	0x10
#define		TARGCRCENDEN    	0x08
#define		TARGCRCCNTEN    	0x04

#define	QOUTCNT         		0x9e

#define	SCSIPHASE       		0x9e
#define		DATA_PHASE_MASK 	0x03
#define		STATUS_PHASE    	0x20
#define		COMMAND_PHASE   	0x10
#define		MSG_IN_PHASE    	0x08
#define		MSG_OUT_PHASE   	0x04
#define		DATA_IN_PHASE   	0x02
#define		DATA_OUT_PHASE  	0x01

#define	SFUNCT          		0x9f
#define		ALT_MODE        	0x80

#define	SCB_BASE        		0xa0

#define	SCB_CDB_PTR     		0xa0
#define	SCB_RESIDUAL_DATACNT		0xa0
#define	SCB_CDB_STORE   		0xa0

#define	SCB_RESIDUAL_SGPTR		0xa4

#define	SCB_SCSI_STATUS 		0xa8

#define	SCB_TARGET_PHASES		0xa9

#define	SCB_TARGET_DATA_DIR		0xaa

#define	SCB_TARGET_ITAG 		0xab

#define	SCB_DATAPTR     		0xac

#define	SCB_DATACNT     		0xb0
#define		SG_HIGH_ADDR_BITS	0x7f
#define		SG_LAST_SEG     	0x80

#define	SCB_SGPTR       		0xb4
#define		SG_RESID_VALID  	0x04
#define		SG_FULL_RESID   	0x02
#define		SG_LIST_NULL    	0x01

#define	SCB_CONTROL     		0xb8
#define		SCB_TAG_TYPE    	0x03
#define		STATUS_RCVD     	0x80
#define		TARGET_SCB      	0x80
#define		DISCENB         	0x40
#define		TAG_ENB         	0x20
#define		MK_MESSAGE      	0x10
#define		ULTRAENB        	0x08
#define		DISCONNECTED    	0x04

#define	SCB_SCSIID      		0xb9
#define		TID             	0xf0
#define		TWIN_TID        	0x70
#define		OID             	0x0f
#define		TWIN_CHNLB      	0x80

#define	SCB_LUN         		0xba
#define		LID             	0x3f
#define		SCB_XFERLEN_ODD 	0x80

#define	SCB_TAG         		0xbb

#define	SCB_CDB_LEN     		0xbc

#define	SCB_SCSIRATE    		0xbd

#define	SCB_SCSIOFFSET  		0xbe

#define	SCB_NEXT        		0xbf

#define	SCB_64_SPARE    		0xc0

#define	SEECTL_2840     		0xc0
#define		CS_2840         	0x04
#define		CK_2840         	0x02
#define		DO_2840         	0x01

#define	STATUS_2840     		0xc1
#define		BIOS_SEL        	0x60
#define		ADSEL           	0x1e
#define		EEPROM_TF       	0x80
#define		DI_2840         	0x01

#define	SCB_64_BTT      		0xd0

#define	CCHADDR         		0xe0

#define	CCHCNT          		0xe8

#define	CCSGRAM         		0xe9

#define	CCSGADDR        		0xea

#define	CCSGCTL         		0xeb
#define		CCSGDONE        	0x80
#define		CCSGEN          	0x08
#define		SG_FETCH_NEEDED 	0x02
#define		CCSGRESET       	0x01

#define	CCSCBRAM        		0xec

#define	CCSCBADDR       		0xed

#define	CCSCBCTL        		0xee
#define		CCSCBDONE       	0x80
#define		ARRDONE         	0x40
#define		CCARREN         	0x10
#define		CCSCBEN         	0x08
#define		CCSCBDIR        	0x04
#define		CCSCBRESET      	0x01

#define	CCSCBCNT        		0xef

#define	SCBBADDR        		0xf0

#define	CCSCBPTR        		0xf1

#define	HNSCB_QOFF      		0xf4

#define	SNSCB_QOFF      		0xf6

#define	SDSCB_QOFF      		0xf8

#define	QOFF_CTLSTA     		0xfa
#define		SCB_QSIZE       	0x07
#define		SCB_QSIZE_256   	0x06
#define		SCB_AVAIL       	0x40
#define		SNSCB_ROLLOVER  	0x20
#define		SDSCB_ROLLOVER  	0x10

#define	DFF_THRSH       		0xfb
#define		WR_DFTHRSH      	0x70
#define		WR_DFTHRSH_MAX  	0x70
#define		WR_DFTHRSH_90   	0x60
#define		WR_DFTHRSH_85   	0x50
#define		WR_DFTHRSH_75   	0x40
#define		WR_DFTHRSH_63   	0x30
#define		WR_DFTHRSH_50   	0x20
#define		WR_DFTHRSH_25   	0x10
#define		RD_DFTHRSH      	0x07
#define		RD_DFTHRSH_MAX  	0x07
#define		RD_DFTHRSH_90   	0x06
#define		RD_DFTHRSH_85   	0x05
#define		RD_DFTHRSH_75   	0x04
#define		RD_DFTHRSH_63   	0x03
#define		RD_DFTHRSH_50   	0x02
#define		RD_DFTHRSH_25   	0x01
#define		RD_DFTHRSH_MIN  	0x00
#define		WR_DFTHRSH_MIN  	0x00

#define	SG_CACHE_SHADOW 		0xfc
#define		SG_ADDR_MASK    	0xf8
#define		LAST_SEG        	0x02
#define		LAST_SEG_DONE   	0x01

#define	SG_CACHE_PRE    		0xfc


#define	MAX_OFFSET_ULTRA2	0x7f
#define	MAX_OFFSET_16BIT	0x08
#define	BUS_8_BIT	0x00
#define	TARGET_CMD_CMPLT	0xfe
#define	STATUS_QUEUE_FULL	0x28
#define	STATUS_BUSY	0x08
#define	MAX_OFFSET_8BIT	0x0f
#define	BUS_32_BIT	0x02
#define	CCSGADDR_MAX	0x80
#define	TID_SHIFT	0x04
#define	SCB_DOWNLOAD_SIZE_64	0x30
#define	HOST_MAILBOX_SHIFT	0x04
#define	CMD_GROUP_CODE_SHIFT	0x05
#define	CCSGRAM_MAXSEGS	0x10
#define	SCB_LIST_NULL	0xff
#define	SG_SIZEOF	0x08
#define	SCB_DOWNLOAD_SIZE	0x20
#define	SEQ_MAILBOX_SHIFT	0x00
#define	TARGET_DATA_IN	0x01
#define	HOST_MSG	0xff
#define	MAX_OFFSET	0x7f
#define	BUS_16_BIT	0x01
#define	SCB_UPLOAD_SIZE	0x20
#define	STACK_SIZE	0x04


/* Downloaded Constant Definitions */
#define	INVERTED_CACHESIZE_MASK	0x03
#define	SG_PREFETCH_ADDR_MASK	0x06
#define	SG_PREFETCH_ALIGN_MASK	0x05
#define	QOUTFIFO_OFFSET	0x00
#define	SG_PREFETCH_CNT	0x04
#define	CACHESIZE_MASK	0x02
#define	QINFIFO_OFFSET	0x01
#define	DOWNLOAD_CONST_COUNT	0x07


/* Exported Labels */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     /*
 * Copyright 2004-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

/*!
 * @file pmic/core/mc13783.c
 * @brief This file contains MC13783 specific PMIC code. This implementaion
 * may differ for each PMIC chip.
 *
 * @ingroup PMIC_CORE
 */

/*
 * Includes
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/pmic_external.h>
#include <linux/pmic_status.h>
#include <linux/spi/spi.h>
#include <linux/mfd/mc13783/core.h>

#include <asm/uaccess.h>

#include "pmic.h"

/*
 * Defines
 */
#define EVENT_MASK_0			0x697fdf
#define EVENT_MASK_1			0x3efffb
#define MXC_PMIC_FRAME_MASK		0x00FFFFFF
#define MXC_PMIC_MAX_REG_NUM		0x3F
#define MXC_PMIC_REG_NUM_SHIFT		0x19
#define MXC_PMIC_WRITE_BIT_SHIFT	31

static unsigned int events_enabled0 = 0;
static unsigned int events_enabled1 = 0;
struct mxc_pmic pmic_drv_data;

/*!
 * This function is called to read a register on PMIC.
 *
 * @param        reg_num     number of the pmic register to be read
 * @param        reg_val   return value of register
 *
 * @return       Returns 0 on success -1 on failure.
 */
int pmic_read(unsigned int reg_num, unsigned int *reg_val)
{
	unsigned int frame = 0;
	int ret = 0;

	if (reg_num > MXC_PMIC_MAX_REG_NUM)
		return PMIC_ERROR;

	frame |= reg_num << MXC_PMIC_REG_NUM_SHIFT;

	ret = spi_rw(pmic_drv_data.spi, (u8 *) & frame, 1);

	*reg_val = frame & MXC_PMIC_FRAME_MASK;

	return ret;
}

/*!
 * This function is called to write a value to the register on PMIC.
 *
 * @param        reg_num     number of the pmic register to be written
 * @param        reg_val   value to be written
 *
 * @return       Returns 0 on success -1 on failure.
 */
int pmic_write(int reg_num, const unsigned int reg_val)
{
	unsigned int frame = 0;
	int ret = 0;

	if (reg_num > MXC_PMIC_MAX_REG_NUM)
		return PMIC_ERROR;

	frame |= (1 << MXC_PMIC_WRITE_BIT_SHIFT);

	frame |= reg_num << MXC_PMIC_REG_NUM_SHIFT;

	frame |= reg_val & MXC_PMIC_FRAME_MASK;

	ret = spi_rw(pmic_drv_data.spi, (u8 *) & frame, 1);

	return ret;
}

void *pmic_alloc_data(struct device *dev)
{
	struct mc13783 *mc13783;

	mc13783 = kzalloc(sizeof(struct mc13783), GFP_KERNEL);
	if (mc13783 == NULL)
		return NULL;

	mc13783->dev = dev;

	return (void *)mc13783;
}

/*!
 * This function initializes the SPI device parameters for this PMIC.
 *
 * @param    spi	the SPI slave device(PMIC)
 *
 * @return   None
 */
int pmic_spi_setup(struct spi_device *spi)
{
	/* Setup the SPI slave i.e.PMIC */
	pmic_drv_data.spi = spi;

	spi->mode = SPI_MODE_2 | SPI_CS_HIGH;
	spi->bits_per_word = 32;

	return spi_setup(spi);
}

/*!
 * This function initializes the PMIC registers.
 *
 * @return   None
 */
int pmic_init_registers(void)
{
	CHECK_ERROR(pmic_write(REG_INTERRUPT_MASK_0, MXC_PMIC_FRAME_MASK));
	CHECK_ERROR(pmic_write(REG_INTERRUPT_MASK_1, MXC_PMIC_FRAME_MASK));
	CHECK_ERROR(pmic_write(REG_INTERRUPT_STATUS_0, MXC_PMIC_FRAME_MASK));
	CHECK_ERROR(pmic_write(REG_INTERRUPT_STATUS_1, MXC_PMIC_FRAME_MASK));
	return PMIC_SUCCESS;
}

/*!
 * This function returns the PMIC version in system.
 *
 * @param 	ver	pointer to the pmic_version_t structure
 *
 * @return       This function returns PMIC version.
 */
void pmic_get_revision(pmic_version_t * ver)
{
	int rev_id = 0;
	int rev1 = 0;
	int rev2 = 0;
	int finid = 0;
	int icid = 0;

	ver->id = PMIC_MC13783;
	pmic_read(REG_REVISION, &rev_id);

	rev1 = (rev_id & 0x018) >> 3;
	rev2 = (rev_id & 0x007);
	icid = (rev_id & 0x01C0) >> 6;
	finid = (rev_id & 0x01E00) >> 9;

	/* Ver 0.2 is actually 3.2a.  Report as 3.2 */
	if ((rev1 == 0) && (rev2 == 2)) {
		rev1 = 3;
	}

	if (rev1 == 0 || icid != 2) {
		ver->revision = -1;
		printk(KERN_NOTICE
		       "mc13783: Not detected.\tAccess failed\t!!!\n");
	} else {
		ver->revision = ((rev1 * 10) + rev2);
		printk(KERN_INFO "mc13783 Rev %d.%d FinVer %x detected\n", rev1,
		       rev2, finid);
	}

	return;

}

/*!
 * This function reads the interrupt status registers of PMIC
 * and determine the current active events.
 *
 * @param 	active_events array pointer to be used to return active
 *		event numbers.
 *
 * @return       This function returns PMIC version.
 */
unsigned int pmic_get_active_events(unsigned int *active_events)
{
	unsigned int count = 0;
	unsigned int status0, status1;
	int bit_set;

	pmic_read(REG_INTERRUPT_STATUS_0, &status0);
	pmic_read(REG_INTERRUPT_STATUS_1, &status1);
	pmic_write(REG_INTERRUPT_STATUS_0, status0);
	pmic_write(REG_INTERRUPT_STATUS_1, status1);
	status0 &= events_enabled0;
	status1 &= events_enabled1;

	while (status0) {
		bit_set = ffs(status0) - 1;
		*(active_events + count) = bit_set;
		count++;
		status0 ^= (1 << bit_set);
	}
	while (status1) {
		bit_set = ffs(status1) - 1;
		*(active_events + count) = bit_set + 24;
		count++;
		status1 ^= (1 << bit_set);
	}

	return count;
}

/*!
 * This function unsets a bit in mask register of pmic to unmask an event IT.
 *
 * @param	event 	the event to be unmasked
 *
 * @return    This function returns PMIC_SUCCESS on SUCCESS, error on FAILURE.
 */
int pmic_event_unmask(type_event event)
{
	unsigned int event_mask = 0;
	unsigned int mask_reg = 0;
	unsigned int event_bit = 0;
	int ret;

	if (event < EVENT_E1HZI) {
		mask_reg = REG_INTERRUPT_MASK_0;
		event_mask = EVENT_MASK_0;
		event_bit = (1 << event);
		events_enabled0 |= event_bit;
	} else {
		event -= 24;
		mask_reg = REG_INTERRUPT_MASK_1;
		event_mask = EVENT_MASK_1;
		event_bit = (1 << event);
		events_enabled1 |= event_bit;
	}

	if ((event_bit & event_mask) == 0) {
		pr_debug("Error: unmasking a reserved/unused event\n");
		return PMIC_ERROR;
	}

	ret = pmic_write_reg(mask_reg, 0, event_bit);

	pr_debug("Enable Event : %d\n", event);

	return ret;
}

/*!
 * This function sets a bit in mask register of pmic to disable an event IT.
 *
 * @param	event 	the event to be masked
 *
 * @return     This function returns PMIC_SUCCESS on SUCCESS, error on FAILURE.
 */
int pmic_event_mask(type_event event)
{
	unsigned int event_mask = 0;
	unsigned int mask_reg = 0;
	unsigned int event_bit = 0;
	int ret;

	if (event < EVENT_E1HZI) {
		mask_reg = REG_INTERRUPT_MASK_0;
		event_mask = EVENT_MASK_0;
		event_bit = (1 << event);
		events_enabled0 &= ~event_bit;
	} else {
		event -= 24;
		mask_reg = REG_INTERRUPT_MASK_1;
		event_mask = EVENT_MASK_1;
		event_bit = (1 << event);
		events_enabled1 &= ~event_bit;
	}

	if ((event_bit & event_mask) == 0) {
		pr_debug("Error: masking a reserved/unused event\n");
		return PMIC_ERROR;
	}

	ret = pmic_write_reg(mask_reg, event_bit, event_bit);

	pr_debug("Disable Event : %d\n", event);

	return ret;
}

/*!
 * This function is called to read all sensor bits of PMIC.
 *
 * @param        sensor    Sensor to be checked.
 *
 * @return       This function returns true if the sensor bit is high;
 *               or returns false if the sensor bit is low.
 */
bool pmic_check_sensor(t_sensor sensor)
{
	unsigned int reg_val = 0;

	CHECK_ERROR(pmic_read_reg
		    (REG_INTERRUPT_SENSE_0, &reg_val, PMIC_ALL_BITS));

	if ((1 << sensor) & reg_val)
		return true;
	else
		return false;
}

/*!
 * This function checks one sensor of PMIC.
 *
 * @param        sensor_bits  structure of all sensor bits.
 *
 * @return    This function returns PMIC_SUCCESS on SUCCESS, error on FAILURE.
 */

PMIC_STATUS pmic_get_sensors(t_sensor_bits * sensor_bits)
{
	int sense_0 = 0;
	int sense_1 = 0;

	memset(sensor_bits, 0, sizeof(t_sensor_bits));

	pmic_read_reg(REG_INTERRUPT_SENSE_0, &sense_0, 0xffffff);
	pmic_read_reg(REG_INTERRUPT_SENSE_1, &sense_1, 0xffffff);

	sensor_bits->sense_chgdets = (sense_0 & (1 << 6)) ? true : false;
	sensor_bits->sense_chgovs = (sense_0 & (1 << 7)) ? true : false;
	sensor_bits->sense_chgrevs = (sense_0 & (1 << 8)) ? true : false;
	sensor_bits->sense_chgshorts = (sense_0 & (1 << 9)) ? true : false;
	sensor_bits->sense_cccvs = (sense_0 & (1 << 10)) ? true : false;
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
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             take cgroup_mutex, keeping cgroup_attach_task() from changing it
 *    anyway.  No need to check that tsk->cgroup != NULL, thanks to
 *    the_top_cgroup_hack in cgroup_exit(), which sets an exiting tasks
 *    cgroup to top_cgroup.
 */

/* TODO: Use a proper seq_file iterator */
static int proc_cgroup_show(struct seq_file *m, void *v)
{
	struct pid *pid;
	struct task_struct *tsk;
	char *buf;
	int retval;
	struct cgroupfs_root *root;

	retval = -ENOMEM;
	buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!buf)
		goto out;

	retval = -ESRCH;
	pid = m->private;
	tsk = get_pid_task(pid, PIDTYPE_PID);
	if (!tsk)
		goto out_free;

	retval = 0;

	mutex_lock(&cgroup_mutex);

	for_each_active_root(root) {
		struct cgroup_subsys *ss;
		struct cgroup *cgrp;
		int subsys_id;
		int count = 0;

		seq_printf(m, "%lu:", root->subsys_bits);
		for_each_subsys(root, ss)
			seq_printf(m, "%s%s", count++ ? "," : "", ss->name);
		seq_putc(m, ':');
		get_first_subsys(&root->top_cgroup, NULL, &subsys_id);
		cgrp = task_cgroup(tsk, subsys_id);
		retval = cgroup_path(cgrp, buf, PAGE_SIZE);
		if (retval < 0)
			goto out_unlock;
		seq_puts(m, buf);
		seq_putc(m, '\n');
	}

out_unlock:
	mutex_unlock(&cgroup_mutex);
	put_task_struct(tsk);
out_free:
	kfree(buf);
out:
	return retval;
}

static int cgroup_open(struct inode *inode, struct file *file)
{
	struct pid *pid = PROC_I(inode)->pid;
	return single_open(file, proc_cgroup_show, pid);
}

struct file_operations proc_cgroup_operations = {
	.open		= cgroup_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

/* Display information about each subsystem and each hierarchy */
static int proc_cgroupstats_show(struct seq_file *m, void *v)
{
	int i;

	seq_puts(m, "#subsys_name\thierarchy\tnum_cgroups\tenabled\n");
	mutex_lock(&cgroup_mutex);
	for (i = 0; i < CGROUP_SUBSYS_COUNT; i++) {
		struct cgroup_subsys *ss = subsys[i];
		seq_printf(m, "%s\t%lu\t%d\t%d\n",
			   ss->name, ss->root->subsys_bits,
			   ss->root->number_of_cgroups, !ss->disabled);
	}
	mutex_unlock(&cgroup_mutex);
	return 0;
}

static int cgroupstats_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_cgroupstats_show, NULL);
}

static struct file_operations proc_cgroupstats_operations = {
	.open = cgroupstats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

/**
 * cgroup_fork - attach newly forked task to its parents cgroup.
 * @child: pointer to task_struct of forking parent process.
 *
 * Description: A task inherits its parent's cgroup at fork().
 *
 * A pointer to the shared css_set was automatically copied in
 * fork.c by dup_task_struct().  However, we ignore that copy, since
 * it was not made under the protection of RCU or cgroup_mutex, so
 * might no longer be a valid cgroup pointer.  cgroup_attach_task() might
 * have already changed current->cgroups, allowing the previously
 * referenced cgroup group to be removed and freed.
 *
 * At the point that cgroup_fork() is called, 'current' is the parent
 * task, and the passed argument 'child' points to the child task.
 */
void cgroup_fork(struct task_struct *child)
{
	task_lock(current);
	child->cgroups = current->cgroups;
	get_css_set(child->cgroups);
	task_unlock(current);
	INIT_LIST_HEAD(&child->cg_list);
}

/**
 * cgroup_fork_callbacks - run fork callbacks
 * @child: the new task
 *
 * Called on a new task very soon before adding it to the
 * tasklist. No need to take any locks since no-one can
 * be operating on this task.
 */
void cgroup_fork_callbacks(struct task_struct *child)
{
	if (need_forkexit_callback) {
		int i;
		for (i = 0; i < CGROUP_SUBSYS_COUNT; i++) {
			struct cgroup_subsys *ss = subsys[i];
			if (ss->fork)
				ss->fork(ss, child);
		}
	}
}

/**
 * cgroup_post_fork - called on a new task after adding it to the task list
 * @child: the task in question
 *
 * Adds the task to the list running through its css_set if necessary.
 * Has to be after the task is visible on the task list in case we race
 * with the first call to cgroup_iter_start() - to guarantee that the
 * new task ends up on its list.
 */
void cgroup_post_fork(struct task_struct *child)
{
	if (use_task_css_set_links) {
		write_lock(&css_set_lock);
		task_lock(child);
		if (list_empty(&child->cg_list))
			list_add(&child->cg_list, &child->cgroups->tasks);
		task_unlock(child);
		write_unlock(&css_set_lock);
	}
}
/**
 * cgroup_exit - detach cgroup from exiting task
 * @tsk: pointer to task_struct of exiting process
 * @run_callback: run exit callbacks?
 *
 * Description: Detach cgroup from @tsk and release it.
 *
 * Note that cgroups marked notify_on_release force every task in
 * them to take the global cgroup_mutex mutex when exiting.
 * This could impact scaling on very large systems.  Be reluctant to
 * use notify_on_release cgroups where very high task exit scaling
 * is required on large systems.
 *
 * the_top_cgroup_hack:
 *
 *    Set the exiting tasks cgroup to the root cgroup (top_cgroup).
 *
 *    We call cgroup_exit() while the task is still competent to
 *    handle notify_on_release(), then leave the task attached to the
 *    root cgroup in each hierarchy for the remainder of its exit.
 *
 *    To do this properly, we would increment the reference count on
 *    top_cgroup, and near the very end of the kernel/exit.c do_exit()
 *    code we would add a second cgroup function call, to drop that
 *    reference.  This would just create an unnecessary hot spot on
 *    the top_cgroup reference count, to no avail.
 *
 *    Normally, holding a reference to a cgroup without bumping its
 *    count is unsafe.   The cgroup could go away, or someone could
 *    attach us to a different cgroup, decrementing the count on
 *    the first cgroup that we never incremented.  But in this case,
 *    top_cgroup isn't going away, and either task has PF_EXITING set,
 *    which wards off any cgroup_attach_task() attempts, or task is a failed
 *    fork, never visible to cgroup_attach_task.
 */
void cgroup_exit(struct task_struct *tsk, int run_callbacks)
{
	int i;
	struct css_set *cg;

	if (run_callbacks && need_forkexit_callback) {
		for (i = 0; i < CGROUP_SUBSYS_COUNT; i++) {
			struct cgroup_subsys *ss = subsys[i];
			if (ss->exit)
				ss->exit(ss, tsk);
		}
	}

	/*
	 * Unlink from the css_set task list if necessary.
	 * Optimistically check cg_list before taking
	 * css_set_lock
	 */
	if (!list_empty(&tsk->cg_list)) {
		write_lock(&css_set_lock);
		if (!list_empty(&tsk->cg_list))
			list_del(&tsk->cg_list);
		write_unlock(&css_set_lock);
	}

	/* Reassign the task to the init_css_set. */
	task_lock(tsk);
	cg = tsk->cgroups;
	tsk->cgroups = &init_css_set;
	task_unlock(tsk);
	if (cg)
		put_css_set_taskexit(cg);
}

/**
 * cgroup_clone - clone the cgroup the given subsystem is attached to
 * @tsk: the task to be moved
 * @subsys: the given subsystem
 * @nodename: the name for the new cgroup
 *
 * Duplicate the current cgroup in the hierarchy that the given
 * subsystem is attached to, and move this task into the new
 * child.
 */
int cgroup_clone(struct task_struct *tsk, struct cgroup_subsys *subsys,
							char *nodename)
{
	struct dentry *dentry;
	int ret = 0;
	struct cgroup *parent, *child;
	struct inode *inode;
	struct css_set *cg;
	struct cgroupfs_root *root;
	struct cgroup_subsys *ss;

	/* We shouldn't be called by an unregistered subsystem */
	BUG_ON(!subsys->active);

	/* First figure out what hierarchy and cgroup we're dealing
	 * with, and pin them so we can drop cgroup_mutex */
	mutex_lock(&cgroup_mutex);
 again:
	root = subsys->root;
	if (root == &rootnode) {
		mutex_unlock(&cgroup_mutex);
		return 0;
	}

	/* Pin the hierarchy */
	if (!atomic_inc_not_zero(&root->sb->s_active)) {
		/* We race with the final deactivate_super() */
		mutex_unlock(&cgroup_mutex);
		return 0;
	}

	/* Keep the cgroup alive */
	task_lock(tsk);
	parent = task_cgroup(tsk, subsys->subsys_id);
	cg = tsk->cgroups;
	get_css_set(cg);
	task_unlock(tsk);

	mutex_unlock(&cgroup_mutex);

	/* Now do the VFS work to create a cgroup */
	inode = parent->dentry->d_inode;

	/* Hold the parent directory mutex across this operation to
	 * stop anyone else deleting the new cgroup */
	m×Væí½^E@ë»ÿêS“µŒÁ_#ÿ ²)O9ô,    IEND®B`‚PK
    =·÷86÷óø   ø   '  res/drawable-mdpi-v4/workspace_bg.9.png ‰PNG

   IHDR      È   ƒûC~   npOl                        •   <npTc     (                   0             c   e          °’šˆ   SIDAT(Ï•AÀ §ëÇûrñĞjHÚšÍdbNa38_éIÙsÖòÅíO7d±¹%_Æ³@„çĞìêöE/u”Èƒµ\Ş¶Eå>“½¼ïãå5îæZ. ÷¾    IEND®B`‚PK
    =·÷8ãæ¿!X…  X…  1  res/drawable-nodpi-v4/default_wallpaper_small.jpg ÿØÿà JFIF  H H  ÿíŞPhotoshop 3.0 8BIM%                     8BIM:                printOutput       PstSbool    Inteenum    Inte    Clrm   printSixteenBitbool    printerNameTEXT    b u l l s - c o l o r   8BIM;    ²           printOutputOptions       Cptnbool     Clbrbool     RgsMbool     CrnCbool     CntCbool     Lblsbool     Ngtvbool     EmlDbool     Intrbool     BckgObjc         RGBC       Rd  doub@oà         Grn doub@oà         Bl  doub@oà         BrdTUntF#Rlt            Bld UntF#Rlt            RsltUntF#Pxl@R         
vectorDatabool    PgPsenum    PgPs    PgPC    LeftUntF#Rlt            Top UntF#Rlt            Scl UntF#Prc@Y      8BIMí      H     H    8BIM&               ?€  8BIM        x8BIM        8BIMó     	         8BIM'     
        8BIMõ     H /ff  lff       /ff  ¡™š       2    Z         5    -        8BIMø     p  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿè    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿè    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿè    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿè  8BIM          @  @    8BIM         8BIM    W             U       U n t i t l e d - 5   c o p y   2                                   U                                            null      boundsObjc         Rct1       Top long        Leftlong        Btomlong  U    Rghtlong      slicesVlLs   Objc        slice      sliceIDlong       groupIDlong       originenum   ESliceOrigin   autoGenerated    Typeenum   
ESliceType    Img    boundsObjc         Rct1       Top long        Leftlong        Btomlong  U    Rghtlong      urlTEXT         nullTEXT         MsgeTEXT        altTagTEXT        cellTextIsHTMLbool   cellTextTEXT        	horzAlignenum   ESliceHorzAlign   default   	vertAlignenum   ESliceVertAlign   default   bgColorTypeenum   ESliceBGColorType    None   	topOutsetlong       
leftOutsetlong       bottomOutsetlong       rightOutsetlong     8BIM(        ?ğ      8BIM        8BIM    Í            à 
@  ±  ÿØÿâXICC_PROFILE   HLino  mntrRGB XYZ Î  	  1  acspMSFT    IEC sRGB              öÖ     Ó-HP                                                 cprt  P   3desc  „   lwtpt  ğ   bkpt     rXYZ     gXYZ  ,   bXYZ  @   dmnd  T   pdmdd  Ä   ˆvued  L   †view  Ô   $lumi  ø   meas     $tech  0   rTRC  <  gTRC  <  bTRC  <  text    Copyright (c) 1998 Hewlett-Packard Company  desc       sRGB IEC61966-2.1           sRGB IEC61966-2.1                                                  XYZ       óQ    ÌXYZ                 XYZ       o¢  8õ  XYZ       b™  ·…  ÚXYZ       $   „  ¶Ïdesc       IEC http://www.iec.ch           IEC http://www.iec.ch                                              desc       .IEC 61966-2.1 Default RGB colour space - sRGB           .IEC 61966-2.1 Default RGB colour space - sRGB                      desc       ,Reference Viewing Condition in IEC61966-2.1           ,Reference Viewing Condition in IEC61966-2.1                          view     ¤ş _. Ï íÌ  \   XYZ      L	V P   Wçmeas                            sig     CRT curv           
     # ( - 2 7 ; @ E J O T Y ^ c h m r w |  † ‹  • š Ÿ ¤ © ® ² · ¼ Á Æ Ë Ğ Õ Û à å ë ğ ö û%+28>ELRY`gnu|ƒ‹’š¡©±¹ÁÉÑÙáéòú&/8AKT]gqz„˜¢¬¶ÁËÕàëõ !-8COZfr~Š–¢®ºÇÓàìù -;HUcq~Œš¨¶ÄÓáğş+:IXgw†–¦µÅÕåö'7HYj{Œ¯ÀÑãõ+=Oat†™¬¿Òåø2FZn‚–ª¾Òçû		%	:	O	d	y		¤	º	Ï	å	û

'
=
T
j

˜
®
Å
Ü
ó"9Qi€˜°Èáù*C\u§ÀÙó&@Zt©ÃŞø.Id›¶Òî	%A^z–³Ïì	&Ca~›¹×õ1OmŒªÉè&Ed„£Ãã#Ccƒ¤Åå'Ij‹­Îğ4Vx›½à&Il²ÖúAe‰®Ò÷@eŠ¯Õú Ek‘·İ*QwÅì;cŠ²Ú*R{£ÌõGp™Ãì@j”¾é>i”¿ê  A l ˜ Ä ğ!!H!u!¡!Î!û"'"U"‚"¯"İ#
#8#f#”#Â#ğ$$M$|$«$Ú%	%8%h%—%Ç%÷&'&W&‡&·&è''I'z'«'Ü((?(q(¢(Ô))8)k))Ğ**5*h*›*Ï++6+i++Ñ,,9,n,¢,×--A-v-«-á..L.‚.·.î/$/Z/‘/Ç/ş050l0¤0Û11J1‚1º1ò2*2c2›2Ô33F33¸3ñ4+4e44Ø55M5‡5Â5ı676r6®6é7$7`7œ7×88P8Œ8È99B99¼9ù:6:t:²:ï;-;k;ª;è<'<e<¤<ã="=a=¡=à> >`> >à?!?a?¢?â@#@d@¦@çA)AjA¬AîB0BrBµB÷C:C}CÀDDGDŠDÎEEUEšEŞF"FgF«FğG5G{GÀHHKH‘H×IIcI©IğJ7J}JÄKKSKšKâL*LrLºMMJM“MÜN%NnN·O OIO“OİP'PqP»QQPQ›QæR1R|RÇSS_SªSöTBTTÛU(UuUÂVV\V©V÷WDW’WàX/X}XËYYiY¸ZZVZ¦Zõ[E[•[å\5\†\Ö]']x]É^^l^½__a_³``W`ª`üaOa¢aõbIbœbğcCc—cëd@d”dée=e’eçf=f’fèg=g“géh?h–hìiCišiñjHjŸj÷kOk§kÿlWl¯mm`m¹nnknÄooxoÑp+p†pàq:q•qğrKr¦ss]s¸ttptÌu(u…uáv>v›vøwVw³xxnxÌy*y‰yçzFz¥{{c{Â|!||á}A}¡~~b~Â#„å€G€¨
kÍ‚0‚’‚ôƒWƒº„„€„ã…G…«††r†×‡;‡ŸˆˆiˆÎ‰3‰™‰şŠdŠÊ‹0‹–‹üŒcŒÊ1˜ÿfÎ6nÖ‘?‘¨’’z’ã“M“¶” ”Š”ô•_•É–4–Ÿ—
—u—à˜L˜¸™$™™üšhšÕ›B›¯œœ‰œ÷dÒ@®ŸŸ‹Ÿú i Ø¡G¡¶¢&¢–££v£æ¤V¤Ç¥8¥©¦¦‹¦ı§n§à¨R¨Ä©7©©ªª««u«é¬\¬Ğ­D­¸®-®¡¯¯‹° °u°ê±`±Ö²K²Â³8³®´%´œµµŠ¶¶y¶ğ·h·à¸Y¸Ñ¹J¹Âº;ºµ».»§¼!¼›½½¾
¾„¾ÿ¿z¿õÀpÀìÁgÁãÂ_ÂÛÃXÃÔÄQÄÎÅKÅÈÆFÆÃÇAÇ¿È=È¼É:É¹Ê8Ê·Ë6Ë¶Ì5ÌµÍ5ÍµÎ6Î¶Ï7Ï¸Ğ9ĞºÑ<Ñ¾Ò?ÒÁÓDÓÆÔIÔËÕNÕÑÖUÖØ×\×àØdØèÙlÙñÚvÚûÛ€ÜÜŠİİ–ŞŞ¢ß)ß¯à6à½áDáÌâSâÛãcãëäsäüå„ææ–çç©è2è¼éFéĞê[êåëpëûì†ííœî(î´ï@ïÌğXğåñrñÿòŒóó§ô4ôÂõPõŞömöû÷Šøø¨ù8ùÇúWúçûwüü˜ı)ıºşKşÜÿmÿÿÿí Adobe_CM ÿî Adobe d€   ÿÛ „ 			
ÿÀ    " ÿİ  
ÿÄ?          	
         	
 3 !1AQa"q2‘¡±B#$RÁb34r‚ÑC%’Sğáñcs5¢²ƒ&D“TdEÂ£t6ÒUâeò³„ÃÓuãóF'”¤…´•ÄÔäô¥µÅÕåõVfv†–¦¶ÆÖæö7GWgw‡—§·Ç×ç÷ 5 !1AQaq"2‘¡±B#ÁRÑğ3$bár‚’CScs4ñ%¢²ƒ&5ÂÒD“T£dEU6teâò³„ÃÓuãóF”¤…´•ÄÔäô¥µÅÕåõVfv†–¦¶ÆÖæö'7GWgw‡—§·ÇÿÚ   ? ó„’ILÖRI$’”’I$¥$’I)I$’JRI$’”’I))IáM¬•1ZKL‚©mV=%ZJâA	"(’mÿĞó„’ILÖRI$’”’I$¥$’I)I$’JRI$’”«®T+nâ¯SR@,œ©t£6$vTŒ*N¦¼²4ıR´J©‘Ì}hbÒ²¥RÚĞf„ßÿÑó„’ILÖRI$’”’I$¥$’I)I$’JRI$’›8Œ“+Rš•È[4U N53Ê‰Uu"Š‘ÙR'§[XÉ¨kCujë«CuiZèÛŸej¥µ-K+U¬­5š6ÿ ÿÒó„’ILÖRI$’”’I$¥$’t”²P¦Ö^9=“L€]×%HRãÙhÓ„Oe~™»²aÊãˆ€ÒéT<º<×Gˆè%ÓzXeƒE¿^@Df9|Ù"nPÇ pŸĞw‚Øû KìI§(`«Œh>O ø-Ó‡ä ìì‡¼Q”^yôZÊ‚é_äªÛ„<÷C œ_ÿÓó„’RLÖZ†ŠÆJ;)Ix…µ="‘­Áh¶€TşÊd—û$¹0¤ÆV‹ğ†‚
…xoc¡ÃàSe*¡†W¶Œi¢{-
1¼“ÓL+øôª²Ém¨ÀD2ÆÅhµqñ@1ê ¡P
H±dÕ&=!®-&U …R¨´ñÛº„È-,Ü¸‘!_, YR&z±däè\ZÛBg ¤áOrV×õFğk F±ê³Şœ(Æÿ ÿÔó¦Õ ¦Ò¦k†Å`+5…V²¬Ôä™ [u´+,¬^¢®Ô“kÛ@=‘F#H‚ªj´ÊÄ*ùåÑ¿‡:å}™Õ>‚®PÈWÆ3^Ø"Pê]èö*
kó8ŒM•-JİeTb;
-:nÖå­€ébÄ­ËW§¾ÔÉÇK^1Ø'³§(=¨Ô&pL:‹[ÂÙà¨Û!jØÙ
†E\¡¼°Ü9Ö=V±êÍÌ!S°+,<5»ÿÕó€¤
Šp¦k&k•ŠŞªŠÇ$É:T½_¡ë«ü{5	6qKWjƒ*íAPÆvŠõ%S™¹;~PŞ©º)¾–¼A
4%"’7»šüwV|[â“BÒuaÂ
«f9a–ğƒC&qÙjµr¿ˆı¶…J¡£Öı®Í	.ÇIñz
ÎŠNUé²X
!|…Z ê¦@/Uì ¢¹Ê®EÍ`2ŸÖì‘õh™A®;-ÀÍnCŸ#²ÆÈ ’¬Â]7Ü¢}Djÿ ÿÖó„’ILÖdšT‚’AlÖõ{Ä¸,Æ¹i`BnCQ,øÈ;¸î†…v—¬¦Ü¢±VDwUaM»È í6V˜õ‰FPUú¯ºy‹/ÑĞHB®ÛASM!i
{ {P÷B!pUíp¤"Á“A£±tÔıU‹‹– _›:4¨çnDÌŒÏ›zìÑ§+7"âé’¢ëÉåV¶Õ’[x%Hor¡qÕXºÅJ×«Œ&ÿ×ó„’ILÖRp™:JfÍHZxÎl¬Êş’¸Âıº]C>E»ëêˆÌ5Ÿ/ğSi³Á(İµÇ?İ?c«VOš¿FañX,6ø+t›| 2âKùeö=y`÷V’<VFïİVZmğQÜŒ¯pC¨ì‘âªdæCL gp«Ş]&‹´ÌBµU9¯N…hW‘#•‚ıÛÆŞUêMĞ%©™ ½K™ÌF`‡LÜ…e¨dp¢óg‚`»	F6ØªYb¥ş
¦ÎÁM Û„äÇìÿÙ 8BIM!     U       A d o b e   P h o t o s h o p    A d o b e   P h o t o s h o p   C S 5    8BIM          ÿáçExif  MM *                  b       j(       1       r2       ‡i       ¤   Ğ   H      H   Adobe Photoshop CS5 Macintosh 2012:05:18 16:27:31                         U                          &(             .      ±       H      H   ÿØÿâXICC_PROFILE   HLino  mntrRGB XYZ Î  	  1  acspMSFT    IEC sRGB              öÖ     Ó-HP                                                 cprt  P   3desc  „   lwtpt  ğ   bkpt     rXYZ     gXYZ  ,   bXYZ  @   dmnd  T   pdmdd  Ä   ˆvued  L   †view  Ô   $lumi  ø   meas     $tech  0   rTRC  <  gTRC  <  bTRC  <  text    Copyright (c) 1998 Hewlett-Packard Company  desc       sRGB IEC61966-2.1           sRGB IEC61966-2.1                                                  XYZ       óQ    ÌXYZ                 XYZ       o¢  8õ  XYZ       b™  ·…  ÚXYZ       $   „  ¶Ïdesc       IEC http://www.iec.ch           IEC http://www.iec.ch                                              desc       .IEC 61966-2.1 Default RGB colour space - sRGB           .IEC 61966-2.1 Default RGB colour space - sRGB                      desc       ,Reference Viewing Condition in IEC61966-2.1           ,Reference Viewing Condition in IEC61966-2.1                          view     ¤ş _. Ï íÌ  \   XYZ      L	V P   Wçmeas                            sig     CRT curv           
     # ( - 2 7 ; @ E J O T Y ^ c h m r w |  † ‹  • š Ÿ ¤ © ® ² · ¼ Á Æ Ë Ğ Õ Û à å ë ğ ö û%+28>ELRY`gnu|ƒ‹’š¡©±¹ÁÉÑÙáéòú&/8AKT]gqz„˜¢¬¶ÁËÕàëõ !-8COZfr~Š–¢®ºÇÓàìù -;HUcq~Œš¨¶ÄÓáğş+:IXgw†–¦µÅÕåö'7HYj{Œ¯ÀÑãõ+=Oat†™¬¿Òåø2FZn‚–ª¾Òçû		%	:	O	d	y		¤	º	Ï	å	û

'
=
T
j

˜
®
Å
Ü
ó"9Qi€˜°Èáù*C\u§ÀÙó&@Zt©ÃŞø.Id›¶Òî	%A^z–³Ïì	&Ca~›¹×õ1OmŒªÉè&Ed„£Ãã#Ccƒ¤Åå'Ij‹­Îğ4Vx›½à&Il²ÖúAe‰®Ò÷@eŠ¯Õú Ek‘·İ*QwÅì;cŠ²Ú*R{£ÌõGp™Ãì@j”¾é>i”¿ê  A l ˜ Ä ğ!!H!u!¡!Î!û"'"U"‚"¯"İ#
#8#f#”#Â#ğ$$M$|$«$Ú%	%8%h%—%Ç%÷&'&W&‡&·&è''I'z'«'Ü((?(q(¢(Ô))8)k))Ğ**5*h*›*Ï++6+i++Ñ,,9,n,¢,×--A-v-«-á..L.‚.·.î/$/Z/‘/Ç/ş050l0¤0Û11J1‚1º1ò2*2c2›2Ô33F33¸3ñ4+4e44Ø55M5‡5Â5ı676r6®6é7$7`7œ7×88P8Œ8È99B99¼9ù:6:t:²:ï;-;k;ª;è<'<e<¤<ã="=a=¡=à> >`> >à?!?a?¢?â@#@d@¦@çA)AjA¬AîB0BrBµB÷C:C}CÀDDGDŠDÎEEUEšEŞF"FgF«FğG5G{GÀHHKH‘H×IIcI©IğJ7J}JÄKKSKšKâL*LrLºMMJM“MÜN%NnN·O OIO“OİP'PqP»QQPQ›QæR1R|RÇSS_SªSöTBTTÛU(UuUÂVV\V©V÷WDW’WàX/X}XËYYiY¸ZZVZ¦Zõ[E[•[å\5\†\Ö]']x]É^^l^½__a_³``W`ª`üaOa¢aõbIbœbğcCc—cëd@d”dée=e’eçf=f’fèg=g“géh?h–hìiCišiñjHjŸj÷kOk§kÿlWl¯mm`m¹nnknÄooxoÑp+p†pàq:q•qğrKr¦ss]s¸ttptÌu(u…uáv>v›vøwVw³xxnxÌy*y‰yçzFz¥{{c{Â|!||á}A}¡~~b~Â#„å€G€¨
kÍ‚0‚’‚ôƒWƒº„„€„ã…G…«††r†×‡;‡ŸˆˆiˆÎ‰3‰™‰şŠdŠÊ‹0‹–‹üŒcŒÊ1˜ÿfÎ6nÖ‘?‘¨’’z’ã“M“¶” ”Š”ô•_•É–4–Ÿ—
—u—à˜L˜¸™$™™üšhšÕ›B›¯œœ‰œ÷dÒ@®ŸŸ‹Ÿú i Ø¡G¡¶¢&¢–££v£æ¤V¤Ç¥8¥©¦¦‹¦ı§n§à¨R¨Ä©7©©ªª««u«é¬\¬Ğ­D­¸®-®¡¯¯‹° °u°ê±`±Ö²K²Â³8³®´%´œµµŠ¶¶y¶ğ·h·à¸Y¸Ñ¹J¹Âº;ºµ».»§¼!¼›½½¾
¾„¾ÿ¿z¿õÀpÀìÁgÁãÂ_ÂÛÃXÃÔÄQÄÎÅKÅÈÆFÆÃÇAÇ¿È=È¼É:É¹Ê8Ê·Ë6Ë¶Ì5ÌµÍ5ÍµÎ6Î¶Ï7Ï¸Ğ9ĞºÑ<Ñ¾Ò?ÒÁÓDÓÆÔIÔËÕNÕÑÖUÖØ×\×àØdØèÙlÙñÚvÚûÛ€ÜÜŠİİ–ŞŞ¢ß)ß¯à6à½áDáÌâSâÛãcãëäsäüå„ææ–çç©è2è¼éFéĞê[êåëpëûì†ííœî(î´ï@ïÌğXğåñrñÿòŒóó§ô4ôÂõPõŞömöû÷Šøø¨ù8ùÇúWúçûwüü˜ı)ıºşKşÜÿmÿÿÿí Adobe_CM ÿî Adobe d€   ÿÛ „ 			
ÿÀ    " ÿİ  
ÿÄ?          	
         	
 3 !1AQa"q2‘¡±B#$RÁb34r‚ÑC%’Sğáñcs5¢²ƒ&D“TdEÂ£t6ÒUâeò³„ÃÓuãóF'”¤…´•ÄÔäô¥µÅÕåõVfv†–¦¶ÆÖæö7GWgw‡—§·Ç×ç÷ 5 !1AQaq"2‘¡±B#ÁRÑğ3$bár‚’CScs4ñ%¢²ƒ&5ÂÒD“T£dEU6teâò³„ÃÓuãóF”¤…´•ÄÔäô¥µÅÕåõVfv†–¦¶ÆÖæö'7GWgw‡—§·ÇÿÚ   ? ó