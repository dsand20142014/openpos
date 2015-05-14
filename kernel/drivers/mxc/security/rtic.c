
#include "sahara2/include/portable_os.h"
#include "rtic.h"
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/mxc_scc_driver.h>

static int dbg = 0;
module_param(dbg, int, S_IRUGO);
#define rtic_debug(fmt, arg...)  do{if(dbg)os_printk(KERN_DEBUG fmt,##arg);}while(0)

#define rtic_info(fmt, arg...)       do{if(dbg)os_printk(KERN_INFO fmt,##arg);}while(0)

static struct clk *imx_rtic_clk=NULL;
static volatile void *rtic_base;
struct rtic_struct *rticptr;

dev_t  rtic_major;
dev_t  rtic_minor = 0;

 extern struct fasync_struct *dryice_async_queue;

static int imx_hash_compute(unsigned char hashtype,unsigned int pinput, unsigned int uilen, unsigned int *output )
{
   unsigned int status;
   unsigned int i;

     if(pinput % 4 ||uilen % 4)
	   return -1;
     
     status = RTIC_READ_REGISTER(RTIC_VERID);
	rtic_info("rtic_veriD=%08x",status); 
     status = RTIC_READ_REGISTER(RTIC_STATUS);
	rtic_info("rtic_status=%08x",status);
     if(!(status & 0x01))
     	{             
             RTIC_WRITE_REGISTER(RTIC_MEMBLOCKB1,pinput);  //write the start address
              status = RTIC_READ_REGISTER(RTIC_MEMBLOCKB1);
	    rtic_info("rtic_Block1=%08x",status);
             RTIC_WRITE_REGISTER(RTIC_MEMLENB1,uilen);
	     status = RTIC_READ_REGISTER(RTIC_MEMLENB1);
	    rtic_info("rtic_Block1len=%08x",status);
	     RTIC_WRITE_REGISTER(RTIC_MEMLENB2,0);     //skip Memory B2
	     if(hashtype == SHA256)	        	  
	          RTIC_WRITE_REGISTER(RTIC_CTRL,0x2020);   //enable Memory B hash256 once 
	      else
		   RTIC_WRITE_REGISTER(RTIC_CTRL,0x20);  //enable Memory B hash1 once
	     RTIC_WRITE_REGISTER(RTIC_CMD,0x04);
		 while(!(RTIC_READ_REGISTER(RTIC_STATUS)&0x02));
		// for(i=0;i<50;i++);
		//if(RTIC_READ_REGISTER(RTIC_STATUS)&0x02)
		{
	            if(hashtype == SHA256)
	             {
	                for(i=0;i<8;i++)
	                {
			    output[i] = RTIC_READ_REGISTER(RTIC_HASHB_A+(4*i));
			    rtic_info("out[%d]=%02x/",i,output[i]);
	                }
	              }
		     else
		     {
	              for(i=0;i<5;i++)//5
	              	{
		            output[i] = RTIC_READ_REGISTER(RTIC_HASHB_A+(4*i));		
			    rtic_info("out[%d]=%02x/",i,output[i]);
	              	}
		     }
	     	  
		RTIC_WRITE_REGISTER(RTIC_CMD,0x03);	
		return 0;
		}
		//else 
		//	return -1;
     }
	 else
	 	return -1;
}

static int imx_hash_compute_runtime(unsigned char hashtype,unsigned int pinput, unsigned int uilen, unsigned int *output )
{
    unsigned int status;
   unsigned int i;

     if(pinput % 4 ||uilen % 4)
	   return -1;
     
     status = RTIC_READ_REGISTER(RTIC_VERID);
	rtic_info("rtic_veriD=%08x",status); 
     status = RTIC_READ_REGISTER(RTIC_STATUS);
	rtic_info("rtic_status=%08x",status);
     if(!(status & 0x01))
     	{
	    RTIC_WRITE_REGISTER(RTIC_MEMBLOCKC1,pinput);  //write the start address             
             RTIC_WRITE_REGISTER(RTIC_MEMLENC1,uilen);	     
	     RTIC_WRITE_REGISTER(RTIC_MEMLENC2,0);     //skip Memory C2
	     if(hashtype == SHA256)		     	
	     	   RTIC_WRITE_REGISTER(RTIC_CTRL,0x4040 );   //enable Memory C hash256 once 	     
	      else      		
		   RTIC_WRITE_REGISTER(RTIC_CTRL,0x40 );  //enable Memory C hash1 once 
	        RTIC_WRITE_REGISTER(RTIC_CMD,0x04);	
		 	
		 while(!(RTIC_READ_REGISTER(RTIC_STATUS)&0x02));
	       
            if(hashtype == SHA256)
             {
                for(i=0;i<8;i++)
                {
		    output[i] = RTIC_READ_REGISTER(RTIC_HASHC_A+(4*i));
		    //printf("out[%d]=%02x/",i,output[i]);
                }
              }
	     else
	     {
              for(i=0;i<5;i++)//5
              	{
	            output[i] = RTIC_READ_REGISTER(RTIC_HASHC_A+(4*i));		
		   // printf("out[%d]=%02x/",i,output[i]);
              	}
	     }     
	    RTIC_WRITE_REGISTER(RTIC_WDOG,0x0FFFFFFF);
	     if(hashtype == SHA256)
                 RTIC_WRITE_REGISTER(RTIC_CTRL,0x4400 ); //enable Memory C hash256 infinite loop   
	     else 
		  RTIC_WRITE_REGISTER(RTIC_CTRL,0x400 );  //enable Memory C hash1 infinite loop 	
		    RTIC_WRITE_REGISTER(RTIC_CMD,0x08);
               return 0;
     	}
	  else
	 	return -1;
 }



