/*
 * Copyright 2011 SAND Corp, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

/*!
 * @file keycrypt.c
 *
 * @brief Main file for key encrypt and decrypt . Contains driver entry/exit
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>		/* Async notification */
#include <linux/uaccess.h>	/* for get_user, put_user, access_ok */
#include <linux/sched.h>	/* jiffies */
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <mach/hardware.h>
#include "keycrypt.h"
#include "dryice.h"
#include "dryice-regs.h"
#include <linux/mxc_scc_driver.h>

static int dbg = 0;
module_param(dbg, int, S_IRUGO);

#define key_debug(fmt, arg...)  do{if(dbg)printk(KERN_DEBUG fmt,##arg);}while(0)

#define key_info(fmt, arg...)       do{if(dbg)printk(KERN_INFO fmt,##arg);}while(0)

extern fsl_shw_return_t rng_drain_fifo(uint32_t * random_p, int count_words);

struct keycrypt_struct *cryptptr=NULL;
extern  struct fasync_struct *dryice_async_queue;
dev_t  imx_keycrypt_major;
dev_t  imx_keycrypt_minor = 0;

void eclater(unsigned char *buf_bit, unsigned char *byte);
void compacter(unsigned char *byte, unsigned char *buf_bit);
void Ks(unsigned char *Key, unsigned char *cd);
void key_permutition(unsigned char *keyin, unsigned char *keyout,unsigned int uiKeyLen);
unsigned int imx_keycrpt_tmk(unsigned char *keybufin,unsigned char *keybufout,unsigned int keylen,unsigned char crptdir);
unsigned int imx_Keycrypt_selectkey(di_key_t di_key, unsigned int  *pkey_selected);
unsigned int imx_keycrpt(unsigned char *keybufin, unsigned int uiKeyLen,unsigned char *pucInitVector,
						unsigned char *crptdata,unsigned char *dataout, unsigned int datalen,
						unsigned char crptdir);
unsigned int imx_keycypt_nopci(unsigned char cryptkeylen,unsigned char *keybufin,unsigned char *keybufout,
	             unsigned char crptdir);
extern unsigned int dryice_get_security_status(void);
extern unsigned int dryice_initial_security_status(void);

unsigned char T7_1_2[56] =
{
	56,48,40,32,24,16, 8,
	 0,57,49,41,33,25,17,
	 9, 1,58,50,42,34,26,
	18,10, 2,59,51,43,35,

	62,54,46,38,30,22,14,
	 6,61,53,45,37,29,21,
	13, 5,60,52,44,36,28,
	20,12, 4,27,19,11, 3
};

/*----------------------------------------------------------------
  Eclater 64 bits en 64 octets
-----------------------------------------------------------------*/
  void eclater(unsigned char *buf_bit, unsigned char *byte)
//unsigned char *buf_bit;
//unsigned char *byte;
{
     int i;
     unsigned char m;

	for (i = 0; i < 8; i++)
	{
		for (m = 0x80; m != 0;  )  // m >>= 1)
		{
                  if ((buf_bit[i] & m) != 0)
				*byte = 1;
			else
				*byte = 0;
                   byte++;
                  m=m/2 ;
		}
	}

}
/*----------------------------------------------------------------
  Compacter 64 octets en 64 bits
-----------------------------------------------------------------*/
void compacter(unsigned char *byte, unsigned char *buf_bit)
//unsigned char *byte;
//unsigned char *buf_bit;
{
	 int i;
	 unsigned char m, n;

	for (i = 0; i < 8; i++)
	{
		n = 0;
		for (m = 0x80; m != 0; )  //  m >>= 1)
		{
			if (*byte++)
			n = n | m;
				   m=m/2 ;

		}
		buf_bit[i] = n;
	}
}


void Ks(unsigned char *Key, unsigned char *cd)
{     
     int i;
	/* choix 1 */
	for (i = 0; i < 56; i++)
	{
		cd[i] = Key[T7_1_2[i]];
	}
}

