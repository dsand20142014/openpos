/*
 * (C) Copyright 2009 Freescale Semiconductor
 *
 * (C) Copyright 2004
 * Texas Instruments.
 * Richard Woodruff <r-woodruff2@ti.com>
 * Kshitij Gupta <kshitij@ti.com>
 *
 * Configuration settings for the Freescale i.MX31 PDK board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/mx25.h>
#include <asm/arch/mx25-regs.h>

/*   enable check file  in uboot   */
/* PCI_CHECK */
//#define PCI_CHECK

/* Define machine,now we support:
 * #define CONFIG_PS4K
 * #define CONFIG_PS400
 * #define CONFIG_DEMOBOARD
 * #define CONFIG_PS4KV2
 * #define CONFIG_IPS420
 * #define CONFIG_IPS420_64M
 * #define CONFIG_IPS420_MDDR
 */
#define CONFIG_IPS420

#if (defined(CONFIG_IPS420_64M) || defined(CONFIG_IPS420_MDDR))
#define CONFIG_IPS420
#endif

/* High Level Configuration Options */
#define CONFIG_ARM926EJS	1	/* This is an arm1136 CPU core */
#define CONFIG_MX25		1	/* in a mx31 */
#define CONFIG_MX25_HCLK_FREQ	24000000
#define CONFIG_MX25_CLK32	32768

//#define USB_DEBUG

#define CONFIG_IMX_CSPI		1
#define IMX_CSPI_VER_0_7	1
//#define CONFIG_IMX_SPI_CPLD

#define CS3_ENABLE  1
#define CS3_DISBALE 0

#define MXC_EHCI_POWER_PINS_ENABLED  (1 << 5)

#define CONFIG_CMD_USB
#define CONFIG_USB_EHCI
//#define CONFIG_USB_EHCI_FSL
#define CONFIG_USB_EHCI_MXC
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET

//PS400 USE USB HOST 2
#define CONFIG_MXC_USB_PORT   2
//#define CONFIG_MXC_USB_PORT   1

#define CONFIG_MXC_USB_PORTSC  (MXC_EHCI_MODE_ULPI | MXC_EHCI_UTMI_8BIT)
//#define CONFIG_MXC_USB_FLAGS   MXC_EHCI_POWER_PINS_ENABLED
#define CONFIG_MXC_USB_FLAGS   0
#define CONFIG_EHCI_IS_TDI
#define CONFIG_USB_STORAGE
#define CONFIG_DOS_PARTITION
#define CONFIG_SUPPORT_VFAT
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
//#define CONFIG_CMD_EXT2

/* IF iMX25 3DS V-1.0 define it */
#if (defined(CONFIG_PS4K) || defined(CONFIG_PS4KV2))
#define CONFIG_FPGA
#define CONFIG_MX25_3DS_V10
#elif defined(CONFIG_IPS420_MDDR)
#define CONFIG_MX25_3DS_V10
#endif

#ifdef CONFIG_MX25_3DS_V10
#define MXC_MEMORY_MDDR
#endif

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define BOARD_LATE_INIT

#define CONFIG_CMDLINE_TAG	    	1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG	    	1

#define CONFIG_FLASH_HEADER 1crmboot
#define CONFIG_FLASH_HEADER_OFFSET   0x400
#define CONFIG_FLASH_HEADER_BARKER   0xB1



/* No support for NAND boot for i.MX31 PDK yet, so we rely on some other
 * program to initialize the SDRAM.
 */
#define CONFIG_SKIP_RELOCATE_UBOOT

/*
 * Size of malloc() pool
 */
//#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 512 * 1024)
//#define CONFIG_SYS_GBL_DATA_SIZE	128 /* bytes reserved initial data */

#define CONFIG_SYS_MALLOC_LEN	 (1*1024*1024)
#define CONFIG_SYS_GBL_DATA_SIZE	128 /* bytes reserved initial data */

#define MTDIDS_DEFAULT "nand0=nandflash0"

/*#define MTDPARTS_DEFAULT            "mtdparts=nandflash0:1m@0(uboot)," \
                                   "1m(fpga)," \
                                   "4m(kernel)," \
                                   "4m(cramfs),"\
                                   "-(rootfs)"*/

