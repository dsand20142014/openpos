/*#
 * ehci-mxc.c
 *
 *  Created on: Nov 22, 2011
 *      Author: root
 */

#include <common.h>
#include <usb.h>
#include <asm/io.h>
//#include <asm/arch/imx-regs.h>
#include <usb/ehci-fsl.h>
//#include <errno.h>

#include "ehci.h"
#include "ehci-core.h"
#include "imx258-regs.h"
#include "errno.h"
#include "io.h"


#define USBCTRL_OTGBASE_OFFSET	0x600

#ifdef CONFIG_MX25
#define MX25_USB_CTRL_IP_PUE_DOWN_BIT	(1<<6)
#define MX25_USB_CTRL_HSTD_BIT		(1<<5)
#define MX25_USB_CTRL_USBTE_BIT		(1<<4)
#define MX25_USB_CTRL_OCPOL_OTG_BIT	(1<<3)
#endif

static int mxc_set_usbcontrol(int port, unsigned int flags)
{
	unsigned int v;

#ifdef CONFIG_MX25
	v = MX25_USB_CTRL_IP_PUE_DOWN_BIT | MX25_USB_CTRL_HSTD_BIT |
		MX25_USB_CTRL_USBTE_BIT | MX25_USB_CTRL_OCPOL_OTG_BIT;
#endif

	writel(v, IMX_USB_BASE + USBCTRL_OTGBASE_OFFSET);
	return 0;
}

int ehci_hcd_init(void)
{
	struct usb_ehci *ehci;

	udelay(80);

	/* Take USB2 */
	ehci = (struct usb_ehci *)(IMX_USB_BASE +
		(0x200 * CONFIG_MXC_USB_PORT));
	hccr = (struct ehci_hccr *)((uint32_t)&ehci->caplength);
	hcor = (struct ehci_hcor *)((uint32_t) hccr +
			HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	//u32 cmd;
	//cmd = ehci_readl(&ehci->portsc);
	//printf("portsc cmd:0x%x\n", cmd);

	setbits_le32(&ehci->usbmode, CM_HOST);

//	setbits_le32(&ehci->control, USB_EN);
//	//__raw_writel(CONFIG_MXC_USB_PORTSC, &ehci->portsc);
	__raw_writel(0xcc000800, &ehci->portsc);
#if  0
	u32 cmd;

	cmd = ehci_readl(&hcor->or_usbcmd);
	cmd |=CMD_RESET;
	ehci_writel(&hcor->or_usbcmd, cmd);
	wait_ms(100);

	setbits_le32(&ehci->usbmode, CM_HOST);
#endif

   //  struct  usb_ehci *ehci = (struct usb_ehci*)(IMX_USB_BASE + 0x200 *CONFIG_MXC_USB_PORT);

   //  cc000800
  //   ehci->portsc);



	mxc_set_usbcontrol(CONFIG_MXC_USB_PORT, CONFIG_MXC_USB_FLAGS);

	udelay(10000);

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(void)
{
	return 0;
}
