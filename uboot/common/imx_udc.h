/*
 * imx_udc.h
 *
 *  Created on: Jan 15, 2012
 *      Author: root
 */

#include  <usbdevice.h>
#include <linux/list.h>
#include <linux/usb/gadget.h>
#include <asm-generic/errno.h>
#include <linux/types.h>
#include <linux/mtd/compat.h>
#include <linux/list.h>
#include <malloc.h>


#ifndef IMX_UDC_H_
#define IMX_UDC_H_
#define	EINVAL		22	/* Invalid argument */

#define IMX_USB_NB_EP 6

/*Helper macros*/
#define EP_NO(ep) ((ep->bEndpointAddress) & ~USB_DIR_IN)
#define EP_DIR(ep) ((ep->bEndpointAddress) & USB_DIR_IN? 1:0)
#define irq_to_ep(irq) (((irq) >= USBD_INT0) || ((irq) <= USBD_INT6)?  \
   ((irq) - USBD_INTO) : (USBD_INT6))

#define ep_to_irq(ep) (EP_NO((ep)) + USBD_INT0)
#define IMX_USB_NB_EP 6

/*Driver structures*/
struct imx_request{
	struct usb_request  req;
	struct list_head    queue;
	unsigned int        in_use;
};

static const char *state_name[]= {
		"EP0_IDLE",
		"EP0_IN_DATA_PHASE",
		"EP0_OUT_DATA_PHASE",
		"EP0_CONFIG",
		"EP0_STALL"
};

struct imx_ep_struct {
	struct usb_ep      ep;
	struct imx_udc_struct   *imx_usb;
	struct list_head  queue;
	unsigned char stopped;
	unsigned char fifosize;
	unsigned char bEndpointAddress;
	unsigned char bmAttributes;
};

#if 0
#define offsetof(TYPE, MEMBER) ((size_t)&((TYPE*)0)->MEMBER)

#define container_of(ptr, type, member) ({                      \
		const typeof(((type*)0)->member) *__mptr = (ptr);       \
		(type*)((char*)__mptr - offsetof(type, member));})
#endif

static const char driver_name[] = "imx_udc";
static const char ep0name[]     = "ep0";

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
#define MXC_CCM_UPCTL    (MXC_CCM_BASE + 0x04)
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

/*USB register*/
#define USB_FRAME          (0x00)
#define USB_SPEC           (0x04)
#define USB_STAT           (0x08)
#define USB_CTRL           (0x0C)
#define USB_DADR           (0x10)
#define USB_DDAT           (0x14)
#define USB_INTR           (0x18)
#define USB_MASK           (0x1C)
#define USB_ENAB           (0x24)
#define USB_EP_STAT(x)     (0x30 + (x*0x30))
#define USB_EP_INTR(x)     (0x34 + (x*0x30))
#define USB_EP_MASK(x)     (0x38 + (x*0x30))
#define USB_EP_FDAT(x)     (0x3C + (x*0x30))
#define USB_EP_FDAT0(x)    (0x3C + (x*0x30))
#define USB_EP_FDAT1(x)    (0x3D + (x*0x30))
#define USB_EP_FDAT2(x)    (0x3E + (x*0x30))
#define USB_EP_FDAT3(x)    (0x3F + (x*0x30))
#define USB_EP_FSTAT(x)    (0x40 + (x*0x30))
#define USB_EP_FCTRL(x)    (0x44 + (x*0x30))
#define USB_EP_LRFP(x)     (0x48 + (x*0x30))
#define USB_EP_LWFP(x)     (0x4C + (x*0x30))
#define USB_EP_FALRM(x)    (0x50 + (x*0x30))
#define USB_EP_FRDP(x)     (0x54 + (x*0x30))
#define USB_EP_FWRP(x)     (0x58 + (x*0x30))
/*USB Control Register Bit Fields.*/
#define CTRL_CMDOVER       (1 << 6)
#define CTRL_CMDERROR      (1 << 5)
#define CTRL_FE_ENA        (1 << 3)
#define CTRL_UDC_RST       (1 << 2)
#define CTRL_AFE_ENA       (1 << 1)
#define CTRL_RESUME        (1 << 0)
/*USB Status Register Bit Fields*/
#define STAT_RST      (1 << 8)
#define STAT_SUSP     (1 << 7)
#define STAT_CFG      (3 << 5)
#define STAT_INTF     (3 << 3)
#define STAT_ALTSET   (7 << 0)
/*USB Status Register Bit Fields*/
#define INTR_WAKEUP   (1 << 31)
#define INTR_MSOF     (1 << 7)
#define INTR_SOF      (1 << 6)
#define INTR_RESET_STOP (1 << 5)
#define INTR_RESET_START (1 << 4)
#define INTR_RESUME     (1 << 3)
#define INTR_SUSPEND    (1 << 2)
#define INTR_FRAME_MATCH (1 << 1)
#define INTR_CFG_CHG     (1 << 0)
/*USB Enable Register Bit Fields*/
#define ENAB_RST     (1 << 31)
#define ENAB_ENAB    (1 << 30)
#define ENAB_SUSPEND (1 << 29)
#define ENAB_ENDIAN  (1 << 28)
#define ENAB_PWRMD   (1 << 0)
/*USB Descriptor Ram Address Register bit fields*/
#define DADR_CFG    (1 << 31)
#define DADR_BSY    (1 << 30)
#define DADR_DADR  (0x1FF)
/*USB Descriptor RAM/Endpoint Buffer Data Register bit fields*/
#define DDAT_DDAT     (0xFF)
/*USB Endpoint Status Register bit fields*/
#define EPSTAT_BCOUNT     (0x7F << 16)
#define EPSTAT_SIP        (1 << 8)
#define EPSTAT_DIR        (1 << 7)
#define EPSTAT_MAX        (3 << 5)
#define EPSTAT_TYP        (3 << 3)
#define EPSTAT_ZLPS       (1 << 2)
#define EPSTAT_FLUSH      (1 << 1)
#define EPSTAT_STALL      (1 << 0)
/*USB Endpoint FIFO Status Register biT fields*/
#define FSTAT_FRAME_STAT  (0xF << 24)
#define FSTAT_ERR         (1 << 22)
#define FSTAT_UF          (1 << 21)
#define FSTAT_OF          (1 << 20)
#define FSTAT_FR          (1 << 19)
#define FSTAT_FULL        (1 << 18)
#define FSTAT_ALRM        (1 << 17)
#define FSTAT_EMPTY       (1 << 16)
/*USB Endpoint FIFO Control Register bit fields*/
#define FCTRL_WFR         (1 << 29)
/*USB Endpoint Interrupt Status Register bit fields*/
#define EPINTR_FIFO_FULL   (1 << 8)
#define EPINTR_FIFO_EMPTY  (1 << 7)
#define EPINTR_FIFO_ERROR  (1 << 6)
#define EPINTR_FIFO_HIGH   (1 << 5)
#define EPINTR_FIFO_LOW    (1 << 4)
#define EPINTR_MDEVREQ     (1 << 3)
#define EPINTR_EOT         (1 << 2)
#define EPINTR_DEVREQ      (1 << 1)
#define EPINTR_EOF         (1 << 0)

