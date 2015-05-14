cmd_t burst)
{
	struct stv0288_state *state = fe->demodulator_priv;

	dprintk("%s\n", __func__);

	if (stv0288_writeregI(state, 0x05, 0x16))/* burst mode */
		return -EREMOTEIO;

	if (stv0288_writeregI(state, 0x06, burst == SEC_MINI_A ? 0x00 : 0xff))
		return -EREMOTEIO;

	if (stv0288_writeregI(state, 0x06, 0x12))
		return -EREMOTEIO;

	return 0;
}

static int stv0288_set_tone(struct dvb_frontend *fe, fe_sec_tone_mode_t tone)
{
	struct stv0288_state *state = fe->demodulator_priv;

	switch (tone) {
	case SEC_TONE_ON:
		if (stv0288_writeregI(state, 0x05, 0x10))/* burst mode */
			return -EREMOTEIO;
		return stv0288_writeregI(state, 0x06, 0xff);

	case SEC_TONE_OFF:
		if (stv0288_writeregI(state, 0x05, 0x13))/* burst mode */
			return -EREMOTEIO;
		return stv0288_writeregI(state, 0x06, 0x00);

	default:
		return -EINVAL;
	}
}

static u8 stv0288_inittab[] = {
	0x01, 0x15,
	0x02, 0x20,
	0x09, 0x0,
	0x0a, 0x4,
	0x0b, 0x0,
	0x0c, 0x0,
	0x0d, 0x0,
	0x0e, 0xd4,
	0x0f, 0x30,
	0x11, 0x80,
	0x12, 0x03,
	0x13, 0x48,
	0x14, 0x84,
	0x15, 0x45,
	0x16, 0xb7,
	0x17, 0x9c,
	0x18, 0x0,
	0x19, 0xa6,
	0x1a, 0x88,
	0x1b, 0x8f,
	0x1c, 0xf0,
	0x20, 0x0b,
	0x21, 0x54,
	0x22, 0x0,
	0x23, 0x0,
	0x2b, 0xff,
	0x2c, 0xf7,
	0x30, 0x0,
	0x31, 0x1e,
	0x32, 0x14,
	0x33, 0x0f,
	0x34, 0x09,
	0x35, 0x0c,
	0x36, 0x05,
	0x37, 0x2f,
	0x38, 0x16,
	0x39, 0xbe,
	0x3a, 0x0,
	0x3b, 0x13,
	0x3c, 0x11,
	0x3d, 0x30,
	0x40, 0x63,
	0x41, 0x04,
	0x42, 0x60,
	0x43, 0x00,
	0x44, 0x00,
	0x45, 0x00,
	0x46, 0x00,
	0x47, 0x00,
	0x4a, 0x00,
	0x50, 0x10,
	0x51, 0x38,
	0x52, 0x21,
	0x58, 0x54,
	0x59, 0x86,
	0x5a, 0x0,
	0x5b, 0x9b,
	0x5c, 0x08,
	0x5d, 0x7f,
	0x5e, 0x0,
	0x5f, 0xff,
	0x70, 0x0,
	0x71, 0x0,
	0x72, 0x0,
	0x74, 0x0,
	0x75, 0x0,
	0x76, 0x0,
	0x81, 0x0,
	0x82, 0x3f,
	0x83, 0x3f,
	0x84, 0x0,
	0x85, 0x0,
	0x88, 0x0,
	0x89, 0x0,
	0x8a, 0x0,
	0x8b, 0x0,
	0x8c, 0x0,
	0x90, 0x0,
	0x91, 0x0,
	0x92, 0x0,
	0x93, 0x0,
	0x94, 0x1c,
	0x97, 0x0,
	0xa0, 0x48,
	0xa1, 0x0,
	0xb0, 0xb8,
	0xb1, 0x3a,
	0xb2, 0x10,
	0xb3, 0x82,
	0xb4, 0x80,
	0xb5, 0x82,
	0xb6, 0x82,
	0xb7, 0x82,
	0xb8, 0x20,
	0xb9, 0x0,
	0xf0, 0x0,
	0xf1, 0x0,
	0xf2, 0xc0,
	0x51, 0x36,
	0x52, 0x09,
	0x53, 0x94,
	0x54, 0x62,
	0x55, 0x29,
	0x56, 0x64,
	0x57, 0x2b,
	0xff, 0xff,
};

