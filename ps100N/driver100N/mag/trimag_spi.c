#include "trimag_spi.h"
#include <linux/device.h>
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h>   /* kmalloc() */
#include <linux/fs.h>   /* everything... */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>  /* O_ACCMODE */
#include <linux/aio.h>
#include <asm/uaccess.h>
//#include <asm/semaphore.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/spinlock.h> //for request_irq
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
//#include <asm/arch-s3c2410/gpio.h>
//#include <asm/arch-s3c2410/regs-gpio.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/mm.h>
//#include <asm/new_fifo.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>

#include <asm/io.h> /* for virt_to_phys */
#include <linux/slab.h> /* for kmalloc and kfree */

//#include <linux/osdriver.h> //sand api

#include <linux/interrupt.h>

#include <linux/timer.h>

#include <linux/io.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/gpio.h>
#include <mach/hardware.h>
#include <mach/irqs.h>


#include <linux/types.h>
#include <mach/gpio.h>
#include "mx25_pins.h"
#include "osdriver.h"

#include <linux/kthread.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <mach/hardware.h>
#include <mach/irqs.h>
#include "iomux.h"


#include <linux/kthread.h>

extern unsigned int spi_single_read(void);
extern unsigned int spi_cpld_read(unsigned int offset);


#define DEVICE_NAME     "s3c2410_mag"
//extern spinlock_t spilock;
extern struct semaphore sem;
struct task_struct *pid_mag;
wait_queue_head_t magqueue;
unsigned int magflag = 0;

/**************************************************************************/
struct mag_data
{
  char    err;
  char    tracklen;
  char    *track;
};

struct trimag_drvdata
{
  struct semaphore    sem;
  struct timer_list   timer;
  struct spi_device *spi;
  char    track_data[TRIMAG_RAWTRACKLEN];
  struct mag_data magtrack[3];
  char    track_valid;
  spinlock_t lock;
  struct work_struct work;
};

static void *kmalloc_ptr;

//struct timer_list  mag_timer;

#define NPAGES 1

/**************************************************************************/


struct trimag_spi_dev
{
    struct cdev cdev;
    struct class *class;
    dev_t devno;
    struct mag_data magtrack[3];
    DRV_OUT *kmalloc_area;
    int wait_flag;
}  *trimag_spi_devp;

struct trimag_spi_dev  *trimag_dev;


void maglib_reversaltrack(char * track,int tracklen)
{
    int i,j;
    char maskp,maskt;
    char ptr[256];

    memset(ptr,0x00,sizeof(ptr));
    memcpy(ptr,track,tracklen);
    memset(track,0x00,tracklen);

    for(i=0;i<tracklen;i++)
    {
      for(j=0,maskp=0x80,maskt=0x01;j<8;j++,maskp>>=1,maskt<<=1)
      {
        if(ptr[i] & maskp)
        {
          track[tracklen -i-1] |= maskt;
        }
      }
    }

    return;
}


