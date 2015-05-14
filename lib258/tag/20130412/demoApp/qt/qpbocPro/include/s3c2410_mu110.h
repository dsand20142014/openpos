#ifndef __S3C2410_PRN_MU110__
#define __S3C2410_PRN_MU110__

#ifdef __cplusplus
extern "C" {
#endif

#define S3C2410_MU110_IOCTL_BASE	'P'
#define DEVICE_NAME "s3c2410_mu110"


#define MU110_FEED_PAPER										_IOW(S3C2410_MU110_IOCTL_BASE, 0, sizeof(int))
#define MU110_BACK_PAPER										_IOW(S3C2410_MU110_IOCTL_BASE, 1, sizeof(int))
#define MU110_CHECK_PAPER  									_IOR(S3C2410_MU110_IOCTL_BASE, 2, sizeof(int))
#define MU110_BLACK_MASK										_IOR(S3C2410_MU110_IOCTL_BASE, 3, sizeof(int))
#define MU110_CHECK_TMPR  									_IOR(S3C2410_MU110_IOCTL_BASE, 4, sizeof(int))
#define MU110_HOME_POS 											_IOW(S3C2410_MU110_IOCTL_BASE, 5, sizeof(int))
#define MU110_STATE_CTL 										_IOR(S3C2410_MU110_IOCTL_BASE, 6, sizeof(int))

#define MU110_PRINT_STRING 									_IOR(S3C2410_MU110_IOCTL_BASE,  7, sizeof(int))
#define MU110_PRINT_STRING_CH8X8 						_IOR(S3C2410_MU110_IOCTL_BASE,  8, sizeof(int))
//#define MU110_PRINT_STRING_ALL 							_IOR(S3C2410_MU110_IOCTL_BASE,  9, sizeof(int))




#define MU110_PRN_ASC 											_IOWR(S3C2410_MU110_IOCTL_BASE, 18, sizeof(int))
#define MU110_PRN_GB 												_IOWR(S3C2410_MU110_IOCTL_BASE, 19, sizeof(int))



#define MU110_DATA_TEST 	 									_IOWR(S3C2410_MU110_IOCTL_BASE, 20, sizeof(int))
#define MU110_ST_TEST 											_IOWR(S3C2410_MU110_IOCTL_BASE, 21, sizeof(int))
#define MU110_SYSCS_TEST 	 									_IOR(S3C2410_MU110_IOCTL_BASE,  22, sizeof(int))
#define MU110_MOTO_CTL_TEST 								_IOWR(S3C2410_MU110_IOCTL_BASE, 23, sizeof(int))
#define MU110_CARRIAGE_TOLEFT_TEST 					_IOWR(S3C2410_MU110_IOCTL_BASE, 24, sizeof(int))
#define MU110_CARRIAGE_TORIGHT_TEST 				_IOWR(S3C2410_MU110_IOCTL_BASE, 25, sizeof(int))
#define MU110_CARRIAGE_START_POS						_IOWR(S3C2410_MU110_IOCTL_BASE, 26, sizeof(int))


//typedef enum { md_5x7en,md_7x9en,md_7x9gb,md_gb2312,md_gb2312_8x8}	fmode_t ;
typedef enum { FONT_5X7_EN,FONT_7X9_EN,FONT_7X9_GB,FONT_16X16_GB2312,FONT_8X8_GB2312,FONT_16X16_GB18030}	fmode_t ;

#ifdef __cplusplus
}
#endif

#endif
//EOF
