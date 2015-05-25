#ifndef __FPGA_REGS_H__
#define __FPGA_REGS_H__

#define FPGA_BASE 0xB2000000

/*KEYBOARD*/
#define KEY_BUZZER (FPGA_BASE+0x07)
#define KEY_LIGHT (FPGA_BASE+0x08)

/*PRINTER*/
#define RegPrintCtl0	(FPGA_BASE+0x10)
#define RegPrintCtl1	(FPGA_BASE+0x11)
#define RegPrintCtl2	(FPGA_BASE+0x12)
#define RegPrintCtl3	(FPGA_BASE+0x13)
#define RegPrintCtl4	(FPGA_BASE+0x14)
#define RegPrintData	(FPGA_BASE+0x130)
#define RegPrintSize	(0x17E8)

/*ICC*/

/*MIFARE*/

/*MAG*/

/*STATUS*/
#define RegTxRxLED	(FPGA_BASE+0x60)
#define RegCharge	(FPGA_BASE+0x61)
#define RegFpgaVer	(FPGA_BASE+0x64)
#define RegFpgaTest	(FPGA_BASE+0x68)
#define RegFpgaReset	(FPGA_BASE+0x6C)

/*POWER*/
#define RegPower	(FPGA_BASE+0x70);

/*GSM*/
#define RegGSM		(FPGA_BASE+0x80);

/*SCAN*/
#define RegSCAN		(FPGA_BASE+0x90);

#endif//__FPGA_REGS_H__