#define MTDPARTS_DEFAULT            "mtdparts=nandflash0:896k@0(uboot)," \
					"128k(cert),"\
                                   "1m(fpga)," \
                                   "4m(kernel)," \
				   "4m(kernel_ro),"\
                                   "4m(cramfs),"\
                                   "-(rootfs)"

#define CONFIG_MTD_DEVICE 1
#define CONFIG_MTD_PARTITIONS 1
#define CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_UBIFS
#define CONFIG_CMD_UBI
#define CONFIG_LZO 1
#define CONFIG_RBTREE 1


/*
 * Hardware drivers
 */
#define CONFIG_HARD_I2C		1
#define CONFIG_I2C_MXC		1
#define CONFIG_SYS_I2C_PORT		I2C1_BASE
#define CONFIG_SYS_I2C_SPEED		40000
#define CONFIG_SYS_I2C_SLAVE		0xfe

#define CONFIG_MX25_UART	1
#define CONFIG_MX25_UART1	1
#define CONFIG_MX25_UART5	1

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{9600, 19200, 38400, 57600, 115200}

/***********************************************************
 * Command definition
 ***********************************************************/

#include <config_cmd_default.h>

#define CONFIG_CMD_I2C

/* SAND_MACHINE */
#if defined(CONFIG_DEMOBOARD)
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#else
#endif

/* #define CONFIG_CMD_SPI */
/* #define CONFIG_CMD_DATE */
#define CONFIG_CMD_NAND
#define CONFIG_CMD_MMC
#define CONFIG_MXC_NAND

/*
 * MMC Configs
 * */
#ifdef CONFIG_CMD_MMC
	//#define CONFIG_FSL_MMC
	#define CONFIG_MMC				1
	#define CONFIG_MMC_BASE   0x0

	#define CONFIG_GENERIC_MMC
	#define CONFIG_IMX_MMC

#endif

/* Disabled due to compilation errors in cmd_bootm.c (IMLS seems to require
 * that CONFIG_NO_FLASH is undefined).
 */
#undef CONFIG_CMD_IMLS

#define CONFIG_BOOTDELAY	1

#define CONFIG_LOADADDR		0x80800000	/* loadaddr env var */

/* SAND_MACHINE */
#if defined(CONFIG_PS4K)
#define	CONFIG_EXTRA_ENV_SETTINGS		                			  \
	"uboot=u-boot.bin\0"						                      \
	"uboot_addr=0xa0000000\0"					                      \
	"kernel=uImage\0"						                          \
	"bootargs_base=mtdparts default;setenv bootargs console=ttymxc4,115200 sand_machine=${sand_machine}\0"	      \
	"bootargs_nand=setenv bootargs $(bootargs) ubi.mtd=6 "			  \
		"root=ubi0:ubifs rootfstype=ubifs\0"							  \
	"bootcmd=fpga_config;run bootcmd_nand;\0"					          \
	"bootcmd_nand=run bootargs_base bootargs_nand; "					  \
		"nand read 0x80800000 kernel;bootm 0x80800000\0"	  \
	"bootargs_cramfs=setenv bootargs $(bootargs) root=/dev/mtdblock5 rootfstype=cramfs\0"	\
	"bootcmd_cramfs=mtdparts default;run bootargs_base bootargs_cramfs; nand read 0x80800000 kernel;bootm 0x80800000\0"	\
	"load_uboot=tftpboot ${loadaddr} ${uboot}\0"                      \
	"eth1addr=64:46:00:00:00:10\0"	\
	"sand_machine=PS4K\0"	\
	"mmc_write_boot=mtdparts default;mmc rescan; fatload mmc 0 80800000 u-boot.bin;nand erase uboot; nand write 80800000 uboot\0" \
	"usb_write_boot=mtdparts default;usb start; fatload usb 0 80800000 u-boot.bin;nand erase uboot; nand write 80800000 uboot\0" \
	"mmc_write_kernel=mtdparts default;mmc rescan; fatload mmc 0 80800000 uImage;nand erase kernel; nand write 80800000 kernel\0" \
	"usb_write_kernel=mtdparts default;usb start; fatload usb 0 80800000 uImage;nand erase kernel; nand write 80800000 kernel\0" \
	"mmc_write_kernel_ro=mtdparts default;mmc rescan; fatload mmc 0 80800000 uImage;nand erase kernel_ro; nand write 80800000 kernel_ro\0" \
	"usb_write_kernel_ro=mtdparts default;usb start; fatload usb 0 80800000 uImage;nand erase kernel_ro; nand write 80800000 kernel_ro\0" \
	"mmc_write_cramfs=mtdparts default;mmc rescan; fatload mmc 0 80800000 cramfs;nand erase cramfs; nand write 80800000 cramfs\0" \
	"usb_write_cramfs=mtdparts default;usb start; fatload usb 0 80800000 cramfs;nand erase cramfs; nand write 80800000 cramfs\0" \
	"ubi_format=mtdparts default;nand erase rootfs;ubi part rootfs;ubi create ubifs\0"