static int  maglib_decodetrack(char *track,int tracklen,unsigned char codebits,struct mag_data * mag_data)
{
  int i,len;
  char mask,j,bits,lrc;
  char  bvalid;

  char ptr[256];

//  printk("raw track data bits %d\n",codebits);

  i = 0;
  len = 0;
  lrc = 0;
  bvalid = false;
  bits =0;

  memset((void*)mag_data,0x00,sizeof(struct mag_data));
  memset(ptr,0x00,sizeof(ptr));

  for(i=0;i<tracklen;i++)
  {
    if((track[i] == 0x00)&&(ptr[len] == 0x00))
    {
      if(bvalid)
      {
        break;
      }
      else
        continue;
    }
//    printk("track[%d]=%02X :",i,track[i]);
    for(j=0,mask=0x80;j<8;j++,mask>>=1)
    {
      if(track[i]&mask)
      {
        ptr[len] |= (0x01<<bits);
//        printk("ptr[%d]=%02X ",len,ptr[len]);
        if(!bvalid)
          bvalid = true;
      }
      if(bvalid)
      {
        bits++;
        if(bits>= codebits)
        {
          bits = 0;
          len ++;
        }

      }
    }
//    printk("\n");

  }

  if(len > 0)
  {
    if(ptr[len-1]==0x00)
      len --;

//    printk("before lrc\n");
    lrc =0;
    for(i=0; i<len; i++)
    {
      lrc ^= ptr[i];
//      printk("%02X ",ptr[i]);
      for(j=0,bits =0,mask =0x01;j<codebits;j++,mask<<=1)
      {
        if( ptr[i] &mask)
        {
          bits ++;
        }
      }

      if(!(bits%2))
      {
        break;
      }
      else
      {
        ptr[i] &= (~(0x01<<((codebits)-1)));
      }
    }
//    printk("\n");
    lrc &= (~(0x01<<((codebits)-1)));

  }

  if((i < len) || (lrc > 0))
  {
    mag_data->err = MAGLIB_LRC;
  }
 
  mag_data->track = (char *)kzalloc(128, GFP_KERNEL);
  
  if(mag_data->track == NULL)
  {
    printk("#####mag_data->track kzalloc failed! ####\n");
    mag_data->err = KZALLOC_FAILED;
    mag_data->tracklen = 0;
    return mag_data->err;
  }

  if(!mag_data->err)
  {

    memcpy(mag_data->track, ptr, len);
    mag_data->tracklen = len;
  }

//  printk("\n\n");
//  printk("track output: [%d]----------------->\n",mag_data->tracklen);

/*  for(i=0;i<mag_data->tracklen;i++)
  {
    if((mag_data->track[i] <= 9))
    printk("%c ",(char)(mag_data->track[i]+0x30));
    if(mag_data->track[i] >= 10)
    printk("%c ",(char)(mag_data->track[i] - 10+'A'));
  }
  printk("\n\n");
  printk("output end\n\n");
*/
  return mag_data->err;
}

static int trimag_spi_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
   struct trimag_spi_dev *dev;

   u8 codebit;
   u16 uiOffset;
   int ret;

   dev = filp->private_data;


//   while(1)

  {
     if(dev->kmalloc_area->gen_status == DRV_ENDED)
     {
       codebit = dev->kmalloc_area->xxdata[0];
       uiOffset = codebit+1;

//       printk("uiOffset 1 =  %d  codebit = %d\n", uiOffset,codebit);

       codebit = dev->kmalloc_area->xxdata[uiOffset+1];
       uiOffset += codebit + 2;
//       printk("uiOffset 2 =  %d codebit = %d\n", uiOffset, codebit);

       codebit = dev->kmalloc_area->xxdata[uiOffset+1];
       uiOffset += codebit + 2;
//       printk("uiOffset 3 =  %d codebit = %d\n", uiOffset, codebit);


       ret = copy_to_user(buff, &dev->kmalloc_area->drv_status , 1);
       if(ret != 0)
       {
         printk("copy_to_user failed  ret = %d\n", ret);
       }
       ret = copy_to_user(&buff[1], dev->kmalloc_area->xxdata, uiOffset);
       if(ret != 0)
       {
         printk("copy_to_user failed  ret = %d\n", ret);
       }

       dev->kmalloc_area->gen_status = 0;

       return uiOffset+1;
     }
   }

   return 0;
}

void simple_vma_open(struct vm_area_struct *vma)
{
      memset(trimag_dev->kmalloc_area, 0, PAGE_SIZE);
}

void simple_vma_close(struct vm_area_struct *vma)
{
}


/*
 * The remap_pfn_range version of mmap.  This one is heavily borrowed
 * from drivers/char/mem.c.
 */

static struct vm_operations_struct simple_remap_vm_ops = {
    .open =  simple_vma_open,
    .close = simple_vma_close,
};

static int trimag_mmap(struct file *filp, struct vm_area_struct *vma)
{
   struct trimag_spi_dev *dev=filp->private_data;

   int ret;
   long length = vma->vm_end - vma->vm_start;

   /* check length - do not allow larger mappings than the number of
      pages allocated */
   if (length > PAGE_SIZE)
   {
       return -EIO;
   }

   vma->vm_ops = &simple_remap_vm_ops;

   vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

   /* map the whole physically contiguous area in one piece */
   if ((ret = remap_pfn_range(vma,
                  vma->vm_start,
                  virt_to_phys((void *)dev->kmalloc_area) >> PAGE_SHIFT,
                  length,
                  vma->vm_page_prot)) < 0) {
                   return ret;
   }

   simple_vma_open(vma);


   return 0;
}
static int spi_trimag_release(struct inode *inode, struct file *filp);
static int spi_trimag_open(struct inode *inode, struct file *filp);

