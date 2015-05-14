#include <linux/module.h>   /* Needed by all modules */
#include <linux/init.h>		/* Needed for the macros */
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <mach/hardware.h>
#include <mach/gpio.h>
#include "board-mx25_3stack.h"
#include "iomux.h"

static char *option = "help";
static short int value = 0;
static long pin_value = 0;

static char *pins_name[157] = {"A10",
				"A13",
				"A14",
				"A15",
				"A16",	
				"A17",	
				"A18",	
				"A19",	
				"A20",	
				"A21",	
				"A22",	
				"A23",
				"A24",
				"A25",
				"EB0",
				"EB1",
				"OE",
				"CS0",
				"CS1",
				"CS4",
				"CS5",
				"NF_CE0",
				"ECB",
				"LBA",
				"BCLK",
				"RW",
				"NFWE_B",
				"NFRE_B",
				"NFALE",
				"NFCLE",
				"NFWP_B",
				"NFRB",
				"D15",
				"D14",
				"D13",
				"D12",
				"D11",
				"D10",
				"D9",
				"D8",
				"D7",
				"D6",
				"D5",
				"D4",
				"D3",
				"D2",
				"D1",
				"D0",
				"LD0",
				"LD1",
				"LD2",
				"LD3",
				"LD4",
				"LD5",
				"LD6",
				"LD7",
				"LD8",
				"LD9",
				"LD10",
				"LD11",
				"LD12",
				"LD13",
				"LD14",
				"LD15",
				"HSYNC",
				"VSYNC",
				"LSCLK",
				"OE_ACD",
				"CONTRAST",
				"PWM",
				"CSI_D2",
				"CSI_D3",
				"CSI_D4",
				"CSI_D5",
				"CSI_D6",
				"CSI_D7",
				"CSI_D8",
				"CSI_D9",
				"CSI_MCLK",
				"CSI_VSYNC",
				"CSI_HSYNC",
				"CSI_PIXCLK",
				"I2C1_CLK",
				"I2C1_DAT",
				"CSPI1_MOSI",
				"CSPI1_MOSO",
				"CSPI1_SS0",
				"CSPI1_SS1",
				"CSPI1_SCLK",
				"CSPI1_RDY",
				"UART1_RXD",
				"UART1_TXD",
				"UART1_RTS",
				"UART1_CTS",
				"UART2_RXD",
				"UART2_TXD",
				"UART2_RTS",
				"UART2_CTS",
				"SD1_CMD",
				"SD1_CLK",
				"SD1_DATA0",
				"SD1_DATA1",
				"SD1_DATA2",
				"SD1_DATA3",
				"KPP_ROW0",
				"KPP_ROW1",
				"KPP_ROW2",
				"KPP_ROW3",
				"KPP_COL0",
				"KPP_COL1",
				"KPP_COL2",
				"KPP_COL3",
				"FEC_MDC",
				"FEC_MDIO",
				"FEC_TDATA0",
				"FEC_TDATA1",
				"FEC_TX_EN",
				"FEC_RDATA0",
				"FEC_RDATA1",
				"FEC_RX_DV",
				"FEC_TX_CLK",
				"RTCK",
				"DE_B",
				"TDO",
				"GPIO_A",
				"GPIO_B",
				"GPIO_C",
				"GPIO_D",
				"GPIO_E",
				"GPIO_F",
				"EXT_ARMCLK",
				"UPLL_BYPCLK",
				"VSTBY_REQ",
				"VSTBY_ACK",
				"POWER_FAIL",
				"CLK0",
				"BOOT_MODE0",
				"BOOT_MODE1",
				"CTL_GRP_DVS_MISC",
				"CTL_GRP_DSE_FEC",
				"CTL_GRP_DVS_JTAG",
				"CTL_GRP_DSE_NFC",
				"CTL_GRP_DSE_CSI",
				"CTL_GRP_DSE_WEIM",
				"CTL_GRP_DSE_DDR",
				"CTL_GRP_DVS_CRM",
				"CTL_GRP_DSE_KPP",
				"CTL_GRP_DSE_SDHC1",
				"CTL_GRP_DSE_LCD",
				"CTL_GRP_DSE_UART",
				"CTL_GRP_DVS_NFC",
				"CTL_GRP_DVS_CSI",
				"CTL_GRP_DSE_CSPI1",
				"CTL_GRP_DDRTYPE",
				"CTL_GRP_DVS_SDHC1",
				"CTL_GRP_DVS_LCD"
};


