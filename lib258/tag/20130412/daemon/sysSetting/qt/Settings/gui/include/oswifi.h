#ifndef _WIFI_H_
#define _WIFI_H_

#include 	"sand_types.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WIFI_DATA
#define _WIFI_DATA extern
#endif

#define OSWIFIERR_SUCCESS				0x00
#define WIFI_ALLRECVDATA				0x20
#define WIFI_INFODATA					0x10
#define WIFI_SENDCMDDATA				0x08
#define WIFI_RECVCMDDATA				0x04
#define WIFI_SENDNETDATA				0x02
#define	WIFI_RECVNETDATA				0x01

#define	WIFI_IPADDRLEN					4
#define	WIFI_PORTLEN					2
#define	WIFI_DHCPSERVERLEN			16
#define WIFI_MAXSSIDLEN				33
#define	WIFI_MAXENCRYKEYLEN			64

typedef struct
{
    unsigned char	ucDHCPFlag;
    unsigned char	aucOwnIP[WIFI_IPADDRLEN];
    unsigned char	aucMask[4];
    unsigned char	aucGetewayIP[WIFI_IPADDRLEN];
    unsigned char	aucDHCPServer[WIFI_DHCPSERVERLEN];
    
}WIFINETCFG,*PWIFINETCFG;

typedef struct
{
    unsigned char			ucSecuritySuite;
	unsigned char			ucAuthMethod;
	unsigned char			ucEncryMethod;
    unsigned char			aucSSID[WIFI_MAXSSIDLEN];
	unsigned char			ucKeyLength;
	unsigned char			ucKeyType;
	unsigned char			aucEncryKey[WIFI_MAXENCRYKEYLEN];
    
}WIFIPARAMCFG,*PWIFIPARAMCFG;

typedef struct
{
	unsigned char 		ucDebug;
	unsigned int		ulUartSpeed;
	WIFINETCFG			WIFINetCfg;
	WIFIPARAMCFG		WIFIParamCfg;
}OSWIFICFG,*POSWIFICFG;

_WIFI_DATA	POSWIFICFG		pWIFICfg;

#ifdef __cplusplus
}
#endif

#endif