static struct file_operations trimag_ops ={
    .owner = THIS_MODULE,
    .read =  trimag_spi_read,
    .open = spi_trimag_open,
    .mmap = trimag_mmap,
    .release = spi_trimag_release,
};

static int trimag_setup_cdev(struct cdev *dev, int index)
{
    int err=0;

    cdev_init(dev,&trimag_ops);

    dev->owner = THIS_MODULE;
    dev->ops = &trimag_ops;
    err = cdev_add(dev,trimag_dev->devno,1);
    if(err)
    {
        printk(KERN_NOTICE "Error %d adding GZ%d", err, index);
        goto a;
    }

    trimag_dev->class = class_create(THIS_MODULE, "trimag_class");
    if (IS_ERR(trimag_dev->class))
    {
        err = -EINVAL;
        pr_err("trimag: failed in creating class.\n");
        goto b;
    }

    if(NULL==device_create(trimag_dev->class, NULL, trimag_dev->devno, NULL, DEVICE_NAME))
    {
        err = -EINVAL;
        pr_err("trimag: failed in creating class device.\n");
        goto c;
    }
    return err;
c:
    device_destroy(trimag_dev->class,trimag_dev->devno);
    class_destroy(trimag_dev->class);
b:
    cdev_del(&trimag_dev->cdev);
a:
//    set_scanpwr(false);
    return err;
}

#if 0
static void spi_int(void)
{
  u8 * addr_mag ;
  u8 data;

  addr_mag = (unsigned char *)ioremap(0xb2800000, 1);

  *addr_mag &= (~(1<<7));
  mdelay(100);
  *addr_mag |= (1<<7);

  mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_FUNC);
  mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_ALT1);
  mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_ALT2);
  mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_ALT3);
  mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_ALT4);
  mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_ALT5);
  mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_ALT6);
  mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_ALT7);
  mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_SION);
//spin_lock(&spilock);
down(&sem);
  mxc_request_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_ALT5);
  mxc_iomux_set_pad(MX25_PIN_VSTBY_ACK, PAD_CTL_PKE_ENABLE|PAD_CTL_100K_PU|PAD_CTL_DRV_3_3V);
  gpio_request(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), "ack");
  gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 1);

  gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 0);
  spi_cpld_read(0x16);
  gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 1);
//spin_unlock(&spilock);
up(&sem);
down(&sem);
//spin_lock(&spilock);
  gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 0);
  data = spi_cpld_read(0x05);
  gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 1);
//spin_unlock(&spilock);
up(&sem);

  iounmap(addr_mag);
}
#endif

static void spi_int(void)
{
  u8 * addr_mag ;
  u8 data;

  //addr_mag = (unsigned char *)ioremap(0xb2800000, 1);

  *addr_mag &= (~(1<<7));
  mdelay(100);
  *addr_mag |= (1<<7);

  mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_FUNC);
  mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_ALT1);
  mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_ALT2);
  mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_ALT3);
  mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_ALT4);
  mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_ALT5);
  mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_ALT6);
  mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_ALT7);
  mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_SION);
//spin_lock(&spilock);
down(&sem);
  mxc_request_iomux(MX25_PIN_CSI_D6, MUX_CONFIG_ALT7);
  mxc_iomux_set_pad(MX25_PIN_CSI_D6, PAD_CTL_PKE_ENABLE|PAD_CTL_100K_PU|PAD_CTL_DRV_3_3V);
  gpio_request(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), "ack");
  gpio_direction_output(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 1);

  gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 0);
  spi_cpld_read(0x16);
  gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 1);
//spin_unlock(&spilock);
up(&sem);
down(&sem);
//spin_lock(&spilock);
  gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 0);
  data = spi_cpld_read(0x05);
  gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 1);
//spin_unlock(&spilock);
up(&sem);

  iounmap(addr_mag);
}