static int stv0288_set_voltage(struct dvb_frontend *fe, fe_sec_voltage_t volt)
{
	dprintk("%s: %s\n", __func__,
		volt == SEC_VOLTAGE_13 ? "SEC_VOLTAGE_13" :
		volt == SEC_VOLTAGE_18 ? "SEC_VOLTAGE_18" : "??");

	return 0;
}

static int stv0288_init(struct dvb_frontend *fe)
{
	struct stv0288_state *state = fe->demodulator_priv;
	int i;
	u8 reg;
	u8 val;

	dprintk("stv0288: init chip\n");
	stv0288_writeregI(state, 0x41, 0x04);
	msleep(50);

	/* we have default inittab */
	if (state->config->inittab == NULL) {
		for (i = 0; !(stv0288_inittab[i] == 0xff &&
				stv0288_inittab[i + 1] == 0xff); i += 2)
			stv0288_writeregI(state, stv0288_inittab[i],
					stv0288_inittab[i + 1]);
	} else {
		for (i = 0; ; i += 2)  {
			reg = state->config->inittab[i];
			val = state->config->inittab[i+1];
			if (reg == 0xff && val == 0xff)
				break;
			stv0288_writeregI(state, reg, val);
		}
	}
	return 0;
}

static int stv0288_read_status(struct dvb_frontend *fe, fe_status_t *status)
{
	struct stv0288_state *state = fe->demodulator_priv;

	u8 sync = stv0288_readreg(state, 0x24);
	if (sync == 255)
		sync = 0;

	dprintk("%s : FE_READ_STATUS : VSTATUS: 0x%02x\n", __func__, sync);

	*status = 0;

	if ((sync & 0x08) == 0x08) {
		*status |= FE_HAS_LOCK;
		dprintk("stv0288 has locked\n");
	}

	return 0;
}

static int stv0288_read_ber(struct dvb_frontend *fe, u32 *ber)
{
	struct stv0288_state *state = fe->demodulator_priv;

	if (state->errmode != STATUS_BER)
		return 0;
	*ber = (stv0288_readreg(state, 0x26) << 8) |
					stv0288_readreg(state, 0x27);
	dprintk("stv0288_read_ber %d\n", *ber);

	return 0;
}


static int stv0288_read_signal_strength(struct dvb_frontend *fe, u16 *strength)
{
	struct stv0288_state *state = fe->demodulator_priv;

	s32 signal =  0xffff - ((stv0288_readreg(state, 0x10) << 8));


	signal = signal * 5 / 4;
	*strength = (signal > 0xffff) ? 0xffff : (signal < 0) ? 0 : signal;
	dprintk("stv0288_read_signal_strength %d\n", *strength);

	return 0;
}
static int stv0288_sleep(struct dvb_frontend *fe)
{
	struct stv0288_state *state = fe->demodulator_priv;

	stv0288_writeregI(state, 0x41, 0x84);
	state->initialised = 0;

	return 0;
}
static int stv0288_read_snr(struct dvb_frontend *fe, u16 *snr)
{
	struct stv0288_state *state = fe->demodulator_priv;

	s32 xsnr = 0xffff - ((stv0288_readreg(state, 0x2d) << 8)
			   | stv0288_readreg(state, 0x2e));
	xsnr = 3 * (xsnr - 0xa100);
	*snr = (xsnr > 0xffff) ? 0xffff : (xsnr < 0) ? 0 : xsnr;
	dprintk("stv0288_read_snr %d\n", *snr);

	return 0;
}

static int stv0288_read_ucblocks(struct dvb_frontend *fe, u32 *ucblocks)
{
	struct stv0288_state *state = fe->demodulator_priv;

	if (state->errmode != STATUS_BER)
		return 0;
	*ucblocks = (stv0288_readreg(state, 0x26) << 8) |
					stv0288_readreg(state, 0x27);
	dprintk("stv0288_read_ber %d\n", *ucblocks);

	return 0;
}

static int stv0288_set_property(struct dvb_frontend *fe, struct dtv_property *p)
{
	dprintk("%s(..)\n", __func__);
	return 0;
}

static int stv0288_get_property(struct dvb_frontend *fe, struct dtv_property *p)
{
	dprintk("%s(..)\n", __func__);
	return 0;
}

