#ifndef __MXC_FUSE_PROGRAM_H__
#define __MXC_FUSE_PROGRAM_H__



#include <mach/iim.h>
#include <mach/hardware.h>


#define IIM_BASE                                     IIM_BASE_ADDR
#define MAX_FUSE_REGS                          32
#define SRK_SZ                                            32
#define SRK_FUSE_BANK_0_ADDR          0x50

#define FUSE_BANK0_BASE        0x800
#define FUSE_BANK1_BASE        0xC00
#define FUSE_BANK2_BASE        0x1000

#define IIM_WRITE_REGISTER(offset,value)  (void)__raw_writeb(value, iim_base+(offset))
#define IIM_READ_REGISTER(offset)    __raw_readb(iim_base+(offset))    



typedef enum
   {
        FUSE_BANK_0 =  0,
	FUSE_BANK_1,
        FUSE_BANK_2,
   }e_fuse_banks;


static int fuse_byte_program(e_fuse_banks fuse_bank, unsigned char fuse_row_addr, unsigned char val);
static int fuse_byte_read(e_fuse_banks fuse_bank, unsigned char fuse_row_addr, unsigned char *val);
static int fuse_bit_program(e_fuse_banks fuse_bank, unsigned char fuse_row_addr,unsigned char fuse_bit_addr);
static void set_fuse_address(e_fuse_banks fuse_bank, unsigned short fuse_row_addr, unsigned char fuse_bit_addr) ;
static int fuse_program(void);

#endif





