static int hash_image_test(unsigned int addr,unsigned int len)
{unsigned int *vitual_addr,*phyaddr;
  int i;
  unsigned int status;

    for(i=0;i<len/4;i++)
    	{    	   
           vitual_addr = (unsigned int *)phys_to_virt((unsigned long)(addr+4*i));           
	    *vitual_addr = 0xf0543201;
		//rtic_info("vitualaddr=%08x\n",(unsigned int)*vitual_addr);
		phyaddr = (unsigned int *)virt_to_phys(vitual_addr);
		//rtic_info("phyaddr=%08x\n",(unsigned int)phyaddr);
    	}
	     status = RTIC_READ_REGISTER(RTIC_STATUS);
	rtic_info("rtic_status=%08x",status);
	return 0;
}

static void hash_failure(void)
{
 unsigned int status;
  rtic_info("kernel image is modified.\n");
   status = RTIC_READ_REGISTER(RTIC_STATUS);
	rtic_info("rtic_status=%08x",status);
  if(dryice_async_queue)
   	{
          printk("asyn signal to task from rtic!\n");
          kill_fasync(&dryice_async_queue, SIGIO, POLL_IN);
   	}
}

static int imx_rtic_open(struct inode *inode, struct file *filp)
{
      struct rtic_struct *dev; /* device information */       
	  
        rtic_info("rtic open.\n");
        dev = container_of(inode->i_cdev, struct rtic_struct, cdev);
        filp->private_data = dev;
        
	return 0;
}

static int imx_rtic_release(struct inode *inode, struct file *filp)
{
	filp->private_data = NULL;
	rtic_info( "rtic close.\n");
	return 0;
}


static int imx_rtic_ioctl(struct inode *inode, struct file *fp,
			unsigned int cmd, unsigned long arg)
{
      int err = 0;
      struct RTIC_HASH_DATA *prtic_hash_data;
    unsigned int  phash;
       
	if (_IOC_TYPE(cmd) != RTIC_IOC_NUM) {
		printk(KERN_ERR
		       "device_ioctl() - Error! IOC_TYPE = %d. Expected %d\n",
		       _IOC_TYPE(cmd), RTIC_IOC_NUM);
		return -ENOTTY;
	}
	
	if (_IOC_NR(cmd) > RTIC_IOCTL_MAXNUMBER) {
		printk(KERN_ERR
		       "device_ioctl() - Error!"
		       "IOC_NR = %d greater than max supported(%d)\n",
		       _IOC_NR(cmd), RTIC_IOCTL_MAXNUMBER);
		return -ENOTTY;
	}

	
	if (_IOC_DIR(cmd) & _IOC_READ)
		err =
		    !access_ok(VERIFY_WRITE, (void __user *)arg,
			       _IOC_SIZE(cmd));

	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =
		    !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	rtic_info("key ioctl enter2\n");
	if (err) {
		printk(KERN_ERR
		       "device_ioctl() - Error! User arg not valid"
		       "for selected access (R/W/RW). Cmd %d\n",
		       _IOC_TYPE(cmd));
		return -EFAULT;
	}
       
	/* Note: Read and writing data to user buffer can be done using regular
	   pointer stuff but we may also use get_user() or put_user() */

	/* Cmd and arg has been verified... */
	switch (cmd) {
		case IOCTL_HASH_COMPUTE:
		{
			 prtic_hash_data=kmalloc(sizeof(struct RTIC_HASH_DATA),GFP_KERNEL);
			 if (copy_from_user(prtic_hash_data,
				   (struct RTIC_HASH_DATA *)arg,
				   sizeof(struct RTIC_HASH_DATA))) {					 
			  return -EFAULT;
		   }		
		   rtic_info("hashtype=%02x addr=%08x hashlen=%d\n",prtic_hash_data->hashtype,(unsigned int)prtic_hash_data->data,prtic_hash_data->hashlen);
		   phash = (unsigned int)virt_to_phys(prtic_hash_data->data);
		   rtic_info("phyaddr=%08x\n",phash);
		       err = 	imx_hash_compute(prtic_hash_data->hashtype,(unsigned int) phash,prtic_hash_data->hashlen,prtic_hash_data->output);		       
		       if(!err)
		       	{
                           if (copy_to_user((struct RTIC_HASH_DATA *)arg,
				 prtic_hash_data, sizeof(struct RTIC_HASH_DATA))) {
			                    return -EFAULT;
		                }
		       	}	
			 kfree(prtic_hash_data);
		         break;
		}
	case IOCTL_HASH_MEMTEST:
	       {
		struct HASH_TEST_DATA hash_test_data;
		unsigned int hash_value[5];
		 if (copy_from_user(&hash_test_data,
				   (struct HASH_TEST_DATA *)arg,
				   sizeof(struct HASH_TEST_DATA))) {					 
			  return -EFAULT;
		   }	
		 rtic_info("modifyaddr0=%08x/ modifylen0 = %d",hash_test_data.addr,hash_test_data.len);
		 err = imx_hash_compute_runtime(1,(unsigned int)hash_test_data.addr,hash_test_data.len,hash_value);
		 //if(!err)
                       //hash_image_test(hash_test_data.addr,20);	//hash_test_data.len	  
		break;
		}
	default:
		printk(KERN_ERR "device_ioctl() - Invalid IOCTL (0x%x)\n", cmd);
		return EINVAL;
		}
	return err;
}
struct file_operations rtic_Fops = {
	.owner = THIS_MODULE,
	.ioctl = imx_rtic_ioctl,
	.open = imx_rtic_open,
	.release =  imx_rtic_release,
};