#elif defined(CONFIG_PS4KV2)
#define	CONFIG_EXTRA_ENV_SETTINGS		                			  \
	"uboot=u-boot.bin\0"						                      \
	"uboot_addr=0xa0000000\0"					                      \
	"kernel=uImage\0"						                          \
	"bootargs_base=mtdparts default;setenv bootargs console=ttymxc4,115200 sand_machine=${sand_machine}\0"	      \
	"bootargs_nand=setenv bootargs $(bootargs) ubi.mtd=6 "			  \
		"root=ubi0:ubifs rootfstype=ubifs\0"							  \
	"bootcmd=fpga_config;run bootcmd_nand;\0"					          \
	"bootcmd_nand=run bootargs_base bootargs_nand; "					  \
		"nand read 0x80800000 kernel;bootm 0x80800000\0"	  \
	"bootargs_cramfs=setenv bootargs $(bootargs) root=/dev/mtdblock5 rootfstype=cramfs\0"	\
	"bootcmd_cramfs=mtdparts default;run bootargs_base bootargs_cramfs; nand read 0x80800000 kernel;bootm 0x80800000\0"	\
	"load_uboot=tftpboot ${loadaddr} ${uboot}\0"                      \
	"eth1addr=64:46:00:00:00:10\0"	\
	"sand_machine=PS4KV2\0" \
	"mmc_write_boot=mtdparts default;mmc rescan; fatload mmc 0 80800000 u-boot.bin;nand erase uboot; nand write 80800000 uboot\0" \
	"usb_write_boot=mtdparts default;usb start; fatload usb 0 80800000 u-boot.bin;nand erase uboot; nand write 80800000 uboot\0" \
	"mmc_write_kernel=mtdparts default;mmc rescan; fatload mmc 0 80800000 uImage;nand erase kernel; nand write 80800000 kernel\0" \
	"usb_write_kernel=mtdparts default;usb start; fatload usb 0 80800000 uImage;nand erase kernel; nand write 80800000 kernel\0" \
	"mmc_write_kernel_ro=mtdparts default;mmc rescan; fatload mmc 0 80800000 uImage;nand erase kernel_ro; nand write 80800000 kernel_ro\0" \
	"usb_write_kernel_ro=mtdparts default;usb start; fatload usb 0 80800000 uImage;nand erase kernel_ro; nand write 80800000 kernel_ro\0" \
	"mmc_write_cramfs=mtdparts default;mmc rescan; fatload mmc 0 80800000 cramfs;nand erase cramfs; nand write 80800000 cramfs\0" \
	"usb_write_cramfs=mtdparts default;usb start; fatload usb 0 80800000 cramfs;nand erase cramfs; nand write 80800000 cramfs\0" \
	"ubi_format=mtdparts default;nand erase rootfs;ubi part rootfs;ubi create ubifs\0"