enum ep0_state{
	EP0_IDLE,
	EP0_IN_DATA_PHASE,
	EP0_OUT_DATA_PHASE,
	EP0_CONFIG,
	EP0_STALL,
};
struct imx_udc_struct {
	struct usb_gadget gadget;
	struct usb_gadget_driver  *driver;
	struct device *dev;
	struct imx_ep_struct  imx_ep[IMX_USB_NB_EP];
	struct clk   *clk;
//	struct timer_list  timer;
	enum ep0_state  ep0state;
	struct resource *res;
	void   *base;
	unsigned char set_config;
	int     cfg,
	        intf,
	        alt,
	        usbd_int[7];
};

void ep0_chg_stat(const char *label,struct imx_udc_struct *imx_usb, enum ep0_state stat);

struct clk{
	const char* name;
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
#if 0
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
	//if (parent != &upll_clk && parent != &ahb_clk)  return -EINVAL;
	clk->parent = parent;
	mcr = readl(MXC_CCM_MCR);

//	if (parent == &ahb_clk) mcr |= (1 << MXC_CCM_MCR_USB_CLK_MUX_OFFSET);
//	else
	mcr &= ~(1 << MXC_CCM_MCR_USB_CLK_MUX_OFFSET);

	writel(mcr, MXC_CCM_MCR);
	return 0;
}

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
#endif

static void _clk_usb_recalc(struct clk *clk);
static void _clk_pll_recalc(struct clk *clk);
static int _clk_upll_enable(struct clk* clk);
static int _clk_usb_set_rate(struct clk *clk, unsigned long rate);
static int _clk_usb_set_parent(struct clk *clk, struct clk *parent);
static void _clk_ahb_recalc(struct clk* clk);
static unsigned long _clk_cpu_round_rate(struct clk* clk, unsigned long rate);
static void _clk_cpu_recalc(struct clk *clk);
static int _clk_cpu_set_rate(struct clk *clk, unsigned long rate);

static void _clk_upll_disable(struct clk* clk)
{
	unsigned long reg;
	reg = readl(MXC_CCM_CCTL);
	reg |= MXC_CCM_CCTL_UPLL_DISABLE;
	writel(reg, MXC_CCM_CCTL);
}



static struct clk osc24m_clk = {
		.name = "osc24m",
		.rate = OSC24M_CLK_FREQ,
		.flags = RATE_PROPAGATES,
};

static struct clk upll_clk = {
		.name = "upll",
		.parent = &osc24m_clk,
		.recalc = _clk_pll_recalc,
		.enable = _clk_upll_enable,
		.disable = _clk_upll_disable,
		.flags = RATE_PROPAGATES,
};

static struct clk mpll_clk = {
		.name  = "mpll",
		.parent = &osc24m_clk,
		.recalc = _clk_pll_recalc,
		.flags = RATE_PROPAGATES,
};

static struct clk cpu_clk = {
		.name = "cpu_clk",
		.parent = &mpll_clk,
		.set_rate = _clk_cpu_set_rate,
		.recalc = _clk_cpu_recalc,
		.round_rate = _clk_cpu_round_rate,
		.flags = RATE_PROPAGATES,

};


static struct clk usbd_clk = {
		.name   = "usbd_clk",
		.parent = &upll_clk,
		.recalc = _clk_usb_recalc,
		.set_rate    = _clk_usb_set_rate,
		.set_parent  = _clk_usb_set_parent,
};


static struct clk ahb_clk = {
		.name = "ahb_clk",
		.parent = &cpu_clk,
		.recalc = _clk_ahb_recalc,
		.flags = RATE_PROPAGATES,
};

#if 1
#define CONFIG_CPU_FREQ_IMX
#define AHB_CLK_DEFAULT 133000000
#define ARM_SRC_DEFAULT 532000000

struct cpu_wp {
		u32 pll_reg;
		u32 pll_rate;
		u32 cpu_rate;
		u32 pdr0_reg;
		u32 pdf;
		u32 mfi;
		u32 mfd;
		u32 mfn;
		u32 cpu_voltage;
		u32 cpu_podf;
};

static struct cpu_wp cpu_wp_mx25[] = {
		{
				.pll_rate = 399000000,
				.cpu_rate = 399000000,
				.cpu_podf = 0x0,
				.cpu_voltage = 1450000
		},

};

int  cpu_wp_nr;

static unsigned long _clk_cpu_round_rate(struct clk* clk, unsigned long rate)
{
	int div = clk->parent->rate /rate;
	if (clk->parent->rate % rate) div++;

	if (div > 4) div = 4;

	return clk->parent->rate / div;
}

