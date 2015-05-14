/*
 * Copyright 2008-2009 Freescale Semiconductor, Inc. All Rights Reserved.
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
 * @file agpsgpiodev.h
 *
 * @brief head file of Simple character device interface for AGPS kernel module.
 *
 * @ingroup
 */

#ifndef KEYCRYPT_H
#define KEYCRPT_H

#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>

#define USE_BLOCKING		/* Test driver with blocking calls */
#undef USE_FASYNC		/* Test driver with async notification */

/* The major device number. We can't rely on dynamic registration any more
   because ioctls need to know it */



#define KEYCRYPT_IOCTL_MAXNUMBER   5
#define KEYBYTES_MAX  256

#define KEY_SELECT_NOLOCK  			0
#define PCI_KEY_SIZE 					16
#define TDES_KEY_MAXSIZE      			24
#define MAX_CRYPTDATA_SIZE 			256*8
#define TRIPLEDES_KEY_SIZE   			 21
#define NOPCI_MKEY_SIZE   			8  
#define NOPCI_KEY_MAXSIZE  			256
#define RANDOM_DATALEN    			128

#define DES_CBC_MODE                               0x02

/* The name of the device file */
#define KEYCRYPT_DEVICE_FILE_NAME "imx_keycrypt"

struct keycrypt_struct
{
   dev_t dev_id;
   struct cdev cdev ;
   struct class *cls;	
   struct device *clsdev;
   struct fasync_struct *async_queue;
   unsigned char keydata[KEYBYTES_MAX];
};

struct KEY_DATA_USERKEY
{
   unsigned char ucMode;
   unsigned char aucInitVector[8];
   unsigned char auckey[TDES_KEY_MAXSIZE];
   unsigned int    uiKeyLen;
   unsigned int    uiInPutLen;
   unsigned char  aucDataIn[MAX_CRYPTDATA_SIZE];
   unsigned char  aucDataOut[MAX_CRYPTDATA_SIZE];
   unsigned int     uiOutPutLen;
};

struct KEYCRYPT_DATA_NOPCI
{
   unsigned char crypydir;
   unsigned int cryptkeylen;
   unsigned char cryptkeyin[NOPCI_KEY_MAXSIZE];
   unsigned char cryptkeyout[NOPCI_KEY_MAXSIZE];
};

struct RANDOM_DATA_GET
{
   unsigned int numberlen;
   unsigned int random_data[RANDOM_DATALEN];   
};

struct KEYCRYPT_KEY
{
   unsigned char crypydir;
   unsigned int cryptkeylen;
   unsigned char cryptkeyin[PCI_KEY_SIZE];
   unsigned char cryptkeyout[PCI_KEY_SIZE];
};

struct SECURITY_DATA
{
   unsigned char ucType;
   unsigned int  ucStatus;  
};

#define KEYCRYPT_IOC_NUM   'C'

#define IOCTL_CRYPT_RANDOMKEY                                  _IOWR(KEYCRYPT_IOC_NUM, 1,int)
#define IOCTL_CRYPT_USERKEY                               _IOWR(KEYCRYPT_IOC_NUM, 2,int)
#define IOCTL_CRYPT_NOPCI                          _IOWR(KEYCRYPT_IOC_NUM, 3,int) //struct KEYCRYPT_DATA_NOPCI
#define IOCTL_GET_RANDOM_NUMBER                  _IOWR(KEYCRYPT_IOC_NUM, 4,int)
#define IOCTL_SECURITY_STATUS                                _IOWR(KEYCRYPT_IOC_NUM, 5,int)

#define RANDOM_KEY_FLAG   (DI_FUNC_FLAG_READ_LOCK  | \
			     DI_FUNC_FLAG_WRITE_LOCK | \
			     DI_FUNC_FLAG_HARD_LOCK )  //not block DI_FUNC_FLAG_ASYNC
			

  /*!
     * No error.  As a function return code in Non-blocking mode, this may
     * simply mean that the operation was accepted for eventual execution.
     */
#define CRYPT_RETURN_OK_S                           0
	/*! Failure for non-specific reason. */
#define  CRYPT_RETURN_ERROR_S                     1

 /*!
    * Operation failed because some resource was not able to be allocated.
    */
//#define FSL_RETURN_NO_RESOURCE_S        2
    /*! Crypto algorithm unrecognized or improper. */
//#define FSL_RETURN_BAD_ALGORITHM_S   3
    /*! Crypto mode unrecognized or improper. */
//#define  FSL_RETURN_BAD_MODE_S             4
    /*! Flag setting unrecognized or inconsistent. */
//#define FSL_RETURN_BAD_FLAG_S               5
    /*! Improper or unsupported key length for algorithm. */
//#define FSL_RETURN_BAD_KEY_LENGTH_S 6
    /*! Improper parity in a (DES, TDES) key. */
