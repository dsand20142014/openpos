#include "sahara2/include/portable_os.h"
#include "fuse_program.h"
#include <mach/gpio.h>
#include <linux/io.h>
#include <linux/module.h>

#include <linux/gpio.h>
#include <linux/platform_device.h>

#include <mach/hardware.h>
#include <mach-mx25/mx25_pins.h>
#include <mach-mx25/iomux.h>

static int dbg = 0;
module_param(dbg, int, S_IRUGO);

#define fuse_debug(fmt, arg...)  do{if(dbg)os_printk(KERN_DEBUG fmt,##arg);}while(0)
#define fuse_info(fmt, arg...)       do{if(dbg)os_printk(KERN_INFO fmt,##arg);}while(0)

static struct clk *iim_clk;
static volatile void *iim_base;
void config_fuse_gpio(void);
void set_fuse_gpio(unsigned char ucMode);
static int fuse_byte_read(e_fuse_banks fuse_bank, unsigned char fuse_row_addr, unsigned char *val);
static int fuse_bit_program(e_fuse_banks fuse_bank, unsigned char fuse_row_addr,unsigned char fuse_bit_addr) ;
static void set_fuse_address(e_fuse_banks fuse_bank, unsigned short fuse_row_addr, unsigned char fuse_bit_addr) ;
static int fuse_program(void);

//#define FUSE_OVERRIDE

void config_fuse_gpio(void)
{
	  mxc_free_iomux(MX25_PIN_EB1,MUX_CONFIG_ALT5);
	  mxc_request_iomux(MX25_PIN_EB1, MUX_CONFIG_ALT5);
	  mxc_iomux_set_pad(MX25_PIN_EB1, PAD_CTL_PKE_ENABLE|PAD_CTL_100K_PU|PAD_CTL_DRV_3_3V);
	  gpio_request(IOMUX_TO_GPIO(MX25_PIN_EB1), NULL);
	  gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_EB1), 0);	 
}

void set_fuse_gpio(unsigned char ucMode)
{
	 gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_EB1), ucMode);
}
static int fuse_byte_program(e_fuse_banks fuse_bank, unsigned char fuse_row_addr, unsigned char val)
{
  int retval=0;
#ifdef FUSE_OVERRIDE
	unsigned int offset;

	offset = (0x0800 + (fuse_bank * 0x400)) + fuse_row_addr;
	*(unsigned char *)(iim_base + offset) = val;
  
#else
  
  int i;
 
	set_fuse_gpio(1);  //should add gpio control the power of fuse_program

 for(i=7;i>=0;i--)
    {
      if((val & (1 << i)))
	  retval = fuse_bit_program(fuse_bank,fuse_row_addr,i);
      if(retval)
	{
	  printk(KERN_INFO "error was 0x%08X\n",retval);
	  break;
	}
    }

  /* Turn it off to aviod an accident */
	set_fuse_gpio(0);
  
#endif  
  return retval;
}

//fuse value read by byte to check it is correct or not.
static int fuse_byte_read(e_fuse_banks fuse_bank, unsigned char fuse_row_addr, unsigned char *val)
{
   unsigned char error;
  //Define the fuse bit address that we want to read. 
  set_fuse_address(fuse_bank, fuse_row_addr, 0); 
  //write a value other than 0xAA to prohibit  fuse-progaram.
  IIM_WRITE_REGISTER(MXC_IIMPRG_P, 0x0);
  //initiates an 1-stressed explicit sense cycle
  IIM_WRITE_REGISTER(MXC_IIMFCTL,0x02);
   //Wait until fuse sensing is finished.
    while( (IIM_READ_REGISTER(MXC_IIMSTAT) & 0x1) == 0 ); 
  //Write 1 to clear PRGD bit
  IIM_WRITE_REGISTER(MXC_IIMSTAT,0x01); 
  error = IIM_READ_REGISTER(MXC_IIMERR); 
  if (error & 0xFE) 
    { 
      //Clear Error Status Register. By writing the same value we got. 
      //These are Clear-on-Write type of bits. 
      IIM_WRITE_REGISTER(MXC_IIMERR, error); 
      //Some error occurred. 
      return error; 
    } 
    *val = IIM_READ_REGISTER(MXC_IIMSDAT);
	return 0;
}

//Returns 0 when Successful.
static int fuse_bit_program(e_fuse_banks fuse_bank, unsigned char fuse_row_addr,unsigned char fuse_bit_addr) 
{ 
  unsigned char error; 
  //Define the fuse bit address that we want to program. 
  set_fuse_address(fuse_bank, fuse_row_addr, fuse_bit_addr); 
  //write 0xAA to Program Protection Register (PRG_P) register 
  
  IIM_WRITE_REGISTER(MXC_IIMPRG_P, 0xAA); 
  //Enable and Start Fuse Programming via Fuse Control Register(FCTL)
 
  IIM_WRITE_REGISTER(MXC_IIMFCTL,0x71); 
  //Wait until fuse blowing is finished.
 
  while( (IIM_READ_REGISTER(MXC_IIMSTAT) & 0x2) == 0 ); 
  //Write 1 to clear PRGD bit
  IIM_WRITE_REGISTER(MXC_IIMSTAT,0x02);  
  //Very good to do too. for safety. 
  IIM_WRITE_REGISTER(MXC_IIMPRG_P, 0x0); 
  //Check Error status.
  
  error = IIM_READ_REGISTER(MXC_IIMERR); 
  if (error & 0xFE) 
    { 
      //Clear Error Status Register. By writing the same value we got. 
      //These are Clear-on-Write type of bits. 
      IIM_WRITE_REGISTER(MXC_IIMERR, error); 
      //Some error occurred. 
      return error; 
    } 
  //No error at all. 
  return 0; 
} 