static void _clk_cpu_recalc(struct clk *clk)
{
	unsigned long div;
	unsigned long cctl = readl(MXC_CCM_CCTL);
	div = (cctl & MXC_CCM_CCTL_ARM_MASK) >> MXC_CCM_CCTL_ARM_OFFSET;
	clk->rate = clk->parent->rate / (div + 1);
	if (cctl & MXC_CCM_CCTL_ARM_SRC){
		clk->rate *= 3;
		clk->rate /= 4;
	}
}

struct cpu_wp *get_cpu_wp(int *wp)
{
	 *wp = 1;
	 return  cpu_wp_mx25;
}

static int _clk_cpu_set_rate(struct clk *clk, unsigned long rate)
{
	unsigned long div = 0x0, reg= 0x0;
	unsigned long cctl = readl(MXC_CCM_CCTL);

#if defined  CONFIG_CPU_FREQ_IMX
	struct cpu_wp *cpu_wp;
	unsigned long ahb_clk_div = 0;
	unsigned long arm_src = 0;
	int i;

	cpu_wp = get_cpu_wp(&cpu_wp_nr);
	for (i = 0; i < cpu_wp_nr; i++){
		if (cpu_wp[i].cpu_rate == rate) {
			div = cpu_wp[i].cpu_podf;
			ahb_clk_div = cpu_wp[i].cpu_rate / AHB_CLK_DEFAULT - 1;
			arm_src = (cpu_wp[i].pll_rate == ARM_SRC_DEFAULT)? 0:1;
			break;
		}
	}
	if (i == cpu_wp_nr)  return -EINVAL;

	reg = (cctl & ~MXC_CCM_CCTL_ARM_MASK) | (div << MXC_CCM_CCTL_ARM_OFFSET);
	reg = (reg  & ~MXC_CCM_CCTL_AHB_MASK) | (ahb_clk_div << MXC_CCM_CCTL_AHB_OFFSET);
	reg = (reg  & ~MXC_CCM_CCTL_ARM_SRC ) | (arm_src << MXC_CCM_CCTL_ARM_SRC_OFFSET);
	writel(reg, MXC_CCM_CCTL);
	clk->rate = rate;
#endif
		return 0;

}

static void _clk_ahb_recalc(struct clk* clk)
{
	unsigned long div;
	unsigned long cctl = readl(MXC_CCM_CCTL);

	div = (cctl & MXC_CCM_CCTL_AHB_MASK) >> MXC_CCM_CCTL_AHB_OFFSET;

	clk->rate = clk->parent->rate / (div + 1);
}

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

	while((readl(MXC_CCM_UPCTL) & MXC_CCM_UPCTL_LF) == 0);
	return 0;
}

/*                    mfi + mfn / (mfd + 1)
 * f = 2*f_ref * ---------------------------
 *                     pd + 1
 * */
unsigned long mxc_decode_pll(unsigned int reg_val, u32 freq)
{
	long long ll;
	int mfn_abs;
	unsigned int mfi, mfn, mfd, pd;

	mfi = (reg_val >> 10) & 0xf;
	mfn = reg_val & 0x3ff;
	mfd = (reg_val >> 16) & 0x3ff;
	pd = (reg_val >> 26) & 0xf;

	mfi = mfi <= 5 ? 5 : mfi;
	mfn_abs = mfn;

	if (mfn >= 0x200)  mfn_abs = 0x400 - mfn;
	freq *= 2;
	freq /= pd + 1;

	ll  = (unsigned long long)freq * mfn_abs;
	do_div(ll, mfd + 1);

	if (mfn >= 0x200) ll = -ll;
	ll = (freq * mfi) + ll;
	return ll;
}

static unsigned long get_rate_upll(void)
{
	ulong mpctl = readl(CCM_BASE_ADDR + CCM_UPCTL);
	return mxc_decode_pll(mpctl, 24000000);
}

static unsigned long get_rate_otg(struct clk* clk)
{
	unsigned long cctl = readl(CCM_BASE_ADDR + CCM_CCTL);
	unsigned long rate = get_rate_upll();
	printf("cctl:0x%x\n", (cctl & (1 << 23)));

	return (cctl & (1 << 23))? 0 : rate / ((0x3F & (cctl >> 16)) + 1);
}

/*
 * Static defined IMX UDC structure
 * */
#if 0
struct usb_gadget_ops{
	int (*get_frame)(struct usb_gadget*);
	int (*wakeup)(struct usb_gadget*);
	int (*set_selfpowered)(struct usb_gadget *, int is_selfpowered);
	int (*vbus_session)(struct usb_gadget*, int is_active);
	int (*vbus_draw)(struct usb_gadget*, unsigned mA);
	int (*pullup)(struct usb_gadget*, int is_on);
	int (*ioctl)(struct usb_gadget*, unsigned code, unsigned long param);
};
#endif



#if 0
struct usb_gadget{
	const struct usb_gadget_ops *ops;
	struct usb_ep   *ep0;
	struct list_head  ep_list;
	enum usb_device_speed speed;
	unsigned    is_dualspeed:1;
	unsigned    is_otg:1;
	unsigned    is_a_peripheral:1;
	unsigned    is_hnp_enable:1;
	unsigned    a_hnp_support:1;
	unsigned    a_alt_hnp_support:1;
	const char  *name;
};
#endif
#if 0
struct device {
	const char *init_name;
};
#endif



void imx_flush(struct imx_ep_struct *imx_ep)
{
	struct imx_udc_struct *imx_usb = imx_ep->imx_usb;
	int temp = readl(imx_usb->base + USB_EP_STAT(EP_NO(imx_ep)));
}

void imx_ep_irq_enable(struct imx_ep_struct *imx_ep)
{
	int i = EP_NO(imx_ep);

	writel(0x1FF, imx_ep->imx_usb->base + USB_EP_MASK(i));
	writel(0x1FF, imx_ep->imx_usb->base + USB_EP_INTR(i));
	writel(0x1FF & ~(EPINTR_EOT | EPINTR_EOF), imx_ep->imx_usb->base + USB_EP_MASK(i));
}