#elif defined(CONFIG_PS400)
#define	CONFIG_EXTRA_ENV_SETTINGS		                			  \
	"uboot=u-boot.bin\0"						                      \
	"uboot_addr=0xa0000000\0"					                      \
	"kernel=uImage\0"						                          \
	"bootargs_base=mtdparts default;setenv bootargs console=ttymxc4,115200 sand_machine=${sand_machine}\0"	      \
	"bootargs_nand=setenv bootargs $(bootargs) ubi.mtd=6 "			  \
		"root=ubi0:ubifs rootfstype=ubifs\0"							  \
	"bootcmd=run bootcmd_nand;\0"					          \
	"bootcmd_nand=run bootargs_base bootargs_nand; "					  \
		"nand read 0x80800000 kernel;bootm 0x80800000\0"	  \
	"bootargs_cramfs=setenv bootargs $(bootargs) root=/dev/mtdblock5 rootfstype=cramfs\0"	\
	"bootcmd_cramfs=mtdparts default;run bootargs_base bootargs_cramfs; nand read 0x80800000 kernel;bootm 0x80800000\0"	\
	"load_uboot=tftpboot ${loadaddr} ${uboot}\0"                      \
	"eth1addr=64:46:00:00:00:10\0"	\
	"sand_machine=PS400"	\
	"mmc_write_boot=mtdparts default;mmc rescan; fatload mmc 0 80800000 u-boot.bin;nand erase uboot; nand write 80800000 uboot\0" \
	"usb_write_boot=mtdparts default;usb start; fatload usb 0 80800000 u-boot.bin;nand erase uboot; nand write 80800000 uboot\0" \
	"mmc_write_kernel=mtdparts default;mmc rescan; fatload mmc 0 80800000 uImage;nand erase kernel; nand write 80800000 kernel\0" \
	"usb_write_kernel=mtdparts default;usb start; fatload usb 0 80800000 uImage;nand erase kernel; nand write 80800000 kernel\0" \
	"mmc_write_kernel_ro=mtdparts default;mmc rescan; fatload mmc 0 80800000 uImage;nand erase kernel_ro; nand write 80800000 kernel_ro\0" \
	"usb_write_kernel_ro=mtdparts default;usb start; fatload usb 0 80800000 uImage;nand erase kernel_ro; nand write 80800000 kernel_ro\0" \
	"mmc_write_cramfs=mtdparts default;mmc rescan; fatload mmc 0 80800000 cramfs;nand erase cramfs; nand write 80800000 cramfs\0" \
	"usb_write_cramfs=mtdparts default;usb start; fatload usb 0 80800000 cramfs;nand erase cramfs; nand write 80800000 cramfs\0" \
	"ubi_format=mtdparts default;nand erase rootfs;ubi part rootfs;ubi create ubifs\0"
#elif defined(CONFIG_IPS420)//lpj=995328
#define	CONFIG_EXTRA_ENV_SETTINGS		           		  \
        "bootargs_base=mtdparts default;setenv bootargs console=ttymxc4,115200 sand_machine=${sand_machine}\0"         \
        "bootargs_nand=setenv bootargs $(bootargs) ubi.mtd=6 "                    \
                "root=ubi0:ubifs rootfstype=ubifs\0"                                                      \
        "bootcmd=run bootcmd_nand;\0"                                             \
        "bootcmd_nand=run bootargs_base bootargs_nand; "                                          \
                "nand read 0x80800000 kernel;bootm 0x80800000\0"       \
	"bootargs_cramfs=setenv bootargs $(bootargs) root=/dev/mtdblock5 rootfstype=cramfs\0"	\
	"bootcmd_cramfs=mtdparts default;run bootargs_base bootargs_cramfs; nand read 0x80800000 kernel;bootm 0x80800000\0"	\
        "mmc_wb=mtdparts default;mmc rescan; fatload mmc 0 80800000 u-boot.bin;"           \
        "nand erase uboot; nand write 80800000 uboot\0"                 \
        "sand_machine=IPS420\0"                                                             \
        "kern_dw=mtdparts default;mmc rescan;fatload mmc 0 80800000 uImage;nand erase kernel;nand write 80800000 kernel\0"  \
        "crmfs_arg=set bootargs noinitrd root=/dev/mtdblock5 console=ttymxc4,115200 sand_machine=${sand_machine} init=/linuxrc\0"  \
        "up_crmfs=mtdparts default;nand read 80800000 kernel;bootm\0"    \
        "gocrm=mtdparts default;run crmfs_arg;run up_crmfs\0"                 \
        "dwcrm=mtdparts default;mmc rescan;fatload mmc 0 80800000 cramfs1202.img;nand erase cramfs;nand write 80800000 cramfs\0" \
	"mmc_write_boot=mtdparts default;mmc rescan; fatload mmc 0 80800000 u-boot.bin;nand erase uboot; nand write 80800000 uboot\0" \
	"usb_write_boot=mtdparts default;usb start; fatload usb 0 80800000 u-boot.bin;nand erase uboot; nand write 80800000 uboot\0" \
	"mmc_write_kernel=mtdparts default;mmc rescan; fatload mmc 0 80800000 uImage;nand erase kernel; nand write 80800000 kernel\0" \
	"usb_write_kernel=mtdparts default;usb start; fatload usb 0 80800000 uImage;nand erase kernel; nand write 80800000 kernel\0" \
	"mmc_write_kernel_ro=mtdparts default;mmc rescan; fatload mmc 0 80800000 uImage;nand erase kernel_ro; nand write 80800000 kernel_ro\0" \
	"usb_write_kernel_ro=mtdparts default;usb start; fatload usb 0 80800000 uImage;nand erase kernel_ro; nand write 80800000 kernel_ro\0" \
	"mmc_write_cramfs=mtdparts default;mmc rescan; fatload mmc 0 80800000 cramfs;nand erase cramfs; nand write 80800000 cramfs\0" \
	"usb_write_cramfs=mtdparts default;usb start; fatload usb 0 80800000 cramfs;nand erase cramfs; nand write 80800000 cramfs\0" \
	"ubi_format=mtdparts default;nand erase rootfs;ubi part rootfs;ubi create ubifs\0"