static int stv0288_set_frontend(struct dvb_frontend *fe,
					struct dvb_frontend_parameters *dfp)
{
	struct stv0288_state *state = fe->demodulator_priv;
	struct dtv_frontend_properties *c = &fe->dtv_property_cache;

	char tm;
	unsigned char tda[3];

	dprintk("%s : FE_SET_FRONTEND\n", __func__);

	if (c->delivery_system != SYS_DVBS) {
			dprintk("%s: unsupported delivery "
				"system selected (%d)\n",
				__func__, c->delivery_system);
			return -EOPNOTSUPP;
	}

	if (state->config->set_ts_params)
		state->config->set_ts_params(fe, 0);

	/* only frequency & symbol_rate are used for tuner*/
	dfp->frequency = c->frequency;
	dfp->u.qpsk.symbol_rate = c->symbol_rate;
	if (fe->ops.tuner_ops.set_params) {
		fe->ops.tuner_ops.set_params(fe, dfp);
		if (fe->ops.i2c_gate_ctrl)
			fe->ops.i2c_gate_ctrl(fe, 0);
	}

	udelay(10);
	stv0288_set_symbolrate(fe, c->symbol_rate);
	/* Carrier lock control register */
	stv0288_writeregI(state, 0x15, 0xc5);

	tda[0] = 0x2b; /* CFRM */
	tda[2] = 0x0; /* CFRL */
	for (tm = -6; tm < 7;) {
		/* Viterbi status */
		if (stv0288_readreg(state, 0x24) & 0x80)
			break;

		tda[2] += 40;
		if (tda[2] < 40)
			tm++;
		tda[1] = (unsigned char)tm;
		stv0288_writeregI(state, 0x2b, tda[1]);
		stv0288_writeregI(state, 0x2c, tda[2]);
		udelay(30);
	}

	state->tuner_frequency = c->frequency;
	state->fec_inner = FEC_AUTO;
	state->symbol_rate = c->symbol_rate;

	return 0;
}

static int stv0288_i2c_gate_ctrl(struct dvb_frontend *fe, int enable)
{
	struct stv0288_state *state = fe->demodulator_priv;

	if (enable)
		stv0288_writeregI(state, 0x01, 0xb5);
	else
		stv0288_writeregI(state, 0x01, 0x35);

	udelay(1);

	return 0;
}

static void stv0288_release(struct dvb_frontend *fe)
{
	struct stv0288_state *state = fe->demodulator_priv;
	kfree(state);
}

static struct dvb_frontend_ops stv0288_ops = {

	.info = {
		.name			= "ST STV0288 DVB-S",
		.type			= FE_QPSK,
		.frequency_min		= 950000,
		.frequency_max		= 2150000,
		.frequency_stepsize	= 1000,	 /* kHz for QPSK frontends */
		.frequency_tolerance	= 0,
		.symbol_rate_min	= 1000000,
		.symbol_rate_max	= 45000000,
		.symbol_rate_tolerance	= 500,	/* ppm */
		.caps = FE_CAN_FEC_1_2 | FE_CAN_FEC_2_3 | FE_CAN_FEC_3_4 |
		      FE_CAN_FEC_5_6 | FE_CAN_FEC_7_8 |
		      FE_CAN_QPSK |
		      FE_CAN_FEC_AUTO
	},

	.release = stv0288_release,
	.init = stv0288_init,
	.sleep = stv0288_sleep,
	.write = stv0288_write,
	.i2c_gate_ctrl = stv0288_i2c_gate_ctrl,
	.read_status = stv0288_read_status,
	.read_ber = stv0288_read_ber,
	.read_signal_strength = stv0288_read_signal_strength,
	.read_snr = stv0288_read_snr,
	.read_ucblocks = stv0288_read_ucblocks,
	.diseqc_send_master_cmd = stv0288_send_diseqc_msg,
	.diseqc_send_burst = stv0288_send_diseqc_burst,
	.set_tone = stv0288_set_tone,
	.set_voltage = stv0288_set_voltage,

	.set_property = stv0288_set_property,
	.get_property = stv0288_get_property,
	.set_frontend = stv0288_set_frontend,
};