void imx_ep_stall(struct imx_ep_struct *imx_ep)
{
	struct imx_udc_struct *imx_usb = imx_ep->imx_usb;
	int temp, i;

	printf("<%s> Forced stall on %s\n",__func__, imx_ep->ep.name);
	imx_flush(imx_ep);

	if (!EP_NO(imx_ep)){
		temp = readl(imx_usb->base + USB_CTRL);
		writel(temp | CTRL_CMDOVER | CTRL_CMDERROR, imx_usb->base + USB_CTRL);
		do{}while(readl(imx_usb->base + USB_CTRL) & CTRL_CMDOVER);
		temp = readl(imx_usb->base + USB_CTRL);
		writel(temp & ~CTRL_CMDERROR, imx_usb->base + USB_CTRL);
	}else{
		temp = readl(imx_usb->base + USB_EP_STAT(EP_NO(imx_ep)));
		writel(temp | EPSTAT_STALL, imx_usb->base + USB_EP_STAT(EP_NO(imx_ep)));

		for (i = 0; i < 100; i ++)
		{
			temp = readl(imx_usb->base + USB_EP_STAT(EP_NO(imx_ep)));
			if (!(temp & EPSTAT_STALL))  break;
			udelay(20);
		}
		if (i == 100) printf("<%s> Non finished stall on %s\n", __func__, imx_ep->ep.name);
	}
}

static int imx_ep_set_halt(struct usb_ep *usb_ep, int value)
{
	struct imx_ep_struct *imx_ep = container_of(usb_ep, struct imx_ep_struct,ep);

	if (unlikely(!usb_ep || !EP_NO(imx_ep))){
		printf("<%s> bad ep \n", __func__);
		return -EINVAL;
	}

	if ((imx_ep->bEndpointAddress & USB_DIR_IN) && !list_empty(&imx_ep->queue)){
	//	local_irq_restore(flags);
		return -EAGAIN;
	}

	imx_ep_stall(imx_ep);
}

static void ep_del_request(struct imx_ep_struct *imx_ep, struct imx_request *req)
{
	if (unlikely(!req)) return;

	list_del_init(&req->queue);
	req->in_use = 0;
}

static void done(struct imx_ep_struct *imx_ep, struct imx_request *req, int status)
{
	ep_del_request(imx_ep, req);

	if (likely(req->req.status == -EINPROGRESS)) req->req.status = status;
	else status = req->req.status;

	if (status && status != -ESHUTDOWN)
		printf("<%s> complete %s req %p stat %d len %u/%u\n", __func__,
				imx_ep->ep.name, &req->req, status, req->req.actual, req->req.length);
}

static int imx_ep_dequeue(struct usb_ep *usb_ep,struct usb_request *usb_req)
{
	struct imx_ep_struct *imx_ep = container_of(usb_ep,struct imx_ep_struct, ep);
	struct imx_request *req;
	unsigned long flags;

	if (unlikely(!usb_ep || !EP_NO(imx_ep))){
		printf("<%s> bad ep\n",__func__);
		return -EINVAL;
	}

	list_for_each_entry(req, &imx_ep->queue, queue)
		if (&req->req == usb_req) break;

	if (&req->req != usb_req) {
		return -EINVAL;
	}

	done(imx_ep, req, -ECONNRESET);

	return 0;

}

static void dump_req(const char *label, struct imx_ep_struct *imx_ep,
		struct usb_request *req)
{
	int i;
	if (!req || !req->buf){
		 printf("<%s> req or req buf is free\n",label);
	return;
	}

	if ((!EP_NO(imx_ep) && imx_ep->imx_usb->ep0state == EP0_IN_DATA_PHASE) ||
			(EP_NO(imx_ep) && EP_DIR(imx_ep))){
		printf("<%s> request dump <", label);
		for (i = 0; i < req->length; i++)
			printk("%02x-", *((u8*)req->buf + i));
		printk(">\n");
	}
}

static void ep_add_request(struct imx_ep_struct *imx_ep, struct imx_request *req)
{
	if (unlikely(!req))  return;
	req->in_use = 1;
	list_add_tail(&req->queue, &imx_ep->queue);

}

unsigned imx_fifo_bcount(struct imx_ep_struct *imx_ep)
{
	struct imx_udc_struct *imx_usb = imx_ep->imx_usb;

	return (readl(imx_usb->base + USB_EP_STAT(EP_NO(imx_ep))) & EPSTAT_BCOUNT) >> 16;
}

static int write_packet(struct imx_ep_struct *imx_ep,struct imx_request *req)
{
	u8 *buf;
	int length, count, temp;

	if (unlikely(readl(imx_ep->imx_usb->base + USB_EP_STAT(EP_NO(imx_ep))) & EPSTAT_ZLPS))
	{
		printf("<%s> zlp still queued in EP %s \n", __func__, imx_ep->ep.name);
		return -1;
	}
	buf = req->req.buf + req->req.actual;
	prefetch(buf);

	length = min(req->req.length - req->req.actual, (u32)imx_ep->fifosize);

	if (imx_fifo_bcount(imx_ep) + length > imx_ep->fifosize){
		printf("<%s> packet overfill %s fifo\n", __func__, imx_ep->ep.name);
		return -1;
	}

	buf = req->req.actual += length;
	count =  length;

	if (!count && req->req.zero){
		temp = readl(imx_ep->imx_usb->base + USB_EP_STAT(EP_NO(imx_ep)));
		writel(temp | EPSTAT_ZLPS, imx_ep->imx_usb->base + USB_EP_STAT(EP_NO(imx_ep)));
		printf("<%s> zero packet\n", __func__);
		return 0;
	}

	while(count--){
		if (count == 0){
			temp = readl(imx_ep->imx_usb->base + USB_EP_FCTRL(EP_NO(imx_ep)));
			writel(temp | FCTRL_WFR, imx_ep->imx_usb->base + USB_EP_FCTRL(EP_NO(imx_ep)));
		}
		writeb(*buf++, imx_ep->imx_usb->base + USB_EP_FDAT0(EP_NO(imx_ep)));
	}

	return length;
}

