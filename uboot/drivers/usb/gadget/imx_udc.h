/*
 * imx_udc.h
 *
 *  Created on: Jan 15, 2012
 *      Author: root
 */

#ifndef IMX_UDC_H_
#define IMX_UDC_H_

#define OSC24M_CLK_FREQ 24000000

/*Clock flags*/
#define RATE_PROPAGATES    (1 << 0)
#define ALWAYS_ENABLED     (1 << 1)
#define RATE_FIXED         (1 << 2)
#define CPU_FREQ_TRIG_UPDATE   (1 << 3)
#define AHB_HIGH_SET_POINT  (1 << 4)
#define AHB_MED_SET_POINT   (1 << 5)


#define MXC_CCM_MCR_USB_CLK_MUX_OFFSET   16

#define AIPS2_BASE_ADDR  0x53F00000
#define CCM_BASE_ADDR    (AIPS2_BASE_ADDR + 0x00080000)
#define MXC_CCM_BASE     ((char*)CCM_BASE_ADDR)

#define MXC_CCM_MPCTL    (MXC_CCM_BASE + 0x00)
#define MXC_CCM_CCTL     (MXC_CCM_BASE + 0x08)
#define MXC_CCM_MCR      (MXC_CCM_BASE + 0x64)

#define MXC_CCM_MPCTL_BRMO         (1 << 31)
#define MXC_CCM_MPCTL_PD_OFFSET     26
#define MXC_CCM_MPCTL_PD_MASK       (0xf << 26)
#define MXC_CCM_MPCTL_MFD_OFFSET    16
#define MXC_CCM_MPCTL_MFD_MASK      (0x3ff << 16)
#define MXC_CCM_MPCTL_MFI_OFFSET    10
#define MXC_CCM_MPCTL_MFI_MASK      (0xf << 10)
#define MXC_CCM_MPCTL_MFN_OFFSET    0
#define MXC_CCM_MPCTL_MFN_MASK      0x3ff
#define MXC_CCM_MPCTL_LF            (1 << 15)

#define MXC_CCM_UPCTL_BRMO         (1 << 31)
#define MXC_CCM_UPCTL_PD_OFFSET     26
#define MXC_CCM_UPCTL_PD_MASK       (0xf << 26)
#define MXC_CCM_UPCTL_MFD_OFFSET    16
#define MXC_CCM_UPCTL_MFD_MASK      (0x3ff << 16)
#define MXC_CCM_UPCTL_MFI_OFFSET    10
#define MXC_CCM_UPCTL_MFI_MASK      (0xf << 10)
#define MXC_CCM_UPCTL_MFN_OFFSET    0
#define MXC_CCM_UPCTL_MFN_MASK      0x3ff
#define MXC_CCM_UPCTL_LF            (1 << 15)

#define MXC_CCM_CCTL_ARM_OFFSET         30
#define MXC_CCM_CCTL_ARM_MASK           (0x3 << 30)
#define MXC_CCM_CCTL_AHB_OFFSET         28
#define MXC_CCM_CCTL_AHB_MASK           (0x3 << 28)
#define MXC_CCM_CCTL_MPLL_RST           (1 << 27)
#define MXC_CCM_CCTL_UPLL_RST           (1 << 26)
#define MXC_CCM_CCTL_LP_CTL_OFFSET      24
#define MXC_CCM_CCTL_LP_CTL_MASK        (0x3 << 24)
#define MXC_CCM_CCTL_MODE_RUN           (0x0 << 24)
#define MXC_CCM_CCTL_MODE_WAIT          (0x1 << 24)
#define MXC_CCM_CCTL_MODE_DOZE          (0x2 << 24)
#define MXC_CCM_CCTL_MODE_STOP          (0x3 << 24)
#define MXC_CCM_CCTL_UPLL_DISABLE       (1 << 23)
#define MXC_CCM_CCTL_MPLL_BYPASS        (1 << 22)
#define MXC_CCM_CCTL_USB_DIV_OFFSET      16
#define MXC_CCM_CCTL_USB_DIV_MASK       (0x3 << 16)
#define MXC_CCM_CCTL_CG_CTRL            (1 << 15)
#define MXC_CCM_CCTL_ARM_SRC            (1 << 14)
#define MXC_CCM_CCTL_ARM_SRC_OFFSET      14

struct clk{
	int id;
	struct clk *parent;
	struct clk *secondary;

	unsigned long rate;
	unsigned char usecount;
	unsigned char enable_shift;
	void   * enable_reg;
	unsigned int flags;
	void (*recalc)(struct clk*);
	int (*set_rate)(struct clk*, unsigned long);
	unsigned long (*round_rate)(struct clk*, unsigned long);
	unsigned long (*enable)(struct clk*);
	void (*disable)(struct clk*);
	int (*set_parent)(struct clk*, struct clk*);
};