struct dvb_frontend *stv0288_attach(const struct stv0288_config *config,
				    struct i2c_adapter *i2c)
{
	struct stv0288_state *state = NULL;
	int id;

	/* allocate memory for the internal state */
	state = kzalloc(sizeof(struct stv0288_state), GFP_KERNEL);
	if (state == NULL)
		goto error;

	/* setup the state */
	state->config = config;
	state->i2c = i2c;
	state->initialised = 0;
	state->tuner_frequency = 0;
	state->symbol_rate = 0;
	state->fec_inner = 0;
	state->errmode = STATUS_BER;

	stv0288_writeregI(state, 0x41, 0x04);
	msleep(200);
	id = stv0288_readreg(state, 0x00);
	dprintk("stv0288 id %x\n", id);

	/* register 0x00 contains 0x11 for STV0288  */
	if (id != 0x11)
		goto error;

	/* create dvb_frontend */
	memcpy(&state->frontend.ops, &stv0288_ops,
			sizeof(struct dvb_frontend_ops));
	state->frontend.demodulator_priv = state;
	return &state->frontend;

error:
	kfree(state);

	return NULL;
}
EXPORT_SYMBOL(stv0288_attach);

module_param(debug_legacy_dish_switch, int, 0444);
MODULE_PARM_DESC(debug_legacy_dish_switch,
		"Enable timing analysis for Dish Network legacy switches");

module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "Turn on/off frontend debugging (default:off).");

MODULE_DESCRIPTION("ST STV0288 DVB Demodulator driver");
MODULE_AUTHOR("Georg Acher, Bob Liu, Igor liplianin");
MODULE_LICENSE("GPL");

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              /*
 * Copyright 2004-2007 Freescale Semiconductor, Inc. All Rights Reserved.
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
 * @file mx27_prphw.c
 *
 * @brief MX27 Video For Linux 2 capture driver
 *
 * @ingroup MXC_V4L2_CAPTURE
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <linux/delay.h>

#include "mx27_prp.h"

#define PRP_MIN_IN_WIDTH	32
#define PRP_MAX_IN_WIDTH	2044
#define PRP_MIN_IN_HEIGHT	32
#define PRP_MAX_IN_HEIGHT	2044

typedef struct _coeff_t {
	unsigned long coeff[2];
	unsigned long cntl;
} coeff_t[2][2];

static coeff_t *PRP_RSZ_COEFF = (coeff_t *) PRP_CH1_RZ_HORI_COEF1;

static unsigned char scale_get(scale_t * t,
			       unsigned char *i, unsigned char *out);
static int gcd(int x, int y);
static int ratio(int x, int y, int *den);
static int prp_scale_bilinear(scale_t * t, int coeff, int base, int nxt);
static int prp_scale_ave(scale_t * t, unsigned char base);
static int ave_scale(scale_t * t, int inv, int outv);
static int scale(scale_t * t, int inv, int outv);

/*!
 * @param t	table
 * @param i	table index
 * @param out	bilinear	# input pixels to advance
 *		average		whether result is ready for output
 * @return	coefficient
*/
static unsigned char scale_get(scale_t * t, unsigned char *i,
			       unsigned char *out)
{
	unsigned char c;

	c = t->tbl[*i];
	(*i)++;
	*i %= t->len;

	if (out) {
		if (t->algo == ALGO_BIL) {
			for ((*out) = 1;
			     (*i) && ((*i) < t->len) && !t->tbl[(*i)]; (*i)++) {
				(*out)++;
			}
			if ((*i) == t->len)
				(*i) = 0;
		} else
			*out = c >> BC_COEF;
	}

	c &= SZ_COEF - 1;

	if (c == SZ_COEF - 1)
		c = SZ_COEF;

	return c;
}

/*!
 * @brief Get maximum common divisor.
 * @param x	First input value
 * @param y	Second input value
 * @return	Maximum common divisor of x and y
 */
static int gcd(int x, int y)
{
	int k;

	if (x < y) {
		k = x;
		x = y;
		y = k;
	}

	while ((k = x % y)) {
		x = y;
		y = k;
	}

	return y;
}

/*!
 * @brief Get ratio.
 * @param x	First input value
 * @param y	Second input value
 * @param den	Denominator of the ratio (corresponding to y)
 * @return	Numerator of the ratio (corresponding to x)
 */
