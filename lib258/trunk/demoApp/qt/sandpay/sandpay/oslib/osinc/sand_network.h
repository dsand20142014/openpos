#ifndef SAND_NETWORK_H
#define SAND_NETWORK_H

unsigned char TCP_CheckOSVersion(void);
unsigned char TCP_Open(int *piTCPHandle);
unsigned char TCP_Connect(int iTCPHandle,unsigned long ulPeerAddr,unsigned short uiPeerPort);

unsigned char TCP_Send(int iTCPHandle,unsigned char *pucInBuf,unsigned short uiInLen);
unsigned char TCP_Recv(int iTCPHandle,unsigned char *pucInBuf,unsigned short *puiInLen,unsigned short uiTimeout);
unsigned char TCP_Close(int iTCPHandle);

/* udp*/
unsigned char UDP_Open(int *piUDPHandle);
unsigned char UDP_Config(int iUDPHandle,unsigned long ulPeerAddr,unsigned short uiPeerPort);
unsigned char UDP_Send(int iUDPHandle,unsigned char *pucInBuf,unsigned short uiInLen);
unsigned char UDP_Recv(int iUDPHandle,unsigned char *pucInBuf,unsigned short *puiInLen,unsigned short uiTimeout);
unsigned char UDP_Close(int iUDPHandle);


#endif

