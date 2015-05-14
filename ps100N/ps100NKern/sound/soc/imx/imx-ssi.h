, 0, 1, 0, 1, 0};

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
		for (i = j = 0; inv > 0; j++) {
			unsigned char nxt;

			k = scale_get(pscale, &i, &nxt);
			if (inv == 1 && k < SZ_COEF) {
				/* needs 2 pixels for this output */
				break;
			}
			inv -= nxt;
		}
		outv = outv + j;
	} else {
		unsigned char i, tot;

		for (tot = i = 0; pscale->ratio[i]; i++)
			tot = tot + pscale->ratio[i];

		outv = (unsigned short)(inv / tot) * i;
		inv %= tot;
		for (i = 0; inv > 0; i++, outv++)
			inv -= pscale->ratio[i];
	}

	if (!(*vout) || ((*vout) > outv))
		*vout = outv;

	if (pout)
		*pout = outv;

	return 0;
}

/*!
 * @brief Reset PrP block
 */
int prphw_reset(void)
{
	unsigned long val;
	unsigned long flag;
	int i;

	flag = PRP_CNTL_RST;
	val = PRP_CNTL_RSTVAL;

	__raw_writel(flag, PRP_CNTL);

	/* timeout */
	for (i = 0; i < 1000; i++) {
		if (!(__raw_readl(PRP_CNTL) & flag)) {
			pr_debug("PrP reset over\n");
			break;
		}
		msleep(1);
	}

	/* verify reset value */
	if (__raw_readl(PRP_CNTL) != val) {
		pr_info("PrP reset err, val = 0x%08X\n", __raw_readl(PRP_CNTL));
		return -1;
	}

	return 0;
}

/*!
 * @brief Enable PrP channel.
 * @param channel	Channel number to be enabled
 * @return		Zero on success, others on failure
 */
int prphw_enable(int channel)
{
	unsigned long val;

	val = __raw_readl(PRP_CNTL);
	if (channel & PRP_CHANNEL_1)
		val |= PRP_CNTL_CH1EN;
	if (channel & PRP_CHANNEL_2)
		val |= (PRP_CNTL_CH2EN | PRP_CNTL_CH2_FLOWEN);

	__raw_writel(val, PRP_CNTL);

	return 0;
}

/*!
 * @brief Disable PrP channel.
 * @param channel	Channel number to be disable
 * @return		Zero on success, others on failure
 */
int prphw_disable(int channel)
{
	unsigned long val;

	val = __raw_readl(PRP_CNTL);
	if (channel & PRP_CHANNEL_1)
		val &= ~PRP_CNTL_CH1EN;
	if (channel & PRP_CHANNEL_2)
		val &= ~(PRP_CNTL_CH2EN | PRP_CNTL_CH2_FLOWEN);

	__raw_writel(val, PRP_CNTL);

	return 0;
}

/*!
 * @brief Set PrP input buffer address.
 * @param cfg	Pointer to PrP configuration parameter
 * @return	Zero on success, others on failure
 */
int prphw_inptr(emma_prp_cfg * cfg)
{
	if (cfg->in_csi & PRP_CSI_EN)
		return -1;

	__raw_writel(cfg->in_ptr, PRP_SOURCE_Y_PTR);
	if (cfg->in_pix == PRP_PIXIN_YUV420) {
		u32 size;

		size = cfg->in_line_stride * cfg->in_height;
		__raw_writel(cfg->in_ptr + size, PRP_SOURCE_CB_PTR);
		__raw_writel(cfg->in_ptr + size + (size >> 2),
			     PRP_SOURCE_CR_PTR);
	}
	return 0;
}

/*!
 * @brief Set PrP channel 1 output buffer 1 address.
 * @param cfg	Pointer to PrP configuration parameter
 * @return	Zero on success, others on failure
 */
int prphw_ch1ptr(emma_prp_cfg * cfg)
{
	if (cfg->ch1_pix == PRP_PIX1_UNUSED)
		return -1;

	__raw_writel(cfg->ch1_ptr, PRP_DEST_RGB1_PTR);

	/* support double buffer in loop mode only */
	if ((cfg->in_csi & PRP_CSI_LOOP) == PRP_CSI_LOOP) {
		if (cfg->ch1_ptr2)
			__raw_writel(cfg->ch1_ptr2, PRP_DEST_RGB2_PTR);
		else
			__raw_writel(cfg->ch1_ptr, PRP_DEST_RGB2_PTR);
	}

	return 0;
}

/*!
 * @brief Set PrP channel 1 output buffer 2 address.
 * @param cfg	Pointer to PrP configuration parameter
 * @return	Zero on success, others on failure
 */
int prphw_ch1ptr2(emma_prp_cfg * cfg)
{
	if (cfg->ch1_pix == PRP_PIX1_UNUSED ||
	    (cfg->in_csi & PRP_CSI_LOOP) != PRP_CSI_LOOP)
		return -1;

	if (cfg->ch1_ptr2)
		__raw_writel(cfg->ch1_ptr2, PRP_DEST_RGB2_PTR);
	else
		return -1;

	return 0;
}

/*!
 * @brief Set PrP channel 2 output buffer 1 address.
 * @param cfg	Pointer to PrP configuration parameter
 * @ret