static void Mag_Poll(unsigned long arg)
{
  struct trimag_spi_dev *dev;
printk("p0ll ---------------------------\n");
  u8 data;
  u16 j;
  u8 track_data_d[300];
  u8 codebit;
  u16 uiOffset;

  dev = (struct trimag_spi_dev *)arg;

//  if last data haven't read , return
//  if(dev->kmalloc_area->gen_status == DRV_ENDED)
//  {
//    mag_timer.expires = jiffies + HZ/10;
//    add_timer(&mag_timer);
//    return;
//  }
//spin_lock(&spilock);
while(1)
	{
	if(magflag)
		{
			break;
		}
		
	  if(!wait_event_interruptible_timeout(magqueue, !dev->wait_flag,100))
		  continue;
//	wait_event_interruptible_timeout(magqueue, !dev->wait_flag,100)
//printk("k");
	  down(&sem);
	  gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 0);
	  data = spi_cpld_read(0x06);
	  gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 1);
	//  up(&sem);
	//  down(&sem);
	//spin_unlock(&spilock);
	  //printk("m=%02X\n",data);

	  if((data&0xFB)==0x00)
	  {
	//spin_lock(&spilock);
	//down(&sem);
		gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 0);
		data = spi_cpld_read(0x04);
		//gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 1);

	//    printk("\nmag command  04 = %02X\n",data);

		if((data&0xFB)==0x00)
		{
	//      printk("driver data start\n\n");
	/******test lock******
	mdelay(5000);
	printk("mag open+++++++++++++++++++++++++++===\n");
	mdelay(5000);
	spin_lock(&spilock);
	while(1)
	{
	mdelay(1000);
	printk("mag");
	}
	spin_unlock(&spilock);
	******test lock******/
		  for(j = 1; j <=TRIMAG_RAWTRACKLEN ; j++)
		  {
		    //gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 0);
		    track_data_d[j] = spi_single_read();
		    //gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 1);

	//        printk("[%d]= %02x  ", j,track_data_d[j]);
	//        if((j % 10) == 0)
	//        {
	//          printk("\n");
	//        }
		  }

		  //gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 0);
		  spi_cpld_read(0x16);
		  //gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 1);

		  //gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 0);
		  spi_cpld_read(0x05);
		  gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 1);
	//spin_unlock(&spilock);
	up(&sem);

		  for(j=0; j<3; j++)
		  {
		    if(j == 0)
		    {
		      codebit = 7;
		    }
		    else
		    {
		      codebit = 5;
		    }

		    maglib_decodetrack(&(track_data_d[1 + 96*j]), 96, codebit, &dev->magtrack[j]);

		    if(dev->magtrack[j].err == MAGLIB_LRC)
		    {
		      if( ((j == 0) && (dev->magtrack[j].track[0] == 0x05))
		            || ((j != 0) && (dev->magtrack[j].track[0] == 0x0B)))
		      {
		        continue;
		      }
		      
	//          printk("***err[%d]= %02X ***\n", j,dev->magtrack[j].err);
		      maglib_reversaltrack(&track_data_d[1 + (96*j)], 96);
		      maglib_decodetrack(&(track_data_d[1 + (96*j)]), 96, codebit, &dev->magtrack[j]);
		    }
		    else
		    {
		      if( ((j == 0) && (dev->magtrack[j].track[0] == 0x05))
		            || ((j != 0) && (dev->magtrack[j].track[0] == 0x0B)))
		      {
		        continue;
		      }
		      
	//          printk("***err[%d]= %02X ***\n", j,dev->magtrack[j].err);
		      maglib_reversaltrack(&track_data_d[1 + (96*j)], 96);
		      maglib_decodetrack(&(track_data_d[1 + (96*j)]), 96, codebit, &dev->magtrack[j]);
		    }
		  }

		  dev->kmalloc_area->drv_status = dev->magtrack[1].err;
		  uiOffset = 0;
		  if (dev->magtrack[1].tracklen >= 3)
		  {
		      dev->kmalloc_area->xxdata[uiOffset++] =  dev->magtrack[1].tracklen - 3;
		      memcpy(&dev->kmalloc_area->xxdata[uiOffset], &dev->magtrack[1].track[1], dev->magtrack[1].tracklen-3);
		      uiOffset +=  dev->magtrack[1].tracklen - 3;
		  }
		  else
		  {
		      dev->kmalloc_area->xxdata[uiOffset++] =  0;
		  }

		  dev->kmalloc_area->xxdata[uiOffset++]=dev->magtrack[2].err;
		  if (dev->magtrack[2].tracklen >= 3)
		  {
		      dev->kmalloc_area->xxdata[uiOffset++] = dev->magtrack[2].tracklen - 3;
		      memcpy(&dev->kmalloc_area->xxdata[uiOffset], &dev->magtrack[2].track[1], dev->magtrack[2].tracklen - 3);
		      uiOffset += dev->magtrack[2].tracklen-3;
		  }
		  else
		  {
		      dev->kmalloc_area->xxdata[uiOffset++]=0;
		  }

		  dev->kmalloc_area->xxdata[uiOffset++] = dev->magtrack[0].err;
		  if (dev->magtrack[0].tracklen >= 3)
		  {
		      dev->kmalloc_area->xxdata[uiOffset++] = dev->magtrack[0].tracklen - 3;
		      memcpy(&dev->kmalloc_area->xxdata[uiOffset], &dev->magtrack[0].track[1], dev->magtrack[0].tracklen - 3);
		  }
		  else
		  {
		      dev->kmalloc_area->xxdata[uiOffset++]=0;
		  }


		  uiOffset = 0;

	//      printk("1kmalloc_area->xxdata[0] =  %d \n", dev->kmalloc_area->xxdata[uiOffset]);

		  codebit = dev->kmalloc_area->xxdata[uiOffset++];

		  for(j=0; j< codebit; j++)
		  {
		    dev->kmalloc_area->xxdata[uiOffset] += '0';

	//        printk("%c ", dev->kmalloc_area->xxdata[uiOffset]);
	//        if ((j+1)%21==0) printk("\n");

		    uiOffset++;
		  }
	//      printk("\n\n");

		  uiOffset++;

	//      printk("2kmalloc_area->xxdata[1] =  %d \n", dev->kmalloc_area->xxdata[uiOffset]);

		  codebit = dev->kmalloc_area->xxdata[uiOffset++];
		  for(j=0; j< codebit; j++)
		  {
		    dev->kmalloc_area->xxdata[uiOffset] += '0';

	//        printk("%c ", dev->kmalloc_area->xxdata[uiOffset]);
	//        if ((j+1)%21==0) printk("\n");

		    uiOffset++;
		  }
	//      printk("\n\n");

		  uiOffset++;

	//      printk("3kmalloc_area->xxdata[2] =  %d \n", dev->kmalloc_area->xxdata[uiOffset]);

		  codebit = dev->kmalloc_area->xxdata[uiOffset++];
		  for(j=0; j< codebit; j++)
		  {
		    dev->kmalloc_area->xxdata[uiOffset] += ' ';

	//        printk("%c ", dev->kmalloc_area->xxdata[uiOffset]);
	//        if ((j+1)%21==0) printk("\n");

		    uiOffset++;
		  }

	 //     printk("\n\n");
	  //    printk("driver data end\n\n");

	  //    for(j=0; j<255; j++)
	  //    {
	  //      if((j%2) == 0)
	  //      {
	  //        dev->kmalloc_area->xxdata[j] = 0x5A;
	  //      }
	  //      else
	  //      {
	  //        dev->kmalloc_area->xxdata[j] = 0xA5;
	  //      }
	  //    }

		  dev->kmalloc_area->gen_status = DRV_ENDED;
		  dev->kmalloc_area->reqnr = 0;

		  kfree(dev->magtrack[0].track);
		  kfree(dev->magtrack[1].track);
		  kfree(dev->magtrack[2].track);
		  //wait_event_interruptible(magqueue, 0);
		  dev->wait_flag = 1;
	////      del_timer(&mag_timer);
	//      mdelay(5);