#else
#define	CONFIG_EXTRA_ENV_SETTINGS		                			  \
	"netdev=eth0\0"						                        	  \
	"ethprime=FEC-PHY\0"					                    	  \
	"uboot=u-boot.bin\0"						                      \
	"uboot_addr=0xa0000000\0"					                      \
	"kernel=uImage\0"						                          \
	"bootargs_base=mtdparts default;setenv bootargs console=ttymxc4,115200 sand_machine=${sand_machine}\0"	      \
	"bootargs_nfs=setenv bootargs $(bootargs) root=/dev/nfs "         \
		"ip=dhcp nfsroot=$(serverip):$(nfsrootfs),v3,tcp\0"           \
	"bootargs_nand=setenv bootargs $(bootargs) ubi.mtd=6 "			  \
		"root=ubi0:ubifs rootfstype=ubifs\0"							  \
	"bootcmd=run bootcmd_nand;\0"					                  \
	"bootcmd_net=run bootargs_base bootargs_nfs; "		        	  \
		"tftpboot ${loadaddr} ${kernel}; bootm\0"	             	  \
	"bootcmd_nand=mtdparts default;run bootargs_base bootargs_nand; "					  \
		"nand read 0x80800000 kernel;bootm 0x80800000\0"	  \
	"bootargs_cramfs=setenv bootargs $(bootargs) root=/dev/mtdblock5 rootfstype=cramfs\0"	\
	"bootcmd_cramfs=mtdparts default;run bootargs_base bootargs_cramfs; nand read 0x80800000 kernel;bootm 0x80800000\0"	\
	"load_uboot=tftpboot ${loadaddr} ${uboot}\0"                      \
	"serverip=172.16.20.116\0"										\
	"ipaddr=172.16.20.144\0"	\
	"eth1addr=64:46:00:00:00:10\0"		\
	"sand_machine=DEMOBOARD"	\
	"mmc_write_boot=mtdparts default;mmc rescan; fatload mmc 0 80800000 u-boot.bin;nand erase uboot; nand write 80800000 uboot\0" \
	"usb_write_boot=mtdparts default;usb start; fatload usb 0 80800000 u-boot.bin;nand erase uboot; nand write 80800000 uboot\0" \
	"mmc_write_kernel=mtdparts default;mmc rescan; fatload mmc 0 80800000 uImage;nand erase kernel; nand write 80800000 kernel\0" \
	"usb_write_kernel=mtdparts default;usb start; fatload usb 0 80800000 uImage;nand erase kernel; nand write 80800000 kernel\0" \
	"mmc_write_kernel_ro=mtdparts default;mmc rescan; fatload mmc 0 80800000 uImage;nand erase kernel_ro; nand write 80800000 kernel_ro\0" \
	"usb_write_kernel_ro=mtdparts default;usb start; fatload usb 0 80800000 uImage;nand erase kernel_ro; nand write 80800000 kernel_ro\0" \
	"mmc_write_cramfs=mtdparts default;mmc rescan; fatload mmc 0 80800000 cramfs;nand erase cramfs; nand write 80800000 cramfs\0" \
	"usb_write_cramfs=mtdparts default;usb start; fatload usb 0 80800000 cramfs;nand erase cramfs; nand write 80800000 cramfs\0" \
	"ubi_format=mtdparts default;nand erase rootfs;ubi part rootfs;ubi create ubifs\0"
