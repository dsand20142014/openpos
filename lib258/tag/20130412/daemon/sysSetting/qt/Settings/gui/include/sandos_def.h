#ifndef __SANDOS_DEF_H__
#define __SANDOS_DEF_H__

#define ZY_TODO Uart_Printf

#ifdef __cplusplus 
extern "C" { 
#endif

/*****************************************************************************
* come from oslib.h
*****************************************************************************/
/* File API definement */
#define O_READ              0x01
#define O_WRITE             0x02
#define O_CREATE            0x04

#ifdef O_TRUNC
#undef O_TRUNC	//
#define O_TRUNC             0x08
#endif

#ifndef SEEK_SET
#define SEEK_SET	        0x00
#endif
#ifndef SEEK_CUR
#define SEEK_CUR	        0x01
#endif
#ifndef SEEK_END
#define SEEK_END	        0x02
#endif

#define	FILE_INVMODE		0x01
#define	FILE_NOEXIST		0x02
#define	FILE_EXIST          0x03
#define	FILE_FULL			0x04
#define	FILE_NOSPACE		0x05
#define	FILE_MUCHHALDLE		0x06
/*End of  File API definement */

#ifdef __cplusplus 
}
#endif


#endif