void key_permutition(unsigned char *keyin, unsigned char *keyout,unsigned int uiKeyLen )
{
   unsigned char key[192];
   unsigned char cd[168];
  
   
   eclater(keyin, key);
   Ks(key,cd);
   compacter(cd,keyout);  //permutition of k1

   eclater(&keyin[8], &key[64]);
   Ks(&key[64],&cd[56]);   
   compacter(&cd[56],&keyout[7]);  //permutition of k2   
   if(uiKeyLen == 16)
   	memcpy(&keyout[14],&keyout[0],7); //k1=k3
   else
   {
	 eclater(&keyin[16], &key[128]);
   	 Ks(&key[128],&cd[112]);   
   	 compacter(&cd[112],&keyout[14]);  //permutition of k3 
   }
}

unsigned int imx_keycrpt_tmk(unsigned char *keybufin,unsigned char *keybufout,unsigned int keylen,unsigned char crptdir)
{
  unsigned int ret;
  unsigned long keyoutcount;   
  scc_return_t scc_code;
  unsigned int  key_selected = 0;

  ret = CRYPT_RETURN_OK_S;
  keyoutcount = PCI_KEY_SIZE;
   ret = imx_Keycrypt_selectkey(DI_KEY_FK,&key_selected);  
  if(ret)
   {
        ret = CRYPT_RETURN_ERROR_S;
  	goto out;
    }
  scc_code = scc_crypt(keylen,keybufin,0,crptdir,SCC_ECB_MODE,SCC_VERIFY_MODE_NONE,
  	            keybufout,&keyoutcount);
  if(scc_code)  
    {
        ret = CRYPT_RETURN_ERROR_S;
  	goto out;
  	}
  out:
  	if (key_selected) {
		(void)dryice_release_key_selection();
	}
  return(ret);
}


unsigned int imx_Keycrypt_selectkey(di_key_t di_key, unsigned int  *pkey_selected)
{
        di_key_t selected_key = -1;
	di_return_t di_code;
         unsigned int ret;
	
	 
       di_code = dryice_select_key(di_key,KEY_SELECT_NOLOCK); //select random key
	      key_info("dryice_keyselect=%02x\n",di_code);
	       if (di_code != DI_SUCCESS)
		{
	            key_info("can not select crypt key!\n ");
		    ret = CRYPT_RETURN_ERROR_S;	
		    goto out;
	       	 }
	   
	 *pkey_selected =1;	 
	 di_code = dryice_check_key(&selected_key);
	 if (di_code != DI_SUCCESS) 
	 {
	   key_info("Error from selected key!\n");	
	   ret = CRYPT_RETURN_ERROR_S;
	   goto out;
	 }
	 if(di_key != selected_key)
	 {
            key_info("Error from check-key!\n");	
	   ret = CRYPT_RETURN_ERROR_S;
	 }
	 key_info("dryice_keyselectA=%02x\n",selected_key);
	 ret = CRYPT_RETURN_OK_S;
    out:	
	return(ret);
}