static void _clk_usb_recalc(struct clk *clk)
{
	unsigned long div = readl(MXC_CCM_CCTL) & MXC_CCM_CCTL_USB_DIV_MASK;
	div >>= MXC_CCM_CCTL_USB_DIV_OFFSET;
	clk->rate = clk->parent->rate / (div + 1);
}

static int _clk_usb_set_rate(struct clk *clk, unsigned long rate)
{
	unsigned long reg;
	unsigned long div;

	div = clk->parent->rate / rate;

	if (clk->parent->rate /div != rate) return -EINVAL;
	if (div > 64)  return -EINVAL;

	reg =  readl(MXC_CCM_CCTL) & ~MXC_CCM_CCTL_USB_DIV_MASK;
	reg |= (div - 1) << MXC_CCM_CCTL_USB_DIV_OFFSET;
	writel(reg, MXC_CCM_CCTL);
}

static int _clk_usb_set_parent(struct clk *clk, struct clk *parent)
{
	unsigned long mcr;
	if (parent != &upll_clk && parent != &ahb_clk)  return -EINVAL;
	clk->parent = parent;
	mcr = readl(MXC_CCM_MCR);

	if (parent == &ahb_clk) mcr |= (1 << MXC_CCM_MCR_USB_CLK_MUX_OFFSET);
	else
	mcr &= ~(1 << MXC_CCM_MCR_USB_CLK_MUX_OFFSET);

	writel(mcr, MXC_CCM_MCR);
}

static struct clk osc24m_clk = {
		.name = "osc24m",
		.rate = OSC24M_CLK_FREQ,
		.flags = RATE_PROPAGETES,
};

static void _clk_pll_recalc(struct clk *clk)
{
	unsigned long mfi = 0, mfn = 0, mfd = 0, pdf = 0;
	unsigned long ref_clk;
	unsigned long reg;
	unsigned long long temp;

	ref_clk = clk->parent->rate;

	if (clk == &mpll_clk){
		reg = __raw_readl(MXC_CCM_MPCTL);
		pdf = (reg & MXC_CCM_MPCTL_PD_MASK ) >> MXC_CCM_MPCTL_PD_OFFSET;
		mfd = (reg & MXC_CCM_MPCTL_MFD_MASK) >> MXC_CCM_MPCTL_MFD_OFFSET;
		mfi = (reg & MXC_CCM_MPCTL_MFI_MASK) >> MXC_CCM_MPCTL_MFI_OFFSET;
		mfn = (reg & MXC_CCM_MPCTL_MFN_MASK) >> MXC_CCM_MPCTL_MFN_OFFSET;
		}else if (clk == &upll_clk)
		{
			reg = readl(MXC_CCM_UPCTL);
			pdf = (reg & MXC_CCM_UPCTL_PD_MASK) >> MXC_CCM_UPCTL_PD_OFFSET;
			mfd = (reg & MXC_CCM_MPCTL_MFD_MASK) >> MXC_CCM_UPCTL_MFD_OFFSET;
			mfi = (reg & MXC_CCM_UPCTL_MFI_MASK) >> MXC_CCM_UPCTL_MFI_OFFSET;
			mfn = (reg & MXC_CCM_UPCTL_MFN_MASK) >> MXC_CCM_UPCTL_MFN_OFFSET;
		}
	mfi = (mfi <= 5)? 5:mfi;
	temp = 2 * ref_clk *mfn;
	do_div(temp, mfd + 1);
	temp = 2 * ref_clk *mfi +temp;
	do_div(temp, pdf + 1);

	clk->rate = temp;
}

static int _clk_upll_enable(struct clk* clk)
{
	unsigned long reg;

	reg = readl(MXC_CCM_CCTL);
	reg &= ~MXC_CCM_CCTL_UPLL_DISABLE;
	writel(reg, MXC_CCM_CCTL);

	while(readl((MXC_CCM_UPCTL) & MXC_CCM_UPCTL_LF) == 0);
	return 0;
}

static void _clk_upll_disable(struct clk* clk)
{
	unsigned long reg;
	reg = readl(MXC_CCM_CCTL);
	reg |= MXC_CCM_CCTL_UPLL_DISABLE;
	writel(reg, MXC_CCM_CCTL);
}


static struct clk upll_clk = {
		.name = "upll",
		.parent = &osc24m_clk,
		.recalc = _clk_pll_recalc,
		.enable = _clk_upll_enable,
		.disable = _clk_upll_disable,
		.flags = RATE_PROPAGATES,
};

static struct clk usbd_clk = {
		.name   = "usbd_clk",
		.parent = &upll_clk,
		.recalc = _clk_usb_recalc,
		.set_rate    = _clk_usb_set_rate,
		.set_parent  = _clk_usb_set_parent,
};


#endif /* IMX_UDC_H_ */