//#define FSL_RETURN_BAD_KEY_PARITY_S  7
  /*!
     * Improper or unsupported data length for algorithm or internal buffer.
     */
//#define FSL_RETURN_BAD_DATA_LENGTH_S 8
    /*! Authentication / Integrity Check code check failed. */
//#define FSL_RETURN_AUTH_FAILED_S            9
    /*! A memory error occurred. */
//#define FSL_RETURN_MEMORY_ERROR_S       10
    /*! An error internal to the hardware occurred. */
//#define FSL_RETURN_INTERNAL_ERROR_S      11
    /*! ECC detected Point at Infinity */
//#define FSL_RETURN_POINT_AT_INFINITY_S  12
    /*! ECC detected No Point at Infinity */
//#define FSL_RETURN_POINT_NOT_AT_INFINITY_S  13
    /*! GCD is One */
//#define FSL_RETURN_GCD_IS_ONE_S              14
    /*! GCD is not One */
//#define FSL_RETURN_GCD_IS_NOT_ONE_S    15
    /*! Candidate is Prime */
//#define FSL_RETURN_PRIME_S                           16
    /*! Candidate is not Prime */
//#define FSL_RETURN_NOT_PRIME_S                  17
    /*! N register loaded improperly with even value */
//#define FSL_RETURN_EVEN_MODULUS_ERROR_S    18
    /*! Divisor is zero. */
//#define FSL_RETURN_DIVIDE_BY_ZERO_ERROR_S   19
    /*! Bad Exponent or Scalar value for Point Multiply */
//#define FSL_RETURN_BAD_EXPONENT_ERROR_S       20
    /*! RNG hardware problem. */
//#define FSL_RETURN_OSCILLATOR_ERROR_S              22
    /*! RNG hardware problem. */
//#define FSL_RETURN_STATISTICS_ERROR_S             23*/

typedef enum fsl_shw_return_t
{
  /*!
     * No error.  As a function return code in Non-blocking mode, this may
     * simply mean that the operation was accepted for eventual execution.
     */
    FSL_RETURN_OK_S = 0,
	/*! Failure for non-specific reason. */
    FSL_RETURN_ERROR_S,
 /*!
    * Operation failed because some resource was not able to be allocated.
    */
    FSL_RETURN_NO_RESOURCE_S,
    /*! Crypto algorithm unrecognized or improper. */
    FSL_RETURN_BAD_ALGORITHM_S,
    /*! Crypto mode unrecognized or improper. */
    FSL_RETURN_BAD_MODE_S,
    /*! Flag setting unrecognized or inconsistent. */
    FSL_RETURN_BAD_FLAG_S,
    /*! Improper or unsupported key length for algorithm. */
    FSL_RETURN_BAD_KEY_LENGTH_S,
    /*! Improper parity in a (DES, TDES) key. */
    FSL_RETURN_BAD_KEY_PARITY_S,
  /*!
     * Improper or unsupported data length for algorithm or internal buffer.
     */
    FSL_RETURN_BAD_DATA_LENGTH_S,
    /*! Authentication / Integrity Check code check failed. */
    FSL_RETURN_AUTH_FAILED_S,
    /*! A memory error occurred. */
    FSL_RETURN_MEMORY_ERROR_S,
    /*! An error internal to the hardware occurred. */
    FSL_RETURN_INTERNAL_ERROR_S,
    /*! ECC detected Point at Infinity */
    FSL_RETURN_POINT_AT_INFINITY_S,
    /*! ECC detected No Point at Infinity */
    FSL_RETURN_POINT_NOT_AT_INFINITY_S,
    /*! GCD is One */
    FSL_RETURN_GCD_IS_ONE_S,
    /*! GCD is not One */
    FSL_RETURN_GCD_IS_NOT_ONE_S,
    /*! Candidate is Prime */
    FSL_RETURN_PRIME_S,
    /*! Candidate is not Prime */
    FSL_RETURN_NOT_PRIME_S,
    /*! N register loaded improperly with even value */
    FSL_RETURN_EVEN_MODULUS_ERROR_S,
    /*! Divisor is zero. */
    FSL_RETURN_DIVIDE_BY_ZERO_ERROR_S,
    /*! Bad Exponent or Scalar value for Point Multiply */
    FSL_RETURN_BAD_EXPONENT_ERROR_S,
    /*! RNG hardware problem. */
    FSL_RETURN_OSCILLATOR_ERROR_S,
    /*! RNG hardware problem. */
    FSL_RETURN_STATISTICS_ERROR_S,
} fsl_shw_return_t;

#define ENCRPTMODE                             1
#define DECRPTMODE                             2

//extern static fsl_shw_return_t rng_drain_fifo(uint32_t * random_p, int count_words);

#endif