/*unsigned int imx_keycrpt(unsigned char *keybufin,unsigned char *crptdata,unsigned char *dataout,
	             unsigned int datalen,unsigned char crptdir)
{
  unsigned int ret;
  unsigned long keyoutcount;  
  di_return_t di_ret;  
  scc_return_t scc_code;
  unsigned int  key_selected = 0;
  unsigned char keybufout[PCI_KEY_SIZE];
  unsigned char tripledes_key[TRIPLEDES_KEY_SIZE];
  
  keyoutcount = PCI_KEY_SIZE;
  ret = CRYPT_RETURN_OK_S;
ret = imx_Keycrypt_selectkey(DI_KEY_FK,&key_selected);  
  if(ret)
   {  
       key_info(" key_select error0= %d\n",ret);
        ret = CRYPT_RETURN_ERROR_S;
  	goto out;
    }
  scc_code = scc_crypt(PCI_KEY_SIZE,keybufin,0,SCC_DECRYPT,SCC_ECB_MODE,
  	            SCC_VERIFY_MODE_NONE,keybufout,&keyoutcount);//deccrypt key with random key
  	     
  if(scc_code)  
    {
        key_info("keycrypt error0=%d\n",scc_code);
        ret = CRYPT_RETURN_ERROR_S;
  	goto out;
  	}
    key_permutition(keybufout,tripledes_key);// permute des key
    di_ret = dryice_set_programmed_key(tripledes_key, TRIPLEDES_KEY_SIZE*8, 
		DI_FUNC_FLAG_READ_LOCK|DI_FUNC_FLAG_HARD_LOCK);    //set crypt key and read lock
    memset(keybufout,0,sizeof(keybufout));
    memset(tripledes_key,0,sizeof(tripledes_key));   //clear plaintext key
    if(di_ret)
	{
	  key_info("setprogram key error=%d\n",di_ret);
          ret = CRYPT_RETURN_ERROR_S;
  	   goto out;
  	}
     (void)dryice_release_key_selection();
     key_selected = 0; 
    ret = imx_Keycrypt_selectkey(DI_KEY_PK,&key_selected);  
    if(ret)
       {
         key_info("key_select error1=%d\n",ret);
          ret = CRYPT_RETURN_ERROR_S;
  	  goto out;
       }
     keyoutcount = MAX_CRYPTDATA_SIZE;
     scc_code = scc_crypt(datalen,crptdata,0,crptdir,SCC_ECB_MODE,
  	            SCC_VERIFY_MODE_NONE,dataout,&keyoutcount);//crypt data cryptmode; 0: ENCRPT  1: DECRYPT
     if(scc_code)  
    {
       key_info("key crypt error1=%d\n",scc_code);
        ret = CRYPT_RETURN_ERROR_S;
  	goto out;
  	}
    out:
	if (key_selected) {
		(void)dryice_release_key_selection();
	}
	   (void)dryice_release_programmed_key();
	 return ret;
}*/
unsigned int imx_keycrpt(unsigned char *keybufin, unsigned int uiKeyLen,unsigned char *pucInitVector,
						unsigned char *crptdata,unsigned char *dataout, unsigned int datalen,
						unsigned char crptdir)
{
  unsigned int ret;
  di_return_t di_ret;  
  scc_return_t scc_code;
  unsigned int  key_selected = 0; 
  unsigned char ucDesMode;
  unsigned char tripledes_key[TRIPLEDES_KEY_SIZE];
  unsigned long uLOutLen;
  scc_enc_dec_t ucMode;
  
  ret = CRYPT_RETURN_OK_S;

  key_permutition(keybufin,tripledes_key,uiKeyLen);// permute des key
  di_ret = dryice_set_programmed_key(tripledes_key, TRIPLEDES_KEY_SIZE*8, 
		DI_FUNC_FLAG_READ_LOCK|DI_FUNC_FLAG_HARD_LOCK);    //set crypt key and read lock    
    memset(tripledes_key,0,sizeof(tripledes_key));   //clear plaintext key
    if(di_ret)
	{
	  key_info("setprogram key error=%d\n",di_ret);
          ret = CRYPT_RETURN_ERROR_S;
  	   goto out;
  	}
     (void)dryice_release_key_selection();
     key_selected = 0; 
    ret = imx_Keycrypt_selectkey(DI_KEY_PK,&key_selected);  
    if(ret)
       {
         key_info("key_select error1=%d\n",ret);
          ret = CRYPT_RETURN_ERROR_S;
  	  goto out;
       }
    if(crptdir & DES_CBC_MODE)
	 	ucDesMode = SCC_CBC_MODE;
     else
	 	ucDesMode = SCC_ECB_MODE;
	 	if(crptdir & 0x01)
	 		ucMode = SCC_DECRYPT;
    else
	 		ucMode = SCC_ENCRYPT;
	 	uLOutLen = datalen;
     scc_code = scc_crypt((unsigned long)datalen,crptdata,pucInitVector,ucMode,(scc_crypto_mode_t)ucDesMode,
  	            SCC_VERIFY_MODE_NONE,dataout,&uLOutLen);//crypt data cryptmode; 0: ENCRPT  1: DECRYPT
     if(scc_code)  
    {
        key_info("key crypt error1=%d\n",scc_code);
        ret = CRYPT_RETURN_ERROR_S;
  	goto out;
  	}
    out:
	if (key_selected) {
		(void)dryice_release_key_selection();
	}
	   (void)dryice_release_programmed_key();
	 return ret;
}

unsigned int imx_keycypt_nopci(unsigned char cryptkeylen,unsigned char *keybufin,unsigned char *keybufout,	             unsigned char crptdir)
{   
	unsigned int ret;  
	unsigned long keyoutcount;    
	scc_return_t scc_code;        
	keyoutcount = cryptkeylen;      
	ret = CRYPT_RETURN_OK_S;      
	scc_code = scc_crypt(cryptkeylen,keybufin,0,crptdir,SCC_ECB_MODE,SCC_VERIFY_MODE_NONE,  	            
		keybufout,&keyoutcount);  //crypt main key            
		if(scc_code)               
			{	       
			ret = CRYPT_RETURN_ERROR_S;	  	
			goto out;  	      
			}         
		out:	 
			return ret;
}