//		  return;
		}
		else
		{  
		//		spin_unlock(&spilock);
		up(&sem);
		}
	  }
	  else
		{  
		up(&sem);
	//		spin_unlock(&spilock);
		}

	////  mag_timer.expires = jiffies + HZ/10;
	////  add_timer(&mag_timer);
	//  mdelay(5);
	msleep_interruptible(20);
	}
}

static int spi_trimag_release(struct inode *inode, struct file *filp)
{
	trimag_dev->wait_flag = 1;
////	del_timer(&mag_timer);
}

static int spi_trimag_open(struct inode *inode, struct file *filp)
{
    struct trimag_spi_dev *dev;

    dev = container_of(inode->i_cdev, struct trimag_spi_dev, cdev);

    filp->private_data = dev;

//    spi_int();
    
////	setup_timer( &mag_timer, Mag_Poll, (unsigned long )trimag_dev );
////    mag_timer.expires = jiffies + HZ/10;
////   add_timer(&mag_timer);
	dev->wait_flag = 0;
	wake_up_interruptible(&magqueue);

    return 0;
}

static int spi_trimag_init(void)
{
    int ret = -ENODEV;
	int rc;
	
    dev_t dev = 0;

    int mag_major =  0;
    int mag_minor =   0;

	magflag = 0;
	
    if (mag_major) {
        dev = MKDEV(mag_major, mag_minor);
        ret = register_chrdev_region(dev, 1, DEVICE_NAME);
    } else {
        ret = alloc_chrdev_region(&dev, mag_minor, 1, DEVICE_NAME);
        mag_major = MAJOR(dev);
    }
    if (ret < 0)
    {
        printk( "##### mag driver: can't get major %d\n", mag_major);
         goto failed_g;
    }

    trimag_dev = (struct trimag_spi_dev *)kmalloc( (sizeof(struct trimag_spi_dev)+sizeof(DRV_OUT))*PAGE_SIZE, GFP_KERNEL);
    if(trimag_dev == NULL)
    {
        ret = -ENOMEM;
        goto failed_g;
    }

	 init_waitqueue_head(&magqueue);
    memset(trimag_dev, 0, sizeof(struct trimag_spi_dev) );
	trimag_dev->wait_flag = 1;
    /* allocate a memory area with kmalloc. Will be rounded up to a page boundary */

    kmalloc_ptr = kmalloc(PAGE_SIZE, GFP_KERNEL);

    if (kmalloc_ptr == NULL)
    {
         ret = -ENOMEM;
         goto failed_g;
    }
    /* round it up to the page bondary */
    trimag_dev->kmalloc_area = (DRV_OUT *)kmalloc_ptr;

   SetPageReserved(virt_to_page((DRV_OUT  *)trimag_dev->kmalloc_area) );

   trimag_dev->devno = MKDEV(mag_major, mag_minor);

   ret=trimag_setup_cdev(&(trimag_dev->cdev), 0);

//   printk("trimag_setup_cdev %d\n", ret);

   spi_int();
   
   printk("before poll-------------------\n");
	pid_mag = kthread_create(Mag_Poll, (unsigned long )trimag_dev, "mag_poll");
	if(IS_ERR(pid_mag))
		printk("kthread_create  --------------err\n");
	wake_up_process(pid_mag);
////   setup_timer( &mag_timer, Mag_Poll, (unsigned long )trimag_dev );
////   mag_timer.expires = jiffies + HZ/10;
////   add_timer(&mag_timer);

   printk("trimag driver init success \n");

   return 0;

failed_g:

   unregister_chrdev_region(trimag_dev->devno, 1);
   device_destroy(trimag_dev->class,trimag_dev->devno);
   class_destroy(trimag_dev->class);
   cdev_del(&trimag_dev->cdev);

   printk("####%s,%d  failed ####\n", __func__,__LINE__);

  return 1;
}

static void __exit spi_trimag_exit(void)
{
    pr_alert("trimag exit\n");

////    del_timer(&mag_timer);
	magflag = 1;
//	mdelay(100);
	kthread_stop(pid_mag);
	pid_mag = NULL;
	
    gpio_request(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), NULL);
    mxc_free_iomux(MX25_PIN_VSTBY_ACK, MUX_CONFIG_GPIO);

    unregister_chrdev_region(trimag_dev->devno, 1);
    device_destroy(trimag_dev->class,trimag_dev->devno);
    class_destroy(trimag_dev->class);
    cdev_del(&trimag_dev->cdev);

//    ClearPageReserved(virt_to_page(p));

    kfree(trimag_dev);
}

module_init(spi_trimag_init);
module_exit(spi_trimag_exit);

MODULE_AUTHOR("wangyoulin<wang.youlin@sand.com.cn>");
MODULE_LICENSE("Dual BSD/GPL");