static int ratio(int x, int y, int *den)
{
	int g;

	if (!x || !y)
		return 0;

	g = gcd(x, y);
	*den = y / g;

	return x / g;
}

/*!
 * @brief Build PrP coefficient entry based on bilinear algorithm
 *
 * @param t	The pointer to scale_t structure
 * @param coeff	The weighting coefficient
 * @param base	The base of the coefficient
 * @param nxt	Number of pixels to be read
 *
 * @return	The length of current coefficient table on success
 *		-1 on failure
 */
static int prp_scale_bilinear(scale_t * t, int coeff, int base, int nxt)
{
	int i;

	if (t->len >= sizeof(t->tbl))
		return -1;

	coeff = ((coeff << BC_COEF) + (base >> 1)) / base;
	if (coeff >= SZ_COEF - 1)
		coeff--;

	coeff |= SZ_COEF;
	t->tbl[(int)t->len++] = (unsigned char)coeff;

	for (i = 1; i < nxt; i++) {
		if (t->len >= MAX_TBL)
			return -1;

		t->tbl[(int)t->len++] = 0;
	}

	return t->len;
}

#define _bary(name)	static const unsigned char name[]

_bary(c1) = {
7};

_bary(c2) = {
4, 4};

_bary(c3) = {
2, 4, 2};

_bary(c4) = {
2, 2, 2, 2};

_bary(c5) = {
1, 2, 2, 2, 1};

_bary(c6) = {
1, 1, 2, 2, 1, 1};

_bary(c7) = {
1, 1, 1, 2, 1, 1, 1};

_bary(c8) = {
1, 1, 1, 1, 1, 1, 1, 1};

_bary(c9) = {
1, 1, 1, 1, 1, 1, 1, 1, 0};

_bary(c10) = {
0, 1, 1, 1, 1, 1, 1, 1, 1, 0};

_bary(c11) = {
0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0};

_bary(c12) = {
0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0};

_bary(c13) = {
0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0};

_bary(c14) = {
0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0};

_bary(c15) = {
0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0};

_bary(c16) = {
1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0};

_bary(c17) = {
0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0};

_bary(c18) = {
0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0};

_bary(c19) = {
0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0};

_bary(c20) = {
0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0};

static const unsigned char *ave_coeff[] = {
	c1, c2, c3, c4, c5, c6, c7, c8, c9, c10,
	c11, c12, c13, c14, c15, c16, c17, c18, c19, c20
};

/*!
 * @brief Build PrP coefficient table based on average algorithm
 *
 * @param t	The pointer to scale_t structure
 * @param base	The base of the coefficient
 *
 * @return	The length of current coefficient table on success
 *		-1 on failure
 */
static int prp_scale_ave(scale_t * t, unsigned char base)
{
	if (t->len + base > sizeof(t->tbl))
		return -1;

	memcpy(&t->tbl[(int)t->len], ave_coeff[(int)base - 1], base);
	t->len = (unsigned char)(t->len + base);
	t->tbl[t->len - 1] |= SZ_COEF;

	return t->len;
}

/*!
 * @brief Build PrP coefficient table based on average algorithm
 *
 * @param t	The pointer to scale_t structure
 * @param inv	Input resolution
 * @param outv	Output resolution
 *
 * @return	The length of current coefficient table on success
 *		-1 on failure
 */
static int ave_scale(scale_t * t, int inv, int outv)
{
	int ratio_count;

	ratio_count = 0;
	if (outv != 1) {
		unsigned char a[20];
		int v;

		/* split n:m into multiple n[i]:1 */
		for (v = 0; v < outv; v++)
			a[v] = (unsigned char)(inv / outv);

		inv %= outv;
		if (inv) {
			/* find start of next layer */
			v = (outv - inv) >> 1;
			inv += v;
			for (; v < inv; v++)
				a[v]++;
		}

		for (v = 0; v < outv; v++) {
			if (prp_scale_ave(t, a[v]) < 0)
				return -1;

			t->ratio[ratio_count] = a[v];
			ratio_count++;
		}
	} else if (prp_scale_ave(t, inv) < 0) {
		return -1;
	} else {
		t->ratio[ratio_count++] = (char)inv;
		ratio_count++;
	}

	return t->len;
}