int imx_ep_empty(struct imx_ep_struct *imx_ep)
{
	struct imx_udc_struct *imx_usb = imx_ep->imx_usb;
	return  readl(imx_usb->base + USB_EP_FSTAT(EP_NO(imx_ep))) & FSTAT_EMPTY;
}

static int read_packet(struct imx_ep_struct *imx_ep, struct imx_request *req)
{
	u8 *buf;
	int bytes_ep, bufferspace, count, i;

	bytes_ep = imx_fifo_bcount(imx_ep);
	bufferspace = req->req.length - req->req.actual;

	buf = req->req.buf + req->req.actual;
	//prefechw(buf);

	if (unlikely(imx_ep_empty(imx_ep))) count = 0;
	else count = min(bytes_ep, bufferspace);

	for (i = count; i > 0; i--)
		*buf++ = readb(imx_ep->imx_usb->base + USB_EP_FDAT0(EP_NO(imx_ep)));
	req->req.actual +=count;

	return count;
}
static int read_fifo(struct imx_ep_struct *imx_ep, struct imx_request *req)
{
	int bytes = 0,
		count,
		completed = 0;

	while(readl(imx_ep->imx_usb->base + USB_EP_FSTAT(EP_NO(imx_ep))) & FSTAT_FR)
	{
		count = read_packet(imx_ep,req);
		bytes += count;

		completed = (count != imx_ep->fifosize);
		if (completed || req->req.actual == req->req.length){
			completed = 1;
			break;
		}
	}

	if (completed || !req->req.length){
		done(imx_ep, req, 0);
		printf("<%s> %s req<%p>%s\n", __func__, imx_ep->ep.name, req,
				completed?"completed":"not completed");
		if (!EP_NO(imx_ep))
			ep0_chg_stat(__func__, imx_ep->imx_usb, EP0_IDLE);
	}
	printf("<%s> bytes read:%d\n", __func__, bytes);
	return completed;
}


static int write_fifo(struct imx_ep_struct *imx_ep, struct imx_request *req)
{
	int bytes = 0,
		count,
		completed = 0;

	while(!completed){
		count = write_packet(imx_ep, req);
		if (count < 0) break;
		bytes +=count;
		/*last packet "must be short (or a zlp)*/
		completed = (count != imx_ep->fifosize);

		if (unlikely(completed)){
			done(imx_ep, req, 0);
			printf("<%s> %s req <%p> %s\n", __func__, imx_ep->ep.name, req,
			completed? "completed": "not completed");
		    if (!EP_NO(imx_ep))
		    	ep0_chg_stat(__func__,imx_ep->imx_usb, EP0_IDLE);
		}
	}
    printf("<%s> bytes sent:%d\n", __func__, bytes);
    return completed;
}

static int handle_ep0(struct imx_ep_struct *imx_ep)
{
	struct imx_request *req = NULL;
	int ret = 0;

	if (!list_empty(&imx_ep->queue)){
		req = list_entry(imx_ep->queue.next, struct imx_request, queue);
		switch(imx_ep->imx_usb->ep0state){
		case EP0_IN_DATA_PHASE:
				write_fifo(imx_ep, req);
				break;
		case EP0_OUT_DATA_PHASE:
				read_fifo(imx_ep,req);
				break;
		default:
			printf("<%s>ep0 i/o, odd state %d\n", __func__, imx_ep->imx_usb->ep0state);
			ep_del_request(imx_ep, req);
			ret = -EL2HLT;
			break;
		}
	}else printf("<%s> no request on %s\n", __func__, imx_ep->ep.name);
	return ret;
}

static void dump_ep_stat(const char *label, struct imx_ep_struct *imx_ep)
{
	int temp = readl(imx_ep->imx_usb->base + USB_EP_INTR(EP_NO(imx_ep)));

	printf("<%s> EP%d_INTR=[%s%s%s%s%s%s%s%s%s]\n",label, EP_NO(imx_ep),
			(temp & EPINTR_FIFO_FULL)?" full":"",
			(temp & EPINTR_FIFO_EMPTY)? " fempty":"",
			(temp & EPINTR_FIFO_ERROR)?" ferr":"",
			(temp & EPINTR_FIFO_HIGH)?" fhigh":"",
			(temp & EPINTR_FIFO_LOW)?" flow":"",
			(temp & EPINTR_MDEVREQ)?" mreq":"",
			(temp & EPINTR_EOF)?" eof":"",
			(temp & EPINTR_DEVREQ)?" devreq":"",
			(temp & EPINTR_EOT)?" eot":"");

	temp = readl(imx_ep->imx_usb->base + USB_EP_STAT(EP_NO(imx_ep)));
	printf("<%s EP%d_STAT=[%s%s bcount=%d]\n", label, EP_NO(imx_ep),
			(temp & EPSTAT_SIP)?" sip":"",
			(temp & EPSTAT_STALL)?" stall":"",
			(temp & EPSTAT_BCOUNT) >> 16);

	temp = readl(imx_ep->imx_usb->base + USB_EP_FSTAT(EP_NO(imx_ep)));
	printf("<%s>EP%d_FSTAT=[%s%s%s%s%s%s%s]\n", label, EP_NO(imx_ep),
			(temp & FSTAT_ERR)?" ferr":"",
			(temp & FSTAT_UF)?" funder":"",
			(temp & FSTAT_OF)?" fover":"",
			(temp & FSTAT_FR)?" fready":"",
			(temp & FSTAT_FULL)?" ffull":"",
			(temp & FSTAT_ALRM)?" falarm":"",
			(temp & FSTAT_EMPTY)?" fempty":"");
}

static int handle_ep(struct imx_ep_struct *imx_ep)
{
	struct imx_request *req;
	int completed = 0;

	do{
		if (!list_empty(&imx_ep->queue)) req = list_entry(imx_ep->queue.next, struct imx_request, queue);
		else {
			printf("<%s> no request on %s\n", __func__, imx_ep->ep.name);
			return 0;
		}

		if (EP_DIR(imx_ep)) completed = write_fifo(imx_ep, req);
		else  completed = read_fifo(imx_ep, req);

		dump_ep_stat(__func__, imx_ep);

	}while(completed);

	return 0;
}