static int imx_keycrypt_open(struct inode *inode, struct file *filp)
{      
	struct keycrypt_struct *dev; /* device information */       	         
	key_info("keycrypt open.\n");         
	dev = container_of(inode->i_cdev, struct keycrypt_struct, cdev);       
	filp->private_data = dev;        	
	return 0;
}
static int imx_keycrypt_fasync(int fd, struct file *filp, int mode)
{
   struct keycrypt_struct *dev;
   int retval;
    key_info("Keycrypt device fasync.\n");
    dev = filp->private_data;
    retval = fasync_helper(fd, filp, mode, &dryice_async_queue);
    key_info("helper_ret=%d async_queue=%d dryice_asyn=%d\n",retval,(unsigned int)dev->async_queue,(unsigned int)dryice_async_queue);	
    return retval;
}

static int imx_keycrypt_release(struct inode *inode, struct file *filp)
{
       imx_keycrypt_fasync(-1,filp,0);
	filp->private_data = NULL;
	key_info( "keycrypt close.\n");
	return 0;
}


static int imx_keycrypt_ioctl(struct inode *inode, struct file *fp,
			unsigned int cmd, unsigned long arg)
{
	int err = 0;
	int i;
  //struct KEYCRYPT_KEY  tmkdata;
	/* Extract the type and number bitfields, and don't decode wrong cmds.
	   Return ENOTTY (inappropriate ioctl) before access_ok() */
     
	if (_IOC_TYPE(cmd) != KEYCRYPT_IOC_NUM) {
		printk(KERN_ERR
		       "device_ioctl() - Error! IOC_TYPE = %d. Expected %d\n",
		       _IOC_TYPE(cmd), KEYCRYPT_IOC_NUM);
		return -ENOTTY;
	}
	
	if (_IOC_NR(cmd) > KEYCRYPT_IOCTL_MAXNUMBER) {
		printk(KERN_ERR
		       "device_ioctl() - Error!"
		       "IOC_NR = %d greater than max supported(%d)\n",
		       _IOC_NR(cmd), KEYCRYPT_IOCTL_MAXNUMBER);
		return -ENOTTY;
	}

	/* The direction is a bitmask, and VERIFY_WRITE catches R/W transfers.
	   `Type' is user-oriented, while access_ok is kernel-oriented, so the
	   concept of "read" and "write" is reversed. I think this is primarily
	   for good coding practice. You can easily do any kind of R/W access
	   without these checks and IOCTL code can be implemented "randomly"! */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err =
		    !access_ok(VERIFY_WRITE, (void __user *)arg,
			       _IOC_SIZE(cmd));

	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =
		    !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	
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
	case IOCTL_CRYPT_FUSEKEY:
	{
		struct KEYCRYPT_KEY  tmkdata;
		
		if (copy_from_user(&tmkdata,
				   (struct KEYCRYPT_KEY *)arg,
				   sizeof(struct KEYCRYPT_KEY))) {			
			return -EFAULT;
		}	
		if(tmkdata.cryptkeylen > PCI_KEY_SIZE)
			return -EFAULT;
		 err = imx_keycrpt_tmk(tmkdata.cryptkeyin,tmkdata.cryptkeyout,tmkdata.cryptkeylen,tmkdata.crypydir);	
		 memset(tmkdata.cryptkeyin,0,sizeof(tmkdata.cryptkeyin)); //clear plaintext TMK
		// for(i=0; i<PCI_KEY_SIZE; i++)
		 	// key_info("tmkout[%d]=%02x",i,tmkdata.cryptkeyout[i]);
                 if(!err)
                 {
			if (copy_to_user((struct KEYCRYPT_KEY *)arg,
				 &tmkdata, sizeof(struct KEYCRYPT_KEY))) {
			                    return -EFAULT;
		                }
                 }
		break;
	}	
	case IOCTL_CRYPT_USERKEY:
	 {
		struct KEY_DATA_USERKEY  *pcryptdata;
		
		pcryptdata = kmalloc(sizeof(struct KEY_DATA_USERKEY),GFP_KERNEL);
		if (pcryptdata == NULL)
			return -ENOMEM;
		if (copy_from_user(pcryptdata,
				   (struct KEY_DATA_USERKEY *)arg,
				   sizeof(struct KEY_DATA_USERKEY))) {	
				   kfree(pcryptdata);
			return -EFAULT;
		}	
		if(pcryptdata->uiInPutLen> MAX_CRYPTDATA_SIZE)
		{
			 kfree(pcryptdata);
			 return -EFAULT;
		}
		//printk("cryptlen=%d\n",pcryptdata->datalen);
		// for(i=0;i<16;i++)		 	
		  //key_info("[%02x]",pcryptdata->cryptkey[i]);
		err = imx_keycrpt(pcryptdata->auckey,pcryptdata->uiKeyLen,pcryptdata->aucInitVector,
					       pcryptdata->aucDataIn,pcryptdata->aucDataOut,
					       pcryptdata->uiInPutLen,pcryptdata->ucMode);  //pcryptdata->crypydir 0:encrypt 1:decrypt
			//for(i=0;i<pcryptdata->datalen;i++)
			//key_info("keyout[%d]=%02x",i,pcryptdata->cryptdataout[i]);
		if(!err)
                 {
			if (copy_to_user((struct KEY_DATA_USERKEY *)arg,
				 pcryptdata, sizeof(struct KEY_DATA_USERKEY))) {
				    kfree(pcryptdata);
			                    return -EFAULT;
		                }
                 }
		kfree(pcryptdata);
		 break;
		}	

	case IOCTL_CRYPT_NOPCI:   /*for no security only 3DES crypt with default key*/
		{
		 struct KEYCRYPT_DATA_NOPCI  nopcidata;	
		
		 if (copy_from_user(&nopcidata,
				   (struct KEYCRYPT_DATA_NOPCI *)arg,
				   sizeof(struct KEYCRYPT_DATA_NOPCI))) {					 
			  return -EFAULT;
		   }	
		 if(nopcidata.cryptkeylen > NOPCI_KEY_MAXSIZE)
		 key_debug("cryptlen=%d\n",nopcidata.cryptkeylen);
		 for(i=0;i<16;i++)		 	
		  key_debug("[%02x]",nopcidata.cryptkeyin[i]);
		 key_debug("cryptmode=%02x\n",nopcidata.crypydir);
		 key_debug("keycrypt nopci in!\n");
		err = imx_keycypt_nopci(nopcidata.cryptkeylen,nopcidata.cryptkeyin,nopcidata.cryptkeyout,nopcidata.crypydir);
		for(i=0;i<nopcidata.cryptkeylen;i++)
			key_debug("keyout[%d]=%02x",i,nopcidata.cryptkeyout[i]);
		if(!err)
                 {
			if (copy_to_user((struct KEYCRYPT_DATA_NOPCI *)arg,
				 &nopcidata, sizeof(struct KEYCRYPT_DATA_NOPCI))) {
			                    return -EFAULT;
		                }
                 }
		 break;
		} 

	case IOCTL_GET_RANDOM_NUMBER:
		{
                struct  RANDOM_DATA_GET randomdata;
		fsl_shw_return_t rng_error;
                  
                   if (copy_from_user(&randomdata,
				   (struct RANDOM_DATA_GET *)arg,
				   sizeof(struct RANDOM_DATA_GET))) {					 
			  return -EFAULT;
		   }	
		key_info("rngnum=%d\n",randomdata.numberlen);	 
		if(randomdata.numberlen > RANDOM_DATALEN)
			return -EFAULT;
                 rng_error = rng_drain_fifo(randomdata.random_data,randomdata.numberlen);
                 err = rng_error;
                 if(!err)
                 {
			if (copy_to_user((struct RANDOM_DATA_GET *)arg,
				 &randomdata, sizeof(struct RANDOM_DATA_GET))) {
			                    return -EFAULT;
		                }
                 }
		 break;				   
		}
	
	case IOCTL_SECURITY_STATUS:
		{
			struct SECURITY_DATA  security_status;

			if (copy_from_user(&security_status,
				   (struct SECURITY_DATA *)arg,
				   sizeof(struct SECURITY_DATA))) {					 
			  return -EFAULT;
		   }	
			if(security_status.ucType == 0)
			{
				err = dryice_initial_security_status();
			}
			else
				security_status.ucStatus= dryice_get_security_status();
			if (copy_to_user((struct SECURITY_DATA*)arg,
				 &security_status, sizeof(struct SECURITY_DATA))) 
			{
			        return -EFAULT;
		        }
		}
		break;		
	default:
		printk(KERN_ERR "device_ioctl() - Invalid IOCTL (0x%x)\n", cmd);
		return EINVAL;
	}
	return err;
}