#endif

//#define CONFIG_ETH1ADDR  64:46:00:00:00:02

#define CONFIG_HAS_ETH1
#define CONFIG_CMD_NET
#define CONFIG_NET_RETRY_COUNT	100
#define CONFIG_CMD_DHCP
#define CONFIG_NET_MULTI
#define CONFIG_ETH_PRIME

#define CONFIG_MXC_FEC
#define CONFIG_MII
#define CONFIG_DISCOVER_PHY

#define CONFIG_DRIVER_TI_EMAC

#define CONFIG_FEC0_IOBASE FEC_BASE
#define CONFIG_FEC0_PINMUX -1
#define CONFIG_FEC0_PHY_ADDR 0x1
#define CONFIG_FEC0_MIIBASE -1

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP	/* undef to save memory */
#define CONFIG_SYS_PROMPT	"MX25 U-Boot > "
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE	256  /* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE /* Boot Argument Buf Size */

#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM_1	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END	0x10000

#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR /* default load address */

#define CONFIG_SYS_HZ			1000

#define UBOOT_IMAGE_SIZE	  0x80000	 //0x80000	


#define CONFIG_CMDLINE_EDITING	1

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128 * 1024) /* regular stack */

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#if defined(CONFIG_IPS420)
 #ifdef CONFIG_IPS420_64M
  #define CONFIG_NR_DRAM_BANKS    1
  #define PHYS_SDRAM_1            CSD0_BASE
  #define PHYS_SDRAM_1_SIZE       (64 * 1024 * 1024)
 #elif defined(CONFIG_IPS420_MDDR)
  #define CONFIG_NR_DRAM_BANKS	1
  #define PHYS_SDRAM_1		CSD0_BASE
  #define PHYS_SDRAM_1_SIZE	(128 * 1024 * 1024)
 #else //ONLY CONFIG_IPS420
  #define CONFIG_NR_DRAM_BANKS	2
  #define PHYS_SDRAM_1		CSD0_BASE
  #define PHYS_SDRAM_1_SIZE	(64 * 1024 * 1024)
  #define PHYS_SDRAM_2		CSD1_BASE
  #define PHYS_SDRAM_2_SIZE	(64 * 1024 * 1024)
 #endif
#else
 #define CONFIG_NR_DRAM_BANKS	1
 #define PHYS_SDRAM_1		CSD0_BASE

 /* iMX25 V-1.0 has 128MB but V-1.1 has only 64MB */
 #ifdef CONFIG_MX25_3DS_V10
  #define PHYS_SDRAM_1_SIZE	(128 * 1024 * 1024)
 #else
  #define PHYS_SDRAM_1_SIZE       (64 * 1024 * 1024)
 #endif
#endif

/* Monitor at beginning of flash */
/* #define CONFIG_FSL_ENV_IN_MMC */
#define CONFIG_FSL_ENV_IN_NAND

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#if defined(CONFIG_FSL_ENV_IN_NAND)
	#define CONFIG_ENV_IS_IN_NAND 1
	#define CONFIG_ENV_OFFSET	0x80000
#elif defined(CONFIG_FSL_ENV_IN_MMC)
	#define CONFIG_ENV_IS_IN_MMC	1
	#define CONFIG_ENV_OFFSET	(768 * 1024)
#else
	#define CONFIG_ENV_IS_NOWHERE	1
#endif

#define CONFIG_ENV_SECT_SIZE	(256 * 1024)
#define CONFIG_ENV_SIZE         CONFIG_ENV_SECT_SIZE

/*-----------------------------------------------------------------------
 * NAND FLASH driver setup
 */
#define CONFIG_SYS_NO_FLASH
#define NAND_MAX_CHIPS	        	1
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000

/*
 * JFFS2 partitions TODO:
 */
#undef CONFIG_JFFS2_CMDLINE
#define CONFIG_JFFS2_DEV	"nand0"

#endif /* __CONFIG_H */