static int imx_ep_queue(struct usb_ep *usb_ep, struct usb_request *usb_req, gfp_t gfp_flags)
{
	struct imx_ep_struct *imx_ep;
	struct imx_udc_struct *imx_usb;
	struct imx_request *req;
	unsigned long flags;
	int ret =0;

	imx_ep = container_of(usb_ep, struct imx_ep_struct, ep);
	imx_usb = imx_ep->imx_usb;
	req = container_of(usb_req, struct imx_request, req);

	if (imx_usb->set_config && !EP_NO(imx_ep))
	{
		imx_usb->set_config = 0;
		printf("<%s> gadget reply set config\n", __func__);
		return 0;
	}

	if (unlikely(!usb_req || !req || !usb_req->complete || !usb_req->buf))
	{
		printf("<%s> bad params\n", __func__);
		return -EINVAL;
	}

	if (unlikely(!usb_ep || !imx_ep)){
		printf("<%s> bad ep\n", __func__);
		return -EINVAL;
	}

	if (!imx_usb->driver || imx_usb->gadget.speed == USB_SPEED_UNKNOWN){
		printf("<%s> bogus device state\n", __func__);
		return -ESHUTDOWN;
	}

	dump_req(__func__, imx_ep, usb_req);

	if (imx_ep->stopped){
		usb_req->status = -ESHUTDOWN;
		return -ESHUTDOWN;
	}

	if (req->in_use){
		printf("<%s> refusing to queue req %p (already queued)\n", __func__, req);
		return 0;
	}

	usb_req->status =  -EINPROGRESS;
	usb_req->actual = 0;

	ep_add_request(imx_ep, req);

	if (!EP_NO(imx_ep))  ret = handle_ep0(imx_ep);
	else                 ret = handle_ep(imx_ep);

	return ret;
}

static void imx_ep_free_request(struct usb_ep *usb_ep, struct usb_request *usb_req)
{
	struct imx_request *req;
	req = container_of(usb_req, struct imx_request, req);
	kfree(req);
}

static struct usb_request *imx_ep_alloc_request(struct usb_ep *usb_ep, gfp_t gfp_flags)
{
	struct imx_request *req;
	if (!usb_ep) return NULL;

	req = malloc(sizeof *req);
	if (!req) return NULL;

	INIT_LIST_HEAD(&req->queue);
	req->in_use = 0;

	return &req->req;
}

static void nuke(struct imx_ep_struct *imx_ep, int status)
{
	struct imx_request *req;

	while(!list_empty(&imx_ep->queue)){
		req = list_entry(imx_ep->queue.next, struct imx_request, queue);
		done(imx_ep, req, status);
	}
}

 void imx_ep_irq_disable(struct imx_ep_struct *imx_ep)
 {
	 int i = EP_NO(imx_ep);

	 writel(0x1FF, imx_ep->imx_usb->base + USB_EP_MASK(i));
	 writel(0x1FF, imx_ep->imx_usb->base + USB_EP_INTR(i));

 }

static int imx_ep_disable(struct usb_ep *usb_ep)
{
	struct imx_ep_struct *imx_ep = container_of(usb_ep, struct imx_ep_struct, ep);
	unsigned long flags;

	if (!usb_ep || EP_NO(imx_ep) || !list_empty(&imx_ep->queue)){
		printf("<%s> %s can not be disabled\n", __func__, usb_ep?imx_ep->ep.name:NULL);
		return -EINVAL;
	}

	imx_ep->stopped = 1;
	nuke(imx_ep, -ESHUTDOWN);
	imx_flush(imx_ep);
	imx_ep_irq_disable(imx_ep);

	printf("<%s> DISABLED %s\n", __func__, usb_ep->name);
	return 0;
}

static int imx_ep_enable(struct usb_ep *usb_ep, const struct usb_endpoint_descriptor *desc)
{
	struct imx_ep_struct *imx_ep = container_of(usb_ep, struct imx_ep_struct, ep);
	struct imx_udc_struct *imx_usb = imx_ep->imx_usb;
	unsigned long flags;

	if (!usb_ep || !desc || !EP_NO(imx_ep) || desc->bDescriptorType != USB_DT_ENDPOINT
	|| imx_ep->bEndpointAddress != desc->bEndpointAddress)
	{
		printf("<%s> bad ep or descriptor \n", __func__);
		return -EINVAL;
	}

	if (imx_ep->bmAttributes != desc->bmAttributes){
		printf("<%s> %s type mismatch \n", __func__, usb_ep->name);
		return -EINVAL;
	}

	if (imx_ep->fifosize < desc->wMaxPacketSize){
		printf("<%s> bad %s maxpacket\n", __func__, usb_ep->name);
		return -ERANGE;
	}

	if (!imx_usb->driver || imx_usb->gadget.speed == USB_SPEED_UNKNOWN){
		printf("<%s> bogus device state\n", __func__);
		return -ESHUTDOWN;
	}

	imx_ep->stopped = 0;
	imx_flush(imx_ep);
	imx_ep_irq_enable(imx_ep);
	printf("<%s ENABLE %s>\n", __func__, usb_ep->name);
	return 0;
}

static int imx_ep_fifo_status(struct usb_ep *usb_ep)
{
	struct imx_ep_struct *imx_ep = container_of(usb_ep,struct imx_ep_struct, ep);

	if (!usb_ep){
		printf("<%s> bad ep\n", __func__);
		return -ENODEV;
	}

	if(imx_ep->imx_usb->gadget.speed == USB_SPEED_UNKNOWN) return 0;
	else return imx_fifo_bcount(imx_ep);

}