/*!
 * @brief Build PrP coefficient table
 *
 * @param t	The pointer to scale_t structure
 * @param inv	input resolution reduced ratio
 * @param outv	output resolution reduced ratio
 *
 * @return	The length of current coefficient table on success
 *		-1 on failure
 */
static int scale(scale_t * t, int inv, int outv)
{
	int v;			/* overflow counter */
	int coeff, nxt;		/* table output */

	t->len = 0;
	if (t->algo == ALGO_AUTO) {
		/* automatic choice - bilinear for shrinking less than 2:1 */
		t->algo = ((outv != inv) && ((2 * outv) > inv)) ?
		    ALGO_BIL : ALGO_AVG;
	}

	/* 1:1 resize must use averaging, bilinear will hang */
	if ((inv == outv) && (t->algo == ALGO_BIL)) {
		pr_debug("Warning: 1:1 resize must use averaging algo\n");
		t->algo = ALGO_AVG;
	}

	memset(t->tbl, 0, sizeof(t->tbl));
	if (t->algo == ALGO_BIL) {
		t->ratio[0] = (char)inv;
		t->ratio[1] = (char)outv;
	} else
		memset(t->ratio, 0, sizeof(t->ratio));

	if (inv == outv) {
		/* force scaling */
		t->ratio[0] = 1;
		if (t->algo == ALGO_BIL)
			t->ratio[1] = 1;

		return prp_scale_ave(t, 1);
	}

	if (inv < outv) {
		pr_debug("Upscaling not supported %d:%d\n", inv, outv);
		return -1;
	}

	if (t->algo != ALGO_BIL)
		return ave_scale(t, inv, outv);

	v = 0;
	if (inv >= 2 * outv) {
		/* downscale: >=2:1 bilinear approximation */
		coeff = inv - 2 * outv;
		v = 0;
		nxt = 0;
		do {
			v += coeff;
			nxt = 2;
			while (v >= outv) {
				v -= outv;
				nxt++;
			}

			if (prp_scale_bilinear(t, 1, 2, nxt) < 0)
				return -1;
		} while (v);
	} else {
		/* downscale: bilinear */
		int in_pos_inc = 2 * outv;
		int out_pos = inv;
		int out_pos_inc = 2 * inv;
		int init_carry = inv - outv;
		int carry = init_carry;

		v = outv + in_pos_inc;
		do {
			coeff = v - out_pos;
			out_pos += out_pos_inc;
			carry += out_pos_inc;
			for (nxt = 0; v < out_pos; nxt++) {
				v += in_pos_inc;
				carry -= in_pos_inc;
			}
			if (prp_scale_bilinear(t, coeff, in_pos_inc, nxt) < 0)
				return -1;
		} while (carry != init_carry);
	}
	return t->len;
}

/*!
 * @brief Build PrP coefficient table
 *
 * @param pscale	The pointer to scale_t structure which holdes
 * 			coefficient tables
 * @param din		Scale ratio numerator
 * @param dout		Scale ratio denominator
 * @param inv		Input resolution
 * @param vout		Output resolution
 * @param pout		Internal output resolution
 * @param retry		Retry times (round the output length) when need
 *
 * @return		Zero on success, others on failure
 */
int prp_scale(scale_t * pscale, int din, int dout, int inv,
	      unsigned short *vout, unsigned short *pout, int retry)
{
	int num;
	int den;
	unsigned short outv;

	/* auto-generation of values */
	if (!(dout && din)) {
		if (!*vout)
			dout = din = 1;
		else {
			din = inv;
			dout = *vout;
		}
	}

	if (din < dout) {
		pr_debug("Scale err, unsupported ratio %d : %d\n", din, dout);
		return -1;
	}

      lp_retry:
	num = ratio(din, dout, &den);
	if (!num) {
		pr_debug("Scale err, unsupported ratio %d : %d\n", din, dout);
		return -1;
	}

	if (num > MAX_TBL || scale(pscale, num, den) < 0) {
		dout++;
		if (retry--)
			goto lp_retry;

		pr_debug("Scale err, unsupported ratio %d : %d\n", num, den);
		return -1;
	}

	if (pscale->algo == ALGO_BIL) {
		unsigned char i, j, k;

		outv =
		    (unsigned short)(inv / pscale->ratio[0] * pscale->ratio[1]);
		inv %= pscale->ratio[0];
		fo