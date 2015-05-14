
#ifndef _TYPEDEF_H
#define _TYPEDEF_H


#ifndef		UCHAR
#define 	UCHAR	unsigned char 
#endif


#ifndef		PUCHAR
#define		PUCHAR	UCHAR *
#endif

#ifndef		USHORT	
#define		USHORT	unsigned short
#endif

#ifndef		UINT
#define		UINT	unsigned int
#endif

#ifndef		NULL
#define		NULL		0
#endif

#ifndef		TRUE
#define		TRUE		1
#endif

#ifndef		true
#define		true		1
#endif

#ifndef		FALSE
#define		FALSE		0
#endif

#ifndef		false
#define		false		0
#endif


typedef		UCHAR		BOOL;

#ifndef		ULONG_C51
#define ULONG_C51 unsigned long
#endif

#ifndef		UINT_C51
#define UINT_C51 unsigned int
#endif


typedef enum  { ASCII_FONT  , GB2312_FONT} FONT;


#endif