static void imx_ep_fifo_flush(struct usb_ep *usb_ep)
{
	struct imx_ep_struct *imx_ep = container_of(usb_ep, struct imx_ep_struct, ep);
	unsigned long flags;

	if (!usb_ep || !EP_NO(imx_ep) || !list_empty(&imx_ep->queue)){
		printf("<%s> bad ep\n", __func__);
		return;
	}
	imx_flush(imx_ep);

}

static struct usb_ep_ops imx_ep_ops = {
		.enable  = imx_ep_enable,
		.disable = imx_ep_disable,

		.alloc_request = imx_ep_alloc_request,
		.free_request  = imx_ep_free_request,

		.queue   = imx_ep_queue,
		.dequeue = imx_ep_dequeue,

		.set_halt = imx_ep_set_halt,
		.fifo_status = imx_ep_fifo_status,
		.fifo_flush = imx_ep_fifo_flush,

};

static int imx_udc_get_frame(struct usb_gadget *_gadget)
{
	struct imx_udc_struct *imx_usb = container_of(_gadget, struct imx_udc_struct, gadget);
	return readl(imx_usb->base + USB_FRAME) & 0x7FF;
}

static int imx_udc_wakeup(struct usb_gadget *_gadget)
{
	printf("udc wakeup!!!\n");
	return 0;
}

static const struct usb_gadget_ops imx_udc_ops = {
		.get_frame = imx_udc_get_frame,
		.wakeup    = imx_udc_wakeup,
		};

static struct imx_udc_struct controller ={
		.gadget = {
				.ops = &imx_udc_ops,
				.ep0 = &controller.imx_ep[0].ep,
				.name = driver_name,
				.dev = {
						.init_name = "gadget",
				},
		},

		.imx_ep[0]  = {
				.ep = {
						.name = ep0name,
						.ops = &imx_ep_ops,
						.maxpacket = 32,
				},
				.imx_usb = &controller,
				.fifosize = 32,
				.bEndpointAddress = 0,
				.bmAttributes = USB_ENDPOINT_XFER_CONTROL,
		},

		.imx_ep[1]  = {
						.ep = {
								.name = "ep1in-bulk",
								.ops = &imx_ep_ops,
								.maxpacket = 64,
						},
						.imx_usb = &controller,
						.fifosize = 64,
						.bEndpointAddress = USB_DIR_IN | 1,
						.bmAttributes = USB_ENDPOINT_XFER_BULK,
				},

		.imx_ep[2]  = {
						.ep = {
								.name = "ep2out-bulk",
								.ops = &imx_ep_ops,
								.maxpacket = 64,
								},
						.imx_usb = &controller,
						.fifosize = 64,
						.bEndpointAddress = USB_DIR_OUT | 2,
						.bmAttributes = USB_ENDPOINT_XFER_BULK,
						},
		.imx_ep[3]  = {
						.ep = {
								.name = "ep3out-bulk",
								.ops = &imx_ep_ops,
								.maxpacket = 32,
								},
								.imx_usb = &controller,
								.fifosize = 32,
								.bEndpointAddress = USB_DIR_OUT | 3,
								.bmAttributes = USB_ENDPOINT_XFER_BULK,
							},
		.imx_ep[4]  = {
						.ep = {
								.name = "ep4in-int",
								.ops = &imx_ep_ops,
								.maxpacket = 32,
								},
								.imx_usb = &controller,
								.fifosize = 32,
								.bEndpointAddress = USB_DIR_IN | 4,
								.bmAttributes = USB_ENDPOINT_XFER_INT,
								},

		.imx_ep[5]  = {
						.ep = {
								.name = "ep5out-int",
								.ops = &imx_ep_ops,
								.maxpacket = 32,
								},
								.imx_usb = &controller,
								.fifosize = 32,
								.bEndpointAddress = USB_DIR_OUT | 5,
								.bmAttributes = USB_ENDPOINT_XFER_INT,
								},
	};

void ep0_chg_stat(const char *label,struct imx_udc_struct *imx_usb, enum ep0_state stat)
{
	printf("<%s> from %15s to %15s\n", label, state_name[imx_usb->ep0state], state_name[stat]);

	if (imx_usb->ep0state == stat) return ;

	imx_usb->ep0state = stat;
}

void imx_udc_init_ep(struct imx_udc_struct *imx_usb)
{
	int i, max, temp;
	struct imx_ep_struct *imx_ep;

	for (i = 0; i < IMX_USB_NB_EP; i++)
	{
		imx_ep = &imx_usb->imx_ep[i];
		switch (imx_ep->fifosize){
		case 8:
			 max = 0;
			 break;
		case 16:
			max = 1;
			break;
		case 32:
			max = 2;
			break;
		case 64:
			max = 3;
			break;
		default:
			max = 1;
			break;
		}
		temp = (EP_DIR(imx_ep) << 7) | (max << 5) | (imx_ep->bmAttributes << 3);
		writel(temp, imx_usb->base + USB_EP_STAT(i));
		writel(temp | EPSTAT_FLUSH, imx_usb->base + USB_EP_STAT(i));
	}
}

void imx_udc_init_irq(struct imx_udc_struct *imx_usb)
{
	int i;

	/*Mask and clear all irqs*/
	writel(0xffffFFFF, imx_usb->base + USB_MASK);
	writel(0xffffFFFF, imx_usb->base + USB_INTR);

	for (i = 0; i < IMX_USB_NB_EP; i++){
		writel(0x1FF, imx_usb->base + USB_EP_MASK(i));
		writel(0x1FF, imx_usb->base + USB_EP_MASK(i));
	}
	/*Enable USB irqs*/
	writel(INTR_MSOF | INTR_FRAME_MATCH, imx_usb->base + USB_MASK);

	/*Enable EP0 irqs*/
	writel(0x1FF &~(EPINTR_DEVREQ | EPINTR_MDEVREQ | EPINTR_EOT | EPINTR_EOF | EPINTR_FIFO_EMPTY |
			EPINTR_FIFO_FULL), imx_usb->base + USB_EP_MASK(0));

}

