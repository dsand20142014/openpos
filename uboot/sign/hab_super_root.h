#ifndef HAB_SUPER_ROOT_H
#define HAB_SUPER_ROOT_H


#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
                                                                               
     Header Name: hab_super_root.h

     General Description: This module contains the HAB Super Root public keys.
 
====================================================================================================
                              Freescale Confidential Proprietary
		  Copyright (c) 2008 Freescale Semiconductor Inc. All Rights Reserved
     
Revision History:
                            Modification     Tracking
Author                          Date          Number     Description of Changes
-------------------------   ------------    ----------   -------------------------------------------

====================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

/*==================================================================================================
                                           CONSTANTS
==================================================================================================*/

/* Generic type definitions */
typedef signed char        INT8;
typedef unsigned char      UINT8;
typedef short int          INT16;
typedef unsigned short int UINT16;
typedef int                INT32;
typedef unsigned int       UINT32;
typedef unsigned char      BOOLEAN;


/* HAB specific type definitions */
typedef UINT8              *hab_bytestring;
typedef UINT16             hab_algorithm;
typedef UINT8              hab_index;
typedef UINT32             hab_address;
typedef UINT8              hab_certificate;
typedef UINT32             hab_data_length;
typedef UINT16             hab_int_length;
typedef UINT8              hab_error;

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* HAB specific definitions */
#define HAB_MAX_EXP_SIZE   ((hab_int_length)4) /* Maximum size of RSA
                                                * public key exponent
                                                * - in bytes
                                                */

/*==================================================================================================
                                            MACROS
==================================================================================================*/

/*==================================================================================================
                                             ENUMS
==================================================================================================*/

/*==================================================================================================
                                 STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

/* RSA public key structure */
typedef struct
{
    UINT8           rsa_exponent[HAB_MAX_EXP_SIZE]; /* RSA public exponent */
    UINT8           *rsa_modulus;                   /* RSA modulus pointer */
    hab_int_length  exponent_size;                  /* Exponent size in bytes */
    hab_int_length  modulus_size;                   /* Modulus size in bytes */
    BOOLEAN         init_flag;                      /* Indicates if key initialised */
} hab_rsa_public_key;

/*==================================================================================================
                                 GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/
/* Super Root keys */
extern const hab_rsa_public_key hab_super_root_key[];

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

/*================================================================================================*/

#ifdef __cplusplus
}
#endif

#endif  /* HAB_SUPER_ROOT_H */