static int __init mx258_rtic_init(void)
{
   int ret;
   scc_return_t scc_code;
  // unsigned int hash_value[5];
         ret = 0;
	rtic_info("RTIC module enter!\n");
	 imx_rtic_clk = clk_get(NULL, "rtic_clk");	
	if (IS_ERR(imx_rtic_clk)) {		
		printk(KERN_ERR"No RTIC clock defined\n");		
		return -ENODEV;	
		}	
		clk_enable(imx_rtic_clk);
       
              rtic_base = (void *)IO_ADDRESS(RTIC_BASE);
	 if (rtic_base == NULL) {				
	 	printk(KERN_ERR"IIM: Register mapping failed.  Exiting.\n");		
		return -ENODEV;
		} 
	   rticptr = kmalloc(sizeof(struct rtic_struct), GFP_KERNEL);
        if (!rticptr) {
                printk(KERN_ERR"RTIC device kmalloc error.\n");
                return -ENOMEM;
        }
        memset(rticptr,0,sizeof(struct rtic_struct));
		
      /* Register character device */
	ret = alloc_chrdev_region(&rticptr->dev_id,rtic_minor, 1,"imx_rtic");	
	if (ret < 0) {
		printk(KERN_ERR"can't get rtic major %d\n",rtic_major);
		goto err3;
	}
	rtic_major = MAJOR(rticptr->dev_id);
	rtic_info("major=%d\n",rtic_major);
	rtic_info("minor=%d\n",rtic_minor);
	cdev_init(&rticptr->cdev, &rtic_Fops);
	rticptr->cdev.owner = THIS_MODULE;

	ret = cdev_add(&rticptr->cdev, rticptr->dev_id, 1);
	if (ret) {
		printk(KERN_ERR"can't add RTIC cdev.\n");
		goto err2;
	}	
	
	/* create class and device for udev information */
	rticptr->cls= class_create(THIS_MODULE, "imx_rtic");
	if (IS_ERR(rticptr->cls)) {
		printk(KERN_ERR"Err!failed to create rtic class\n");
		ret = -ENOMEM;
		goto err1;
	}

	rticptr->clsdev= device_create(rticptr->cls, NULL, rticptr->dev_id, NULL,
					RTIC_DEVICE_FILE_NAME); 
	if (IS_ERR(rticptr->clsdev)) {
		printk(KERN_ERR"Err!failed to create rtic class device\n");
		ret = -ENOMEM;
		goto err0;
	}
       scc_code = scc_monitor_security_failure(hash_failure);
	   //ret=imx_hash_compute_runtime(1,0x80008000,10,hash_value);
	 rtic_debug("ret1=%d\n",ret);
	 return ret;
err0:
	class_destroy(rticptr->cls);
err1:
	cdev_del(&rticptr->cdev);
err2:
	unregister_chrdev_region(rticptr->dev_id, 1);
err3:	
	return ret;
}


static void __exit mx258_rtic_exit(void)
{
    if(rtic_base){
 	 iounmap((void *)rtic_base);
 	}
   clk_disable(imx_rtic_clk);
	clk_put(imx_rtic_clk);
	 device_destroy(rticptr->cls, rticptr->dev_id); 
	class_destroy(rticptr->cls);
	/* Unregister the device */
	cdev_del(&rticptr->cdev);
	unregister_chrdev_region(rticptr->dev_id, 1);
	kfree(rticptr);
	  rtic_info("RTIC module exit!\n");
}


module_init(mx258_rtic_init);
module_exit(mx258_rtic_exit);
MODULE_DESCRIPTION("rtic hash compute");
MODULE_AUTHOR("SAND<lijinmei>");
MODULE_LICENSE("GPL");