void imx_udc_init_fifo(struct imx_udc_struct *imx_usb)
{
	int i, temp;
	struct imx_ep_struct *imx_ep;
	for (i = 0; i < IMX_USB_NB_EP; i++){
		imx_ep = &imx_usb->imx_ep[i];
		/*Fifo control*/
		temp = EP_DIR(imx_ep)? 0x0B000000:0x0F000000;
		writel(temp, imx_usb->base + USB_EP_FCTRL(i));

		/*Fifo alarm*/
		temp = (i? imx_ep->fifosize/2:0);
		writel(temp, imx_usb->base + USB_EP_FALRM(i));
	}
}

void imx_udc_reset(struct imx_udc_struct *imx_usb)
{
	int temp = readl(imx_usb->base + USB_ENAB);
    printf("[%s: %d]\n", __func__, __LINE__);
	/*set RST bit*/
	writel(temp | ENAB_RST, imx_usb->base + USB_ENAB);
	printf("[%s: %d]\n", __func__, __LINE__);
	/*wait RST bit to clear*/
	do{}while(readl(imx_usb->base + USB_ENAB) & ENAB_RST);
	printf("[%s: %d]\n", __func__, __LINE__);
	/*wait CFG bit to assert*/
	do{}while(readl(imx_usb->base + USB_DADR) & DADR_CFG);
	printf("[%s: %d]\n", __func__, __LINE__);
}

void imx_udc_config(struct imx_udc_struct *imx_usb)
{
	u8 ep_conf[5];
	u8 i, j, cfg;

	struct imx_ep_struct *imx_ep;
	/*wait CFG bit to assert*/
	do{}while(!(readl(imx_usb->base + USB_DADR) & DADR_CFG));

	/*Download the endpoint buffer for endpoint 0.*/
	for (j = 0; j < 5; j++)
	{
		i=(j == 2? imx_usb->imx_ep[0].fifosize:0x0);
		writeb(i, imx_usb->base + USB_DDAT);
		do{}while(readl(imx_usb->base + USB_DADR) & DADR_BSY);
	}

	/*Download the endpoint buffers for endpoint 1-5
	 * We specify two configurations, one interface*/
	for (cfg = 1; cfg < 3; cfg++){
		for (i = 1; i< IMX_USB_NB_EP; i++){
			imx_ep = &imx_usb->imx_ep[i];
			/*EP NO | Config no*/
			ep_conf[0] = (1 << 4) | (cfg << 2);
			/*Type | Direction*/
			ep_conf[1] = (imx_ep->bmAttributes << 3) | (EP_DIR(imx_ep) << 2);
			/*Max packet size*/
			ep_conf[2] = imx_ep->fifosize;
			/*TRXTYP*/
			ep_conf[3] = 0xC0;
			/*FIFO no*/
		    ep_conf[4] = i;

		    for (j = 0; j < 5; j++){
		    	writeb(ep_conf[j], imx_usb->base + USB_DDAT);
		    	do{}while(readl(imx_usb->base + USB_DADR) & DADR_BSY);
		    }
		}
	}
	/*wait CFG bit to clear*/
	do{}while(readl(imx_usb->base + USB_DADR) & DADR_CFG);
}

static void imx_udc_init(struct imx_udc_struct *imx_usb)
{
	 printf("imx_udc_init start\n");
	/*Reset UDC*/
	imx_udc_reset(imx_usb);
    printf("imx_udc_reset over");
	/*Download config to enpoint buffer*/
	imx_udc_config(imx_usb);
	 printf("imx_udc_config over");
	/*Setup interrupts*/
	imx_udc_init_irq(imx_usb);
	 printf("imx_udc_init_irq over");
	/*Setup endpoints*/
	imx_udc_init_ep(imx_usb);
	 printf("imx_udc_init_ep over");
	/*Setup fifos*/
	imx_udc_init_fifo(imx_usb);
	 printf("imx_udc_init_ep over");
}

static void handle_config(unsigned long data)
{
#if 0
	struct imx_udc_struct *imx_usb = (void*)data;
	struct usb_ctrlrequest u;
	int temp, cfg, intf, alt;

	temp = readl(imx_usb->base + USB_STAT);
	cfg = (temp & STAT_CFG) >> 5;
	intf = (temp & STAT_INTF) >> 3;
	alt = temp & STAT_ALTSET;

	if (cfg == 1 || cfg ==2){
		if (imx_usb->cfg != cfg){
			u.bRequest = USB_REQ_SET_CONFIGURATION;
			u.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
			u.wValue = cfg;
			u.wIndex = 0;
			u.wLength = 0;
			imx_usb->cfg = cfg;
			imx_usb->driver->setup(&imx_usb->gadget, &u);
		}
		if (imx_usb->intf != intf || imx_usb->alt != alt){
			u.bRequest = USB_REQ_SET_INTERFACE;
			u.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_INTERFACE;
			u.wValue = alt;
			u.wIndex = intf;
			u.wLength = 0;
			imx_usb->intf = intf;
			imx_usb->alt = alt;
			imx_usb->driver->setup(&imx_usb->gadget,&u);
		}
	}
	imx_usb->set_config = 0;
#endif
}



static void usb_init_data(struct imx_udc_struct *imx_usb)
{
	struct imx_ep_struct *imx_ep;
	u8 i;

	INIT_LIST_HEAD(&imx_usb->gadget.ep_list);
	INIT_LIST_HEAD(&imx_usb->gadget.ep0->ep_list);
	ep0_chg_stat(__func__,imx_usb, EP0_IDLE);

	for (i = 0; i < IMX_USB_NB_EP; i++)
	{
		 imx_ep = &imx_usb->imx_ep[i];

		 if(i){
			 list_add_tail(&imx_ep->ep.ep_list, &imx_usb->gadget.ep_list);
			 imx_ep->stopped = 1;
		 }else
			 imx_ep->stopped = 0;
		 INIT_LIST_HEAD(&imx_ep->queue);
	}
}


#endif
#endif /* IMX_UDC_H_ */


