struct file_operations Keycrypt_Fops = {
	.owner = THIS_MODULE,
	.ioctl = imx_keycrypt_ioctl,
	.open = imx_keycrypt_open,
	.release =  imx_keycrypt_release,
	.fasync = imx_keycrypt_fasync,
};


/*!
 * Entry point for keycrpt module.
 *
 */
static int __init mx258_keycrypt_init(void)
{       
	int ret;

	key_info("Keycrypt device enter  info.\n");
        cryptptr = kmalloc(sizeof(struct keycrypt_struct), GFP_KERNEL);
        if (!cryptptr) {
                printk(KERN_ERR"Keycrypt device kmalloc error.\n");
                return -ENOMEM;
        }
        memset(cryptptr,0,sizeof(struct keycrypt_struct));
		
      /* Register character device */
	ret = alloc_chrdev_region(&cryptptr->dev_id,imx_keycrypt_minor, 1,"imx_keycrypt");	
	if (ret < 0) {
		printk(KERN_ERR"can't get keycrypt major %d\n",imx_keycrypt_major);
		goto err3;
	}
	imx_keycrypt_major = MAJOR(cryptptr->dev_id);
	key_info("major=%d\n",imx_keycrypt_major);
	key_info("minor=%d\n",imx_keycrypt_minor);
	cdev_init(&cryptptr->cdev, &Keycrypt_Fops);
	cryptptr->cdev.owner = THIS_MODULE;

	ret = cdev_add(&cryptptr->cdev, cryptptr->dev_id, 1);
	if (ret) {
		printk(KERN_ERR"can't add keycrypt cdev.\n");
		goto err2;
	}	
	
	/* create class and device for udev information */
	cryptptr->cls= class_create(THIS_MODULE, "keycrypt");
	if (IS_ERR(cryptptr->cls)) {
		printk(KERN_ERR"Err!failed to create keycrpt class\n");
		ret = -ENOMEM;
		goto err1;
	}

	cryptptr->clsdev= device_create(cryptptr->cls, NULL, cryptptr->dev_id, NULL,
					KEYCRYPT_DEVICE_FILE_NAME); //MKDEV(imx_keycrypt_major, imx_keycrypt_minor)
	if (IS_ERR(cryptptr->clsdev)) {
		printk(KERN_ERR"Err!failed to create gps keycrypt class device\n");
		ret = -ENOMEM;
		goto err0;
	}
	dryice_async_queue = cryptptr->async_queue ; 
           key_debug("ret1=%d\n",ret);
	// di_ret = dryice_set_random_key(RANDOM_KEY_FLAG); //set random key RANDOM_KEY_FLAG
	 //if(di_ret == DI_ERR_HLOCK ||di_ret == DI_SUCCESS||di_ret == DI_ERR_SLOCK) //set ok or set already
	    // ret = 0;
	// else
	 	//ret = EBUSY;
	// printk("ret1=%d\n",ret);
	 return ret;
err0:
	class_destroy(cryptptr->cls);
err1:
	cdev_del(&cryptptr->cdev);
err2:
	unregister_chrdev_region(cryptptr->dev_id, 1);
err3:	
	return ret;
	
}


/*!
 * unloading module.
 *
 */
static void __exit mx258_keycrypt_exit(void)
{
        device_destroy(cryptptr->cls, cryptptr->dev_id); //MKDEV(imx_keycrypt_major, imx_keycrypt_minor)
	class_destroy(cryptptr->cls);
	/* Unregister the device */
	cdev_del(&cryptptr->cdev);
	unregister_chrdev_region(cryptptr->dev_id, 1);
	kfree(cryptptr);
	key_info("exit kercrypt!\n");
}

module_init(mx258_keycrypt_init);
module_exit(mx258_keycrypt_exit);
MODULE_DESCRIPTION("Key Encryption and Decryption");
MODULE_AUTHOR("SAND<lijinmei>");
MODULE_LICENSE("GPL");