static void set_fuse_address(e_fuse_banks fuse_bank, unsigned short fuse_row_addr, unsigned char fuse_bit_addr) 
{ 
  unsigned char upper_addr; 
  unsigned char lower_addr; 
  unsigned char fuse_index; 
  //A fuse bank contains 256 fuse rows. for a total of 2048 fuse bits. 
  //Dividing the fuse_address by 4 will give us the fuse index. 
  fuse_index = (unsigned char)(fuse_row_addr >> 2); 
  upper_addr = ( (unsigned char)fuse_bank << 3); 
  upper_addr |= ((fuse_index >> 5) & 0x7); 
  lower_addr = ((fuse_index & 0x1F) << 3); 
  lower_addr |= (fuse_bit_addr & 0x7); 
  //write address to UA LA register
  IIM_WRITE_REGISTER(MXC_IIMUA, upper_addr);
  IIM_WRITE_REGISTER(MXC_IIMLA, lower_addr); 
  }



static int fuse_program(void)
{
  int retval;
  unsigned char fuse_value;
  int i,srk_index;
   unsigned char SRK_hash[]= {
  	        0x79,0xca,0x3c,0xc1,0xca,0xae,0x69,0x28,0x03,
		0xb9,0xb5,0x4a,0xfd,0xeb,0x2d,0xad,0xe3,0x94,
		0x17,0xe3,0x95,0x57,0xd0,0x68,0x92,0xbf,0x88,
		0xc3,0x62,0x59,0xb1,0xb0};  
     	                                         
    fuse_value = 0xff;

  	config_fuse_gpio();      
     fuse_byte_program(FUSE_BANK_0,0x04,0x70);   
     fuse_byte_program(FUSE_BANK_0,0x10,0x2A);   //0x29: engineering mode  2A:  security mode
     fuse_byte_read(FUSE_BANK_0,0x10,&fuse_value);
    //fuse_value = *(unsigned char *)(iim_base+FUSE_BANK0_BASE+0x10);
     fuse_info("fuse_value10=%02x\n",fuse_value);
     //fuse_value = *(unsigned char *)(iim_base+FUSE_BANK0_BASE+0x04);
    fuse_byte_read(FUSE_BANK_0,0x04,&fuse_value);
     printk("fuse0_04=%02x\n",fuse_value);
	
	fuse_byte_program(FUSE_BANK_0,0x14,0x43); 
	 
       fuse_byte_read(FUSE_BANK_0,0x14,&fuse_value);
       //fuse_value = *(unsigned char *)(iim_base+FUSE_BANK0_BASE+0x14);
   fuse_info("fuse_value14=%02x\n",fuse_value);  

   for(srk_index=MAX_FUSE_REGS-1, i=0;i<(SRK_SZ-1);i++,srk_index--)
    {       
      fuse_byte_program(FUSE_BANK_2,srk_index*4,SRK_hash[i]);
    }
   
   fuse_byte_program(FUSE_BANK_0,0x50,SRK_hash[SRK_SZ-1]);
  for(i=1;i<SRK_SZ;i++)
  	{
            //fuse_value = *(unsigned char *)(iim_base+FUSE_BANK2_BASE+(4*i));
            fuse_byte_read(FUSE_BANK_2,(4*i),&fuse_value);
            fuse_info("fuse2_%d=%02x\n",4*i,fuse_value);
  	}
  //fuse_value = *(unsigned char *)(iim_base+FUSE_BANK0_BASE+SRK_FUSE_BANK_0_ADDR);
 	fuse_byte_read(FUSE_BANK_0,SRK_FUSE_BANK_0_ADDR,&fuse_value);
            fuse_info("fuse0_50=%02x\n",fuse_value);

     fuse_byte_program(FUSE_BANK_0,0x00,0x40);	//   override protect  	
     fuse_byte_program(FUSE_BANK_1,0x00,0x40);       // override protect	
     fuse_byte_program(FUSE_BANK_2,0x00,0x40);		//override  protect	

  return 0;
}

static int __init mx258_iim_init(void)
{

	fuse_info("IIM module enter!\n");
	 iim_clk = clk_get(NULL, "iim_clk");	
	if (IS_ERR(iim_clk)) {		
		printk( KERN_ERR"No IIM clock defined\n");		
		return -ENODEV;	
		}	
		clk_enable(iim_clk);
       
            iim_base = (void *)IO_ADDRESS(IIM_BASE);
	 if (iim_base == NULL) {				
	 	printk(KERN_ERR"IIM: Register mapping failed.  Exiting.\n");		
		return -ENODEV;
		} 
	  else {
                     fuse_program();					   	
	  	}
      return 0;
}


static void __exit mx258_iim_exit(void)
{
  fuse_info("IIM module exit!\n");

    if(iim_base){
 	 iounmap((void *)iim_base);
 	}
   clk_disable(iim_clk);
	clk_put(iim_clk);
}


module_init(mx258_iim_init);
module_exit(mx258_iim_exit);
MODULE_DESCRIPTION("fuse program");
MODULE_AUTHOR("SAND<lijinmei>");
MODULE_LICENSE("GPL");