static char *pin_name = "D13";


module_param(option, charp, 0000);
MODULE_PARM_DESC(option, "HELP, WRITE, READ");

module_param(pin_name, charp, 0000);
MODULE_PARM_DESC(pin_name, "FOR PINs NAME, SEE README FILE");

module_param(value, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(value, "0 or 1");


void return_pin_value (char *pin_name)
{
	int i;

	for (i = 0; i < 156; i++)
	{
		if (! strnicmp (pin_name, pins_name[i], strlen (pin_name)))
			break;
	}

	switch (i)
	{
		case 0: pin_value = MX25_PIN_A10;
			break;
		
		case 1: pin_value = MX25_PIN_A13;
			break;
		
		case 2: pin_value = MX25_PIN_A14;
			break;
		
		case 3: pin_value = MX25_PIN_A15;
			break;
		
		case 4: pin_value = MX25_PIN_A16;
			break;
		
		case 5: pin_value = MX25_PIN_A17;
			break;
		
		case 6: pin_value = MX25_PIN_A18;
			break;
		
		case 7: pin_value = MX25_PIN_A19;
			break;
		
		case 8: pin_value = MX25_PIN_A20;
			break;
		
		case 9: pin_value = MX25_PIN_A21;
			break;
		
		case 10: pin_value = MX25_PIN_A22;
			break;
		
		case 11: pin_value = MX25_PIN_A23;
			break;
		
		case 12: pin_value = MX25_PIN_A24;
			break;
		
		case 13: pin_value = MX25_PIN_A25;
			break;
		
		case 14: pin_value = MX25_PIN_EB0;
			break;
		
		case 15: pin_value = MX25_PIN_EB1;
			break;
		
		case 16: pin_value = MX25_PIN_OE;
			break;
		
		case 17: pin_value = MX25_PIN_CS0;
			break;
		
		case 18: pin_value = MX25_PIN_CS1;
			break;
		
		case 19: pin_value = MX25_PIN_CS4;
			break;
		
		case 20: pin_value = MX25_PIN_CS5;
			break;
		
		case 21: pin_value = MX25_PIN_NF_CE0;
			break;
		
		case 22: pin_value = MX25_PIN_ECB;
			break;
		
		case 23: pin_value = MX25_PIN_LBA;
			break;
		
		case 24: pin_value = MX25_PIN_BCLK;
			break;
		
		case 25: pin_value = MX25_PIN_RW;
			break;
		
		case 26: pin_value = MX25_PIN_NFWE_B;
			break;
		
		case 27: pin_value = MX25_PIN_NFRE_B;
			break;
		
		case 28: pin_value = MX25_PIN_NFALE;
			break;
		
		case 29: pin_value = MX25_PIN_NFCLE;
			break;
		
		case 30: pin_value = MX25_PIN_NFWP_B;
			break;
		
		case 31: pin_value = MX25_PIN_NFRB;
			break;
		
		case 32: pin_value = MX25_PIN_D15;
			break;
		
		case 33: pin_value = MX25_PIN_D14;
			break;
		
		case 34: pin_value = MX25_PIN_D13;
			break;
		
		case 35: pin_value = MX25_PIN_D12;
			break;
		
		case 36: pin_value = MX25_PIN_D11;
			break;
		
		case 37: pin_value = MX25_PIN_D10;
			break;
		
		case 38: pin_value = MX25_PIN_D9;
			break;
		
		case 39: pin_value = MX25_PIN_D8;
			break;
		
		case 40: pin_value = MX25_PIN_D7;
			break;
		
		case 41: pin_value = MX25_PIN_D6;
			break;
		
		case 42: pin_value = MX25_PIN_D5;
			break;
		
		case 43: pin_value = MX25_PIN_D4;
			break;
		
		case 44: pin_value = MX25_PIN_D3;
			break;
		
		case 45: pin_value = MX25_PIN_D2;
			break;
		
		case 46: pin_value = MX25_PIN_D1;
			break;
		
		case 47: pin_value = MX25_PIN_D0;
			break;
		
		case 48: pin_value = MX25_PIN_LD0;
			break;
		
		case 49: pin_value = MX25_PIN_LD1;
			break;
		
		case 50: pin_value = MX25_PIN_LD2;
			break;
		
		case 51: pin_value = MX25_PIN_LD3;
			break;
		
		case 52: pin_value = MX25_PIN_LD4;
			break;
		
		case 53: pin_value = MX25_PIN_LD5;
			break;
		
		case 54: pin_value = MX25_PIN_LD6;
			break;
		
		case 55: pin_value = MX25_PIN_LD7;
			break;
		
		case 56: pin_value = MX25_PIN_LD8;
			break;
		
		case 57: pin_value = MX25_PIN_LD9;
			break;
		
		case 58: pin_value = MX25_PIN_LD10;
			break;
		
		case 59: pin_value = MX25_PIN_LD11;
			break;
		
		case 60: pin_value = MX25_PIN_LD12;
			break;
		
		case 61: pin_value = MX25_PIN_LD13;
			break;
		
		case 62: pin_value = MX25_PIN_LD14;
			break;
		
		case 63: pin_value = MX25_PIN_LD15;
			break;
		
		case 64: pin_value = MX25_PIN_HSYNC;
			break;
		
		case 65: pin_value = MX25_PIN_VSYNC;
			break;
		
		case 66: pin_value = MX25_PIN_LSCLK;
			break;
		
		case 67: pin_value = MX25_PIN_OE_ACD;
			break;
		
		case 68: pin_value = MX25_PIN_CONTRAST;
			break;
		
		case 69: pin_value = MX25_PIN_PWM;
			break;
		
		case 70: pin_value = MX25_PIN_CSI_D2;
			break;
		
		case 71: pin_value = MX25_PIN_CSI_D3;
			break;
		
		case 72: pin_value = MX25_PIN_CSI_D4;
			break;
		
		case 73: pin_value = MX25_PIN_CSI_D5;
			break;
		
		case 74: pin_value = MX25_PIN_CSI_D6;
			break;
		
		case 75: pin_value = MX25_PIN_CSI_D7;
			break;
		
		case 76: pin_value = MX25_PIN_CSI_D8;
			break;
		
		case 77: pin_value = MX25_PIN_CSI_D9;
			break;
		
		case 78: pin_value = MX25_PIN_CSI_MCLK;
			break;
		
		case 79: pin_value = MX25_PIN_CSI_VSYNC;
			break;
		
		case 80: pin_value = MX25_PIN_CSI_HSYNC;
			break;
		
		case 81: pin_value = MX25_PIN_CSI_PIXCLK;
			break;
		
		case 82: pin_value = MX25_PIN_I2C1_CLK;
			break;
		
		case 83: pin_value = MX25_PIN_I2C1_DAT;
			break;
		
		case 84: pin_value = MX25_PIN_CSPI1_MOSI;
			break;
		
		case 85: pin_value = MX25_PIN_CSPI1_MISO;
			break;
		
		case 86: pin_value = MX25_PIN_CSPI1_SS0;
			break;
		
		case 87: pin_value = MX25_PIN_CSPI1_SS1;
			break;
		
		case 88: pin_value = MX25_PIN_CSPI1_SCLK;
			break;
		
		case 89: pin_value = MX25_PIN_CSPI1_RDY;
			break;
		
		case 90: pin_value = MX25_PIN_UART1_RXD;
			break;
		
		case 91: pin_value = MX25_PIN_UART1_TXD;
			break;
		
		case 92: pin_value = MX25_PIN_UART1_RTS;
			break;
		
		case 93: pin_value = MX25_PIN_UART1_CTS;
			break;
		
		case 94: pin_value = MX25_PIN_UART2_RXD;
			break;
		
		case 95: pin_value = MX25_PIN_UART2_TXD;
			break;
		
		case 96: pin_value = MX25_PIN_UART2_RTS;
			break;
		
		case 97: pin_value = MX25_PIN_UART2_CTS;
			break;
		
		case 98: pin_value = MX25_PIN_SD1_CMD;
			break;
		
		case 99: pin_value = MX25_PIN_SD1_CLK;
			break;
		
		case 100: pin_value = MX25_PIN_SD1_DATA0;
			break;
		
		case 101: pin_value = MX25_PIN_SD1_DATA1;
			break;
		
		case 102: pin_value = MX25_PIN_SD1_DATA2;
			break;
		
		case 103: pin_value = MX25_PIN_SD1_DATA3;
			break;
		
		case 104: pin_value = MX25_PIN_KPP_ROW0;
			break;
		
		case 105: pin_value = MX25_PIN_KPP_ROW1;
			break;
		
		case 106: pin_value = MX25_PIN_KPP_ROW2;
			break;
		
		case 107: pin_value = MX25_PIN_KPP_ROW3;
			break;
		
		case 108: pin_value = MX25_PIN_KPP_COL0;
			break;
		
		case 109: pin_value = MX25_PIN_KPP_COL1;
			break;
		
		case 110: pin_value = MX25_PIN_KPP_COL2;
			break;
		
		case 111: pin_value = MX25_PIN_KPP_COL3;
			break;
		
		case 112: pin_value = MX25_PIN_FEC_MDC;
			break;
		
		case 113: pin_value = MX25_PIN_FEC_MDIO;
			break;
		
		case 114: pin_value = MX25_PIN_FEC_TDATA0;
			break;
		
		case 115: pin_value = MX25_PIN_FEC_TDATA1;
			break;
		
		case 116: pin_value = MX25_PIN_FEC_TX_EN;
			break;
		
		case 117: pin_value = MX25_PIN_FEC_RDATA0;
			break;
		
		case 118: pin_value = MX25_PIN_FEC_RDATA1;
			break;
		
		case 119: pin_value = MX25_PIN_FEC_RX_DV;
			break;
		
		case 120: pin_value = MX25_PIN_FEC_TX_CLK;
			break;
		
		case 121: pin_value = MX25_PIN_RTCK;
			break;
		
		case 122: pin_value = MX25_PIN_DE_B;
			break;
		
		case 123: pin_value = MX25_PIN_TDO;
			break;
		
		case 124: pin_value = MX25_PIN_GPIO_A;
			break;
		
		case 125: pin_value = MX25_PIN_GPIO_B;
			break;
		
		case 126: pin_value = MX25_PIN_GPIO_C;
			break;
		
		case 127: pin_value = MX25_PIN_GPIO_D;
			break;
		
		case 128: pin_value = MX25_PIN_GPIO_E;
			break;
		
		case 129: pin_value = MX25_PIN_GPIO_F;
			break;
		
		case 130: pin_value = MX25_PIN_EXT_ARMCLK;
			break;
		
		case 131: pin_value = MX25_PIN_UPLL_BYPCLK;
			break;
		
		case 132: pin_value = MX25_PIN_VSTBY_REQ;
			break;
		
		case 133: pin_value = MX25_PIN_VSTBY_ACK;
			break;
		
		case 134: pin_value = MX25_PIN_POWER_FAIL;
			break;
		
		case 135: pin_value = MX25_PIN_CLKO;
			break;
		
		case 136: pin_value = MX25_PIN_BOOT_MODE0;
			break;
		
		case 137: pin_value = MX25_PIN_BOOT_MODE1;
			break;
		
		case 138: pin_value = MX25_PIN_CTL_GRP_DVS_MISC;
			break;
		
		case 139: pin_value = MX25_PIN_CTL_GRP_DSE_FEC;
			break;
		
		case 140: pin_value = MX25_PIN_CTL_GRP_DVS_JTAG;
			break;
		
		case 141: pin_value = MX25_PIN_CTL_GRP_DSE_NFC;
			break;
		
		case 142: pin_value = MX25_PIN_CTL_GRP_DSE_CSI;
			break;
		
		case 143: pin_value = MX25_PIN_CTL_GRP_DSE_WEIM;
			break;
		
		case 144: pin_value = MX25_PIN_CTL_GRP_DSE_DDR;
			break;
		
		case 145: pin_value = MX25_PIN_CTL_GRP_DVS_CRM;
			break;
		
		case 146: pin_value = MX25_PIN_CTL_GRP_DSE_KPP;
			break;
		
		case 147: pin_value = MX25_PIN_CTL_GRP_DSE_SDHC1;
			break;
		
		case 148: pin_value = MX25_PIN_CTL_GRP_DSE_LCD;
			break;
		
		case 149: pin_value = MX25_PIN_CTL_GRP_DSE_UART;
			break;
		
		case 150: pin_value = MX25_PIN_CTL_GRP_DVS_NFC;
			break;
		
		case 151: pin_value = MX25_PIN_CTL_GRP_DVS_CSI;
			break;
		
		case 152: pin_value = MX25_PIN_CTL_GRP_DSE_CSPI1;
			break;
		
		case 153: pin_value = MX25_PIN_CTL_GRP_DDRTYPE;
			break;
		
		case 154: pin_value = MX25_PIN_CTL_GRP_DVS_SDHC1;
			break;
		
		case 155: pin_value = MX25_PIN_CTL_GRP_DVS_LCD;
			break;

		default: printk ("\nWRONG PIN NAME, ABORTING... REMOVE THE MODULE !\n\n");
			break;
	}
}


int init_module (void)
{
	// gets pin value (address + offset)
	return_pin_value (pin_name);
	
	// do nothing if a wrong pin name was passed
	if (! pin_value) 
		return 0;

	// io function
	mxc_request_iomux (pin_value, MUX_CONFIG_ALT5);

	// set its pad
	mxc_iomux_set_pad (pin_value, 0);

	// give it a name
	gpio_request (IOMUX_TO_GPIO (pin_value), "gpio");

	
	if (! strnicmp (option, "WRITE", 5))
	{
		// set direction accordingly to the passed parameter (write - output)
		gpio_direction_output (IOMUX_TO_GPIO(pin_value), 0);

		// set value accordingly to the passed parameter
		gpio_set_value (IOMUX_TO_GPIO(pin_value), value);

		printk ("\nPin Wrote: %d\n", value);

	}
	else if (! strnicmp (option, "READ", 4))
	{
		// set direction accordingly to the passed parameter (read - input)
		gpio_direction_output (IOMUX_TO_GPIO(pin_value), 1);

		// get value from pin
		value = gpio_get_value (IOMUX_TO_GPIO(pin_value));

		printk ("\nPin Read: %d\n", value);
	}
	else
	{
		printk ("\nWrong Option\n");
	}


	printk ("\n\nRemove this module to peform another test\n\n");

	return 0;
}


void cleanup_module(void)
{
	mxc_free_iomux (pin_value, MUX_CONFIG_GPIO);	
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andre Silva");
MODULE_DESCRIPTION("GPIO KERNEL SPACE READ/WRITE TEST APPLICATION"); 
