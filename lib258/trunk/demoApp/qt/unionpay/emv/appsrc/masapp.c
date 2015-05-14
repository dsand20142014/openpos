#include	"include.h"
#include	"global.h"
#include	"xdata.h"

#include	<masapp.h>
#include	<EMV41.h>
#include 	<util.h>
#include	<msg.h>
#include	<emvinterface.h>
#include	<emvparam.h>
#include	<digital.h>
#include	<EMVError.h>
#if SANDREADER
#include <QPBOC.h>
#endif

#include	<new_drv.h>

unsigned char TestSentData[512];
unsigned char TestRevData[512];

extern const DATAPROPERTY	ICCardProperty[];
void ProcessMag(UCHAR bFallBack,unsigned char* bufMag);
UCHAR SelectAccountType(void);
UCHAR	TransCapture(TRANSRESULT	enTransResult);
void	PrintTransTicket(void);
UCHAR	ReadICTransLog(void);
UCHAR	ReadICBalance(void);
UCHAR 	MagTrack2[40];

UCHAR QEMVICC(QICCIN *pQICCIn ,QICCOUT  *pQICCOut)
{
	unsigned char ucResult=SUCCESS;
	unsigned short uiI;

	util_Printf("\n digital QEMVICC Send ICC Data:\n");
	util_Printf("%02X %02X %02X %02X ",pQICCIn->ucCla,pQICCIn->ucIns,pQICCIn->ucP1,pQICCIn->ucP2);
	for(uiI=0;uiI<pQICCIn->uiLc;uiI++)
		util_Printf("%02X ",*(pQICCIn->aucDataIn+uiI));
	if(pQICCIn->uiLe) util_Printf("00");


	ucResult =QIccIsoCommand(0,pQICCIn,pQICCOut);
	if(!ucResult)
	{
		util_Printf("\nRecv ICC Data:\n");
		for(uiI=0;uiI<pQICCOut->uiLenOut;uiI++)
			util_Printf("%02X ",pQICCOut->aucDataOut[uiI]);
		util_Printf("\nSW: %02X %02X",pQICCOut->ucSWA,pQICCOut->ucSWB);
	}
	util_Printf("\n digital  QEMVICC ucResult:[%02x]\n",ucResult );
	return ucResult;
}

UCHAR	EMVICC(EMVICCIN *pEMVICCIn ,EMVICCOUT *pEMVICCOut)
{
	unsigned char ucResult=SUCCESS;
	unsigned short uiI;


	/*add debug   2009-9-2 16:18  cbf*/

	util_Printf("\nSend ICC Data:\n");
	util_Printf("%02X %02X %02X %02X ",pEMVICCIn->ucCla,pEMVICCIn->ucIns,pEMVICCIn->ucP1,pEMVICCIn->ucP2);
	for(uiI=0;uiI<pEMVICCIn->uiLc;uiI++)
		util_Printf("%02X ",*(pEMVICCIn->aucDataIn+uiI));
	if(pEMVICCIn->uiLe) util_Printf("00");
	////////////////////////////////////////
#if 1
	ucResult =IccIsoCommand(0,pEMVICCIn,pEMVICCOut);
#else
	memset(aucBuff,0x00,sizeof(aucBuff));
	uiLen =0;
	aucBuff[uiLen++] =0x00;
	memcpy(&aucBuff[uiLen],&pEMVICCIn->ucCla,4);
	uiLen += 4;
	if(pEMVICCIn->uiLc)
	{
		//aucBuff[uiLen++] =pEMVICCIn->uiLc;
		memcpy(&aucBuff[uiLen],pEMVICCIn->aucDataIn,pEMVICCIn->uiLc);
		uiLen += pEMVICCIn->uiLc;
	}
	if(pEMVICCIn->uiLe)
		aucBuff[uiLen++] =0x00;

	//DIGITAL_Package(aucBuff,uiLen);
//////////////////////////////log1///////////////////////////////////////////////
#if 0											//\x00\xA4\x04\x00\x31\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44\x46\x30\x31\x00
	if(!memcmp(&aucBuff[1],"\x00\xA4\x04\x00\x31\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44\x46\x30\x31\x00",19))
	{
		pEMVICCOut->uiLenOut =23;
		memcpy(pEMVICCOut->aucDataOut,"\x6F\x15\x84\x0E\x31\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44\x46\x30\x31\xA5\x03\x88\x01\x01",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x01\x0C\x00",5))
	{
		pEMVICCOut->uiLenOut =29;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x1B\x61\x19\x4F\x08\xA0\x00\x00\x03\x33\x01\x01\x01\x50\x0A\x50\x42\x4F\x43\x20\x44\x65\x62\x69\x74\x87\x01\x02",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x02\x0C\x00",5)&&(ucFlag==0))
	{
		ucFlag =1;
		//pEMVICCOut->uiLenOut =29;
		//memcpy(pEMVICCOut->aucDataOut,"\x70\x1B\x61\x19\x4F\x08\xA0\x00\x00\x03\x33\x01\x01\x01\x50\x0A\x50\x42\x4F\x43\x20\x44\x65\x62\x69\x74\x87\x01\x02",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x6A;
		pEMVICCOut->ucSWB= 0x83;
	}
	else if(!memcmp(&aucBuff[1],"\x00\xA4\x04\x00\xA0\x00\x00\x03\x33\x01\x01\x01\x00",13))
	{
		pEMVICCOut->uiLenOut =65;
		memcpy(pEMVICCOut->aucDataOut,"\x6F\x3F\x84\x08\xA0\x00\x00\x03\x33\x01\x01\x01\xA5\x33\x50\x0A\x50\x42\x4F\x43\x20\x44\x65\x62\x69\x74\x87\x01\x01\x9F\x38\x03\x9F\x1A\x02\x5F\x2D\x02\x7A\x68\x9F\x11\x01\x01\x9F\x12\x0A\x50\x42\x4F\x43\x20\x44\x65\x62\x69\x74\xBF\x0C\x05\x9F\x4D\x02\x0B\x0A",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x80\xA8\x00\x00\x83\x02\x01\x56\x00",9))
	{
		pEMVICCOut->uiLenOut =12;
		memcpy(pEMVICCOut->aucDataOut,"\x80\x0A\x7D\x00\x08\x01\x03\x00\x10\x01\x04\x01",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x01\x0C\x00",5))
	{
		pEMVICCOut->uiLenOut =38;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x23\x5F\x20\x0C\x43\x55\x50\x20\x74\x65\x73\x74\x20\x30\x30\x32\x9F\x1F\x00\x57\x0F\x62\x25\x00\x00\x00\x00\x00\x22\xD1\x51\x02\x01\x00\x11\x00\x0F",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x02\x0C\x00",5))
	{
		pEMVICCOut->uiLenOut =87;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x55\x5F\x28\x02\x01\x56\x8E\x10\x00\x00\x00\x00\x00\x00\x00\x00\x41\x00\x42\x03\x60\x03\x1F\x00\x9F\x07\x02\xAB\x00\x9F\x08\x02\x00\x20\x9F\x0D\x05\xFC\xF0\xE4\x08\x00\x9F\x0E\x05\x00\x10\x00\x00\x00\x9F\x0F\x05\xFC\xF8\xE4\xF8\x80\x9F\x61\x11\x33\x32\x30\x31\x30\x33\x30\x30\x30\x30\x30\x30\x30\x30\x39\x39\x39\x9F\x62\x01\x00\x9F\x14\x01\x7F",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x03\x0C\x00",5))
	{
		pEMVICCOut->uiLenOut =72;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x46\x8C\x18\x9F\x02\x06\x9F\x03\x06\x9F\x1A\x02\x95\x05\x5F\x2A\x02\x9A\x03\x9F\x21\x03\x9C\x01\x9F\x37\x04\x8D\x1A\x8A\x02\x9F\x02\x06\x9F\x03\x06\x9F\x1A\x02\x95\x05\x5F\x2A\x02\x9A\x03\x9F\x21\x03\x9C\x01\x9F\x37\x04\x9F\x42\x02\x01\x56\x9F\x49\x03\x9F\x37\x04\x5F\x30\x02\x02\x01",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x01\x14\x00",5))
	{
		pEMVICCOut->uiLenOut =12;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x0A\x5A\x08\x62\x25\x00\x00\x00\x00\x00\x22",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x02\x14\x00",5))
	{
		pEMVICCOut->uiLenOut =215;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x81\xD4\x8F\x01\x08\x90\x81\x90\x98\x84\x31\xEF\x60\x93\x5A\xD3\x51\xA0\xBB\xAF\x1F\x38\x48\x16\x99\xA8\x75\xAB\x74\x0E\x56\xBA\xC7\x99\xD8\xAA\x58\x48\xBC\x95\x45\xA5\x8D\x83\xA3\xF1\xB2\xAE\xD5\x12\xA6\x0D\x6B\x87\x25\x26\xE1\x51\x14\x40\x92\xA9\xF4\xED\xAC\xF3\xC5\x7B\xD3\x16\xB7\xB7\x75\xEA\x9C\x64\xDB\xD2\xA7\x26\x6E\xF0\x5D\xFB\xEB\x0B\x4D\x52\x6F\x90\xC1\x6C\xD6\x0C\xF5\x8F\x8E\x72\xC4\xA9\x26\x29\x3D\xA5\x44\x5B\x4C\xE9\x91\x2F\x0D\x92\x7C\xFB\xBC\x38\x3E\xD9\x14\x47\xFA\xA3\x25\x87\x23\x9D\x77\xFE\x48\xA9\xA6\x5A\xAB\x8E\xAB\x6C\x64\xCB\x14\x98\xBF\x1E\xFD\x77\x94\x93\x1E\x7C\x19\xE2\x59\xE9\x92\x24\xFF\xAB\xC8\xEC\xF6\x2F\xE4\xB0\x96\x94\x9F\xAE\x88\xA3\x31\x79\x28\x73\x16\x3B\xEC\xD9\x0D\x75\xD8\xF1\x57\x0F\x47\xED\x40\xF7\x86\x90\xB7\xFB\x9F\x32\x01\x03\x5F\x24\x03\x15\x10\x31\x5F\x25\x03\x05\x09\x01\x5F\x34\x01\x02\x9F\x23\x01\xFF",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x03\x14\x00",5))
	{
		pEMVICCOut->uiLenOut =150;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x81\x93\x93\x81\x90\x6B\xF7\x43\xC0\x4E\xA9\x7F\x2E\x50\x37\x38\x75\x87\x9C\x25\x30\x77\xB8\x74\xB0\xC2\x2E\x21\x52\x62\xE4\x7F\x14\x8D\x96\x6F\x6C\xE3\xA2\x44\xCE\x92\x73\xAF\xB9\x92\x7A\xDE\xA2\xFD\x04\xA3\x13\xAE\x9B\x88\xC2\xE4\xE0\x79\xAB\x6D\x55\xF0\x40\xA8\x01\x3B\xC4\xC5\xED\x10\x86\x4E\x9A\xFB\xD8\xAF\xDD\xDC\xA7\x0F\x18\xC4\x55\xAE\x0A\x92\x1C\xC3\xAC\x80\xAF\x57\x55\xCF\x42\x61\xE9\x73\x2F\x65\x40\xD6\xA1\x0D\x93\xD7\x1E\xCC\x05\x43\x80\xDB\xEB\x8C\x1A\x11\xE7\x42\xF9\x36\xEC\xF5\xD8\xF7\xD5\xE4\x81\x72\x41\x4A\x72\xB3\x3F\xF3\x21\x4D\x6B\x07\x58\x19\xC2\x21\x12\xCF\xA5\x1B\x9D",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x04\x14\x00",5))
	{
		pEMVICCOut->uiLenOut =184;
		memcpy(pEMVICCOut->aucDataOut,"\x70\x81\xB5\x9F\x46\x81\x90\x0D\xED\xDA\x66\xB2\xA2\x1B\xB8\x89\x61\x7C\xD1\x58\xAC\x7D\xC3\x81\xD4\x78\xA8\x4E\xDD\x08\xDE\x8C\x53\xA6\x9D\x94\x05\xE0\x77\xF5\x6E\x96\xCB\xC7\xC7\xDD\x65\xAE\xCF\x1E\x18\x62\x61\x36\x23\x81\x9D\x3D\x07\xE1\xBB\x60\x88\x65\x4C\x4E\x05\x67\xF6\x18\x26\x65\xF6\xEC\xDB\xFC\xB8\xF9\xA4\x12\x01\x42\x3C\x23\x97\x8A\x12\x71\xB1\xB0\xA0\xCA\x99\x08\x3A\x47\x04\xC7\x93\x17\xA9\xCE\x76\x27\x47\x4E\x3C\xF5\xE2\xB4\xBA\x69\xB8\xE9\x24\x66\xAA\x18\xB5\x2A\x27\x2A\x5F\xD3\xA7\x21\x80\x9C\x51\x40\x28\xFC\xEB\x76\x46\x1E\xDF\x66\x0D\x4A\x02\x4B\x32\xA9\x6E\x8A\xDD\xC9\xE2\x4A\xA3\x9F\x47\x01\x03\x9F\x48\x1A\xD8\x77\xF9\xA7\xD6\x60\x56\x5C\x76\xAF\x91\xB4\xC0\x3A\xB4\x4B\x4F\x45\x4A\xD1\xED\xDF\x7A\xC7\x84\x73",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x80\xCA\x9F\x17\x00",5))
	{
		pEMVICCOut->uiLenOut =4;
		memcpy(pEMVICCOut->aucDataOut,"\x9F\x17\x01\x0F",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x80\xCA\x9F\x36\x00",5))
	{
		pEMVICCOut->uiLenOut =5;
		memcpy(pEMVICCOut->aucDataOut,"\x9F\x36\x02\x00\x98",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x80\xCA\x9F\x13\x00",5))
	{
		pEMVICCOut->uiLenOut =5;
		memcpy(pEMVICCOut->aucDataOut,"\x9F\x13\x02\x00\x97",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x80\xAE\x90\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x01\x56\x00\x00\x0C\x00\x00\x01\x56\x09\x09\x16\x16\x37\x26\x00\xF1\x9A\x09\x0D\x00",36))
	{
		pEMVICCOut->uiLenOut =216;
		memcpy(pEMVICCOut->aucDataOut,"\x77\x81\xA3\x9F\x27\x01\x80\x9F\x36\x02\x00\x98\x9F\x4B\x81\x80\x92\x32\xCC\x6D\xDB\xEF\xC7\x16\x49\xA9\xDF\x70\x2E\xAE\x93\x4C\x3D\x9D\x1A\x7B\xC0\x00\xF9\xB3\xD1\x79\x79\x40\x79\xA7\xD7\x90\x20\xB1\x4B\x83\x1B\x9F\x9E\xEA\x84\xE9\x6C\x4E\x77\x61\xA2\xD9\xE9\xE0\x1B\xA7\x40\x88\xD1\x8A\xF6\xC2\xE8\xB7\x8D\xE9\x03\x69\x63\x14\x23\x4A\x4E\x92\x3C\x17\x88\xB7\x92\xFC\x69\xD4\xF2\xC3\x50\xF9\xDF\xF3\x19\xA7\x79\xCE\x98\x1D\xD9\x0D\x05\xEB\xB7\xDA\xB0\x3E\x68\x9D\xF7\x23\xBE\xD4\x8D\x70\x35\x8F\x51\xE5\x55\x00\x38\xB3\x33\x80\x92\x81\x7C\xB3\x34\xB6\xC9\x0E\x08\x13\x6B\xC2\x9F\x10\x13\x07\x01\x01\x03\xA0\x00\x02\x01\x0A\x01\x00\x00\x00\x00\x00\x5B\xEE\x95\x82",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}

#endif
#if 1
	if(!memcmp(&aucBuff[1],"\x00\xA4\x04\x00\x31\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44\x46\x30\x31\x00",19))
	{
		pEMVICCOut->uiLenOut =23;
		memcpy(pEMVICCOut->aucDataOut,"\x6F\x15\x84\x0E\x31\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44\x46\x30\x31\xA5\x03\x88\x01\x01",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x01\x0C\x00",5))
	{
		pEMVICCOut->uiLenOut =30;      //\x70\x1C\x61\x1A\x4F\x08\xA0\x00\x00\x03\x33\x01\x01\x02\x50\x0B\x50\x42\x4F\x43\x20\x43\x72\x65\x64\x69\x74\x87\x01\x81
		memcpy(pEMVICCOut->aucDataOut,"\x70\x1C\x61\x1A\x4F\x08\xA0\x00\x00\x03\x33\x01\x01\x02\x50\x0B\x50\x42\x4F\x43\x20\x43\x72\x65\x64\x69\x74\x87\x01\x81",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x00\xB2\x02\x0C\x00",5))
	{
		//pEMVICCOut->uiLenOut =40;
		//memcpy(pEMVICCOut->aucDataOut,"\x70\x1C\x61\x1A\x4F\x08\xA0\x00\x00\x03\x33\x01\x01\x02\x50\x0B\x50\x42\x4F\x43\x20\x43\x72\x65\x64\x69\x74\x87\x01\x81",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x6A;
		pEMVICCOut->ucSWB= 0x83;
	}
	else if(!memcmp(&aucBuff[1],"\x00\xA4\x04\x00\xA0\x00\x00\x03\x33\x01\x01\x02\x00",13))
	{
		pEMVICCOut->uiLenOut =82;
		memcpy(pEMVICCOut->aucDataOut,"\x6F\x50\x84\x08\xA0\x00\x00\x03\x33\x01\x01\x02\xA5\x44\x50\x0B\x50\x42\x4F\x43\x20\x43\x72\x65\x64\x69\x74\x87\x01\x81\x9F\x38\x0C\x9F\x7A\x01\x9F\x02\x06\x5F\x2A\x02\x9F\x1A\x02\x5F\x2D\x02\x7A\x68\x9F\x11\x01\x01\x9F\x12\x0B\x50\x42\x4F\x43\x20\x43\x72\x65\x64\x69\x74\xBF\x0C\x05\x9F\x4D\x02\x0B\x0A\x00\x00\x00\x00\x00\x00",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
	else if(!memcmp(&aucBuff[1],"\x00\xA4\x04\x00\xA0\x00\x00\x03\x33\x01\x01\x02\x00",13))
	{
		pEMVICCOut->uiLenOut =82;
		memcpy(pEMVICCOut->aucDataOut,"\x6F\x50\x84\x08\xA0\x00\x00\x03\x33\x01\x01\x02\xA5\x44\x50\x0B\x50\x42\x4F\x43\x20\x43\x72\x65\x64\x69\x74\x87\x01\x81\x9F\x38\x0C\x9F\x7A\x01\x9F\x02\x06\x5F\x2A\x02\x9F\x1A\x02\x5F\x2D\x02\x7A\x68\x9F\x11\x01\x01\x9F\x12\x0B\x50\x42\x4F\x43\x20\x43\x72\x65\x64\x69\x74\xBF\x0C\x05\x9F\x4D\x02\x0B\x0A\x00\x00\x00\x00\x00\x00",pEMVICCOut->uiLenOut);
		pEMVICCOut->ucSWA = 0x90;
		pEMVICCOut->ucSWB= 0x00;
	}
#endif
#endif
	/*add debug   2009-9-2 16:18  cbf*/
	if(!ucResult)
	{
		util_Printf("\nRecv ICC Data:\n");
		for(uiI=0;uiI<pEMVICCOut->uiLenOut;uiI++)
			util_Printf("%02X ",pEMVICCOut->aucDataOut[uiI]);
		util_Printf("\nSW: %02X %02X",pEMVICCOut->ucSWA,pEMVICCOut->ucSWB);
	}
	return ucResult;
}

unsigned char MASAPP_Event(unsigned char ucDispInfoFlag , unsigned char * ucOutKey)
{
	unsigned char ucKey;
	unsigned char ucResult;
	unsigned int    uiTimesOut;
	unsigned char ucReadCardFlag;
	unsigned char ucICCardType,ucReadCardMode,ucInputMode,ucCardSelect;
	unsigned char ucReadMagErrFlag;

	DRV_OUT *pxMag;
	DRV_OUT *pxIcc;
	DRV_OUT *pxKey;
	UCHAR	aucResetBuf[50],ucResetBufLen;
	UCHAR	bFallBack;

NEW_DRV_TYPE  new_drv;
uchar ucDrv;

	ucReadCardFlag=0;
	UTIL_SetInputCardMode(&ucReadCardMode);//设置终端支持的读卡模式

	ucResult = MAG_EMVSetInputMode(&ucInputMode);
	if(ucResult != SUCCESS)/*设置终端+交易支持的读卡模式[SwipeCard][Manual][Insert]*/
	{
		return ucResult;
	}

	bFallBack = FALSE;
	ucReadMagErrFlag = false;
	util_Printf("\nucDispInfoFlag=%d",ucDispInfoFlag);
	if(ucDispInfoFlag==true)
		MAG_DisplaySwipeCardMode(G_NORMALTRANS_ucTransType,ucInputMode);
	while(1)
	{
		util_Printf("\nG_NORMALTRANS_ucFallBackFlagABC=%02x",G_NORMALTRANS_ucFallBackFlag);
		if(ucReadCardFlag==true||G_NORMALTRANS_ucFallBackFlag)
		{
			ucInputMode = TRANS_INPUTMODE_SWIPECARD;
			MAG_DisplaySwipeCardMode(G_NORMALTRANS_ucTransType,ucInputMode);
		}
		/*
		Os__abort_drv(drv_mmi);
		Os__abort_drv(drv_mag);
		Os__abort_drv(drv_icc);
		ucResult = SUCCESS;
		uiTimesOut = 30;
		uiTimesOut = uiTimesOut*100;
		SEL_AccumulatRunTime();//累积各运行时间
		pxKey = Os__get_key();
		if(ucInputMode&TRANS_INPUTMODE_SWIPECARD)
		{
			pxMag = Os__mag_read();
		}
		if(ucInputMode&TRANS_INPUTMODE_INSERTCARD)
		{
			pxIcc = Os__ICC_insert();
		}
		Os__timer_start(&uiTimesOut);
		while(uiTimesOut)
		{
			if((ucInputMode&TRANS_INPUTMODE_SWIPECARD)&&(pxMag->gen_status == DRV_ENDED))
				break;
			if(pxKey->gen_status == DRV_ENDED)
				break;
			if((ucInputMode&TRANS_INPUTMODE_INSERTCARD)&&(pxIcc->gen_status == DRV_ENDED))
				break;
			if(ucDispInfoFlag==false&& !ucReadCardFlag)
				DispDateAndTime(3,DISPCOLFIRST);
		}
		Os__timer_stop(&uiTimesOut);
		if(!uiTimesOut)
		{
			Os__abort_drv(drv_mmi);
			Os__abort_drv(drv_mag);
			Os__abort_drv(drv_icc);
			if(!DataSave0.ConstantParamData.BackLightFlag)
				Os__light_off();
			Os__light_off_pp();
			*ucOutKey = KEY_CLEAR;
			return(ERR_END);
		}else
		{
			Os__light_on();
			Os__light_on_pp();
		}
		*/
		if( ucInputMode&TRANS_INPUTMODE_SWIPECARD)
			ucDrv = KEY_DRV| MAG_DRV;
		if( ucInputMode&TRANS_INPUTMODE_INSERTCARD)
			ucDrv = ucDrv|KEY_DRV;

		util_Printf("\n1.MASAPP_Event Os_Wait_Event begin...");
//		ucResult = Os_Wait_Event(ucDrv, &new_drv, 1000*60);
		memset((char*)&new_drv, 0x00, sizeof(NEW_DRV_TYPE));
		memcpy( (char*)&new_drv, (char*)&ProUiFace.OweDrv,sizeof(NEW_DRV_TYPE));
		ucResult = 0;		
		//util_Printf("\n2. Os_Wait_Event:%02x",ret);
		util_Printf("\n3. new_drv.drv_type:%02x",new_drv.drv_type);
		util_Printf("\n4. new_drv.drv_data.gen_status:%02x",new_drv.drv_data.gen_status);
		
		if(ucResult == 0)                                                              
		{                                                                         
			if(new_drv.drv_type == KEY_DRV)                                     
			{                                                                   
				if(new_drv.drv_data.gen_status== DRV_ENDED){
					pxKey = (DRV_OUT *)&new_drv.drv_data;
				}
			}                                                                   
			else if(new_drv.drv_type == ICC_DRV)                                
			{                                                                   
				if(new_drv.drv_data.gen_status== DRV_ENDED)
				{
					pxIcc =  (DRV_OUT *)&new_drv.drv_data;
				}
			}                                                                   
			else if(new_drv.drv_type == MAG_DRV)                                
			{                                                                   
				if(new_drv.drv_data.gen_status== DRV_ENDED)
				{
					pxMag = (DRV_OUT *)& new_drv.drv_data;
				}
			}                                                                   
			else if(new_drv.drv_type == DRV_TIMEOUT)                            
			{    
				return ERR_CANCEL;			
			}	        
			util_Printf("\n OWE_NewDrvtts End.....\n");
			//break;
		}       
					
		util_Printf("pxIcc->gen_status:%02x\n",pxIcc->gen_status);
		util_Printf("ucICCardType:%02x\n",pxIcc->xxdata[1]);
		util_Printf("ucInputMode:%02x\n\n",ucInputMode);
util_Printf("\n 0000001 . insert :%02x %02x",ucInputMode&TRANS_INPUTMODE_INSERTCARD, pxIcc->gen_status);
util_Printf("\n 0000001 . swipt :%02x %02x",ucInputMode&TRANS_INPUTMODE_SWIPECARD, pxMag->gen_status);


		if((ucInputMode&TRANS_INPUTMODE_INSERTCARD)&&(pxIcc->gen_status== DRV_ENDED))
		{
util_Printf("\n 0000002 . insert :%02x %02x",ucInputMode&TRANS_INPUTMODE_INSERTCARD, pxIcc->gen_status);
			Os__abort_drv(drv_mag);
			Os__abort_drv(drv_mmi);

			if(UTIL_Check_BasetSet() == SUCCESS)
				return ERR_CANCEL;
			ucResult =SUCCESS;
			ucICCardType =pxIcc->xxdata[1];
			util_Printf("ucICCardTypedfdfdfdafd = %02x\n",ucICCardType);
			ucResetBufLen=sizeof(aucResetBuf);
			if(ucICCardType==0x39)
				ucICCardType=SMART_WarmReset(0,0,aucResetBuf,(unsigned short*)&ucResetBufLen);
			util_Printf("ucICCardType = %02x\n",ucICCardType);
			if(ucICCardType!=ICC_ASY
				&&ucICCardType!=ICC_SYN)/*同步或异步SMART卡*/
			{
				ucReadCardFlag = true;
				ucInputMode&=~TRANS_INPUTMODE_INSERTCARD;
				G_NORMALTRANS_ucFallBackFlag =2;
				switch(G_NORMALTRANS_ucTransType)
				{
					case TRANS_EC_CASHLOAD:
					case TRANS_EC_ASSIGNLOAD:
					case TRANS_EC_UNASSIGNLOAD:
						return ERR_ICCARD;
					default:
						break;
				}
				continue;
			}else if(ucICCardType!=ICC_SYN)
			{
				G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_INSERTCARD;
				G_NORMALTRANS_euCardSpecies=CARDSPECIES_EMV;
				break;
			}
			else
			{
				ucReadCardFlag = true;
				ucInputMode&=~TRANS_INPUTMODE_INSERTCARD;
				G_NORMALTRANS_ucFallBackFlag =2;
				switch(G_NORMALTRANS_ucTransType)
				{
					case TRANS_EC_CASHLOAD:
					case TRANS_EC_ASSIGNLOAD:
					case TRANS_EC_UNASSIGNLOAD:
						return ERR_ICCARD;
					default:
						break;
				}
				continue;
			}
		}
		else
		if((ucInputMode&TRANS_INPUTMODE_SWIPECARD)&&(pxMag->gen_status == DRV_ENDED))
		{
util_Printf("\n 0000003 . insert :%02x %02x",ucInputMode&TRANS_INPUTMODE_INSERTCARD, pxIcc->gen_status);
			Os__abort_drv(drv_icc);
			Os__abort_drv(drv_mmi);
			if(UTIL_Check_BasetSet() == SUCCESS)
				return ERR_CANCEL;
			
			ucResult =SUCCESS;
			ucResult= MAG_GetTrackInfo((unsigned char*)pxMag);
			util_Printf("\nMAG_GetTrackInfo()------ucResult = %02x\n",ucResult);
			if (ucResult == ERR_SANDMAGCARD)
			    return(ucResult);
			if(!ucResult||ucResult==ERR_USEICCARDFIRST)
			{
				G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_SWIPECARD;
				G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
				ucReadMagErrFlag = false;
			}else
			{
				ucDispInfoFlag = true;
				ucReadMagErrFlag = true;
				continue;
			}
			
			util_Printf("ucM=[%02x]Result=[%02x]CardFlag=[%02x]\n",ucInputMode,ucResult,ucReadCardFlag);
			if(ucInputMode&TRANS_INPUTMODE_INSERTCARD
				&&ucResult==ERR_USEICCARDFIRST
				&& !ucReadCardFlag)
			{
				if((G_NORMALTRANS_ucTransType == TRANS_VOIDPREAUTH)
					||(G_NORMALTRANS_ucTransType == TRANS_PREAUTHSETTLE)
					||(G_NORMALTRANS_ucTransType == TRANS_PREAUTHFINISH))
				{
					ucCardSelect = CARDSPECIES_MAG;
					ucResult = SUCCESS;
				}
				else
				{
					ucCardSelect=MAG_ManuSelectCard(G_NORMALTRANS_ucTransType);
					util_Printf("Select =%02x\n",ucCardSelect);
					if(ucCardSelect==ERR_CANCEL)
					{
						ucResult=ERR_CANCEL;
					}
				}
			}else
			{
				if(!ucResult)
				{
					G_NORMALTRANS_ucInputMode = TRANS_INPUTMODE_SWIPECARD;
					G_NORMALTRANS_euCardSpecies = CARDSPECIES_MAG;
					ucCardSelect = CARDSPECIES_MAG;
					ucResult = SUCCESS;
				}
				else if (ucResult ==ERR_USEICCARDFIRST)
				{
					if((G_NORMALTRANS_ucTransType == TRANS_VOIDPREAUTH)
						||(G_NORMALTRANS_ucTransType == TRANS_PREAUTHSETTLE)
						||(G_NORMALTRANS_ucTransType == TRANS_PREAUTHFINISH))
					{
						ucCardSelect = CARDSPECIES_MAG;
						ucResult = SUCCESS;
					}
					else
					{
						ucCardSelect=MAG_ManuSelectCard(G_NORMALTRANS_ucTransType);
						util_Printf("Select2009-10-20 11:26 =%02x\n",ucCardSelect);
						if(ucCardSelect == CARDSPECIES_MAG)
						{
							G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_SWIPECARD;
							G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
							ucCardSelect=CARDSPECIES_MAG;
							G_NORMALTRANS_ucFallBackFlag=0;
						}
						else if(ucCardSelect ==CARDSPECIES_EMV)
						{
							G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_SWIPECARD;
							G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
							ucCardSelect=CARDSPECIES_MAG;
							G_NORMALTRANS_ucFallBackFlag =2;
						}
					}
				}
					if(ucCardSelect==ERR_CANCEL)
						ucResult=ERR_CANCEL;
					else 
						ucResult = SUCCESS;
			}

			if(ucResult==ERR_USEICCARDFIRST
				&&ucCardSelect==CARDSPECIES_EMV
				&&(ucInputMode&TRANS_INPUTMODE_INSERTCARD))
			{
				ucResult = MASAPP_WaitReadIC(ucInputMode );
				if((ucResult== ERR_FALLBACK||ucResult== ERR_ICCARD) &&G_RUNDATA_ucReadCardFlag == EMVFIRSTMAG)
				{
					G_NORMALTRANS_ucInputMode = TRANS_INPUTMODE_SWIPECARD;
					G_NORMALTRANS_euCardSpecies = CARDSPECIES_MAG;
					ucCardSelect=CARDSPECIES_MAG;
					G_NORMALTRANS_ucFallBackFlag = 2;
					ucResult = SUCCESS;
				}else
				{
					if(!ucResult)
						ucResult=MAG_UerICCardFirstProcess(ucCardSelect,G_NORMALTRANS_ucFallBackFlag,ucResult);
					if(!ucResult)
					{
						G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_INSERTCARD;
						G_NORMALTRANS_euCardSpecies=CARDSPECIES_EMV;
						G_NORMALTRANS_ucFallBackFlag =0;
					}
				}
			}

			util_Printf("\n 20120618 - 001 ucResult = %02x\n",ucResult);
			util_Printf("ucCardSelect = %02x\n",ucCardSelect);
			util_Printf("G_NORMALTRANS_ucInputMode = %02x\n",G_NORMALTRANS_ucInputMode);
			if((ucResult == ERR_USEICCARDFIRST||ucResult ==SUCCESS)
				&&(ucCardSelect==CARDSPECIES_MAG)
				&&(G_NORMALTRANS_ucInputMode==TRANS_INPUTMODE_SWIPECARD)
			  )
				ucResult = MAG_DispCardNo();
			break;
		}
		else
		if(!ucResult&&(pxKey->gen_status == DRV_ENDED))
		{
util_Printf("\n 0000004 . insert :%02x %02x",ucInputMode&TRANS_INPUTMODE_INSERTCARD, pxIcc->gen_status);		
			Os__abort_drv(drv_mag);
			Os__abort_drv(drv_icc);
			ucKey = pxKey->xxdata[1];

			if(ucKey>='0'&&ucKey<='9')
			{
				G_NORMALTRANS_ucInputMode = TRANS_INPUTMODE_MANUALINPUT;
				G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
				switch( G_NORMALTRANS_ucTransType )
				{
					case TRANS_PREAUTH:
					case TRANS_QUERYBAL:
					case TRANS_VOIDPREAUTH:
					case TRANS_VOIDPURCHASE:
					case TRANS_PREAUTHSETTLE:
					case TRANS_PREAUTHFINISH:
					case TRANS_VOIDPREAUTHFINISH:
					case TRANS_OFFPREAUTHFINISH:
					case TRANS_PREAUTHADD:
					case TRANS_CREDITSVOID:
						return(SEL_GetCardMsg(ucKey));
					default:
						ucResult=ERR_CANCEL;
						break;
				}
			}
			else
			{
util_Printf("\n 0000005 . insert :%02x %02x",ucInputMode&TRANS_INPUTMODE_INSERTCARD, pxIcc->gen_status);		
			
				if(ucReadMagErrFlag == false)
				{
					*ucOutKey = ucKey;
				}
				ucResult = ERR_CANCEL;
			}
			
			if (ucKey != KEY_CLEAR)
			    continue;
			 else
			    break;
		}
		break;
	}

	util_Printf("\n 20120618 - 002 ucResult = %02x\n",ucResult);
	util_Printf("ucCardSelect = %02x\n",ucCardSelect);
	util_Printf("G_NORMALTRANS_ucInputMode = %02x\n",G_NORMALTRANS_ucInputMode);

	return ucResult;
}


#if SANDREADER
unsigned char MASAPP_Event_SAND(unsigned char ucDispInfoFlag , unsigned char * ucOutKey)
{
	unsigned char ucKey;
	unsigned char ucResult;
	unsigned int    uiTimesOut;
	unsigned char ucReadCardFlag;
	unsigned char ucICCardType,ucInputMode,ucCardSelect;
	unsigned char ucReadMagErrFlag;
	DRV_OUT *pxMag;
	DRV_OUT *pxIcc;
	DRV_OUT *pxKey;
	DRV_OUT *pxMifare;
	QTRANSTYPE	enTransType;
	UCHAR	aucResetBuf[50],ucResetBufLen;
	UCHAR	bFallBack;
	
	NEW_DRV_TYPE  new_drv;
	uchar ucDrv;
	unsigned int uiI=0;

	ucReadCardFlag=0;
	bFallBack = FALSE;
	ucReadMagErrFlag = false;
	ucResult = MAG_EMVSetInputMode(&ucInputMode);
	if(ucResult != SUCCESS)/*设置终端+交易支持的读卡模式[SwipeCard][Manual][Insert]*/
	{
		return ucResult;
	}
	ucInputMode =0x15;
	if(G_NORMALTRANS_ucECTrans==TRANS_Q_PURCHASE)
	{
		ucInputMode&=~TRANS_INPUTMODE_INSERTCARD;
		ucInputMode&=~TRANS_INPUTMODE_SWIPECARD;
		if(ucDispInfoFlag==true)
			MAG_DisplaySwipeCardMode(G_NORMALTRANS_ucTransType,0x18);
	}
	else
	{
		if(ucDispInfoFlag==true)
			MAG_DisplaySwipeCardMode(G_NORMALTRANS_ucTransType,ucInputMode);
	}
	while(1)
	{
		util_Printf("\n G_NORMALTRANS_ucFallBackFlagABC_SAND=%02x",G_NORMALTRANS_ucFallBackFlag);
		if(ucReadCardFlag==true||G_NORMALTRANS_ucFallBackFlag)
		{
			ucInputMode = TRANS_INPUTMODE_SWIPECARD;
			MAG_DisplaySwipeCardMode(G_NORMALTRANS_ucTransType,ucInputMode);
		}
/*		Os__abort_drv(drv_mmi);
		Os__abort_drv(drv_mag);
		Os__abort_drv(drv_icc);

		ucResult = SUCCESS;
		uiTimesOut = 30;
		uiTimesOut = uiTimesOut*100;
		SEL_AccumulatRunTime();//累积各运行时间
		pxKey = Os__get_key();
		if(ucInputMode&TRANS_INPUTMODE_SWIPECARD)
		{
			pxMag = Os__mag_read();
		}
		if(ucInputMode&TRANS_INPUTMODE_INSERTCARD)
		{
			pxIcc = Os__ICC_insert();
		}
		//使用MIFARE先关闭，避免假死问题
		Os__MIFARE_PowerRF(0);
		pxMifare = Os__MIFARE_Polling();
		util_Printf("\n======++++非接数据:%s\n",pxMifare->xxdata);
		Os__timer_start(&uiTimesOut);
		while(uiTimesOut)
		{
			if((ucInputMode&TRANS_INPUTMODE_SWIPECARD)&&(pxMag->gen_status == DRV_ENDED))
				break;
			if(pxKey->gen_status == DRV_ENDED)
				break;
			if((ucInputMode&TRANS_INPUTMODE_INSERTCARD)&&(pxIcc->gen_status == DRV_ENDED))
				break;
  		       if(pxMifare->gen_status== DRV_ENDED)
			{
				break;
			}
			if(ucDispInfoFlag==false&& !ucReadCardFlag)
				DispDateAndTime(3,DISPCOLFIRST);
		}
		Os__timer_stop(&uiTimesOut);
		if(!uiTimesOut)
		{
			Os__abort_drv(drv_mmi);
			Os__abort_drv(drv_mag);
			Os__abort_drv(drv_icc);
			if(!DataSave0.ConstantParamData.BackLightFlag)
				Os__light_off();
			Os__light_off_pp();
			*ucOutKey = KEY_CLEAR;
			return(ERR_END);
		}else
		{
			Os__light_on();
			Os__light_on_pp();
		}
*/

		//ucDrv = KEY_DRV|MFR_DRV;
		//if( ucInputMode&TRANS_INPUTMODE_SWIPECARD) ucDrv = ucDrv | MAG_DRV;
		//if( ucInputMode&TRANS_INPUTMODE_INSERTCARD)	ucDrv = ucDrv | ICC_DRV;
		//ucResult = Os_Wait_Event(ucDrv, &new_drv, 1000*30);
		//util_Printf("\n2. Os_Wait_Event:%02x",ucResult);	
/*
		if(ucResult == 0)                                                              
		{                                                                         
			if(new_drv.drv_type == KEY_DRV)                                     
			{                                                                   
				if(new_drv.drv_data.gen_status== DRV_ENDED){
					pxKey = (DRV_OUT *)& new_drv.drv_data;
				}
			}                                                                   
			else if(new_drv.drv_type == ICC_DRV)                                
			{                                                                   
				if(new_drv.drv_data.gen_status== DRV_ENDED)
				{
					pxIcc = (DRV_OUT *)& new_drv.drv_data;
				}
			}                                                                   
			else if(new_drv.drv_type == MAG_DRV)                                
			{                                                                   
				if(new_drv.drv_data.gen_status== DRV_ENDED)
				{
					pxMag =  (DRV_OUT *)&new_drv.drv_data;
				}
			}                                                                   
			else if(new_drv.drv_type == MFR_DRV)                                
			{                                                                   
				if(new_drv.drv_data.gen_status== DRV_ENDED)
				{
					pxMifare =  (DRV_OUT *)&new_drv.drv_data;
				}
			}                                                                   
			else if(new_drv.drv_type == DRV_TIMEOUT)                            
			{    
				if(!DataSave0.ConstantParamData.BackLightFlag)	Os__light_off();
				Os__light_off_pp();
				*ucOutKey = KEY_CLEAR;
				return(ERR_END);
			}	        
			Os__light_on();
			Os__light_on_pp();
		}       
*/	
		util_Printf("\n1.MASAPP_Event_SAND Os_Wait_Event begin.[%02x]\n",ProUiFace.UiToPro.CardInfo.ucInputMode);
		memset((char*)&new_drv,0,sizeof(NEW_DRV_TYPE));
		pxMag =  (DRV_OUT *)&new_drv.drv_data;
		pxMifare =  (DRV_OUT *)&new_drv.drv_data;
		pxIcc =  (DRV_OUT *)&new_drv.drv_data;

		if(ProUiFace.UiToPro.CardInfo.ucInputMode == TRANS_INPUTMODE_SWIPECARD)
		{
			memset((char*)&new_drv, 0x00, sizeof(NEW_DRV_TYPE));
			memcpy( (char*)&new_drv, (char*)&ProUiFace.OweDrv,sizeof(NEW_DRV_TYPE));	

			pxIcc->gen_status=0x00;
			pxMifare->gen_status=0x00;

			if(new_drv.drv_data.gen_status== DRV_ENDED)
				pxMag =  (DRV_OUT *)&new_drv.drv_data;

			util_Printf("\n3. new_drv.drv_type:%02x\n",new_drv.drv_type);
			for(uiI=0; uiI<24; uiI++)
				util_Printf("%02X ", new_drv.drv_data.xxdata[uiI]);
 		}else if(ProUiFace.UiToPro.CardInfo.ucInputMode  == TRANS_INPUTMODE_SAND_PUTCARD){
			pxMag->gen_status=0x00;
			pxIcc->gen_status=0x00;
			pxMifare->gen_status=DRV_ENDED;

		}else if(ProUiFace.UiToPro.CardInfo.ucInputMode  == TRANS_INPUTMODE_INSERTCARD){			
			new_drv.drv_data.gen_status = DRV_ENDED;
			new_drv.drv_data.xxdata[0] = 0x00;
			pxIcc =  (DRV_OUT *)&new_drv.drv_data;

			pxMag->gen_status=0x00;
			pxMifare->gen_status=0x00;
		}else{
		
		}
		ucResult = 0;

util_Printf("\n4.pxMifare->gen_status  .................[%02x]\n",pxMifare->gen_status);
util_Printf(".A:[%02X] [%02X] \n",ucInputMode&TRANS_INPUTMODE_SWIPECARD, pxMag->gen_status);
util_Printf(".B:[%02X] [%02X] \n",ucInputMode&TRANS_INPUTMODE_SAND_PUTCARD, pxMifare->gen_status);
util_Printf(".C:[%02X] [%02X] \n",ucInputMode&TRANS_INPUTMODE_INSERTCARD, pxIcc->gen_status);
		//if((ucInputMode&TRANS_INPUTMODE_INSERTCARD)&&(pxIcc->gen_status== DRV_ENDED))
		if(ProUiFace.UiToPro.CardInfo.ucInputMode == TRANS_INPUTMODE_INSERTCARD)
		{
			Os__abort_drv(drv_mag);
			Os__abort_drv(drv_mmi);
			util_Printf("\n2.MASAPP_Event_SAND  .................\n");
			if(UTIL_Check_BasetSet() == SUCCESS)
				return ERR_CANCEL;
			
			ucResult =SUCCESS;
			ucICCardType =pxIcc->xxdata[1];
			util_Printf("ucICCardTypedfdfdfdafd = %02x\n",ucICCardType);
			ucResetBufLen=sizeof(aucResetBuf);
			if(ucICCardType==0x39)
				ucICCardType=SMART_WarmReset(0,0,aucResetBuf,(unsigned short*)&ucResetBufLen);
			util_Printf("ucICCardType = %02x\n",ucICCardType);
			if(ucICCardType!=ICC_ASY
				&&ucICCardType!=ICC_SYN)/*同步或异步SMART卡*/
			{
				ucReadCardFlag = true;
				ucInputMode&=~TRANS_INPUTMODE_INSERTCARD;
				G_NORMALTRANS_ucFallBackFlag =2;
				switch(G_NORMALTRANS_ucTransType)
				{
					case TRANS_EC_CASHLOAD:
					case TRANS_EC_ASSIGNLOAD:
					case TRANS_EC_UNASSIGNLOAD:
						return ERR_ICCARD;
					default:
						break;
				}
				continue;
			}else
			if(ucICCardType!=ICC_SYN)
			{
				G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_INSERTCARD;
				G_NORMALTRANS_euCardSpecies=CARDSPECIES_EMV;
				break;
			}
			else
			{
				ucReadCardFlag = true;
				ucInputMode&=~TRANS_INPUTMODE_INSERTCARD;
				G_NORMALTRANS_ucFallBackFlag =2;
				switch(G_NORMALTRANS_ucTransType)
				{
					case TRANS_EC_CASHLOAD:
					case TRANS_EC_ASSIGNLOAD:
					case TRANS_EC_UNASSIGNLOAD:
						return ERR_ICCARD;
					default:
						break;
				}
				continue;
			}
		}
		else
		//if((ucInputMode&TRANS_INPUTMODE_SWIPECARD)&&(pxMag->gen_status == DRV_ENDED))
		if(ProUiFace.UiToPro.CardInfo.ucInputMode == TRANS_INPUTMODE_SWIPECARD)
		{
			util_Printf("\n3.MASAPP_Event_SAND  .................\n");
		
			Os__abort_drv(drv_icc);
			Os__abort_drv(drv_mmi);
			if(UTIL_Check_BasetSet() == SUCCESS)
				return ERR_CANCEL;
			ucResult =SUCCESS;
			ucResult= MAG_GetTrackInfo((unsigned char*)pxMag);
			util_Printf("\nMAG_GetTrackInfo()------ucResult = %02x\n",ucResult);
			if (ucResult == ERR_SANDMAGCARD)
			    return(ucResult);
			if(!ucResult||ucResult==ERR_USEICCARDFIRST)
			{
				G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_SWIPECARD;
				G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
				//if(G_NORMALTRANS_ucFallBackFlag) ucResult=SUCCESS;
				ucReadMagErrFlag = false;
			}else
			{
				ucDispInfoFlag = true;
				ucReadMagErrFlag = true;
				continue;
			}
			util_Printf("ucM=[%02x]Result=[%02x]CardFlag=[%02x]\n",ucInputMode,ucResult,ucReadCardFlag);
			if(ucInputMode&TRANS_INPUTMODE_INSERTCARD
				&&ucResult==ERR_USEICCARDFIRST
				&& !ucReadCardFlag)
			{
				if((G_NORMALTRANS_ucTransType == TRANS_VOIDPREAUTH)
					||(G_NORMALTRANS_ucTransType == TRANS_PREAUTHSETTLE)
					||(G_NORMALTRANS_ucTransType == TRANS_PREAUTHFINISH))
				{
					ucCardSelect = CARDSPECIES_MAG;
					ucResult = SUCCESS;
				}
				else
				{
					ucCardSelect=MAG_ManuSelectCard(G_NORMALTRANS_ucTransType);
					util_Printf("Select =%02x\n",ucCardSelect);
					if(ucCardSelect==ERR_CANCEL)
					{
						ucResult=ERR_CANCEL;
					}
				}
			}else
			{
				if(!ucResult)
				{
					G_NORMALTRANS_ucInputMode = TRANS_INPUTMODE_SWIPECARD;
					G_NORMALTRANS_euCardSpecies = CARDSPECIES_MAG;
					ucCardSelect = CARDSPECIES_MAG;
					ucResult = SUCCESS;
				}
				else if (ucResult ==ERR_USEICCARDFIRST)
				{
					ucCardSelect=MAG_ManuSelectCard(G_NORMALTRANS_ucTransType);
					util_Printf("Select2009-10-20 11:26 =%02x\n",ucCardSelect);
					if(ucCardSelect == CARDSPECIES_MAG)
					{
						G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_SWIPECARD;
						G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
						ucCardSelect=CARDSPECIES_MAG;
						G_NORMALTRANS_ucFallBackFlag=0;
					}
					else if(ucCardSelect ==CARDSPECIES_EMV)
					{
						G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_SWIPECARD;
						G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
						ucCardSelect=CARDSPECIES_MAG;
						G_NORMALTRANS_ucFallBackFlag =2;
					}
				}
					if(ucCardSelect==ERR_CANCEL)
						ucResult=ERR_CANCEL;
					else ucResult = SUCCESS;

			}

			if(ucResult==ERR_USEICCARDFIRST
				&&ucCardSelect==CARDSPECIES_EMV
				&&(ucInputMode&TRANS_INPUTMODE_INSERTCARD))
			{
				ucResult = MASAPP_WaitReadIC(ucInputMode );
				if((ucResult== ERR_FALLBACK||ucResult== ERR_ICCARD) &&G_RUNDATA_ucReadCardFlag == EMVFIRSTMAG)
				{
					G_NORMALTRANS_ucInputMode = TRANS_INPUTMODE_SWIPECARD;
					G_NORMALTRANS_euCardSpecies = CARDSPECIES_MAG;
					ucCardSelect=CARDSPECIES_MAG;
					G_NORMALTRANS_ucFallBackFlag = 2;
					ucResult = SUCCESS;
				}else
				if(ucResult== ERR_ICCARD&&G_RUNDATA_ucReadCardFlag == EMVFIRSTMAG)
				{
					return ERR_ICCARD;
				}else
				{
					if(!ucResult)
						ucResult=MAG_UerICCardFirstProcess(ucCardSelect,
															G_NORMALTRANS_ucFallBackFlag,
															ucResult);
					if(!ucResult)
					{
						G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_INSERTCARD;
						G_NORMALTRANS_euCardSpecies=CARDSPECIES_EMV;
						G_NORMALTRANS_ucFallBackFlag =0;
					}
				}
			}

			util_Printf("ucResult = %02x\n",ucResult);
			util_Printf("ucCardSelect = %02x\n",ucCardSelect);
			util_Printf("G_NORMALTRANS_ucInputMode = %02x\n",G_NORMALTRANS_ucInputMode);
			if((ucResult == ERR_USEICCARDFIRST||ucResult ==SUCCESS)
				&&(ucCardSelect==CARDSPECIES_MAG)
				&&(G_NORMALTRANS_ucInputMode==TRANS_INPUTMODE_SWIPECARD)
			  )
				ucResult = MAG_DispCardNo();
			break;
		}
		else 
		//if(pxMifare->gen_status == DRV_ENDED)
		if(ProUiFace.UiToPro.CardInfo.ucInputMode == TRANS_INPUTMODE_SAND_PUTCARD)
		{
			util_Printf("\n4.MASAPP_Event_SAND  .................[pxMifare]\n");
		
			Os__abort_drv(drv_mmi);
			Os__abort_drv(drv_mag);
			Os__abort_drv(drv_icc);
			if(pxMifare->xxdata[0] ==0x00)
			{
				ucResult = Os__MIFARE_Active();
				G_NORMALTRANS_ucInputMode = TRANS_INPUTMODE_SAND_PUTCARD; //非接卡模式
	 			G_NORMALTRANS_euCardSpecies = CARDSPECIES_EMV;
				enTransType =QINQUIRY;

				if(!ucResult)
				{
					if (!ucResult)
					ucResult =QTransProcess(enTransType,G_NORMALTRANS_ulAmount,0);
				}
				else
					ucResult =0xED;
				
				util_Printf("\nQTransProcess=003=%d",ucResult);
			}
			else ucResult =0xED;
		    break;
		}
		else
		if((pxKey->gen_status == DRV_ENDED))
		{
			util_Printf("\n5.MASAPP_Event_SAND  .................");
		
			Os__abort_drv(drv_mag);
			Os__abort_drv(drv_icc);
			ucKey = pxKey->xxdata[1];
			util_Printf("\nenter get key=%02x",ucKey);
			if(ucKey>='0'&&ucKey<='9')
			{
				G_NORMALTRANS_ucInputMode = TRANS_INPUTMODE_MANUALINPUT;
				G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
				switch( G_NORMALTRANS_ucTransType )
				{
					case TRANS_PREAUTH:
					case TRANS_QUERYBAL:
					case TRANS_VOIDPREAUTH:
					case TRANS_VOIDPURCHASE:
					case TRANS_PREAUTHSETTLE:
					case TRANS_PREAUTHFINISH:
					case TRANS_VOIDPREAUTHFINISH:
					case TRANS_OFFPREAUTHFINISH:
					case TRANS_PREAUTHADD:
						return(SEL_GetCardMsg(ucKey));
					default:
						ucResult=ERR_CANCEL;
						break;
				}
			}
			else
			{
				if(ucReadMagErrFlag == false)
				{
					*ucOutKey = ucKey;
				}
				ucResult = ERR_CANCEL;
			}
			util_Printf("\nucResult =101=%d",ucResult);
			if (ucKey != KEY_CLEAR)
			    continue;
			else
			    break;
		}

		util_Printf("\n MAS_SAND_TEST RST:%02X",ucResult);
		
		break;
	}
	
	util_Printf("\n ucResult :%d\n",ucResult);

	return ucResult;
}

#endif
/***********************************************************************************************************
//为了处理非接卡传输过来的数据
***********************************************************************************************************/
unsigned char MASAPP_External_Event(DRV_OUT *aucEventOutData,unsigned char InputMode,unsigned char * ucOutKey)
{
	unsigned char ucCount=0;
	unsigned char ucKey;
	unsigned char ucResult;
	unsigned int  uiTimesOut;
	unsigned char ucDispInfoFlag=true;
	unsigned char ucICCardType,ucReadCardMode,ucInputMode,ucCardSelect;
	unsigned char ucReadMagErrFlag;
	DRV_OUT *pxMag;
	DRV_OUT *pxIcc;
	DRV_OUT *pxKey;
	UCHAR	ucLen1,ucLen2,aucResetBuf[50],ucResetBufLen;
	UCHAR	bFallBack;

NEW_DRV_TYPE  new_drv;
uchar ucDrv;


	UTIL_SetInputCardMode(&ucReadCardMode);//获取输入类型
	util_Printf("\nucReadCardMode11 =%x",ucReadCardMode);
	ucResult = MAG_EMVSetInputMode(&ucInputMode);
	if(ucResult != SUCCESS)
		return ucResult;
	util_Printf("\nG_NORMALTRANS_ucFallBackFlag==%x",G_NORMALTRANS_ucFallBackFlag);
	if(G_NORMALTRANS_ucFallBackFlag )
	{
		ucInputMode = TRANS_INPUTMODE_SWIPECARD;
		G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_SWIPECARD;
		if(G_EXTRATRANS_uiISO2Len)
		{
			G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
			return SUCCESS;
		}
	}
	bFallBack = FALSE;
	ucReadMagErrFlag = false;
	util_Printf("\nucInputMode=cbf=%02x",ucInputMode);
	util_Printf("\nInputMode=cbf=%02x",InputMode);
	if((ucInputMode==0x81)||(ucInputMode==0x84))
		MAG_DisplaySwipeCardMode(G_NORMALTRANS_ucTransType,ucInputMode);
	while(1)
	{
		ucResult = SUCCESS;
/*
		Os__abort_drv(drv_mmi);
		Os__abort_drv(drv_mag);
		Os__abort_drv(drv_icc);
		uiTimesOut = 45;
		uiTimesOut = uiTimesOut*100;
		SEL_AccumulatRunTime();
		pxKey = Os__get_key();
		if(ucCount ==0)
		{
			if(InputMode == 0x02)  //磁卡
			{
				pxMag =	aucEventOutData;
				pxMag->gen_status = DRV_ENDED;
				pxIcc->gen_status =DRV_STOPPED;
			}else if(InputMode == 0x01)  //IC卡
			{
				pxIcc = aucEventOutData;
				pxMag->gen_status = DRV_STOPPED;
				pxIcc->gen_status = DRV_ENDED;
			}
			ucCount++;
		}
		else                       //刷卡或插卡错要重新刷卡或插卡
		{
	 		if(ucInputMode&TRANS_INPUTMODE_SWIPECARD)
	 			pxMag = Os__mag_read();
	 		if(ucInputMode&TRANS_INPUTMODE_INSERTCARD)
	 			pxIcc = Os__ICC_insert();
 		}
		Os__timer_start(&uiTimesOut);
		while(uiTimesOut)
		{
			if((ucInputMode&TRANS_INPUTMODE_SWIPECARD)&&(pxMag->gen_status == DRV_ENDED))
				break;
			if(pxKey->gen_status == DRV_ENDED)
				break;
			if((ucInputMode&TRANS_INPUTMODE_INSERTCARD)&&(pxIcc->gen_status == DRV_ENDED))
				break;
			if(ucDispInfoFlag==false)
				DispDateAndTime(3,DISPCOLFIRST);
		}
		Os__timer_stop(&uiTimesOut);
		util_Printf("\nuiTimesOut  eee==%x",uiTimesOut);
		if(!uiTimesOut)
		{
			Os__abort_drv(drv_mmi);
			Os__abort_drv(drv_mag);
			Os__abort_drv(drv_icc);
			Os__light_off();
			Os__light_off_pp();
			return(ERR_CANCEL);
		}
		else
		{
			Os__light_on();
			Os__light_on_pp();
		}
*/

		ucDrv = KEY_DRV|MFR_DRV;
		if( ucInputMode&TRANS_INPUTMODE_SWIPECARD)
			ucDrv = ucDrv | MAG_DRV;
		if( ucInputMode&TRANS_INPUTMODE_INSERTCARD)
			ucDrv = ucDrv | ICC_DRV;

		util_Printf("\n1.MASAPP_External_Event Os_Wait_Event begin...");
		ucResult = Os_Wait_Event(ucDrv, &new_drv, 1000*30);
		//util_Printf("\n2. Os_Wait_Event:%02x",ret);
		//util_Printf("\n3. new_drv.drv_type:%02x",new_drv.drv_type);
		if(ucResult == 0)                                                              
		{                                                                         
			if(new_drv.drv_type == KEY_DRV)                                     
			{                                                                   
				if(new_drv.drv_data.gen_status== DRV_ENDED){
					pxKey = (DRV_OUT *)& new_drv.drv_data;
				}
			}                                                                   
			else if(new_drv.drv_type == ICC_DRV)                                
			{                                                                   
				if(new_drv.drv_data.gen_status== DRV_ENDED)
				{
					pxIcc = (DRV_OUT *)& new_drv.drv_data;
				}
			}                                                                   
			else if(new_drv.drv_type == MAG_DRV)                                
			{                                                                   
				if(new_drv.drv_data.gen_status== DRV_ENDED)
				{
					pxMag = (DRV_OUT *)& new_drv.drv_data;
				}
			}                                                                   			                                                    
			else if(new_drv.drv_type == DRV_TIMEOUT)                            
			{    
				if(!DataSave0.ConstantParamData.BackLightFlag)	Os__light_off();
				Os__light_off_pp();
				*ucOutKey = KEY_CLEAR;
				return(ERR_END);
			}	        
			Os__light_on();
			Os__light_on_pp();
			util_Printf("\n OWE_NewDrvtts End.....\n");
		}       

		if((ucInputMode&TRANS_INPUTMODE_INSERTCARD)&&(pxIcc->gen_status== DRV_ENDED))
		{
			Os__abort_drv(drv_mag);
			Os__abort_drv(drv_mmi);

			ucResult = UTIL_Check_BasetSet();
			util_Printf("\nUTIL_Check_BasetSet=abc==%d",ucResult);
			if(!ucResult)
				return ERR_CANCEL;
			ucResult =SUCCESS;
			ucICCardType =pxIcc->xxdata[1];
			util_Printf("ucICCardType1121 = %02x\n",ucICCardType);
			ucResetBufLen=sizeof(aucResetBuf);
			if(ucICCardType==0x39)
				ucICCardType=SMART_WarmReset(0,0,aucResetBuf,(unsigned short*)&ucResetBufLen);
			if(ucICCardType!=ICC_ASY) //不是异步卡(同步卡)
			{
				//ucReadCardFlag = true;
				ucInputMode&=~TRANS_INPUTMODE_INSERTCARD;
				G_NORMALTRANS_ucFallBackFlag =2;
				//continue;
			}
			else if(ucICCardType!=ICC_SYN)  //不是同步卡(异步卡)
			{
				G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_INSERTCARD;
				G_NORMALTRANS_euCardSpecies=CARDSPECIES_EMV;
				break;
			}
			else
			{
				//ucReadCardFlag = true;
				ucInputMode&=~TRANS_INPUTMODE_INSERTCARD;
				G_NORMALTRANS_ucFallBackFlag =2;
				continue;
				//return(ERR_ICCARD);
			}


			util_Printf("ucICCardType = %02x\n",ucICCardType);
			if(ucICCardType!=ICC_ASY)
			{
				while(1)
				{
			    		MAG_DisplaySwipeCardMode(G_NORMALTRANS_ucTransType,ucInputMode); //读IC卡错,请重新插卡

					ucDrv = KEY_DRV|MAG_DRV;
					util_Printf("\n2.MASAPP_External_Event Os_Wait_Event begin...");
					ucResult = Os_Wait_Event(ucDrv, &new_drv, 1000*30);
					//util_Printf("\n2. Os_Wait_Event:%02x",ret);
					//util_Printf("\n3. new_drv.drv_type:%02x",new_drv.drv_type);
					if(ucResult == 0)                                                              
					{                                                                         
						if(new_drv.drv_type == KEY_DRV)                                     
						{                                                                   
							if(new_drv.drv_data.gen_status== DRV_ENDED){
								pxKey = (DRV_OUT *)& new_drv.drv_data;
							}
						}                                                                   
						else if(new_drv.drv_type == MAG_DRV)                                
						{                                                                   
							if(new_drv.drv_data.gen_status== DRV_ENDED)
							{
								pxMag = (DRV_OUT *)& new_drv.drv_data;
							}
						}                                                                   			                                                    
						else if(new_drv.drv_type == DRV_TIMEOUT)                            
						{    
							ucResult=ERR_APP_TIMEOUT;
							break;
						}	        
					}       

/*
					pxMag = Os__mag_read();
					pxKey = Os__get_key();
					uiTimesOut = 30*100;
					Os__timer_start(&uiTimesOut);
					while(  (uiTimesOut)
					  	&&(pxMag->gen_status != DRV_ENDED)
					      &&(pxKey->gen_status != DRV_ENDED)
					     );
					Os__timer_stop(&uiTimesOut);
					Os__abort_drv(drv_mag);
					Os__abort_drv(drv_mmi);
*/
					

					if(!uiTimesOut)
					{
						ucResult=ERR_APP_TIMEOUT;
						break;
					}
					else if(pxKey->gen_status == DRV_ENDED)
					{
						ucKey = pxKey->xxdata[1];
						if(ucKey==KEY_CLEAR)
						{
							ucResult=ERR_CANCEL;
							break;
						}
					}
					else if(pxMag->gen_status==DRV_ENDED)
					{
						ucLen1 = pxMag->xxdata[0];
						ucLen2 = pxMag->xxdata[ucLen1+2];
						ucResult=MAG_GetTrackInfo((unsigned char*)pxMag);
                        			if (ucResult == ERR_SANDMAGCARD)
                        			    return(ucResult);
						if(!ucResult)
						{
							G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_SWIPECARD;
							G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
							G_NORMALTRANS_ucFallBackFlag=0;
						}
						else if(ucResult == ERR_USEICCARDFIRST)
						{
							ucCardSelect=MAG_ManuSelectCard(G_NORMALTRANS_ucTransType);

							if(ucCardSelect == CARDSPECIES_MAG)
							{
								G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_SWIPECARD;
								G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
								G_NORMALTRANS_ucFallBackFlag=0;
							}
							else if(ucCardSelect ==CARDSPECIES_EMV)
							{
								G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_SWIPECARD;
								G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
								G_NORMALTRANS_ucFallBackFlag =2;
							}
							if(ucCardSelect==ERR_CANCEL)
								ucResult=ERR_CANCEL;
							else ucResult = SUCCESS;
						}
						break;
					}
				}
			}
			else
			{
				G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_INSERTCARD;
				G_NORMALTRANS_euCardSpecies=CARDSPECIES_EMV;
				break;
			}
		}
		else if((ucInputMode&TRANS_INPUTMODE_SWIPECARD)&&(pxMag->gen_status == DRV_ENDED))
		{
			Os__abort_drv(drv_icc);
			Os__abort_drv(drv_mmi);

			ucResult = UTIL_Check_BasetSet();
			if(!ucResult)
				return ERR_CANCEL;
			ucResult =SUCCESS;
			ucResult= MAG_GetTrackInfo((unsigned char*)pxMag);
			util_Printf("\nMAG_GetTrackInfo()------ucResult = %02x\n",ucResult);
    			if (ucResult == ERR_SANDMAGCARD)
    			    return(ucResult);
			if(!ucResult||ucResult==ERR_USEICCARDFIRST)
			{
				G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_SWIPECARD;
				G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
				ucCardSelect=CARDSPECIES_MAG;
				if(G_NORMALTRANS_ucFallBackFlag) ucResult=SUCCESS;
				ucReadMagErrFlag = false;
			}
			else
			{
				ucDispInfoFlag = true;
				ucReadMagErrFlag = true;
				continue;
			}
			if(ucInputMode&TRANS_INPUTMODE_INSERTCARD&&ucResult==ERR_USEICCARDFIRST)
			{
				ucCardSelect=MAG_ManuSelectCard(G_NORMALTRANS_ucTransType);
				if(ucCardSelect==ERR_CANCEL)
					ucResult=ERR_CANCEL;
			}

			if(ucResult==ERR_USEICCARDFIRST
				&&ucCardSelect==CARDSPECIES_EMV
				&&(ucInputMode&TRANS_INPUTMODE_INSERTCARD))
			{
				ucResult = MASAPP_WaitReadIC(ucInputMode );
				if((ucResult==ERR_ICCARD||ucResult ==ERR_FALLBACK)&&G_RUNDATA_ucReadCardFlag == EMVFIRSTMAG)
				{
					G_NORMALTRANS_ucInputMode = TRANS_INPUTMODE_SWIPECARD;
					G_NORMALTRANS_euCardSpecies = CARDSPECIES_MAG;
					ucCardSelect=CARDSPECIES_MAG;
					G_NORMALTRANS_ucFallBackFlag = 2;
					ucResult = SUCCESS;
				}
				else
				{
					if(!ucResult)
						ucResult=MAG_UerICCardFirstProcess(ucCardSelect,G_NORMALTRANS_ucFallBackFlag,ucResult);
					if(!ucResult)
					{
						G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_INSERTCARD;
						G_NORMALTRANS_euCardSpecies=CARDSPECIES_EMV;
						G_NORMALTRANS_ucFallBackFlag = 0;
					}
				}
			}

			if((ucResult == ERR_USEICCARDFIRST||ucResult ==SUCCESS)
				&&(ucCardSelect==CARDSPECIES_MAG)
				&&(G_NORMALTRANS_ucInputMode==TRANS_INPUTMODE_SWIPECARD)
			  )
				ucResult = MAG_DispCardNo();
			break;
		}
		else if(!ucResult&&(pxKey->gen_status == DRV_ENDED))
		{
			Os__abort_drv(drv_mag);
			Os__abort_drv(drv_icc);
			ucKey = pxKey->xxdata[1];

			if(ucKey>='0'&&ucKey<='9')
			{
				G_NORMALTRANS_ucInputMode = TRANS_INPUTMODE_MANUALINPUT;
				G_NORMALTRANS_euCardSpecies=CARDSPECIES_MAG;
				switch( G_NORMALTRANS_ucTransType )
				{
					case TRANS_VOIDPURCHASE:
					case TRANS_REFUND:
					case TRANS_VOIDPREAUTHFINISH:
					case TRANS_VOIDPREAUTH:
						break;
					default:
						break;
				}
				return(SEL_GetCardMsg(ucKey));

			}
			else
			{
				if(ucReadMagErrFlag == false)
				{
					*ucOutKey = ucKey;
				}
				ucResult = ERR_CANCEL;
			}
		}
		break;
	}
	return ucResult;
}

unsigned char MASAPP_UnTouchCard_Event(void)
{
	unsigned char ucResult=SUCCESS;
	unsigned char ucLen,ucJ;
	unsigned char ucBuf[256],aucDate[50];
	USHORT uiLen;

	ucResult = UTIL_Check_BasetSet();
	if(!ucResult)
		return ERR_CANCEL;
	ucResult =SUCCESS;
	G_NORMALTRANS_ucISO2Len= 0;
	G_NORMALTRANS_ucISO3Len= 0;
	G_EXTRATRANS_uiISO2Len = 0;
	G_EXTRATRANS_uiISO3Len = 0;
	memset(G_EXTRATRANS_aucISO2,0,TRANS_ISO2LEN);
	memset(G_EXTRATRANS_aucISO3,0,TRANS_ISO3LEN);
	G_NORMALTRANS_ucSourceAccType = '0';

	ucLen=InterfaceTransData.ucTrack2Len*2;
	util_Printf("\nucLen=abc=%d",ucLen);
	if(ucLen == 0)
	{
		ucResult = ERR_ICCARD;
		return ucResult;
	}
	else
	{
		hex_asc(aucDate,InterfaceTransData.aucTrack2Data,ucLen);
// 		util_Printf("\nInterfaceTransData.aucTrack2Data转变aucDate = %s\n",aucDate);
		for(ucJ=0;ucJ<ucLen;ucJ++)
		{
			if((aucDate[ucJ] == 'f')||(aucDate[ucJ] == 'F'))
				break;
			if(aucDate[ucJ]=='D')
				aucDate[ucJ]='=';
		}

		G_EXTRATRANS_uiISO2Len =ucJ;
		G_NORMALTRANS_ucISO2Len =ucJ;
		memcpy(G_EXTRATRANS_aucISO2,aucDate,G_EXTRATRANS_uiISO2Len);
		asc_bcd(G_NORMALTRANS_aucISO2,G_NORMALTRANS_ucISO2Len/2,aucDate,G_NORMALTRANS_ucISO2Len);
		util_Printf("WangAn Test G_EXTRATRANS_uiISO2Len=%02x\n",G_EXTRATRANS_uiISO2Len);

		/**************取卡号后12位减一位******************/
		for(ucJ=0;ucJ<G_EXTRATRANS_uiISO2Len;ucJ++)
		{
			if((G_EXTRATRANS_aucISO2[ucJ] == '=')||(G_EXTRATRANS_aucISO2[ucJ] =='D')||(G_EXTRATRANS_aucISO2[ucJ] =='d'))//非接触卡返回不是=号是D形式
			{

				/*************取卡号***************/
				util_Printf("\nG_EXTRATRANS_aucISO2 = %s\n",G_EXTRATRANS_aucISO2);
				memcpy(G_NORMALTRANS_ucDispSourceAcc, G_EXTRATRANS_aucISO2, ucJ);
				util_Printf("G_NORMALTRANS_ucDispSourceAcc = %s\n",G_NORMALTRANS_ucDispSourceAcc);
				G_NORMALTRANS_ucDispSourceAccLen = ucJ;

				G_NORMALTRANS_ucSourceAccLen = G_NORMALTRANS_ucDispSourceAccLen;
				asc_hex(G_NORMALTRANS_aucSourceAcc,(G_NORMALTRANS_ucSourceAccLen+1)/2,
				G_NORMALTRANS_ucDispSourceAcc,((G_NORMALTRANS_ucSourceAccLen+1)/2)*2);

				if(ucJ>=13)
					asc_bcd(G_NORMALTRANS_aucCardPan,6,&G_EXTRATRANS_aucISO2[ucJ-13],12);
				else
					asc_bcd(G_NORMALTRANS_aucCardPan,6,&G_EXTRATRANS_aucISO2[0],12);
				break;
			}
		}
	}
	G_NORMALTRANS_ucInputMode = TRANS_INPUTMODE_HB_PUTCARD;//非接卡模式
 	G_NORMALTRANS_euCardSpecies = CARDSPECIES_EMV;

 	//以非接读卡器日期时间为准
	asc_bcd(G_NORMALTRANS_aucDate,4,InterfaceTransData.aucTransDate,8);
	asc_bcd(&G_NORMALTRANS_aucTime[0],3,InterfaceTransData.aucTransTime,6);

	//---------- AC --------------------------------------------------------------
	memset(ucBuf,  0, sizeof(ucBuf));
	ucResult=INTERFACE_GetTransTagValue((PUCHAR)"\x9F\x26",ucBuf,&uiLen);
	if((ucResult == SUCCESS)&&(uiLen))
		memcpy( G_NORMALTRANS_AppCrypt, ucBuf, 8);

	util_Printf("\nTransType  %d",G_NORMALTRANS_ucTransType);
	util_Printf("\nG_NORMALTRANS_euCardSpecies  %d",G_NORMALTRANS_euCardSpecies);

	if((G_NORMALTRANS_ucTransType == TRANS_PURCHASE)&&(G_NORMALTRANS_euCardSpecies==CARDSPECIES_EMV))
		if(InterfaceTransData.bForceOnline  ==0x00 )
		{
		    G_NORMALTRANS_ucTransType =TRANS_OFFPURCHASE;
		}

	//---------- AID --------------------------------------------------------------
	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	ucResult=INTERFACE_GetTransTagValue((PUCHAR)"\x4F",ucBuf,&uiLen);
	if(ucResult == SUCCESS)
	{
#if 1
 		util_Printf("\nAID TEST TAG 4F Len= %d\n",uiLen);
		for(ucJ=0;ucJ<uiLen;ucJ++)
		{
			if (ucJ%20==0)  util_Printf("\n");
			util_Printf("%02x ",ucBuf[ucJ]);
		}
#endif
		G_NORMALTRANS_ucAIDLen = uiLen;
		if(uiLen)
			memcpy( G_NORMALTRANS_aucAID, ucBuf, G_NORMALTRANS_ucAIDLen);
	       HBMasAPP_CheckAID();
	}
	ucResult=INTERFACE_GetTransTagValue((PUCHAR)"\x9F\x5D",ucBuf,&uiLen);
	util_Printf("\nucResult=9F5D==%d",ucResult);
	if(!ucResult)
	{
			G_NORMALTRANS_ucECTrans =TRANS_EC_PURCHASE;
	   	ECTransInfo.uiNBalance =CONV_BcdLong(ucBuf,12);
	   	memcpy((uchar *)&G_NORMALTRANS_PECTransInfo,(uchar *)&ECTransInfo,sizeof(ECTRANSINFO));
	}
	if((memcmp(G_NORMALTRANS_aucAID,"\xA0\x00\x00\x03\x33\x01\x01\x06",8)==0)&&InterfaceTransData.bForceOnline)
	{
		util_Printf("\nECTransInfo.uiBalance=21213=%d,G_NORMALTRANS_ulAmount=%d",ECTransInfo.uiBalance,G_NORMALTRANS_ulAmount);
		if(G_NORMALTRANS_ulAmount>ECTransInfo.uiNBalance)
			return ERR_BALANCE;//余额不足
		else
			return ERR_NOTPROC;//"交易类不支持
	}
	//---------- TVR ---------------------------------------------------------------
	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
  memcpy(ucBuf,InterfaceTransData.EMVTVR,INTERFACE_TVRLEN);
	memcpy( G_NORMALTRANS_TVR, ucBuf, INTERFACE_TVRLEN);
	//---------- TSI ---------------------------------------------------------------
	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	ucResult=INTERFACE_GetTransTagValue((PUCHAR)"\x9B",ucBuf,&uiLen);
	if((ucResult == SUCCESS)&&(uiLen))
		memcpy( G_NORMALTRANS_TSI, ucBuf, 5);
	//---------- ATC  --------------------------------------------------------------
	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	ucResult=INTERFACE_GetTransTagValue((PUCHAR)"\x9F\x36",ucBuf,&uiLen);
	if((ucResult == SUCCESS)&&(uiLen))
		memcpy( G_NORMALTRNAS_ATC, ucBuf, 2);

	//---------- CVM ---------------------------------------------------------------
	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	ucResult=INTERFACE_GetTransTagValue((PUCHAR)"\x9F\x34",ucBuf,&uiLen);
	if((ucResult == SUCCESS)&&(uiLen))
		memcpy( G_NORMALTRNAS_CVM, ucBuf, 2);
	//---------- App Label ---------------------------------------------------------
	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	ucResult=INTERFACE_GetTransTagValue((PUCHAR)"\x50",ucBuf,&uiLen);
	if(ucResult == SUCCESS)
	{
		G_NORMALTRANS_ucAppLabelLen = uiLen;
		if(uiLen) memcpy( G_NORMALTRANS_aucAppLabel, ucBuf, 16);
	}
	//---------- Application Prefer Name  ------------
	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	ucResult=INTERFACE_GetTransTagValue((PUCHAR)"\x9F\x11",ucBuf,&uiLen);
	util_Printf("\n9F11==cccd==%d",ucResult);
	//if((ucResult == SUCCESS)/*&&(ucBuf[0] == 0x01&&uiLen==1)*/)
	{

		uiLen = sizeof(ucBuf);
		memset( ucBuf,  0, sizeof(ucBuf));
		ucResult=INTERFACE_GetTransTagValue((PUCHAR)"\x9F\x12",ucBuf,&uiLen);
		util_Printf("\n9F12==cccd==%d,%d",ucResult,uiLen);
		if(ucResult == SUCCESS)
		{
			G_NORMALTRANS_ucAppPreferNameLen = uiLen;
			if(uiLen) memcpy( G_NORMALTRANS_aucAppPreferName, ucBuf, 16);
		}
	}
	memset( ucBuf,  0, sizeof(ucBuf));
	uiLen =sizeof(ucBuf);

	ucResult = INTERFACE_GetTransTagValue((PUCHAR)"\x5F\x34",ucBuf,&uiLen);
	if(ucResult == SUCCESS)
	{
		G_NORMALTRANS_ucPANSeq=ucBuf[0];
		util_Printf("\nG_NORMALTRANS_ucPANSeq**********=%02x",G_NORMALTRANS_ucPANSeq);
		G_NORMALTRANS_ucPANSeqExist=true;
	}

	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	ucResult = INTERFACE_GetTransTagValue((PUCHAR)"\x5F\x24",ucBuf,&uiLen);//有效期
	util_Printf("\n5F24====:%d",ucResult);
	if(!ucResult)
	{
		if(!memcmp(G_NORMALTRANS_ucExpiredDate,"\x00\x00\x00\x00",4))
			memcpy( G_NORMALTRANS_ucExpiredDate, ucBuf, 2);
	}
	TRANS_SetUntouchField55(G_NORMALTRANS_ucField55Data ,(ushort *)&G_NORMALTRANS_uiField55Len);

	util_Printf("\n\n *********************************************** ");
	memset( ucBuf,  0, sizeof(ucBuf));
	hex_asc(ucBuf,G_NORMALTRANS_AppCrypt,sizeof(G_NORMALTRANS_AppCrypt)*2);
	util_Printf("\n| 	TC:  %s ",ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	hex_asc(ucBuf,G_NORMALTRANS_aucAID,G_NORMALTRANS_ucAIDLen*2);
	util_Printf("\n| 	AID: %s",ucBuf);
	util_Printf("\n| 	TVR: %02X %02X %02X %02X %02X",NormalTransData.TVR[0],NormalTransData.TVR[1],
													NormalTransData.TVR[2],NormalTransData.TVR[3],
													NormalTransData.TVR[4]);
	util_Printf("\n| 	TSI: %02X %02X ",NormalTransData.TSI[0],NormalTransData.TSI[1]);
	util_Printf("\n| 	ATC: %02X %02X",NormalTransData.ATC[0],NormalTransData.ATC[1]);
	util_Printf("\n| 	CVM: %02X %02X %02X",NormalTransData.CVM[0],NormalTransData.CVM[1],NormalTransData.CVM[2]);
	util_Printf("\n| 	App Label: %s",G_NORMALTRANS_aucAppLabel);
	util_Printf("\n| 	App Prefer Name: %s",G_NORMALTRANS_aucAppPreferName);
	util_Printf("\n *********************************************** \n\n");
	if(DataSave0.ConstantParamData.uICCInfo=='1')
	{
		//----- LCD DISPLAY -----
		//Os__clr_display(255);
		OSMMI_DisplayASCII(0x30,0,0, (uchar *)"--- IC Trans Info -----");

		memset( ucBuf,  0, sizeof(ucBuf));
		memcpy( ucBuf, "AID:",4 );
		hex_asc(&ucBuf[4], G_NORMALTRANS_aucAID, G_NORMALTRANS_ucAIDLen*2);
		OSMMI_DisplayASCII(0x30,1,0, ucBuf);

		memset( ucBuf,  0, sizeof(ucBuf));
		memcpy( ucBuf, "TVR:",4 );
		hex_asc(&ucBuf[4], NormalTransData.TVR, 10);
		OSMMI_DisplayASCII(0x30,2,0, ucBuf);
		memset( ucBuf,  0, sizeof(ucBuf));
		memcpy( ucBuf, "TSI:",4 );
		hex_asc(&ucBuf[4], G_NORMALTRANS_TSI, 4);
		OSMMI_DisplayASCII(0x30,3,0, ucBuf);

		memset( ucBuf,  0, sizeof(ucBuf));
		memcpy( ucBuf, "CVM:",4 );
		hex_asc(&ucBuf[4], G_NORMALTRNAS_CVM, 6);
		OSMMI_DisplayASCII(0x30,4,0, ucBuf);

		memset( ucBuf,  0, sizeof(ucBuf));
		memcpy( ucBuf, "App Label:",9 );
		memcpy(&ucBuf[9] ,G_NORMALTRANS_aucAppLabel , 16);
		OSMMI_DisplayASCII(0x30,5,0, ucBuf);

		memset( ucBuf,  0, sizeof(ucBuf));
		memcpy( ucBuf, "App Prefer Name:",16 );
		OSMMI_DisplayASCII(0x30,6,0, (uchar *)"App Prefer Name:");
		memcpy(&ucBuf[0] ,G_NORMALTRANS_aucAppPreferName , 16);
		OSMMI_DisplayASCII(0x30,7,0, ucBuf);
	    MSG_WaitKey(30);
	}
	return SUCCESS;
}
unsigned char MASAPP_SwipeCard(void)
{
	unsigned char	ucResult,KEY,ucCardKey;
	unsigned int  uiTimesOut;
	unsigned char length1,length2,aucDis[30];

	DRV_OUT *pxMag;
	DRV_OUT *pxKey;
	union seve_in eve_in;

NEW_DRV_TYPE  new_drv;
uchar ucDrv;
	
	while(1)
	{
		//Os__clr_display(255);
		MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
		//Os__GB2312_display(2,0,(uchar *)"请刷卡(转出卡)");
		/*
		pxMag = Os__mag_read();
		pxKey = Os__get_key();
		uiTimesOut = 60*100;
		Os__timer_start(&uiTimesOut);
		while(  (uiTimesOut)
		      &&(pxMag->gen_status != DRV_ENDED)
		      &&(pxKey->gen_status != DRV_ENDED)
		     );
		Os__timer_stop(&uiTimesOut);
*/
		ucDrv = KEY_DRV|MAG_DRV;
		util_Printf("\n1.MASAPP_SwipeCard Os_Wait_Event begin...");
		ucResult = Os_Wait_Event(ucDrv, &new_drv, 1000*60);
		//util_Printf("\n2. Os_Wait_Event:%02x",ret);
		//util_Printf("\n3. new_drv.drv_type:%02x",new_drv.drv_type);
		if(ucResult == 0)                                                              
		{                                                                         
			if(new_drv.drv_type == KEY_DRV)                                     
			{                                                                   
				if(new_drv.drv_data.gen_status== DRV_ENDED){
					pxKey = (DRV_OUT *)& new_drv.drv_data;
				}
			}                                                                   
			else if(new_drv.drv_type == MAG_DRV)                                
			{                                                                   
				if(new_drv.drv_data.gen_status== DRV_ENDED)
				{
					pxMag = (DRV_OUT *)& new_drv.drv_data;
				}
			}                                                                   			                                                    
			else if(new_drv.drv_type == DRV_TIMEOUT)                            
			{    
				uiTimesOut=0;
			}	        
		}       


		
		if(!uiTimesOut)
		{
			return(ERR_CANCEL);
		}else
		{
			if( pxMag->gen_status == DRV_ENDED)
			{
				Os__abort_drv(drv_mmi);
				length1 = pxMag->xxdata[0];
				length2 = pxMag->xxdata[length1+2];
				if((length1==0)&&(length2==0))
				{
				}else
				{
					Os__light_on();
					memcpy(eve_in.e_mag_card.mag_card,pxMag,length1+length2+7);
					if( MASAPP_GetTrackInfo(&(eve_in.e_mag_card.mag_card[0])) != SUCCESS)
					{
						return(ERR_NOTPROC);
					}
					//显示卡号
					while(1)
					{
						memset(aucDis,0,sizeof(aucDis));
						memcpy(aucDis,G_NORMALTRANS_aucUnAssignAccountISO2,G_NORMALTRANS_ucMainAccountLen);
						Os__light_on();
						//Os__clr_display(255);
						//Os__GB2312_display(0, 0, (unsigned char * )"卡号:");
						Os__display(1, 0, aucDis);
						Os__display(2, 0, &aucDis[16]);
						//Os__GB2312_display(3, 0, (unsigned char * )"按[确认]键继续");

						ucCardKey = UTIL_GetKey(30);
						if(ucCardKey==KEY_CLEAR)
							return ERR_CANCEL;
						else
							return SUCCESS;
						util_Printf("\nucCardKey=%02x\n",ucCardKey);
						switch(ucCardKey)
						{
							case KEY_ENTER:
								return SUCCESS;
								break;
							case 99:
							case KEY_CLEAR:
								ucResult = ERR_CANCEL;
								break;
							default:
								continue;
						}
						break;
					}
				}
			}
			if( pxKey->gen_status == DRV_ENDED)
			{
				Os__abort_drv(drv_mag);
				Os__light_on();
				KEY = pxKey->xxdata[1];
				if(KEY==KEY_CLEAR)
					return(ERR_CANCEL);
			}
		}
	}
	return(SUCCESS);
}
unsigned char MASAPP_GetTrackInfo(unsigned char *pucTrack)
{
	unsigned short uiI,uiISO2Len,uiISO3Len;
	unsigned char  *pucPtr,ucJ;
	unsigned char ucISO2Status,ucISO3Status;
	util_Printf("\nMASAPP_GetTrackInfo===abc");
	pucPtr = pucTrack;
	pucPtr += 2;
	/* Track2 information */
	ucISO2Status = *(pucPtr);
	pucPtr ++;
	uiISO2Len = *(pucPtr);
	util_Printf("\nucISO2Status=%02x",ucISO2Status);
	util_Printf("\nuiISO2Len=%d\n",uiISO2Len);
	/***判断二磁道数据是否正确***/
	if(ucISO2Status != SUCCESS_TRACKDATA)
			return ERR_MAG_TRACKDATA;
	if(uiISO2Len > TRANS_ISO2LEN)
		return(ERR_MAG_TRACKDATA);
	pucPtr ++;
	for( uiI=0; uiI<uiISO2Len; uiI++,pucPtr ++)
	{
   		 G_NORMALTRANS_aucUnAssignAccountISO2[uiI] =  *(pucPtr) | 0x30;
	}
	G_NORMALTRANS_ucUAAISOLen2 =uiISO2Len;
	util_Printf("\nG_NORMALTRANS_aucUnAssignAccountISO2:%s",G_NORMALTRANS_aucUnAssignAccountISO2);
  /* Track3 information */
  	ucISO3Status = *(pucPtr);
	pucPtr ++;
	uiISO3Len = *(pucPtr);

	pucPtr ++;
	util_Printf("\nucISO3Status=%02x",ucISO3Status);
	util_Printf("\nuiISO3Len=%d\n",uiISO3Len);

	for( uiI=0; uiI<uiISO3Len; uiI++,pucPtr ++)
	{
    		G_NORMALTRANS_aucUnAssignAccountISO3[uiI] =  *(pucPtr) | 0x30;
  	}
	G_NORMALTRANS_ucUAAISOLen3 =uiISO3Len;
	util_Printf("\aG_NORMALTRANS_aucUnAssignAccountISO2:%s",G_NORMALTRANS_aucUnAssignAccountISO2);


	/***过滤三磁道状态不正确和磁道数据超长***/
	if((ucISO3Status != SUCCESS_TRACKDATA)
		||(uiISO3Len > TRANS_ISO3LEN))
	{
		G_NORMALTRANS_ucUAAISOLen3 = 0;
		util_Printf("\nG_NORMALTRANS_ucUAAISOLen3=%d",G_NORMALTRANS_ucUAAISOLen3);
	}
	for(ucJ=0;ucJ<G_NORMALTRANS_ucUAAISOLen2;ucJ++)
	{
		if(G_NORMALTRANS_aucUnAssignAccountISO2[ucJ] == '=')
		{
			/*************取卡号***************/
			G_NORMALTRANS_ucMainAccountLen = ucJ;
			if(ucJ>=13)
				asc_bcd(G_NORMALTRANS_aucCardPan_UnAssign,6,&G_NORMALTRANS_aucUnAssignAccountISO2[ucJ-13],12);
			else
				asc_bcd(G_NORMALTRANS_aucCardPan_UnAssign,6,&G_NORMALTRANS_aucUnAssignAccountISO2[0],12);
			break;
		}
	}
	return(SUCCESS);
}
#define		MAXDISPROWS		8

UCHAR	CardHolderConfirmApp(PCANDIDATELIST pCandidateList,PUCHAR pucAppIndex)
{
	UCHAR		ucI,ucCurPage,ucBeginIndex;
	UCHAR		aucBuff[30],ucKey;
	PCANDIDATEAPP	pCandidateApp;


	ucCurPage =0;
/*
	bDispFlag = TRUE;
	for(ucI=0;ucI<pCandidateList->CandidateNums;ucI++)
	{
		pCandidateApp =&pCandidateList->CandidateApp[ucI];
		if(pCandidateApp->ucADFNameLen*2 > MAXDISPCHAR-2)
		{
			bDispFlag = FALSE;
			break;
		}
	}
*/
	do
	{
		ucBeginIndex =ucCurPage*MAXDISPROWS;
		//Os__clr_display(255);
		for(ucI =0; ucI <MAXDISPROWS; ucI++)
		{
			if(ucI+ucBeginIndex >= pCandidateList->CandidateNums) break;
			pCandidateApp =&pCandidateList->CandidateApp[ucI+ucBeginIndex];
			memset(aucBuff,0x00,sizeof(aucBuff));
			aucBuff[0] = ucI+ucBeginIndex+0x31;
			aucBuff[1] = '.';
			if(pCandidateApp->bICTIExist
			   &&(pCandidateApp->ucICTI ==0x01)
			   &&pCandidateApp->ucAppPreferNameLen)
				memcpy(&aucBuff[2],pCandidateApp->aucAppPreferName,pCandidateApp->ucAppPreferNameLen);
			else if(pCandidateApp->ucAppLabelLen)
				memcpy(&aucBuff[2],pCandidateApp->aucAppLabel,pCandidateApp->ucAppLabelLen);
			else if(pCandidateApp->ucADFNameLen)
				CONV_HexAsc(&aucBuff[2],pCandidateApp->aucADFName,pCandidateApp->ucADFNameLen*2);

//			if(bDispFlag)
//				//Os__GB2312_display(ucI,0,aucBuff);
//			else
				OSMMI_DisplayASCII(0x30,ucI,0,aucBuff);

		}

		ucKey = Os__xget_key();
		if(ucKey ==KEY_ENTER)
		{
			*pucAppIndex =0;
			return EMVERROR_SUCCESS;
		}
		else if(ucKey == KEY_CLEAR)
			return EMVERROR_CANCEL;
		else if((ucKey>'0')&&(ucKey < pCandidateList->CandidateNums +'1'))
		{
			*pucAppIndex =ucKey -0x31;
			return EMVERROR_SUCCESS;
		}

		if(ucKey == KEY_F3 &&ucCurPage)
			ucCurPage --;
		if(ucKey == KEY_F4 && (ucCurPage+1)*MAXDISPROWS<pCandidateList->CandidateNums)
			ucCurPage++;
	}while(1);
}
#if SANDREADER
UCHAR	QCardHolderConfirmApp(PQCANDIDATELIST pCandidateList,PUCHAR pucAppIndex)
{
	UCHAR		ucI,ucCurPage,ucBeginIndex;
	UCHAR		aucBuff[30],ucKey;
	PQCANDIDATEAPP	pCandidateApp;


	ucCurPage =0;
/*
	bDispFlag = TRUE;
	for(ucI=0;ucI<pCandidateList->CandidateNums;ucI++)
	{
		pCandidateApp =&pCandidateList->CandidateApp[ucI];
		if(pCandidateApp->ucADFNameLen*2 > MAXDISPCHAR-2)
		{
			bDispFlag = FALSE;
			break;
		}
	}
*/
	do
	{
		ucBeginIndex =ucCurPage*MAXDISPROWS;
		//Os__clr_display(255);
		for(ucI =0; ucI <MAXDISPROWS; ucI++)
		{
			if(ucI+ucBeginIndex >= pCandidateList->CandidateNums) break;
			pCandidateApp =&pCandidateList->CandidateApp[ucI+ucBeginIndex];
			memset(aucBuff,0x00,sizeof(aucBuff));
			aucBuff[0] = ucI+ucBeginIndex+0x31;
			aucBuff[1] = '.';
			if(pCandidateApp->bICTIExist
			   &&(pCandidateApp->ucICTI ==0x01)
			   &&pCandidateApp->ucAppPreferNameLen)
				memcpy(&aucBuff[2],pCandidateApp->aucAppPreferName,pCandidateApp->ucAppPreferNameLen);
			else if(pCandidateApp->ucAppLabelLen)
				memcpy(&aucBuff[2],pCandidateApp->aucAppLabel,pCandidateApp->ucAppLabelLen);
			else if(pCandidateApp->ucADFNameLen)
				CONV_HexAsc(&aucBuff[2],pCandidateApp->aucADFName,pCandidateApp->ucADFNameLen*2);

//			if(bDispFlag)
//				//Os__GB2312_display(ucI,0,aucBuff);
//			else
				OSMMI_DisplayASCII(0x30,ucI,0,aucBuff);

		}

		ucKey = Os__xget_key();
		if(ucKey ==KEY_ENTER)
		{
			*pucAppIndex =0;
			return EMVERROR_SUCCESS;
		}
		else if(ucKey == KEY_CLEAR)
			return EMVERROR_CANCEL;
		else if((ucKey>'0')&&(ucKey < pCandidateList->CandidateNums +'1'))
		{
			*pucAppIndex =ucKey -0x31;
			return EMVERROR_SUCCESS;
		}

		if(ucKey == KEY_F3 &&ucCurPage)
			ucCurPage --;
		if(ucKey == KEY_F4 && (ucCurPage+1)*MAXDISPROWS<pCandidateList->CandidateNums)
			ucCurPage++;
	}while(1);
}
#endif

UCHAR	EMVTransProcess(TRANSTYPE enTransType,UINT uiAmount,UINT uiOtherAmount)
{
	UCHAR	ucResult,ucAPPResult=SUCCESS,keyflag,ucECResult,ucLen;
	CRYPTTYPE	CryptType;
	UINT		uiLogSumAmount;
	UCHAR	bConfirm,bConnectSuccess;
	UCHAR	ucAppIndex,ucI;
	CANDIDATELIST	CandidateList;
	UCHAR	aucRecvBuff[300],aucBuff[60],ucKey,ucRstBuf[2],ucFileResult;
	USHORT  uiRecvLen;
	UCHAR ucHostResult,ucResultPIN=SUCCESS,ucPindata[30],ucPinlen;
	CONFIG_PARAM	cp;


	ucResult = EMVERR_SUCCESS;
	ucAPPResult = EMVERR_SUCCESS;
	ucHostResult = EMVERR_SUCCESS ;
	MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
	//Os__GB2312_display(2,1,"数据处理中... ");

	memcpy(&EMVConfig,&TermConfigDefault,sizeof(TERMCONFIG));

	EMVConfig.ucTermType = 0x22;
	EMVConfig.ucCAPKNums =ucTermCAPKNum;
	EMVConfig.pTermSupportCAPK =TermCAPK;
	EMVConfig.ucTermSupportAppNum = ucTermAIDNum;
	EMVConfig.pTermSupportApp = TermAID;
	EMVConfig.ucIPKRevokeNum =ucIPKRevokeNum;
	EMVConfig.pIPKRevoke =IPKRevoke;
	EMVConfig.ucExceptFileNum = ucExceptFileNum;
	EMVConfig.pExceptPAN = ExceptFile;
	EMVConfig.pfnEMVICC = EMVICC;
	EMVConfig.pfnOfflinePIN = UTIL_OfflinePIN;
	EMVConfig.pfnOnlinePIN = UTIL_InputEncryptPIN;//UTIL_OnlinePIN;
	EMVConfig.pfnEMVInformMsg =UTIL_EMVInformMsg;
	EMVConfig.pfnEMVIDCardVerify =UTIL_IDCardVerify;
	EMVConfig.bECSupport =DataSave0.ChangeParamData.bECSupport;

	memset(&cp,0x00,sizeof(CONFIG_PARAM));
	cp =OSCFG_ReadParam();
	ucLen =strlen((char*)cp.aucSerialNo);
	if(ucLen>16) ucLen =16;
	util_Printf("\nPOS序列号Len[%d],值:%02x %02x %02x %02x %02x %02x %02x %02x\n",ucLen,cp.aucSerialNo[0],cp.aucSerialNo[1],cp.aucSerialNo[2],
			cp.aucSerialNo[3],cp.aucSerialNo[4],cp.aucSerialNo[5],cp.aucSerialNo[6],cp.aucSerialNo[7]);
	if(ucLen>=MAXIFDLEN)
	{
		memcpy(EMVConfig.aucIFD,&cp.aucSerialNo[ucLen-MAXIFDLEN],MAXIFDLEN);
	}else if (!ucLen)
		memset(EMVConfig.aucIFD,0x30,sizeof(EMVConfig.aucIFD));
	else
		memcpy(EMVConfig.aucIFD,cp.aucSerialNo,ucLen);
	for(ucI=0;ucI<8;ucI++)
	{
		if (EMVConfig.aucIFD[ucI]<0x30 || EMVConfig.aucIFD[ucI] >0x39)
		{
			memset(EMVConfig.aucIFD,0x30,sizeof(EMVConfig.aucIFD));
			break;
		}
	}
       util_Printf("\nPOS序列号:%s",EMVConfig.aucIFD);
	util_Printf("\nEMVConfig.ucTermType = %02x\n",EMVConfig.ucTermType);
	util_Printf("\nEMVConfig.bPBOC20 = %02x\n",EMVConfig.bPBOC20);
	util_Printf("\nEMVConfig.uiFloorLimit = %d\n",EMVConfig.uiFloorLimit);
	util_Printf("\nEMVConfig.bECSupport= %d\n",EMVConfig.bECSupport);
	//EMV 内核配置初始化
	EMVDATA_TermInit(&EMVConfig);
	//建立终端与卡片共同支持的应用.
	ucResult = APPSELECT_CreateAppCandidate(&CandidateList);
	util_Printf("---0100----ucResult=%02x\n",ucResult);
	bConfirm =FALSE;
	if(CandidateList.CandidateNums ==1 &&CandidateList.CandidateApp[0].bAppPriorityExist)
	{
		if(CandidateList.CandidateApp[0].ucAppPriorityIndicator &APPNEEDCONFIRM)
			bConfirm = TRUE;
	}
	else
	if (CandidateList.CandidateNums >1)
		bConfirm = TRUE;
	util_Printf("\nucAPPREsult1212 =%d",ucAPPResult);
	while(!ucResult && CandidateList.CandidateNums)
	{
		util_Printf("CandidateList.CandidateNums = %02X\n",CandidateList.CandidateNums);
		ucAppIndex = 0;

		if(bConfirm)
		{
			ucResult = CardHolderConfirmApp(&CandidateList,&ucAppIndex);
			util_Printf("---0101----ucResult=%02x\n",ucResult);
			util_Printf("\nucAPPREsult0101 =%d",ucAPPResult);
		}
		if(!ucResult)
		{
			//选择应用来完成交易
			ucResult = APPSELECT_FinalSelect(ucAppIndex,&CandidateList);
			util_Printf("---0102----ucResult=%02x\n",ucResult);
			util_Printf("\nucAPPREsult0102 =%d",ucAPPResult);
			//ucAPPResult =SUCCESS;
			if(ucResult == EMVERROR_RESELECT)
			{
				MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
				//Os__GB2312_display(2,1,(PUCHAR)"请再试一次！");
				Os__xget_key();
				ucResult =EMVERROR_SUCCESS;
				continue;
			}
		}
		if(!ucResult)
		{
			util_Printf("\nucAPPREsult0103 =%d",ucAPPResult);
			UTIL_SavEMVTransData();//设置交易信息TransReqInfo
			util_Printf("\nucAPPREsult0104 =%d",ucAPPResult);
			//交易日期、时间、随机数、交易类型(GOODS)、金额、终端、商户、商户名称、销售点输入方式(0x02)、是否强制联机
			//UTIL_IncreaseTraceNumber();
			EMVDATA_TransInit(&TransReqInfo);//交易前把交易相关信息发给EMV内核.
			util_Printf("\nucAPPREsult0105 =%d",ucAPPResult);
			memset(aucBuff,0,sizeof(aucBuff));
			switch(G_NORMALTRANS_ucTransType)
			{
				case TRANS_QUERYBAL:
					aucBuff[0] = 0x31;
					break;
				case TRANS_PURCHASE:
					aucBuff[0] = 0x00;
					break;
				case TRANS_PREAUTH:
					aucBuff[0] = 0x03;
					break;
				case TRANS_EC_ASSIGNLOAD:
					aucBuff[0] =0x60;
					break;
				case TRANS_EC_UNASSIGNLOAD:
					aucBuff[0] =0x62;
					break;
				case TRANS_EC_CASHLOAD:
					aucBuff[0] =0x63;
					break;
				default:
					break;
			}
			EMVTRANSTAG_SetTag(ALLPHASETAG,(PUCHAR)"\x9C",aucBuff ,1 );
		}
#ifndef EMVTEST
		if(!ucResult)
		{
			//找到匹配的应用，设置应用参数
			ucResult = EMV_SetEMVConfig();
			if(ucResult != SUCCESS) return ucResult;
			//EMVDATA_TermInit_Again(&EMVConfig);
		}
#endif
		util_Printf("\nucAPPREsult999 =%d",ucAPPResult);
		switch(G_NORMALTRANS_ucTransType)
		{
			//case TRANS_PURCHASE:
			case TRANS_PREAUTH:
			case TRANS_QUERYBAL:
			case TRANS_EC_ASSIGNLOAD:
				if((!ucResult)&&(memcmp(G_RUNDATA_aucAID,"\xA0\x00\x00\x03\x33\x01\x01\06" ,8)==0))
					return ERR_NOTPROC;//"交易类不支持
				break;
			default:
				break;

		}
		//2009-9-4 14:02让客户选择电子现金还是EMV交易
		if((!ucResult)&&(memcmp(G_RUNDATA_aucAID,"\xA0\x00\x00\x03\x33" ,5)==0))
		{
			if(G_NORMALTRANS_ucTransType ==TRANS_PURCHASE)
			{
				ucECResult = EMVDATA_CheckCardSupportEC();
				util_Printf("\n EMVDATA_CheckCardSupportEC==%d",ucECResult);
				if(ucECResult == 1)
				{
#ifdef GUI_PROJECT
#ifdef GUI_TTS_DATA
					ucKey = 0x30;
#else
					if(ProUiFace.UiToPro.ucEPTrans)
						ucKey = 0x31;
					else
						ucKey = 0x30;
#endif

#else
					MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
					//Os__GB2312_display(1,0,(PUCHAR)"该卡具备电子现金");
					//Os__GB2312_display(2,0,(PUCHAR)"功能,是否使用?");
					//Os__GB2312_display(3,0,(PUCHAR)"1.是       0:否");
					do
					{
						ucKey=Os__xget_key();
						if(ucKey=='0'||ucKey=='1'||ucKey==KEY_CLEAR)
							break;
					}while(1);
#endif
					switch(ucKey)
					{
						case '0':
							EMVDATA_TerminalSupportEC(FALSE);
							break;
						case '1':
							EMVDATA_TerminalSupportEC(TRUE);
								if((!ucResult)&&(memcmp(G_RUNDATA_aucAID,"\xA0\x00\x00\x03\x33\x01\x01\06",8)==0))
								{
									ucResult =EMVICC_GetData((PUCHAR)"\x9F\x79",2);
									if(ucResult)
										break;
									ucResult = EMVICC_CommandResult();
									if(ucResult)
										break;

									uiAmount =CONV_BcdLong(EMVIccOut.aucDataOut,EMVIccOut.uiLenOut*2);
									util_Printf("\nECTransInfo.uiBalance=11232=%d",uiAmount);
									if(G_NORMALTRANS_ulAmount>uiAmount)
										return ERR_BALANCE;//余额不足
								}

							break;
						default:
						ucResult =ERR_CANCEL;
							break;
					}
				}
			}
			else if((G_NORMALTRANS_ucTransType==TRANS_EC_ASSIGNLOAD
				     ||G_NORMALTRANS_ucTransType==TRANS_EC_UNASSIGNLOAD
				     ||G_NORMALTRANS_ucTransType==TRANS_EC_CASHLOAD))
			{
				ucECResult =EMVDATA_CheckCardSupportEC();
				util_Printf("\nEMVDATA_CheckCardSupportEC12==%d",ucECResult);
				if(ucECResult==1)
				{
					EMVDATA_TerminalSupportEC(TRUE);
					ucResult =EMVICC_GetData((PUCHAR)"\x9F\x79",2);
					if(ucResult)
						ucResult = EMVICC_CommandResult();
					util_Printf("\nEMVDATA_CheckCardSupportEC13==%d",ucResult);
					if(!ucResult)
					{
						ECTransInfo.uiBalance =CONV_BcdLong(EMVIccOut.aucDataOut,EMVIccOut.uiLenOut*2);
					}
				}
			}
		}
		util_Printf("\nucAPPREsult888 =%d",ucAPPResult);
		if(!ucResult)
		{
			MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
			//Os__GB2312_display(2,1,(PUCHAR)"数据处理中... ");

			//初始化应用.
			ucResult = INTIAPP_GPO();
			util_Printf("---0103----ucResult=%02x\n",ucResult);
			if(ucResult == EMVERROR_RESELECT)
			{
				//从内核中获取终端与卡片共同支持的应用信息
				ucResult = APPSELECT_GetAppCandidate(&CandidateList);
				util_Printf("---0104----ucResult=%02x\n",ucResult);
				MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
				//Os__GB2312_display(2,1,(PUCHAR)"请再试一次！");
				Os__xget_key();
				continue;
			}
			else
				break;
		}
	}

	//读应用数据过程.
	if(!ucResult) ucResult = READAPP_ReadData();
	util_Printf("---0105----ucResult=%02x\n",ucResult);
	if(!ucResult)
	{
		uiLogSumAmount = CalcLogTransAmount();
	}
	util_Printf("---0106----uiLogSumAmount=%02x\n",uiLogSumAmount);

	//脱机数据认证过程.
	if(!ucResult) ucResult = DATAAUTH_Process();
	util_Printf("---0107----ucResult=%02x\n",ucResult);

	//处理限制过程.
	if(!ucResult) ucResult = PROCESSRESTRICT_Process();
	util_Printf("---0108----ucResult=%02x\n",ucResult);
	util_Printf("\nEMVConfig.ucTermType 2111= %02x\n",EMVConfig.ucTermType);

	if(!ucResult) MasApp_SetEMVTransInfo();//从内核获取交易信息:主账号、卡序列号、2磁道等同数据
	if(!ucResult && G_NORMALTRANS_ucTransType==TRANS_EC_ASSIGNLOAD)
	{
		uchar ucPindata[20],ucPinlen;
		ucResult = UTIL_InputEncryptPIN(ucPindata ,&ucPinlen);
		if(ucResult==EMVERROR_BYPASS)
				ucResult =SUCCESS;
	}
	//持卡人验证过程
	util_Printf("\nEMVConfig.ucTermType 222= %02x\n",EMVConfig.ucTermType);
	if(!ucResult) ucResult = EMVVERIFY_Process();
	util_Printf("---0109----ucResult=%02x\n",ucResult);
	if(ucResult == ERR_END)
		return ucResult;
	util_Printf("---0110----ucResult=%02x\n",ucResult);
	if(!ucResult)
	{
		//终端风险管理过程
		ucResult = TERMRISKMANAGE_Process(uiLogSumAmount);
		util_Printf("---0111----ucResult=%02x\n",ucResult);
	}
	util_Printf("\nucAPPREsult777 =%d",ucAPPResult);
	if(!ucResult)
	{
		//终端行为分析过程
		CryptType = TERMACTANALYSIS_Process(true);
		util_Printf("---0112----CryptType=%02x\n",CryptType);

		//把终端分析交易结果发给卡片,卡片做行为分析
		ucResult = APPCRYPT_OfflineProcess(CryptType);
		util_Printf("APPCRYPT_OfflineProcess()_______ucResult = %02x\n",ucResult);
		MasApp_SetEMVTransInfo();//从内核获取交易信息:主账号、卡序列号、2磁道等同数据
		//ucResult = EMVERROR_NEEDARQC;
		util_Printf("\nucAPPREsult666 =%d",ucAPPResult);
		if(ucResult ==EMVERROR_NEEDARQC)
		{
			//COMMS_PreComm();
			util_Printf("APPCRYPT_OfflineProcess()____2___ucResult = %02x\n",ucResult);
			G_NORMALTRANS_ucFallBackFlag =0;//Add 08-04-02
			memset(aucRecvBuff,0x00,sizeof(aucRecvBuff));
			uiRecvLen = sizeof(aucRecvBuff);

			if(G_NORMALTRANS_ucTransType==TRANS_PURCHASE)
			 {
			 	util_Printf("\nEMVTransInfo.bECTrans:%d",EMVTransInfo.bECTrans);
				util_Printf("\nECTransInfo.uiBalance=21212=%d,G_NORMALTRANS_ulAmount=%d",ECTransInfo.uiBalance,G_NORMALTRANS_ulAmount);
				if((memcmp(G_RUNDATA_aucAID,"\xA0\x00\x00\x03\x33\x01\x01\06",8)==0))
				{
					EMVTRANSTAGDATA   *pIAD;
					pIAD=EMVTRANSTAG_SearchTag(FIRSTGACPHASETAG,(PUCHAR)"\x9F\x10");
					if(pIAD&&(pIAD->uiLen>15))
					{
					   ECTransInfo.uiNBalance =CONV_BcdLong(pIAD->pTagValue+10,10);
					}
					util_Printf("\nECTransInfo.uiBalance=21213=%d,G_NORMALTRANS_ulAmount=%d",ECTransInfo.uiBalance,G_NORMALTRANS_ulAmount);
					if(G_NORMALTRANS_ulAmount>ECTransInfo.uiBalance)
						return ERR_BALANCE;//余额不足
					else
						return ERR_NOTPROC;//"交易类不支持
				}
			}
			if((G_NORMALTRANS_ucTransType==TRANS_QUERYBAL)&& G_RUNDATA_ucQInputPinFlag!=1)
				ucResultPIN= UTIL_InputEncryptPIN(ucPindata ,&ucPinlen);
			if(!ucResultPIN)
			    ucResult = TRANS_OnlineProcess();
			else
			    return(ucResultPIN);
			util_Printf("TRANS_OnlineProcess() ---- ucResult = %02x\n",ucResult);
			if((ucResult==ERR_DIAL)
				||ucResult==DIAL_BUSY
				||ucResult==DIAL_NO_TONE
				||ucResult==DIAL_NO_CARRIER
				||ucResult==DIAL_LINE_BUSY
				||ucResult==ERR_CANCEL
			  )
			{
				bConnectSuccess = FALSE;
			}
			else
			{
				bConnectSuccess = TRUE;
				memset(ucRstBuf ,0,sizeof(ucRstBuf));
				if(ucResult == ERR_HOSTCODE )
				{
					char_asc(ucRstBuf, 2, &G_RUNDATA_ucErrorExtCode);
				}
				else
				{
					char_asc(ucRstBuf, 2, &ucResult);
				}
				if(ucResult)
				{
					ucHostResult =ucResult ;
				}

				EMVTRANSTAG_SetTag(ALLPHASETAG,(uchar *)"\x8A",ucRstBuf,2);//授权应答码

				ucResult=ISO8583_GetBitValue(55,aucRecvBuff,(short*)&uiRecvLen,sizeof(aucRecvBuff));
				if(ucResult ==EMVERR_SUCCESS)
				{
					// 联机请求后,把发卡行IC卡信息发给卡片,完成发卡行认证与脚本处理
					ucResult = APPCRYPT_OnlineRespDataProcess(ucRstBuf,&aucRecvBuff[0],uiRecvLen);
				}
			}

			ucResult = APPCRYPT_OnlineProcess(bConnectSuccess);
			if(EMVTransInfo.ucScriptResultNum)
			{
				Trans_EMVProcessScript();
			}
		}
	}
	util_Printf("\nucAPPREsult333 =%d",ucAPPResult);
	util_Printf("\n EMV return %02x",ucResult);
	util_Printf("\n bConnectSuccess = %02x\n",bConnectSuccess);
	util_Printf("\nTSI:%02x  TVR:%02x",EMVTransInfo.EMVTSI[0],EMVTransInfo.EMVTVR[4]);
	if((G_NORMALTRANS_ucTransType==TRANS_EC_ASSIGNLOAD)
	    ||(G_NORMALTRANS_ucTransType==TRANS_EC_UNASSIGNLOAD
	    ||G_NORMALTRANS_ucTransType==TRANS_EC_CASHLOAD)
		&&(EMVTransInfo.enTransResult==ONLINEAPPROVED)&&(EMVTransInfo.EMVTSI[0] & 0x04)&&(EMVTransInfo.EMVTVR[4] & 0x10))				//发卡行认证失败
	{
		/*圈存脚本更新失败后需要冲正*/
		util_Printf("\nENTER11212");
		if(EMVICC_SW()!=0x9000)
		{
			EMVTransInfo.bReversal =TRUE;
			{
				util_Printf("\ntestetest2009-9-16 11:26");
				//取出冲正数据XDATA_Write_Vaild_File(DataSaveTrans8583);
			 	XDATA_Read_Vaild_File(DataSaveTrans8583);
				ISO8583_RestoreISO8583Data((unsigned char *)&ISO8583Data,
								(unsigned char *)&DataSave0.Trans_8583Data.ReversalISO8583Data.BackupISO8583Data);
				memset(aucRecvBuff,0x00,sizeof(aucRecvBuff));
				uiRecvLen = sizeof(aucRecvBuff);
				TRANS_SetField55_Reversal(FIRSTGACPHASETAG,Msg_EMV_BatchTrans,aucRecvBuff,&uiRecvLen);
				ucResult=ISO8583_SetBitValue(55,aucRecvBuff,uiRecvLen);

				// Copy Data from Global ISO8583Data
				ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
							(unsigned char *)&DataSave0.Trans_8583Data.ReversalISO8583Data.BackupISO8583Data);
				ucResult = XDATA_Write_Vaild_File(DataSaveTrans8583);
			}
			ucResult =EMVERROR_ISSUREDATA;
		}

	}

	util_Printf("\nucAPPREsult444 =%d",ucAPPResult);
	if(ucResult == EMVERR_SUCCESS)
	{
		keyflag = TransCapture(EMVTransInfo.enTransResult);
		util_Printf("\n判断UCHAR[Keyflag]=%d\n",keyflag);
		//if(keyflag = TransCapture(EMVTransInfo.enTransResult))
		if(keyflag)
		{
			util_Printf("\n判断UCHAR[Keyflag]=%d\n",keyflag);
			if(EMVTransInfo.enTransResult ==ONLINEAPPROVED||EMVTransInfo.enTransResult ==UNABLEONLINE_OFFLINEAPPROVED)
			{

				TRANS_SetField55(SECONDGACPHASETAG,Msg_EMV_BatchTrans,G_NORMALTRANS_ucField55Data,(ushort *)&G_NORMALTRANS_uiField55Len);
			}
			else
				TRANS_SetField55(FIRSTGACPHASETAG,Msg_EMV_BatchTrans,G_NORMALTRANS_ucField55Data,(ushort *)&G_NORMALTRANS_uiField55Len);
		}else
		{
			ucAPPResult = ERR_CANCEL;
		}
		util_Printf("\nucAPPREsult555 =%d",ucAPPResult);
		AddTransLog(Msg_EMV_BatchAdvice);
		G_NORMALTRANS_ucFallBackFlag = 0;
		util_Printf("\nucAPPREsult111 =%d",ucAPPResult);
		if(G_NORMALTRANS_ucTransType!=TRANS_QUERYBAL)
		UTIL_DispTransResult(EMVTransInfo.enTransResult);
		util_Printf("\nEMVTransInfo.bECTrans==%d",EMVTransInfo.bECTrans);
		if(EMVTransInfo.bECTrans
			||G_NORMALTRANS_ucTransType==TRANS_EC_CASHLOAD
				||G_NORMALTRANS_ucTransType==TRANS_EC_UNASSIGNLOAD
				||G_NORMALTRANS_ucTransType==TRANS_EC_ASSIGNLOAD)
		{
			if (G_NORMALTRANS_ucTransType != TRANS_PREAUTHFINISH)
			G_NORMALTRANS_ucECTrans =TRANS_EC_PURCHASE;
			memcpy((uchar *)&G_NORMALTRANS_PECTransInfo,(uchar *)&ECTransInfo,sizeof(ECTRANSINFO));
			if(G_NORMALTRANS_ucTransType==TRANS_EC_CASHLOAD
				||G_NORMALTRANS_ucTransType==TRANS_EC_UNASSIGNLOAD
				||G_NORMALTRANS_ucTransType==TRANS_EC_ASSIGNLOAD)
			{
					ucResult =EMVICC_GetData((PUCHAR)"\x9F\x79",2);
					if(ucResult)
						ucResult = EMVICC_CommandResult();
					util_Printf("\nEMVDATA_CheckCardSupportEC14==%d",ucResult);
					if(!ucResult)
					{
						ECTransInfo.uiNBalance =CONV_BcdLong(EMVIccOut.aucDataOut,EMVIccOut.uiLenOut*2);
						G_NORMALTRANS_PECTransInfo.uiNBalance =ECTransInfo.uiNBalance;
						G_NORMALTRANS_PECTransInfo.uiBalance =ECTransInfo.uiBalance;
					}
			}
			//MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
			if(DataSave0.ConstantParamData.Pinpadflag ==1)
                    {
                        ////Os__GB2312_display(1,0,(unsigned char *)"    交易成功");
                        ////Os__GB2312_display(2,0,(unsigned char *)" 请查看密码键盘");
                        Os__clr_display_pp(255);
                        Os__GB2312_display_pp(0,0,(PUCHAR)"电子现金余额:");
                        memset(aucBuff,0x00,sizeof(aucBuff));
                        UTIL_Form_Montant(aucBuff,G_NORMALTRANS_PECTransInfo.uiNBalance,2);
                        Os__display_pp(1,0,aucBuff);
                    }
                    else
                    {
//                        //Os__GB2312_display(1,0,(PUCHAR)"电子现金余额:");
//                        memset(aucBuff,0x00,sizeof(aucBuff));
//                        UTIL_Form_Montant(aucBuff,G_NORMALTRANS_PECTransInfo.uiNBalance,2);
//                        //Os__GB2312_display(2,0,aucBuff);
                    }
			//UTIL_WaitGetKey(3);
		}
		util_Printf("\nucAPPREsult 222=%d",ucAPPResult);
	}else
	{
		util_Printf("EMV-TPro=%02x\n",ucResult);
		if((ucResult == EMVERROR_FALLBACK)
			//||(ucResult == EMVERROR_NOSUPPORTAPP)
			//||(ucResult == EMVERROR_ICCDATA)
		   )
		{
			G_NORMALTRANS_ucFallBackFlag=2;
		}
		else
		{
			G_NORMALTRANS_ucFallBackFlag=0;
		}
		if (ucResult !=ERR_CANCEL)
		EMVERROR_DisplayMsg(ucResult);
		if(ucResult ==ERR_CANCEL)
			ucAPPResult =ERR_CANCEL;
		else
			ucAPPResult = EMV_TRANSFAIL;
	}
	util_Printf("\nEMVTransInfo.bReversal2009-10-19 9:45=%02x\n",EMVTransInfo.bReversal);
	if(EMVTransInfo.bReversal&&G_NORMALTRANS_ucTransType!=TRANS_QUERYBAL)
	{
		DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid = DATASAVEVALIDFLAG;
		XDATA_Write_Vaild_File(DataSaveTrans8583);
	}else if (G_NORMALTRANS_ucTransType != TRANS_OFFPURCHASE
		&&(ucHostResult== ERR_HOSTCODE || !ucHostResult))
	{
		memset(&DataSave0.Trans_8583Data.ReversalISO8583Data,0x00,sizeof(REVERSALISO8583));
		XDATA_Write_Vaild_File(DataSaveTrans8583);
	}

	util_Printf("ucHostResult=%02x\n",ucHostResult);
	if(ucHostResult)
	{
		//Os__clr_display(255);
		MSG_DisplayErrMsg(ucHostResult);
		ucAPPResult = ERR_CANCEL;
	}
	MasApp_CopytEMVTransInfoToNormalTrans();

	//当卡片存在冲正，且后台有脚返回时的冲正包需要重新打包
	if(DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid == DATASAVEVALIDFLAG
		&& EMVTransInfo.ucScriptResultNum)
	{
		//取出冲正数据
	 	XDATA_Read_Vaild_File(DataSaveTrans8583);
		ISO8583_RestoreISO8583Data((unsigned char *)&ISO8583Data,
						(unsigned char *)&DataSave0.Trans_8583Data.ReversalISO8583Data.BackupISO8583Data);
		memset(aucRecvBuff,0x00,sizeof(aucRecvBuff));
		uiRecvLen = sizeof(aucRecvBuff);
		TRANS_SetField55_Reversal(FIRSTGACPHASETAG,Msg_EMV_BatchTrans,aucRecvBuff,&uiRecvLen);
		ucFileResult=ISO8583_SetBitValue(55,aucRecvBuff,uiRecvLen);

		// Copy Data from Global ISO8583Data
		ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
					(unsigned char *)&DataSave0.Trans_8583Data.ReversalISO8583Data.BackupISO8583Data);
		ucFileResult = XDATA_Write_Vaild_File(DataSaveTrans8583);
	}

	//发卡行验证失败的情况，冲正需要重新打包
	if(( DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid == DATASAVEVALIDFLAG)
		&&((G_NORMALTRANS_ucTransType == TRANS_PREAUTH)||(G_NORMALTRANS_ucTransType == TRANS_PURCHASE))
 		&&(NormalTransData.TVR[4] & 0x40))
	{
		util_Printf("\ntestetest2009-9-16 11:26");
		//取出冲正数据XDATA_Write_Vaild_File(DataSaveTrans8583);
	 	XDATA_Read_Vaild_File(DataSaveTrans8583);
		ISO8583_RestoreISO8583Data((unsigned char *)&ISO8583Data,
						(unsigned char *)&DataSave0.Trans_8583Data.ReversalISO8583Data.BackupISO8583Data);
		memset(aucRecvBuff,0x00,sizeof(aucRecvBuff));
		uiRecvLen = sizeof(aucRecvBuff);
		TRANS_SetField55_Reversal(FIRSTGACPHASETAG,Msg_EMV_BatchTrans,aucRecvBuff,&uiRecvLen);
		ucFileResult=ISO8583_SetBitValue(55,aucRecvBuff,uiRecvLen);

		// Copy Data from Global ISO8583Data
		ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
					(unsigned char *)&DataSave0.Trans_8583Data.ReversalISO8583Data.BackupISO8583Data);
		ucFileResult = XDATA_Write_Vaild_File(DataSaveTrans8583);
	}
	util_Printf("\nucAPPREsult =%d",ucAPPResult);
	return ucAPPResult;
}
#if SANDREADER
UCHAR	QTransProcess(QTRANSTYPE enTransType,UINT uiAmount,UINT uiOtherAmount)
{
	UCHAR	ucResult,ucAPPResult=SUCCESS,keyflag,ucCVMResult;
	UCHAR bConnectSuccess;
	UCHAR	ucAppIndex;
	QCANDIDATELIST	QCandidateList;
	UCHAR	aucRecvBuff[300],ucRstBuf[2],aucBuff[12];
	USHORT  uiRecvLen;
	UCHAR ucHostResult,ucResultPIN=SUCCESS,ucPindata[30],ucPinlen;

	util_Printf("\n  1. QTransProcess.....................\n");

	ucResult = EMVERR_SUCCESS;
	ucAPPResult = EMVERR_SUCCESS;
	ucHostResult = EMVERR_SUCCESS ;
	MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
	//Os__GB2312_display(2,1,"数据处理中... ");

	memcpy(&QEMVConfig,&QTermConfigDefault,sizeof(QTERMCONFIG));
	QEMVConfig.ucCAPKNums =ucTermCAPKNum;
	QEMVConfig.pTermSupportCAPK =(PQCAPK)TermCAPK;
	QEMVConfig.ucTermSupportAppNum = ucTermAIDNum;
	QEMVConfig.pTermSupportApp = QTemAID;
	QEMVConfig.ucQIPKRevokeNum =ucIPKRevokeNum;
	QEMVConfig.pIPKRevoke =(PQIPKREVOKE)IPKRevoke;
	QEMVConfig.ucQExceptFileNum = ucExceptFileNum;
	QEMVConfig.pExceptPAN = (PQEXCEPTPAN)ExceptFile;
#if WKPINPAD
	QEMVConfig.pfnQICC = QEMVICC;
#else
	QEMVConfig.pfnQICC = QICC;
#endif
	QEMVConfig.pfnOfflinePIN = UTIL_OfflinePIN;
	QEMVConfig.pfnOnlinePIN = UTIL_InputEncryptPIN; //UTIL_OnlinePIN;
	QEMVConfig.pfnEMVInformMsg =UTIL_EMVInformMsg;
	QEMVConfig.pfnEMVIDCardVerify =UTIL_IDCardVerify;

	util_Printf("\nEMVConfig.ucTermType = %02x\n",QEMVConfig.ucTermType);
	util_Printf("\nEMVConfig.bPBOC20 = %02x\n",QEMVConfig.bPBOC20);
	util_Printf("\nEMVConfig.uiFloorLimit = %d\n",QEMVConfig.uiFloorLimit);
	//EMV 内核配置初始化
	//QAID_Dump();	//内核监控数据
	QDATA_TermInit(&QEMVConfig);
	//建立终端与卡片共同支持的应用.
	ucResult = QAPPSELECT_CreateAppCandidate(&QCandidateList);
	util_Printf("\n---0100----ucResult=%02x\n",ucResult);
	while(!ucResult &&QCandidateList.CandidateNums)
	{
		util_Printf("CandidateList.CandidateNums = %02X\n",QCandidateList.CandidateNums);
		ucAppIndex = 0;
		if(!ucResult)
		{
			//选择应用来完成交易
			ucResult = QAPPSELECT_FinalSelect(ucAppIndex,&QCandidateList);
			util_Printf("\n ---0102----ucResult=%02x\n",ucResult);
			util_Printf("\n ucAPPREsult0102 =%d",ucAPPResult);
			ucAPPResult =SUCCESS;
			if(ucResult == EMVERROR_RESELECT)
			{
				MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
				//Os__GB2312_display(2,1,(PUCHAR)"请再试一次！");
				Os__xget_key();
				ucResult =EMVERROR_SUCCESS;
				continue;
			}
		}
		if(!ucResult)
		{
			util_Printf("\n ucAPPREsult0103 =%d",ucAPPResult);
			UTIL_SavEMVTransData();//设置交易信息TransReqInfo

			util_Printf("\n ucAPPREsult0104 =%d",ucAPPResult);
			//交易日期、时间、随机数、交易类型(GOODS)、金额、终端、商户、商户名称、销售点输入方式(0x02)、是否强制联机
			//UTIL_IncreaseTraceNumber();
			QDATA_TransInit((PQTRANSREQINFO)&TransReqInfo);//交易前把交易相关信息发给EMV内核.
			util_Printf("\n ucAPPREsult0105 =%d",ucAPPResult);
			memset(aucBuff,0,sizeof(aucBuff));
			switch(G_NORMALTRANS_ucTransType)
			{
				case TRANS_QUERYBAL:
					aucBuff[0] = 0x31;
					break;
				case TRANS_PURCHASE:
					aucBuff[0] = 0x00;
					break;
				case TRANS_PREAUTH:
					aucBuff[0] = 0x03;
					break;
				case TRANS_EC_ASSIGNLOAD:
					aucBuff[0] =0x60;
					break;
				case TRANS_EC_UNASSIGNLOAD:
					aucBuff[0] =0x62;
					break;
				case TRANS_EC_CASHLOAD:
					aucBuff[0] =0x63;
					break;
				default:
					break;
			}
			QTRANSTAG_SetTag(ALLPHASETAG,(PUCHAR)"\x9C",aucBuff ,1 );
		}
#ifndef EMVTEST
		if(!ucResult)
		{
			//找到匹配的应用，设置应用参数
			ucResult = QEMV_SetEMVConfig();
			if(ucResult != SUCCESS) return ucResult;
			//EMVDATA_TermInit_Again(&EMVConfig);
		}
#endif
		util_Printf("\n ucAPPREsult888 =%d",ucAPPResult);
		if(!ucResult)
		{
			MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
			//Os__GB2312_display(2,1,(PUCHAR)"数据处理中... ");

			//初始化应用.
			util_Printf("\n QINTIAPP_GPO\n");
			ucResult = QINTIAPP_GPO();
			
			util_Printf("\n ---0103----ucResult=%02x\n",ucResult);
			if(ucResult == EMVERROR_RESELECT)
			{
				//从内核中获取终端与卡片共同支持的应用信息
				ucResult = QAPPSELECT_GetAppCandidate(&QCandidateList);
				util_Printf("---0104----ucResult=%02x\n",ucResult);
				MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
				//Os__GB2312_display(2,1,(PUCHAR)"请再试一次！");
				Os__xget_key();
				continue;
			}
			else
			{
				if(G_NORMALTRANS_ucTransType==TRANS_PURCHASE)
				 {
					util_Printf("\nEMVTransInfo.bECTrans:%d",EMVTransInfo.bECTrans);
					util_Printf("\nECTransInfo.uiBalance=21212=%d,G_NORMALTRANS_ulAmount=%d",ECTransInfo.uiBalance,G_NORMALTRANS_ulAmount);
					if((memcmp(G_RUNDATA_aucAID,"\xA0\x00\x00\x03\x33\x01\x01\06",8)==0))
					{
						QTRANSTAGDATA   *pIAD;
						unsigned int		uiBalance;
						util_Printf("\nG_RUNDATA_aucAID=111112=%d",G_RUNDATA_ucAIDLen);
						pIAD=QTRANSTAG_SearchTag(QALLPHASETAG,(PUCHAR)"\x9F\x10");
						if(pIAD&&(pIAD->uiLen>15))
						{
						   uiBalance=CONV_BcdLong(pIAD->pTagValue+10,10);
						   util_Printf("\nuiBalance :=======%d",uiBalance);
						   if(G_NORMALTRANS_ulAmount>uiBalance)
							return ERR_BALANCE;//余额不足
						}
					}
				}
				break;
			}
		}
	}
	if(!ucResult)
	{
		MasApp_QSetEMVTransInfo();/*从内核获取交易信息:主账号、卡序列号、2磁道等同数据*/
		if (G_NORMALTRANS_ucTransType == TRANS_OPENINGCREDITS
			||G_NORMALTRANS_ucTransType == TRANS_CREDITSALE
			||G_NORMALTRANS_ucTransType == TRANS_CREDITSVOID)
		{
			util_Printf("\n------信用交易-----------\n");
			return(SUCCESS);
		}
		
		switch(QEMVTransInfo.EntryMode)
		{
			case ENTRYMODE_MSD:
			   ucResult = QREADAPP_ReadData();
			   if(!ucResult)
			    ucResult = MSD_TrackInfo();
			   if(!ucResult)
			    ucResult = QERROR_NEEDARQC;
			   break;
			case ENTRYMODE_QVSDC:
			   ucResult = QVSDC_ProcessFlow();
			   if(!memcmp(G_RUNDATA_aucAID,"\xA0\x00\x00\x03\x33\x01\x01\06",8)&&(ucResult==QERROR_NEEDARQC))
			   	return ERR_NOTPROC;//"交易类不支持
			   util_Printf("\nucResult =12129999999=%d",ucResult);
			   if(!ucResult||ucResult==QERROR_NEEDARQC)
			   {
			     ucCVMResult = QVERIFY_qVSDC(uiAmount);
	                   util_Printf("\nucResult =1212000000=%d",ucCVMResult);
			     if(ucCVMResult)
			     	 ucResult =QERROR_TRANSOVER;
			   }
				else if(ucResult==QERROR_TRANSOVER)
				break;
			case ENTRYMODE_CSVSDC:
				break;

		}
	}

	util_Printf("\n  2. QTransProcess....................result:%02x.\n",ucResult);

	if( ucResult==0x00
	|| ucResult==QERROR_NEEDARQC)
	{
		//MasApp_QSetEMVTransInfo();/*从内核获取交易信息:主账号、卡序列号、2磁道等同数据*/
		//ucResult = EMVERROR_NEEDARQC;
		util_Printf("\nucAPPREsult666 =%d",ucAPPResult);
		if(ucResult ==EMVERROR_NEEDARQC)
		{
			util_Printf("APPCRYPT_OfflineProcess()____2___ucResult = %02x\n",ucResult);
			G_NORMALTRANS_ucFallBackFlag =0;//Add 08-04-02
			memset(aucRecvBuff,0x00,sizeof(aucRecvBuff));
			uiRecvLen = sizeof(aucRecvBuff);
			if((memcmp(G_RUNDATA_aucAID,"\xA0\x00\x00\x03\x33\x01\x01\06",8)==0))//纯电子现金，不能联机
				return ERR_NOTPROC;
			if((G_NORMALTRANS_ucTransType==TRANS_QUERYBAL)||G_RUNDATA_ucQInputPinFlag!=1)
			{
				ucResultPIN= UTIL_InputEncryptPIN(ucPindata ,&ucPinlen);
				if(ucResultPIN==EMVERROR_BYPASS)
					ucResultPIN =SUCCESS;
			}
			if(!ucResultPIN)
				ucResult = TRANS_OnlineProcess();
			else
			    return (ucResultPIN);
			util_Printf("TRANS_OnlineProcess() ---- ucResult = %02x\n",ucResult);
			if((ucResult==ERR_DIAL)
				||ucResult==DIAL_BUSY
				||ucResult==DIAL_NO_TONE
				||ucResult==DIAL_NO_CARRIER
				||ucResult==DIAL_LINE_BUSY
				||ucResult==ERR_CANCEL
			  )
			{
				bConnectSuccess = FALSE;
				QEMVTransInfo.bReversal =false;
			}
			else
			{
				bConnectSuccess = TRUE;
				memset(ucRstBuf ,0,sizeof(ucRstBuf));
				if(ucResult == ERR_HOSTCODE )
				{
					char_asc(ucRstBuf, 2, &G_RUNDATA_ucErrorExtCode);
					QEMVTransInfo.bReversal =false;
				}
				else
				{
					char_asc(ucRstBuf, 2, &ucResult);
				}
				if(ucResult)
				{
					ucHostResult =ucResult ;
				}

				if(ucResult ==EMVERR_SUCCESS)
				{

					QEMVTransInfo.enTransResult =QONLINEAPPROVED;

				}
				else
				{
				     QEMVTransInfo.enTransResult =QONLINEDECLINED;
				     //QEMVTransInfo.bReversal = TRUE;
				     ucResult=QERROR_SUCCESS;
				}
			}
		}
	}
	util_Printf("\nucAPPREsult333 =%d",ucAPPResult);
	util_Printf("\n EMV return %02x",ucResult);
	util_Printf("\n bConnectSuccess = %02x\n",bConnectSuccess);

	util_Printf("\nucAPPREsult444 =%d",ucAPPResult);
	if(ucResult == EMVERR_SUCCESS)
	{
		keyflag = QTransCapture(QEMVTransInfo.enTransResult);
		util_Printf("\n判断UCHAR[Keyflag]=%d\n",keyflag);
		//if(keyflag = TransCapture(EMVTransInfo.enTransResult))
		if(keyflag)
		{
			util_Printf("\n判断UCHAR[Keyflag]=%d\n",keyflag);
			if(QEMVTransInfo.enTransResult ==ONLINEAPPROVED||QEMVTransInfo.enTransResult ==UNABLEONLINE_OFFLINEAPPROVED)
			{
				QTRANS_SetField55(SECONDGACPHASETAG,Msg_EMV_BatchTrans,G_NORMALTRANS_ucField55Data,(ushort *)&G_NORMALTRANS_uiField55Len);
			}
			else
				QTRANS_SetField55(FIRSTGACPHASETAG,Msg_EMV_BatchTrans,G_NORMALTRANS_ucField55Data,(ushort *)&G_NORMALTRANS_uiField55Len);
		}else
		{
			ucAPPResult = ERR_CANCEL;
		}
		util_Printf("\nucAPPREsult555 =%d",ucAPPResult);
		//AddTransLog(Msg_EMV_BatchAdvice);
		G_NORMALTRANS_ucFallBackFlag = 0;
		util_Printf("\nucAPPREsult111 =%d",ucAPPResult);
		if(G_NORMALTRANS_ucTransType!=TRANS_QUERYBAL)
			UTIL_DispTransResult(QEMVTransInfo.enTransResult);
		util_Printf("\nucAPPREsult 222=%d",ucAPPResult);
	}else
	{
		util_Printf("EMV-TPro=%02x\n",ucResult);
		if((ucResult == EMVERROR_FALLBACK)
			//||(ucResult == EMVERROR_NOSUPPORTAPP)
			//||(ucResult == EMVERROR_ICCDATA)
		   )
		{
			G_NORMALTRANS_ucFallBackFlag=2;
		}
		else
		{
			G_NORMALTRANS_ucFallBackFlag=0;
		}
		EMVERROR_DisplayMsg(ucResult);
		ucAPPResult = ERR_CANCEL;
	}
	util_Printf("\nQEMVTransInfo.bReversal2009-10-20 9:56=%d",QEMVTransInfo.bReversal);
	if(QEMVTransInfo.bReversal&&G_NORMALTRANS_ucTransType!=TRANS_QUERYBAL)
	{
		DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid = DATASAVEVALIDFLAG;
		XDATA_Write_Vaild_File(DataSaveTrans8583);
	}else  if (G_NORMALTRANS_ucTransType != TRANS_OFFPURCHASE
		&&(ucHostResult== ERR_HOSTCODE || !ucHostResult))
	{
		memset(&DataSave0.Trans_8583Data.ReversalISO8583Data,0x00,sizeof(REVERSALISO8583));
		XDATA_Write_Vaild_File(DataSaveTrans8583);
	}
	util_Printf("ucHostResult=%02x\n",ucHostResult);
	if(ucHostResult)
	{
		//Os__clr_display(255);
		MSG_DisplayErrMsg(ucHostResult);
		ucAPPResult = ERR_CANCEL;
	}
	QMasApp_CopytEMVTransInfoToNormalTrans();
	//发卡行验证失败的情况，冲正需要重新打包
	if(( DataSave0.Trans_8583Data.ReversalISO8583Data.ucValid == DATASAVEVALIDFLAG)
		&&((G_NORMALTRANS_ucTransType == TRANS_PREAUTH)||(G_NORMALTRANS_ucTransType == TRANS_PURCHASE))
 		&&(NormalTransData.TVR[4] & 0x40))
	{
		util_Printf("\ntestetest2009-9-16 11:26");
		//取出冲正数据XDATA_Write_Vaild_File(DataSaveTrans8583);
	 	XDATA_Read_Vaild_File(DataSaveTrans8583);
		ISO8583_RestoreISO8583Data((unsigned char *)&ISO8583Data,
						(unsigned char *)&DataSave0.Trans_8583Data.ReversalISO8583Data.BackupISO8583Data);
		memset(aucRecvBuff,0x00,sizeof(aucRecvBuff));
		uiRecvLen = sizeof(aucRecvBuff);
		QTRANS_SetField55_Reversal(FIRSTGACPHASETAG,Msg_EMV_BatchTrans,aucRecvBuff,&uiRecvLen);
		ucResult=ISO8583_SetBitValue(55,aucRecvBuff,uiRecvLen);

		// Copy Data from Global ISO8583Data
		ISO8583_SaveISO8583Data((unsigned char *)&ISO8583Data,
					(unsigned char *)&DataSave0.Trans_8583Data.ReversalISO8583Data.BackupISO8583Data);
		ucResult = XDATA_Write_Vaild_File(DataSaveTrans8583);
	}
	util_Printf("\nucAPPREsult =%d",ucAPPResult);
	return ucAPPResult;
}
#endif

unsigned char HBMasAPP_CheckAID(void)
{
        const TERMSUPPORTAPP TermSupportAID[]=
                {
                    {0,5,"\xD1\x56\x00\x00\x01"},		 	//CUP
                    {0,5,"\xA0\x00\x00\x03\x33"},		 	//CUP
                    {0,7,"\xA0\x00\x00\x00\x03\x10\x10"},	//Visa - VSDC,also used in EMV test.
                    {0,7,"\xA0\x00\x00\x00\x99\x90\x90"},	//EMV - test
                    {0,7,"\xA0\x00\x00\x00\x03\x20\x10"},	//Visa - Electron
                    {0,7,"\xA0\x00\x00\x00\x04\x10\x10"},	//Mastercard - M/Chip
                    {0,7,"\xA0\x00\x00\x00\x04\x30\x60"},	//Mastercard - Maestro
                    {0,7,"\xA0\x00\x00\x00\x04\x60\x00"},	//Mastercard - Cirrus
                    {0,7,"\xA0\x00\x00\x00\x04\x60\x10"},	//Mastercard - app
                    {0,7,"\xA0\x00\x00\x00\x10\x10\x30"},	//Mastercard - eurocheque
                    {0,7,"\xA0\x00\x00\x00\x65\x10\x10"}	//JCB - J/Smart AID.
                };

	if(!memcmp(TermSupportAID[0].aucAID,G_NORMALTRANS_aucAID,5)
		||!memcmp(TermSupportAID[1].aucAID,G_NORMALTRANS_aucAID,5))
	{
		G_NORMALTRANS_ucCardType = TRANS_CARDTYPE_INTERNAL;//内卡
		memcpy(G_NORMALTRANS_aucAuthGroup,"CUP",TRANS_GROUPLEN);
	}else
	if(!memcmp(TermSupportAID[2].aucAID,G_NORMALTRANS_aucAID,5))
	{
		G_NORMALTRANS_ucCardType = TRANS_CARDTYPE_VISA;//VISA
		memcpy(G_NORMALTRANS_aucAuthGroup,"VIS",TRANS_GROUPLEN);
	}
	else
	if(!memcmp(TermSupportAID[5].aucAID,G_NORMALTRANS_aucAID,5))
	{
		G_NORMALTRANS_ucCardType = TRANS_CARDTYPE_MASTER;//MASTER
		memcpy(G_NORMALTRANS_aucAuthGroup,"MAE",TRANS_GROUPLEN);
	}
	else
	if(!memcmp(TermSupportAID[10].aucAID,G_NORMALTRANS_aucAID,TermSupportAID[10].ucAIDLen))
	{
		G_NORMALTRANS_ucCardType = TRANS_CARDTYPE_JCB;//JCB
		memcpy(G_NORMALTRANS_aucAuthGroup,"JCB",TRANS_GROUPLEN);
	}
	else
	if(!memcmp(TermSupportAID[9].aucAID,G_NORMALTRANS_aucAID,7))
	{
		G_NORMALTRANS_ucCardType = TRANS_CARDTYPE_MASTER;//MASTER
		memcpy(G_NORMALTRANS_aucAuthGroup,"MAE",TRANS_GROUPLEN);
	}
	else
	{
		G_NORMALTRANS_ucCardType = TRANS_CARDTYPE_FOREIGN;//外卡
	}
	return SUCCESS;
}

unsigned char MasAPP_CheckAID(void)
{
	unsigned char ucResult,ucBuf[EMVAIDLEN];
	unsigned short uiLen;

	const TERMSUPPORTAPP TermSupportAID[]=
						{
							{0,5,"\xD1\x56\x00\x00\x01"},		 	//CUP
							{0,5,"\xA0\x00\x00\x03\x33"},		 	//CUP
							{0,7,"\xA0\x00\x00\x00\x03\x10\x10"},	//Visa - VSDC,also used in EMV test.
							{0,7,"\xA0\x00\x00\x00\x99\x90\x90"},	//EMV - test
							{0,7,"\xA0\x00\x00\x00\x03\x20\x10"},	//Visa - Electron
							{0,7,"\xA0\x00\x00\x00\x04\x10\x10"},	//Mastercard - M/Chip
							{0,7,"\xA0\x00\x00\x00\x04\x30\x60"},	//Mastercard - Maestro
							{0,7,"\xA0\x00\x00\x00\x04\x60\x00"},	//Mastercard - Cirrus
							{0,7,"\xA0\x00\x00\x00\x04\x60\x10"},	//Mastercard - app
							{0,7,"\xA0\x00\x00\x00\x10\x10\x30"},	//Mastercard - eurocheque
							{0,7,"\xA0\x00\x00\x00\x65\x10\x10"}	//JCB - J/Smart AID.
						};
	memset( ucBuf,  0, sizeof(ucBuf));
	uiLen=EMVAIDLEN;
	ucResult = EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x4F",ucBuf,&uiLen);
	if(ucResult != SUCCESS)
		return ucResult;

	memcpy( G_NORMALTRANS_aucAID, ucBuf, uiLen);
	//UTIL_TestDispStr(G_NORMALTRANS_aucAID, 5, (UCHAR *)"\n----G_NORMALTRANS_aucAID--2--\n", NULL);
	if(!memcmp(TermSupportAID[0].aucAID,G_NORMALTRANS_aucAID,5)
		||!memcmp(TermSupportAID[1].aucAID,G_NORMALTRANS_aucAID,5))
	{
		G_NORMALTRANS_ucCardType = TRANS_CARDTYPE_INTERNAL;//内卡
		memcpy(G_NORMALTRANS_aucAuthGroup,"CUP",TRANS_GROUPLEN);
	}else
	if(!memcmp(TermSupportAID[2].aucAID,G_NORMALTRANS_aucAID,5))
	{
		G_NORMALTRANS_ucCardType = TRANS_CARDTYPE_VISA;//VISA
		memcpy(G_NORMALTRANS_aucAuthGroup,"VIS",TRANS_GROUPLEN);
	}
	else
	if(!memcmp(TermSupportAID[5].aucAID,G_NORMALTRANS_aucAID,5))
	{
		G_NORMALTRANS_ucCardType = TRANS_CARDTYPE_MASTER;//MASTER
		memcpy(G_NORMALTRANS_aucAuthGroup,"MAE",TRANS_GROUPLEN);
	}
	else
	if(!memcmp(TermSupportAID[10].aucAID,G_NORMALTRANS_aucAID,TermSupportAID[10].ucAIDLen))
	{
		G_NORMALTRANS_ucCardType = TRANS_CARDTYPE_JCB;//JCB
		memcpy(G_NORMALTRANS_aucAuthGroup,"JCB",TRANS_GROUPLEN);
	}
	else
	if(!memcmp(TermSupportAID[9].aucAID,G_NORMALTRANS_aucAID,7))
	{
		G_NORMALTRANS_ucCardType = TRANS_CARDTYPE_MASTER;//MASTER
		memcpy(G_NORMALTRANS_aucAuthGroup,"MAE",TRANS_GROUPLEN);
	}
	else
	{
		G_NORMALTRANS_ucCardType = TRANS_CARDTYPE_FOREIGN;//外卡
	}
	return SUCCESS;

}
#if SANDREADER
unsigned char QMasAPP_CheckAID(void)
{
	unsigned char ucResult,ucBuf[EMVAIDLEN];
	unsigned short uiLen;

	const TERMSUPPORTAPP TermSupportAID[]=
						{
							{0,5,"\xD1\x56\x00\x00\x01"},		 	//CUP
							{0,5,"\xA0\x00\x00\x03\x33"},		 	//CUP
							{0,7,"\xA0\x00\x00\x00\x03\x10\x10"},	//Visa - VSDC,also used in EMV test.
							{0,7,"\xA0\x00\x00\x00\x99\x90\x90"},	//EMV - test
							{0,7,"\xA0\x00\x00\x00\x03\x20\x10"},	//Visa - Electron
							{0,7,"\xA0\x00\x00\x00\x04\x10\x10"},	//Mastercard - M/Chip
							{0,7,"\xA0\x00\x00\x00\x04\x30\x60"},	//Mastercard - Maestro
							{0,7,"\xA0\x00\x00\x00\x04\x60\x00"},	//Mastercard - Cirrus
							{0,7,"\xA0\x00\x00\x00\x04\x60\x10"},	//Mastercard - app
							{0,7,"\xA0\x00\x00\x00\x10\x10\x30"},	//Mastercard - eurocheque
							{0,7,"\xA0\x00\x00\x00\x65\x10\x10"}	//JCB - J/Smart AID.
						};
	memset( ucBuf,  0, sizeof(ucBuf));
	uiLen=EMVAIDLEN;
	ucResult = QTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x4F",ucBuf,&uiLen);
	if(ucResult != SUCCESS)
		return ucResult;

	memcpy( G_NORMALTRANS_aucAID, ucBuf, uiLen);
	//UTIL_TestDispStr(G_NORMALTRANS_aucAID, 5, (UCHAR *)"\n----G_NORMALTRANS_aucAID--2--\n", NULL);
	if(!memcmp(TermSupportAID[0].aucAID,G_NORMALTRANS_aucAID,5)
		||!memcmp(TermSupportAID[1].aucAID,G_NORMALTRANS_aucAID,5))
	{
		G_NORMALTRANS_ucCardType = TRANS_CARDTYPE_INTERNAL;//内卡
		memcpy(G_NORMALTRANS_aucAuthGroup,"CUP",TRANS_GROUPLEN);
	}else
	if(!memcmp(TermSupportAID[2].aucAID,G_NORMALTRANS_aucAID,5))
	{
		G_NORMALTRANS_ucCardType = TRANS_CARDTYPE_VISA;//VISA
		memcpy(G_NORMALTRANS_aucAuthGroup,"VIS",TRANS_GROUPLEN);
	}
	else
	if(!memcmp(TermSupportAID[5].aucAID,G_NORMALTRANS_aucAID,5))
	{
		G_NORMALTRANS_ucCardType = TRANS_CARDTYPE_MASTER;//MASTER
		memcpy(G_NORMALTRANS_aucAuthGroup,"MAE",TRANS_GROUPLEN);
	}
	else
	if(!memcmp(TermSupportAID[10].aucAID,G_NORMALTRANS_aucAID,TermSupportAID[10].ucAIDLen))
	{
		G_NORMALTRANS_ucCardType = TRANS_CARDTYPE_JCB;//JCB
		memcpy(G_NORMALTRANS_aucAuthGroup,"JCB",TRANS_GROUPLEN);
	}
	else
	if(!memcmp(TermSupportAID[9].aucAID,G_NORMALTRANS_aucAID,7))
	{
		G_NORMALTRANS_ucCardType = TRANS_CARDTYPE_MASTER;//MASTER
		memcpy(G_NORMALTRANS_aucAuthGroup,"MAE",TRANS_GROUPLEN);
	}
	else
	{
		G_NORMALTRANS_ucCardType = TRANS_CARDTYPE_FOREIGN;//外卡
	}

}
#endif

unsigned char EMV_SimpleProcess(TRANSTYPE enTransType,UINT uiAmount,UINT uiOtherAmount)
{
	UCHAR	ucResult;
	UINT	uiLogSumAmount;
	UCHAR	bConfirm;
	UCHAR	ucAppIndex;
	CANDIDATELIST	CandidateList;


	MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
	//Os__GB2312_display(2,1,"数据处理中... ");
	memcpy(&EMVConfig,&TermConfigDefault,sizeof(TERMCONFIG));

	EMVConfig.ucTermType = 0x22;
	EMVConfig.ucCAPKNums =ucTermCAPKNum;
	EMVConfig.pTermSupportCAPK =TermCAPK;
	EMVConfig.ucTermSupportAppNum = ucTermAIDNum;
	EMVConfig.pTermSupportApp = TermAID;
	EMVConfig.ucIPKRevokeNum =ucIPKRevokeNum;
	EMVConfig.pIPKRevoke =IPKRevoke;
	EMVConfig.ucExceptFileNum = ucExceptFileNum;
	EMVConfig.pExceptPAN = ExceptFile;
	EMVConfig.pfnEMVICC = EMVICC;
	EMVConfig.pfnOfflinePIN = UTIL_OfflinePIN;
	EMVConfig.pfnOnlinePIN = UTIL_InputEncryptPIN;//UTIL_OnlinePIN;;
	EMVConfig.pfnEMVInformMsg =UTIL_EMVInformMsg;
	EMVConfig.bPBOC20 =ConstParam.bPBOC20;
	EMVConfig.pfnEMVIDCardVerify =UTIL_IDCardVerify;

	//memcpy( EMVConfig.aucTerminalID, DataSave0.ConstantParamData.aucTerminalID, 8);

	util_Printf("\nEMVConfig.uiFloorLimit = %d\n",EMVConfig.uiFloorLimit);

//#ifdef EMVTEST
//	EMVDATA_TermInit_Test(&EMVConfig);
//#else
	EMVDATA_TermInit(&EMVConfig);
//#endif

	ucResult = APPSELECT_CreateAppCandidate(&CandidateList);
	util_Printf("---0100----ucResult=%02x\n",ucResult);
	bConfirm =FALSE;
	if(CandidateList.CandidateNums ==1 &&CandidateList.CandidateApp[0].bAppPriorityExist)
	{
		if(CandidateList.CandidateApp[0].ucAppPriorityIndicator &APPNEEDCONFIRM)
			bConfirm = TRUE;
	}
	else
	if (CandidateList.CandidateNums >1)
		bConfirm = TRUE;

	while(!ucResult && CandidateList.CandidateNums)
	{
		util_Printf("CandidateList.CandidateNums = %02X\n",CandidateList.CandidateNums);
		ucAppIndex = 0;

		if(bConfirm)
		{
			ucResult = CardHolderConfirmApp(&CandidateList,&ucAppIndex);
			util_Printf("---0101----ucResult=%02x\n",ucResult);
		}
		if(!ucResult)
		{
			ucResult = APPSELECT_FinalSelect(ucAppIndex,&CandidateList);
			util_Printf("---0102----ucResult=%02x\n",ucResult);
			if(ucResult == EMVERROR_RESELECT)
			{
				MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
				//Os__GB2312_display(2,1,(PUCHAR)"请再试一次！");
				Os__xget_key();
				ucResult =EMVERROR_SUCCESS;
				continue;
			}
		}
		if(!ucResult)
		{
			UTIL_SavEMVTransData();
			//UTIL_IncreaseTraceNumber();
			EMVDATA_TransInit(&TransReqInfo);
		}

#ifndef EMVTEST
		if(!ucResult)
		{
			ucResult = EMV_SetEMVConfig();
			if(ucResult != SUCCESS) return ucResult;
			//EMVDATA_TermInit_Again(&EMVConfig);
		}
#endif

		if(!ucResult)
		{
			MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
			//Os__GB2312_display(2,1,(PUCHAR)"数据处理中... ");
			ucResult = INTIAPP_GPO();
			util_Printf("---0103----ucResult=%02x\n",ucResult);
			if(ucResult == EMVERROR_RESELECT)
			{
				ucResult = APPSELECT_GetAppCandidate(&CandidateList);
				util_Printf("---0104----ucResult=%02x\n",ucResult);
				MSG_DisplyTransType(G_NORMALTRANS_ucTransType ,0,0,true,255);
				//Os__GB2312_display(2,1,(PUCHAR)"请再试一次！");
				Os__xget_key();
				continue;
			}else
				break;
		}
	}

	if(!ucResult) ucResult = READAPP_ReadData();

	if(!ucResult)
	{
		uiLogSumAmount = CalcLogTransAmount();
	}

	if(!ucResult)
	{
		ucResult = TERMRISKMANAGE_Process(uiLogSumAmount);
	}

	if(!ucResult)
	{
		MasApp_SetEMVTransInfo();
	}

	if(ucResult)
	{
		if((ucResult == EMVERROR_FALLBACK)||(ucResult == EMVERROR_NOSUPPORTAPP)
			||(ucResult == EMVERROR_ICCDATA) )
			G_NORMALTRANS_ucFallBackFlag=2;
		EMVERROR_DisplayMsg(ucResult);
	}
	return ucResult;
}
#ifdef EMVTEST
UCHAR	CheckTransTVR(void)
{
	UCHAR ucI,aucVer[16],aucBuff[32],aucTempBuff[16];
	UCHAR aucCfgToSign[512],ucIndex,aucCfgHash[20];
	EMVTRANSINFO EMVTransInfo;

	//Os__clr_display(255);
	//Os__GB2312_display(0,0,"TVR value:");


	memset(aucBuff,0,sizeof(aucBuff));
	EMVDATA_TransInfo(&EMVTransInfo);
	for(ucI=0;ucI<5;ucI++)
	{
		sprintf(aucTempBuff,"%02X ",EMVTransInfo.EMVTVR[ucI]);
		strcat((char*)aucBuff,(char*)aucTempBuff);
	}
	//Os__GB2312_display(1,0,aucBuff);


	sprintf(aucBuff,"TSI: %02X %02X",EMVTransInfo.EMVTSI[0],EMVTransInfo.EMVTSI[1]);
	//Os__GB2312_display(2,0,aucBuff);



	memset(aucVer,0,16);
	EMVDATA_KernelVer(aucVer);
	sprintf(aucBuff,"EMVVer: %s",(char*)aucVer);
	//Os__GB2312_display(3,0,aucBuff);

	Os__xget_key();

	//Display check sum(4-20 bytes) for Configurable Kernel
	ucIndex=0;
	memset(aucCfgToSign,0,sizeof(aucCfgToSign));

	memcpy(aucCfgToSign+ucIndex,EMVConfig.aucTermCapab,sizeof(EMVConfig.aucTermCapab));
	ucIndex+=sizeof(EMVConfig.aucTermCapab);

	memcpy(aucCfgToSign+ucIndex,EMVConfig.aucTermAddCapab,sizeof(EMVConfig.aucTermAddCapab));
	ucIndex+=sizeof(EMVConfig.aucTermAddCapab);

	memcpy(aucCfgToSign+ucIndex,&(EMVConfig.ucTermType),sizeof(EMVConfig.ucTermType));
	ucIndex+=sizeof(EMVConfig.ucTermType);


/*
	memcpy(aucCfgToSign+ucIndex,&(TermInfo.bForceAccept),sizeof(TermInfo.bForceAccept));
	ucIndex+=sizeof(TermInfo.bForceAccept);
*/
	memcpy(aucCfgToSign+ucIndex,&(ConstParam.bForceOnline),sizeof(ConstParam.bForceOnline));
	ucIndex+=sizeof(ConstParam.bForceOnline);

	memcpy(aucCfgToSign+ucIndex,&(EMVConfig.ucDefaultDOLLen),sizeof(EMVConfig.ucDefaultDOLLen));
	ucIndex+=sizeof(EMVConfig.ucDefaultDOLLen);

	memcpy(aucCfgToSign+ucIndex,EMVConfig.aucDefaultDOL,EMVConfig.ucDefaultDOLLen);
	ucIndex+=EMVConfig.ucDefaultDOLLen;

	memcpy(aucCfgToSign+ucIndex,&(EMVConfig.ucDefaultTDOLLen),sizeof(EMVConfig.ucDefaultTDOLLen));
	ucIndex+=sizeof(EMVConfig.ucDefaultTDOLLen);

	memcpy(aucCfgToSign+ucIndex,EMVConfig.aucDefaultTDOL,EMVConfig.ucDefaultTDOLLen);
	ucIndex+=EMVConfig.ucDefaultTDOLLen;


	SHA1_Compute(aucCfgToSign,ucIndex,aucCfgHash);


	//Os__clr_display(255);
	//Os__GB2312_display(0,0,"Cfg Check Sum:");

	memset(aucBuff,0,sizeof(aucBuff));
	for(ucI=0;ucI<4;ucI++)
	{
		sprintf(aucTempBuff,"%02X ",aucCfgHash[ucI]);
		strcat((char*)aucBuff,(char*)aucTempBuff);
	}
	//Os__GB2312_display(1,1,aucBuff);
	Os__xget_key();

}
#endif
UCHAR	AddTransLog(unsigned char ucMSGType)
{
	UCHAR		ucResult;
	UCHAR 		ucAppCrypt[16];
	TRANSRECORD	TransRecord;
	USHORT		uiLen;
	util_Printf("EMVTransInfo.enTransResult = %02x\n",EMVTransInfo.enTransResult);
	switch(EMVTransInfo.enTransResult)
	{
		case ONLINEDECLINED:
		case OFFLINEAPPROVED:
			return	SUCCESS;
		case ONLINEAPPROVED:
				if(NormalTransData.TVR[4] & 0x40)				//发卡行认证失败
					break;
				else
					return SUCCESS;
		case UNABLEONINE_OFFLINEDECLINED:
		case OFFLINEDECLINED:
		case UNABLEONLINE_OFFLINEAPPROVED:
			break;
	}
	uiLen=2;

	ucResult = EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x8A",TransRecord.aucAuthRespCode,&uiLen);
	util_Printf("TransRecord.aucAuthRespCode = %02x %02x\n",TransRecord.aucAuthRespCode[0],TransRecord.aucAuthRespCode[1]);
	if(EMVTransInfo.enTransResult==OFFLINEDECLINED&&memcmp(TransRecord.aucAuthRespCode,"Z3",2))
		memcpy( TransRecord.aucAuthRespCode,"Z1",2);

	memset(&TransRecord, 0x00 ,sizeof(TRANSRECORD));
	TransRecord.ucTransType = ConstParam.ucTransType;

//	uiLen = MAXPANDATALEN;
//	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x5A",TransRecord.aucPAN,&uiLen);
	memcpy(TransRecord.aucPAN ,G_NORMALTRANS_aucSourceAcc , (G_NORMALTRANS_ucSourceAccLen+1)/2);
	TransRecord.ucPANLen = G_NORMALTRANS_ucSourceAccLen;

	uiLen = sizeof(UCHAR);
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x5F\x34",&TransRecord.ucPANSeq,&uiLen);

	uiLen = AUTHCODELEN;
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x89",TransRecord.aucAuthCode,&uiLen);

	uiLen = sizeof(UINT);
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x9F\x02",(PUCHAR)&TransRecord.ulAmount,&uiLen);

	uiLen=3;
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x5F\x2A",(PUCHAR)&TransRecord.aucCurrencyCode,&uiLen);

	TransRecord.ulAmount =EMVTransInfo.uiTransAmount;
	TransRecord.uiOtherAmount =EMVTransInfo.uiOtherTransAmount;
	memcpy(TransRecord.aucTransDate,TransReqInfo.aucTransDate,sizeof(TransRecord.aucTransDate));
	memcpy(TransRecord.aucTransTime,TransReqInfo.aucTransTime,sizeof(TransRecord.aucTransTime));

	TransRecord.ulTraceNum = ConstParam.uiTraceNumber++;

	TransRecord.ucTransFlag =0x02;
	TransRecord.ucTransType = ucMSGType;
	if(EMVTransInfo.enTransResult==UNABLEONINE_OFFLINEDECLINED
		||EMVTransInfo.enTransResult==UNABLEONLINE_OFFLINEAPPROVED)
	{
		uiLen=sizeof(ucAppCrypt);
		memcpy( ucAppCrypt, 0, sizeof(ucAppCrypt));
		ucResult = EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x91",(PUCHAR)ucAppCrypt,&uiLen);
		if(!ucResult)
			EMVTRANSTAG_SetTag(ALLPHASETAG,(PUCHAR)"\x9F\x26",ucAppCrypt ,uiLen );
	}

	TRANS_SetField55(FIRSTGACPHASETAG,ucMSGType,TransRecord.aucLogBuff,&TransRecord.uiLogBuffLen);

	ucResult =FILE_InsertRecordByFileName(ERRICCTRANS,&TransRecord,sizeof(TRANSRECORD));
	if(ucResult)
	{
		ConstParam.uiTotalAmount +=TransRecord.ulAmount;
		ConstParam.uiTotalNums++;
		DataSave0.ChangeParamData.ucEMVICTransFailNum++;

		ucResult = UTIL_WriteConstParamFile(&ConstParam);
		XDATA_Write_Vaild_File(DataSaveChange);
	}

	return ucResult;

}


UINT	CalcLogTransAmount(void)
{
	UINT	uiLogAmount,uiRecordNums,uiI;
	UCHAR	ucResult,aucPAN[MAXPANDATALEN];
	USHORT	uiLen;
	TRANSRECORD	TransRecord;
	uiLogAmount = 0;

	ucResult = FILE_ReadRecordNumByFileName(TRANSLOG,&uiRecordNums);

	if(!ucResult)
	{
		uiLen = MAXPANDATALEN;
		memset(aucPAN,0x00,sizeof(aucPAN));
		ucResult = EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x5A",aucPAN,&uiLen);
	}
	for(uiI =1; !ucResult && uiI <= uiRecordNums; uiI++)
	{
		ucResult = FILE_ReadRecordByFileName(TRANSLOG,uiI,&TransRecord,sizeof(TRANSRECORD));
		if(!ucResult)
		{
			if(!memcmp(aucPAN,&TransRecord.aucPAN,MAXPANDATALEN))
			{
				uiLogAmount += TransRecord.ulAmount;
//				uiLogAmount += TransRecord.uiOtherAmount;
			}
		}
	}

	return uiLogAmount;

}
UCHAR	TransCapture(TRANSRESULT	enTransResult)
{

	if(enTransResult == OFFLINEAPPROVED
		||enTransResult == UNABLEONLINE_OFFLINEAPPROVED)
	{
		G_NORMALTRANS_ucTransType=TRANS_OFFPURCHASE;
		MasAPP_CheckAID();
	}

	util_Printf("\nenTransResult=%d\n",enTransResult);
	switch(enTransResult)
	{
	case	ONLINEAPPROVED:
	case	OFFLINEAPPROVED:
	case	UNABLEONLINE_OFFLINEAPPROVED:
		return	TRUE;
	case	OFFLINEDECLINED:
	case	ONLINEDECLINED:
	case	UNABLEONINE_OFFLINEDECLINED:
		return	FALSE;
	default:
		return FALSE;

	}
}
#if SANDREADER
UCHAR	QTransCapture(TRANSRESULT	enTransResult)
{

	if(enTransResult == OFFLINEAPPROVED
		||enTransResult == UNABLEONLINE_OFFLINEAPPROVED)
	{
		G_NORMALTRANS_ucTransType=TRANS_OFFPURCHASE;
		QMasAPP_CheckAID();
	}

	util_Printf("\nenTransResult=%d\n",enTransResult);
	switch(enTransResult)
	{
	case	ONLINEAPPROVED:
	case	OFFLINEAPPROVED:
	case	UNABLEONLINE_OFFLINEAPPROVED:
		return	TRUE;
	case	OFFLINEDECLINED:
	case	ONLINEDECLINED:
	case	UNABLEONINE_OFFLINEDECLINED:
		return	FALSE;

	}
}
#endif
UCHAR	ReadICTransLog(void)
{
	DRV_OUT *pxIcc;
	DRV_OUT *pxKey;
	unsigned long ulAmount;
	UCHAR	aucResetBuf[50],ucAppIndex,ucResult=SUCCESS,aucEntryBuf[10],ucKey;
	USHORT	uiLen;
	UCHAR	bConfirm;
	unsigned char ucDOLLen,aucDOL[100],*pDOL,*pDataOut,ucTagLen,aucAmtBuf[30];
	UCHAR	ucRecordSFI,ucRecordNums,ucI,aucBuff[30],ucICCardType,ucResetBufLen;
	unsigned char ucRecordNum,ucJ;
	unsigned char aucTransDate[3],aucTransTime[3],aucAuthAmount[6],aucOtherAmount[6],
				  aucCountryCode[2],aucTransCurcyCode[2],aucMerchantName[20];
	unsigned char ucTransType,aucATC[4],ucInputType;
	CANDIDATELIST	CandidateList;
	DRV_OUT  aucDRVOut;
	UINT	Timeout;

NEW_DRV_TYPE  new_drv;
uchar ucDrv;

	while(1)
	{
		util_Printf("\nstart ReadICTransLog=============");
		ucInputType=0x06;
		memset(&aucDRVOut,0,sizeof(DRV_OUT));
		#if 0
		if(ReaderSupport&&(ReaderType==READER_HONGBAO))
		{
			pxKey->gen_status =DRV_STOPPED;
			pxIcc->gen_status =DRV_STOPPED;
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"   (明细查询)");
			//Os__GB2312_display(2,0,"请插IC卡或挥卡");
			memset((unsigned char *)&InterfaceTransData,0,sizeof(InterfaceTransData));
			INTERFACE_TransToReader(1,&InterfaceTransData,&aucDRVOut,&ucInputType);
			util_Printf("\nRead返回刷卡模式ucInputType= %02x\n",ucInputType);
    	if(InterfaceTransData.unTransResult!=SUCCESS)//IC卡和磁卡不要判断
    	{
    		ucResult = InterfaceTransData.unTransResult;
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"   (明细查询)");
				//Os__GB2312_display(2,0,(uchar *)"    读卡错！");
				UTIL_WaitGetKey(3);
				INTERFACE_RefurbishReaderLCD(4);
				return SUCCESS;
    	}
    	else
    	{
    		//BIT 0  IC;  BIT 1  mag ; BIT 2 interface
      	util_Printf("\nINTERFACE_TransToReader ucInputType=%02x",ucInputType);
      	if(ucInputType == 0x01)	     //IC卡
      	{
        	pxIcc = &aucDRVOut;
      		pxIcc->gen_status = DRV_ENDED;
      	}
      	else if(ucInputType == 0x04) //非接卡
      	{

      	}
      	else
      	{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar *)"   (明细查询)");
					//Os__GB2312_display(2,0,(uchar *)"    读卡错！");
					UTIL_WaitGetKey(3);
					break;
     		}
    	}
			INTERFACE_RefurbishReaderLCD(4);/*交易完成*/
		}
		else
	  #endif
		{
		/*
			Timeout=60*100;
			Os__light_on();
			Os__timer_start(&Timeout);
			pxIcc = Os__ICC_insert();
			pxKey = Os__get_key();

			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"   (明细查询)");
			//Os__GB2312_display(2,2,"请插IC卡");
			while((Timeout!=0)&&(pxIcc->gen_status != DRV_ENDED)
			      &&(pxKey->gen_status != DRV_ENDED));
			Os__timer_stop(&Timeout);
			if(Timeout==0)
			{
				Os__abort_drv(drv_icc);
				Os__abort_drv(drv_mmi);
				return ERR_CANCEL;
			}
*/
			ucDrv = KEY_DRV|ICC_DRV;
			util_Printf("\n1.ReadICTransLog Os_Wait_Event begin...");
			ucResult = Os_Wait_Event(ucDrv, &new_drv, 1000*60);
			//util_Printf("\n2. Os_Wait_Event:%02x",ret);
			//util_Printf("\n3. new_drv.drv_type:%02x",new_drv.drv_type);
			if(ucResult == 0)                                                              
			{                                                                         
				if(new_drv.drv_type == KEY_DRV)                                     
				{                                                                   
					if(new_drv.drv_data.gen_status== DRV_ENDED){
						pxKey =  (DRV_OUT *)&new_drv.drv_data;
					}
				}                                                                   
				else if(new_drv.drv_type == ICC_DRV)                                
				{                                                                   
					if(new_drv.drv_data.gen_status== DRV_ENDED)
					{
						pxIcc =  (DRV_OUT *)&new_drv.drv_data;
					}
				}                                                            
				else if(new_drv.drv_type == DRV_TIMEOUT)                            
				{    
					return(ERR_END);
				}	        
				util_Printf("\n OWE_NewDrvtts End.....\n");
			}       

			
		}
		if(pxKey->gen_status==DRV_ENDED)
		{
			Os__abort_drv(drv_icc);
			ucKey = pxKey->xxdata[1];
			if(ucKey==KEY_CLEAR) return ERR_CANCEL;
		}
		else if(pxIcc->gen_status== DRV_ENDED)
		{

			Os__abort_drv(drv_mmi);
			ucICCardType =pxIcc->xxdata[1];
			ucResetBufLen=sizeof(aucResetBuf);
			util_Printf("\nICCCproccess ucICCardtype:%d",ucICCardType);
			if(ucICCardType==0x39)
				ucICCardType=SMART_WarmReset(0,0,aucResetBuf,(unsigned short*)&ucResetBufLen);

			if(ucICCardType!=ICC_ASY)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"   (明细查询)");
				//Os__GB2312_display(2,1,(unsigned char*)"卡片不支持");
				//Os__GB2312_display(3,1,(unsigned char*)"请拔出卡");
				Os__ICC_remove();
				break;
			}
			util_Printf("\niCCCCCCC");
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"   (明细查询)");
			//Os__GB2312_display(2,0,"处理中,请稍候");
		//	EMVConfig.ucTransCurrencyExp = 0x02;
			EMVConfig.ucCAPKNums =ucTermCAPKNum;
			EMVConfig.pTermSupportCAPK =TermCAPK;
			EMVConfig.ucTermSupportAppNum = ucTermAIDNum;
			EMVConfig.pTermSupportApp = TermAID;
			EMVConfig.ucIPKRevokeNum =ucIPKRevokeNum;
			EMVConfig.pIPKRevoke =IPKRevoke;
			EMVConfig.ucExceptFileNum = ucExceptFileNum;
			EMVConfig.pExceptPAN = ExceptFile;
			EMVConfig.pfnEMVICC = EMVICC;

			EMVDATA_TermInit(&EMVConfig);
			ucResult = APPSELECT_CreateAppCandidate(&CandidateList);

			bConfirm =FALSE;
			if(CandidateList.CandidateNums ==1 &&CandidateList.CandidateApp[0].bAppPriorityExist)
			{
				if(CandidateList.CandidateApp[0].ucAppPriorityIndicator &APPNEEDCONFIRM)
					bConfirm = TRUE;
			}
			else if (CandidateList.CandidateNums >1)
				bConfirm = TRUE;

			while(!ucResult && CandidateList.CandidateNums)
			{
				ucAppIndex = 0;

				if(bConfirm)
				{
					ucResult = CardHolderConfirmApp(&CandidateList,&ucAppIndex);
				}

				if(!ucResult)
				{
					ucResult = APPSELECT_FinalSelect(ucAppIndex,&CandidateList);
					if(EMVICC_CommandResult() == CARDSWDEF_INVALIDFILE)
					{
						ucResult =EMVERROR_SUCCESS;
						break;
					}
					if(ucResult == EMVERROR_RESELECT)
					{
						ucResult =EMVERROR_SUCCESS;
						continue;
					}
					else break;
				}
			}

			if(ucResult)
			{
				EMVERROR_DisplayMsg(ucResult);
				break;
			}
			else
			{
				EMVTAG_INIT(( DATAPROPERTY*)&ICCardProperty,FALSE);
				if(!ucResult)
					ucResult = EMVTAG_Unpack(EMVIccOut.aucDataOut,EMVIccOut.uiLenOut);
				if(!ucResult)
					ucResult = EMVTRANSTAG_CopyICDataInfo(&ICDataInfo,ADFPHASETAG);
			}
			if(ucResult) break;

			uiLen =sizeof(aucEntryBuf);
			ucResult=EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x9F\x4D",aucEntryBuf,&uiLen);
			if(ucResult||uiLen!=2)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(2,1,"无交易记录");
				Os__xget_key();
				break;
			}
			ucRecordSFI=aucEntryBuf[0];
			ucRecordNums=aucEntryBuf[1];


			ucResult=EMVICC_GetData((PUCHAR)"\x9F\x4F",2);
			if(ucResult||EMVICC_CommandResult() !=CARDSWDEF_SUCCESS)
				break;

			ucDOLLen =EMVIccOut.aucDataOut[2];
			memset(aucDOL,0x00,sizeof(aucDOL));
			memcpy(aucDOL,&EMVIccOut.aucDataOut[3],ucDOLLen);
			ucRecordNum=0;

			ucResult=EMVERROR_SUCCESS;
			for(ucI=1;!ucResult&&ucI<=ucRecordNums;ucI++)
			{
				ucResult=EMVICC_ReadRecordbySFI(ucRecordSFI,ucI);
				if(!ucResult&&EMVICC_CommandResult() == CARDSWDEF_SUCCESS)
				{

					ucRecordNum++;
					pDOL=aucDOL;
					pDataOut=(unsigned char *)&EMVIccOut.aucDataOut;
					memset(aucTransDate,0x00,sizeof(aucTransDate));
					memset(aucTransTime,0x00,sizeof(aucTransTime));
					memset(aucAuthAmount,0x00,sizeof(aucAuthAmount));
					memset(aucOtherAmount,0x00,sizeof(aucOtherAmount));
					memset(aucCountryCode,0x00,sizeof(aucCountryCode));
					memset(aucTransCurcyCode,0x00,sizeof(aucTransCurcyCode));
					memset(aucMerchantName,0x00,sizeof(aucMerchantName));
					memset(aucATC,0x00,sizeof(aucATC));
					ucTransType=CASH;

					while(pDataOut<EMVIccOut.aucDataOut+EMVIccOut.uiLenOut)
					{

						ucTagLen =TagByteNums(*pDOL);

						if(!memcmp(pDOL,"\x9A",ucTagLen))
							memcpy(aucTransDate,pDataOut,*(pDOL+ucTagLen));
						else if(!memcmp(pDOL,"\x9F\x21",ucTagLen))
							memcpy(aucTransTime,pDataOut,*(pDOL+ucTagLen));
						else if(!memcmp(pDOL,"\x9F\x02",ucTagLen))
							memcpy(aucAuthAmount,pDataOut,*(pDOL+ucTagLen));
						else if(!memcmp(pDOL,"\x9F\x03",ucTagLen))
							memcpy(aucOtherAmount,pDataOut,*(pDOL+ucTagLen));
						else if(!memcmp(pDOL,"\x9F\x1A",ucTagLen))
							memcpy(aucCountryCode,pDataOut,*(pDOL+ucTagLen));
						else if(!memcmp(pDOL,"\x5F\x2A",ucTagLen))
							memcpy(aucTransCurcyCode,pDataOut,*(pDOL+ucTagLen));
						else if(!memcmp(pDOL,"\x9F\x4E",ucTagLen))
							memcpy(aucMerchantName,pDataOut,*(pDOL+ucTagLen));
						else if(!memcmp(pDOL,"\x9C",ucTagLen))
							memcpy(&ucTransType,pDataOut,*(pDOL+ucTagLen));
						else if(!memcmp(pDOL,"\x9F\x36",ucTagLen))
							memcpy(aucATC,pDataOut,*(pDOL+ucTagLen));

						pDataOut+=*(pDOL+ucTagLen);
						pDOL+=ucTagLen+1;



					}

					//Os__clr_display(255);
					memset(aucBuff,0x00,sizeof(aucBuff));
					sprintf(aucBuff,"第%d笔/共%d笔",ucI,ucRecordNums);
					//OSMMI_DisplayASCII(0x30,0,0,aucBuff);
					//Os__GB2312_display(0,0,(uchar *)aucBuff);

					memset(aucBuff,0x00,sizeof(aucBuff));
					strcpy((char*)aucBuff,"交易日期:");
					bcd_asc(aucBuff+strlen((char*)aucBuff),aucTransDate,6);
					//OSMMI_DisplayASCII(0x30,1,0,aucBuff);
					//Os__GB2312_display(1,0,(uchar *)aucBuff);

					memset(aucBuff,0x00,sizeof(aucBuff));
					strcpy((char*)aucBuff,"交易时间:");
					bcd_asc(aucBuff+strlen((char*)aucBuff),aucTransTime,6);
					//OSMMI_DisplayASCII(0x30,2,0,aucBuff);
					//Os__GB2312_display(2,0,(uchar *)aucBuff);

					memset(aucBuff,0x00,sizeof(aucBuff));
					memset(aucAmtBuf,0x00,sizeof(aucAmtBuf));
					strcpy((char *)aucBuff,(char *)"金额: ");
					util_Printf("\naucAuthAmount:");
					for(ucJ=0;ucJ<6;ucJ++)
						util_Printf("%02x ",aucAuthAmount[ucJ]);
					ulAmount=bcd_long(aucAuthAmount,12);
					util_Printf("\nulAmount =%d",ulAmount);
					UTIL_Form_Montant(&aucBuff[5],ulAmount,2);
					//Os__GB2312_display(3,0,(uchar *)aucBuff);
//					for(ucJ=0;ucJ<12;ucJ++)
//						if(aucAmtBuf[ucJ]!='0') break;
//					if(ucJ>=10)
//					{
//						memcpy(&aucBuff[9],"0.",2);
//						memcpy(&aucBuff[11],&aucAmtBuf[ucJ],12-ucJ);
//					}else
//					{
//						memcpy((char*)aucBuff+strlen((char*)aucBuff),(char*)&aucAmtBuf[ucJ],12-ucJ-2);
//						strcat((char*)aucBuff,".");
//						memcpy((char*)aucBuff+strlen((char*)aucBuff),(char*)&aucAmtBuf[10],2);
//					}
					//OSMMI_DisplayASCII(0x30,3,0,aucBuff);

//					memset(aucBuff,0x00,sizeof(aucBuff));
//
//					strcpy((char *)aucBuff,(char *)"Other Amt:");
					//OSMMI_DisplayASCII(0x30,4,0,aucBuff);

					//memset(aucBuff,0x00,sizeof(aucBuff));
					//strcpy((char *)aucBuff,(char *)"---:");
					//memset(aucAmtBuf,0x00,sizeof(aucAmtBuf));

//					bcd_asc(aucAmtBuf,aucOtherAmount,12);
//					for(ucJ=0;ucJ<12;ucJ++)
//						if(aucAmtBuf[ucJ]!='0') break;
//					if(ucJ>=10)
//					{
//					  if(ucJ==12)
//					  {
//					  	 memcpy(&aucBuff[10],"0.00",4);
//
//					  }
//					else
//					{
//						memcpy(&aucBuff[10],"0.",2);
//						memcpy(&aucBuff[12],&aucAmtBuf[ucJ],12-ucJ);
//						}
//
//
//					}else
//					{
//						memcpy((char*)aucBuff+strlen((char*)aucBuff),(char*)&aucAmtBuf[ucJ],12-ucJ-2);
//						strcat((char*)aucBuff,".");
//						memcpy((char*)aucBuff+strlen((char*)aucBuff),(char*)&aucAmtBuf[10],2);
//					}
//					OSMMI_DisplayASCII(0x30,5,0,aucBuff);

//					memset(aucBuff,0x00,sizeof(aucBuff));
//					strcpy((char*)aucBuff,"Country:");
//					bcd_asc(aucBuff+strlen((char*)aucBuff),aucCountryCode,4);
//					OSMMI_DisplayASCII(0x30,6,0,aucBuff);
//
//					memset(aucBuff,0x00,sizeof(aucBuff));
//					strcpy((char*)aucBuff,"Currency:");
//					bcd_asc(aucBuff+strlen((char*)aucBuff),aucTransCurcyCode,4);
//					//Os__display(4,0,aucBuff);
//					OSMMI_DisplayASCII(0x30,7,0,aucBuff);
//					Os__xget_key();
//					//Os__clr_display(255);
//					memset(aucBuff,0x00,sizeof(aucBuff));
//					strcpy((char*)aucBuff,"Name:");
//					memcpy(aucBuff+strlen((char*)aucBuff),aucMerchantName,16);
//					OSMMI_DisplayASCII(0x30,0,0,aucBuff);
//					memset(aucBuff,0x00,sizeof(aucBuff));
//
//					memcpy(aucBuff,&aucMerchantName[16],sizeof(aucMerchantName)-16);
//					OSMMI_DisplayASCII(0x30,1,0,aucBuff);
					Os__xget_key();
					//Os__clr_display(255);
					memset(aucBuff,0x00,sizeof(aucBuff));
					strcpy((char*)aucBuff,"交易类型:");
					hex_asc(aucBuff+strlen((char*)aucBuff),&ucTransType,2);
					//OSMMI_DisplayASCII(0x30,2,0,aucBuff);
					//Os__GB2312_display(0,0,(uchar *)aucBuff);


					////Os__clr_display(255);
					memset(aucBuff,0x00,sizeof(aucBuff));
					strcpy((char*)aucBuff,"ATC:");
					hex_asc(aucBuff+strlen((char*)aucBuff),aucATC,4);
					//Os__GB2312_display(1,0,(uchar *)aucBuff);
					//Os__GB2312_display(3,0,(uchar *)" 按任意键继续");
					//OSMMI_DisplayASCII(0x30,6,0,(unsigned char*)"Any Key To Countinue");

					Os__xget_key();
				}
			}
			if(ucRecordNum==0)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(2,0,(unsigned char *)"无交易流水");
				Os__xget_key();
			}
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(unsigned char*)"请拔出卡");
			Os__ICC_remove();
			return SUCCESS;
		}
		break;
	}
	return SUCCESS;
}
unsigned char MASAPP_WaitReadIC(unsigned char ucInputMode)
{
	unsigned char ucKey;
	unsigned char ucResult;
	unsigned int  uiTimesOut;
	unsigned char ucICCardType;
	DRV_OUT *pxIcc;
	DRV_OUT *pxKey;
	UCHAR	aucResetBuf[50],ucResetBufLen;

	NEW_DRV_TYPE  new_drv;
	uchar ucDrv;

	Os__abort_drv(drv_mmi);
	Os__abort_drv(drv_mag);
	Os__abort_drv(drv_icc);

	while(1)
	{
		MAG_DisplaySwipeCardMode(G_NORMALTRANS_ucTransType,TRANS_INPUTMODE_INSERTCARD);
		//pxKey = Os__get_key();
		//if(ucInputMode&TRANS_INPUTMODE_INSERTCARD)
		//{
		//	pxIcc = Os__ICC_insert();/*等待插卡并对卡上电*/
		//}
		ucDrv = KEY_DRV;
		if( ucInputMode&TRANS_INPUTMODE_INSERTCARD)
			ucDrv = ucDrv | ICC_DRV;
	
		util_Printf("\n1.MASAPP_WaitReadIC Os_Wait_Event begin...");
		ucResult = Os_Wait_Event(KEY_DRV| ICC_DRV |MFR_DRV, &new_drv, 1000*60);
		//util_Printf("\n2. Os_Wait_Event:%02x",ret);
		//util_Printf("\n3. new_drv.drv_type:%02x",new_drv.drv_type);
		if(ucResult == 0)                                                              
		{                                                                         
			if(new_drv.drv_type == KEY_DRV)                                     
			{                                                                   
				if(new_drv.drv_data.gen_status== DRV_ENDED){
					pxKey =  (DRV_OUT *)&new_drv.drv_data;
				}
			}else if(new_drv.drv_type == ICC_DRV)                                
			{                                                                   
				if(new_drv.drv_data.gen_status== DRV_ENDED)
				{
					pxIcc = (DRV_OUT *)& new_drv.drv_data;
				}
			}else if(new_drv.drv_type == DRV_TIMEOUT)                            
			{    
				if(!DataSave0.ConstantParamData.BackLightFlag)	Os__light_off();
				Os__light_off_pp();
				return(ERR_CANCEL);
			}	        
			util_Printf("\n OWE_NewDrvtts End.....\n");
			break;
		}   
		
		G_NORMALTRANS_ucFallBackFlag = FALSE;


		SEL_AccumulatRunTime();
/*
		ucResult = SUCCESS;
		uiTimesOut = 45;
		uiTimesOut = uiTimesOut*100;

		Os__timer_start(&uiTimesOut);
		while(uiTimesOut)
		{
			if(pxKey->gen_status == DRV_ENDED)
				break;
			if((ucInputMode&TRANS_INPUTMODE_INSERTCARD)
				&&(pxIcc->gen_status == DRV_ENDED))
				break;
		}
		Os__timer_stop(&uiTimesOut);
		if(!uiTimesOut)
		{
			Os__abort_drv(drv_mmi);
			Os__abort_drv(drv_icc);
			if(!DataSave0.ConstantParamData.BackLightFlag)
				Os__light_off();
			Os__light_off_pp();
			return(ERR_CANCEL);
		}
*/
		if((ucInputMode&TRANS_INPUTMODE_INSERTCARD)
					&&(pxIcc->gen_status== DRV_ENDED))
		{
			Os__abort_drv(drv_mmi);
			ucICCardType =pxIcc->xxdata[1];
			ucResetBufLen=sizeof(aucResetBuf);
			if(ucICCardType==0x39)
				ucICCardType=SMART_WarmReset(0,0,aucResetBuf,(unsigned short*)&ucResetBufLen);
			util_Printf("ucICCardType --1101--= %02x\n",ucICCardType);
			/*==========
			if(ucICCardType!=ICC_ASY)
			{
				return ERR_ICCARD;
			}
			=============*/
			if(ucICCardType!=ICC_ASY
				&&ucICCardType!=ICC_SYN)/*同步或异步SMART卡*/
			{
				ucInputMode&=~TRANS_INPUTMODE_INSERTCARD;
				G_NORMALTRANS_ucFallBackFlag =2;
				return ERR_FALLBACK;
			}else
			if(ucICCardType!=ICC_SYN)
			{
				G_NORMALTRANS_ucInputMode=TRANS_INPUTMODE_INSERTCARD;
				G_NORMALTRANS_euCardSpecies=CARDSPECIES_EMV;
				break;
			}
			else
			{
				ucInputMode&=~TRANS_INPUTMODE_INSERTCARD;
				G_NORMALTRANS_ucFallBackFlag =2;
				return ERR_FALLBACK;
			}
			break;
		}else if(!ucResult&&(pxKey->gen_status == DRV_ENDED))
		{
			util_Printf("exit================\n");
			Os__abort_drv(drv_icc);
			ucKey = pxKey->xxdata[1];
			if(ucKey == KEY_CLEAR)
			{
				ucResult = ERR_CANCEL;
				break;
			}
		}
	}
	util_Printf("插入卡ucResult=%02x\n",ucResult);
	return ucResult;

}
void MasApp_SetEMVTransInfo(void)
{
	UCHAR ucResult , ucBuf[256];
	USHORT uiLen;
	unsigned char ucI;

	memset( ucBuf, 0, sizeof(ucBuf));
	util_Printf("+++1AccLen:[%d]\n",G_NORMALTRANS_ucSourceAccLen);
	//从内核获取交易信息:主账号、卡序列号、2磁道等同数据
	MasApp_CARDPAN(ucBuf, &G_NORMALTRANS_ucSourceAccLen);

	{
		int i;
		util_Printf("PAN:\n");
		for (i=0;i<G_NORMALTRANS_ucSourceAccLen;i++)
		{
			util_Printf("[%02x] ",ucBuf[i]);
		}
	}
	util_Printf("\n+++2AccLen:[%d]\n",G_NORMALTRANS_ucSourceAccLen);

	asc_bcd(G_NORMALTRANS_aucSourceAcc,(G_NORMALTRANS_ucSourceAccLen+1)/2,ucBuf,((G_NORMALTRANS_ucSourceAccLen+1)/2)*2);

	asc_bcd(G_NORMALTRANS_aucCardPan,6,ucBuf+G_NORMALTRANS_ucSourceAccLen-12-1,12);

	memset( ucBuf, 0, sizeof(ucBuf));
	uiLen =1;
	if(!(ucResult = EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x5F\x34",ucBuf,&uiLen)))
	{
		G_NORMALTRANS_ucPANSeq=ucBuf[0];
		G_NORMALTRANS_ucPANSeqExist=true;
	}
	util_Printf("\nG_NORMALTRANS_ucPANSeq=abc=\n%02x",G_NORMALTRANS_ucPANSeq);
	memset( ucBuf, 0, sizeof(ucBuf));
	uiLen =37;
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x57",ucBuf,&uiLen);
	bcd_asc( G_EXTRATRANS_aucISO2, ucBuf, uiLen*2);
	memcpy(G_NORMALTRANS_aucISO2,ucBuf,uiLen);
	for(ucI=0;ucI<uiLen*2;ucI++)
	{
		util_Printf("%02x ",G_NORMALTRANS_aucISO2[ucI]);
		if(G_EXTRATRANS_aucISO2[ucI]==0x3F)
		{
			G_EXTRATRANS_aucISO2[ucI]=0x00;
			break;
		}
	}
	if (ucI>37) ucI=37;
	G_NORMALTRANS_ucISO2Len = G_EXTRATRANS_uiISO2Len= ucI;
//	asc_bcd(G_NORMALTRANS_aucISO2, G_NORMALTRANS_ucISO2Len/2,
//		G_EXTRATRANS_aucISO2,    G_NORMALTRANS_ucISO2Len);

	util_Printf("G_EXTRATRANS_uiISO2Len = %d\n",G_EXTRATRANS_uiISO2Len);

		if (G_NORMALTRANS_ucTransType==TRANS_OFFPREAUTHFINISH
		||G_NORMALTRANS_ucTransType==TRANS_TIPADJUST
		||G_NORMALTRANS_ucTransType==TRANS_TIPADJUSTOK)
	{
		uiLen = sizeof(ucBuf);
		memset( ucBuf,  0, sizeof(ucBuf));
		ucResult=EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x5F\x24",ucBuf,&uiLen);//有效期
		util_Printf("内核处理有效期返回码:%02x\n",ucResult);
		if(uiLen)
		{
			if(!memcmp(G_NORMALTRANS_ucExpiredDate,"\x00\x00\x00\x00",4))
				memcpy( G_NORMALTRANS_ucExpiredDate, ucBuf, 2);
		}
	}
}
#if SANDREADER
void MasApp_QSetEMVTransInfo(void)
{
	UCHAR ucResult , ucBuf[256];
	USHORT uiLen;
	unsigned char ucI,ucJ,uiI;
	memset( ucBuf, 0, sizeof(ucBuf));
	util_Printf("+++1AccLen:[%d]\n",G_NORMALTRANS_ucSourceAccLen);
	//从内核获取交易信息:主账号、卡序列号、2磁道等同数据
	QMasApp_CARDPAN(ucBuf, &G_NORMALTRANS_ucSourceAccLen);
	util_Printf("\nG_NORMALTRANS_ucSourceAccLen==%d",G_NORMALTRANS_ucSourceAccLen);
	{
		int i;
		util_Printf("PAN:\n");
		for (i=0;i<G_NORMALTRANS_ucSourceAccLen;i++)
		{
			util_Printf("[%02x] ",ucBuf[i]);
		}
	}
	util_Printf("\n+++2AccLen:[%d]\n",G_NORMALTRANS_ucSourceAccLen);

	asc_bcd(G_NORMALTRANS_aucSourceAcc,(G_NORMALTRANS_ucSourceAccLen+1)/2,ucBuf,((G_NORMALTRANS_ucSourceAccLen+1)/2)*2);

	asc_bcd(G_NORMALTRANS_aucCardPan,6,ucBuf+G_NORMALTRANS_ucSourceAccLen-12-1,12);

	memset( ucBuf, 0, sizeof(ucBuf));
	uiLen =1;
	if(!(ucResult = QTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x5F\x34",ucBuf,&uiLen)))
	{
		G_NORMALTRANS_ucPANSeq=ucBuf[0];
		G_NORMALTRANS_ucPANSeqExist=true;
	}
	util_Printf("\nG_NORMALTRANS_ucPANSeq=abc=%02x",G_NORMALTRANS_ucPANSeq);
	memset( ucBuf, 0, sizeof(ucBuf));
	uiLen =37;
	QTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x57",ucBuf,&uiLen);
	bcd_asc( G_EXTRATRANS_aucISO2, ucBuf, uiLen*2);
	util_Printf("G_EXTRANS_aucISO2==%d",G_EXTRATRANS_aucISO2);
	for(ucI=0;ucI<uiLen*2;ucI++)
	{
		if(G_EXTRATRANS_aucISO2[ucI]==0x3F)
		{
			G_EXTRATRANS_aucISO2[ucI]=0x00;
			break;
		}
	}
	if (ucI>37) ucI=37;

	G_NORMALTRANS_ucISO2Len = G_EXTRATRANS_uiISO2Len= ucI;
	asc_bcd(G_NORMALTRANS_aucISO2, G_NORMALTRANS_ucISO2Len/2,
		G_EXTRATRANS_aucISO2,    G_NORMALTRANS_ucISO2Len);
	if(G_NORMALTRANS_ucSourceAccLen==0)
	{
		for(ucJ=0;ucJ<G_EXTRATRANS_uiISO2Len;ucJ++)
		{
			if(G_EXTRATRANS_aucISO2[ucJ] == '=')
			{
				/*************取卡号***************/
				//---------FANGBO MODIFY DATE:2006/01/05------
				memset(G_NORMALTRANS_ucDispSourceAcc,0,sizeof(G_NORMALTRANS_ucDispSourceAcc));
				memcpy(G_NORMALTRANS_ucDispSourceAcc, G_EXTRATRANS_aucISO2, ucJ);
				util_Printf("\nG_EXTRATRANS_aucISO2 = %s\n",G_EXTRATRANS_aucISO2);
				util_Printf("G_NORMALTRANS_ucDispSourceAcc = %s\n",G_NORMALTRANS_ucDispSourceAcc);
				G_NORMALTRANS_ucDispSourceAccLen = ucJ;
				G_NORMALTRANS_ucSourceAccLen =ucJ;
				asc_bcd(G_NORMALTRANS_aucSourceAcc,(G_NORMALTRANS_ucSourceAccLen+1)/2,G_EXTRATRANS_aucISO2,((G_NORMALTRANS_ucSourceAccLen+1)/2)*2);
				asc_bcd(G_NORMALTRANS_aucCardPan,6,G_EXTRATRANS_aucISO2+G_NORMALTRANS_ucSourceAccLen-12-1,12);

				util_Printf("\nCandPan122222222");
				for(uiI=0;uiI<6;uiI++)
					util_Printf("%02x ",G_NORMALTRANS_aucCardPan[uiI]);
				//------------end----------------
				//if(ucJ>=13)
				//	asc_bcd(G_NORMALTRANS_aucCardPan,6,&G_EXTRATRANS_aucISO2[ucJ-13],12);
				//else
				//	asc_bcd(G_NORMALTRANS_aucCardPan,6,&G_EXTRATRANS_aucISO2[0],12);
				break;
			}
		}
	}

	util_Printf("G_EXTRATRANS_uiISO2Len =21 %d\n",G_EXTRATRANS_uiISO2Len);
}
#endif
unsigned char  MasApp_CopytEMVTransInfoToNormalTrans(void)
{
	unsigned char ucBuf[256],ucResult;
	USHORT uiLen,ucI;
#if 1
	//---------- AC ------------
	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	if(EMVTransInfo.enTransResult == ONLINEAPPROVED )
		EMVTRANSTAG_GetTagValue(SECONDGACPHASETAG ,(PUCHAR)"\x9F\x26",ucBuf,&uiLen);//应用密码TC
	else
	if(EMVTransInfo.enTransResult == OFFLINEAPPROVED)
		EMVTRANSTAG_GetTagValue(FIRSTGACPHASETAG ,(PUCHAR)"\x9F\x26",ucBuf,&uiLen);
	else
	if(EMVTransInfo.enTransResult == UNABLEONLINE_OFFLINEAPPROVED)
		EMVTRANSTAG_GetTagValue(SECONDGACPHASETAG ,(PUCHAR)"\x9F\x26",ucBuf,&uiLen);
	if(uiLen) memcpy( G_NORMALTRANS_AppCrypt, ucBuf, 8);
	//---------- AID -------------------
	uiLen = EMVAIDLEN-1;
	memset( ucBuf,  0, sizeof(ucBuf));
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x4F",ucBuf,&uiLen);//AID应用
	G_NORMALTRANS_ucAIDLen = uiLen;
	if(uiLen) memcpy( G_NORMALTRANS_aucAID, ucBuf, G_NORMALTRANS_ucAIDLen);
	//---------- TVR ------------
	memcpy(G_NORMALTRANS_TVR , EMVTransInfo.EMVTVR, 5);
	//---------- TSI ------------
	memcpy(G_NORMALTRANS_TSI , EMVTransInfo.EMVTSI, 5);
	//---------- ATC  ------------
	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x9F\x36",ucBuf,&uiLen);
	if(uiLen) memcpy( G_NORMALTRNAS_ATC, ucBuf, 2);
	//---------- CVM -------	-----
	memcpy(&G_NORMALTRNAS_CVM[0] , &EMVTransInfo.CVMResult.ucCVMCode, sizeof(uchar));
	memcpy(&G_NORMALTRNAS_CVM[1] , &EMVTransInfo.CVMResult.ucConditionCode, sizeof(uchar));
	memcpy(&G_NORMALTRNAS_CVM[2] , &EMVTransInfo.CVMResult.ucCVMResult, sizeof(uchar));
	//---------- App Label ------------
	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x50",ucBuf,&uiLen);//应用标签
	G_NORMALTRANS_ucAppLabelLen = uiLen;
	if(uiLen) memcpy( G_NORMALTRANS_aucAppLabel, ucBuf, 16);
	//---------- Application Prefer Name  ------------
	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x9F\x12",ucBuf,&uiLen);//应用首选名
	G_NORMALTRANS_ucAppPreferNameLen = uiLen;
	if(uiLen) memcpy( G_NORMALTRANS_aucAppPreferName, ucBuf, 16);

	memset( ucBuf,  0, sizeof(ucBuf));
	uiLen=sizeof(ucBuf);
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x89",ucBuf,&uiLen);//Auth  Code授权码

	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x5F\x24",ucBuf,&uiLen);//有效期
	if(uiLen)
	{
		if(!memcmp(G_NORMALTRANS_ucExpiredDate,"\x00\x00\x00\x00",4))
			memcpy( G_NORMALTRANS_ucExpiredDate, ucBuf, 2);
	}


	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
       ucResult=EMVTRANSTAG_GetTagValue(ALLPHASETAG ,(PUCHAR)"\x9F\x63",ucBuf,&uiLen);

       util_Printf("内核处理9f63返回码:%02x\n",ucResult);
       util_Printf("++++9f63 len=%d\n",uiLen);

       for(ucI=0;ucI<uiLen;ucI++)
           util_Printf("%02x ",ucBuf[ucI]);

       if ((ucBuf[15]&0x80)==0x80)
            G_RUNDATA_ucPTicketFlag=1;
        else
           G_RUNDATA_ucPTicketFlag=0;


	util_Printf("\n\n *********************************************** ");
	util_Printf("\n|   AUTH CODE: %02x",ucBuf[0]);
	util_Printf("\n| 	EMVTransInfo.enTransResult : %02X",EMVTransInfo.enTransResult);
	memset( ucBuf,  0, sizeof(ucBuf));
	hex_asc(ucBuf,G_NORMALTRANS_AppCrypt,sizeof(G_NORMALTRANS_AppCrypt)*2);
	util_Printf("\n| 	TC:  %s ",ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	hex_asc(ucBuf,G_NORMALTRANS_aucAID,G_NORMALTRANS_ucAIDLen*2);
	util_Printf("\n| 	AID: %s",ucBuf);
	util_Printf("\n| 	TVR: %02X %02X %02X %02X %02X",EMVTransInfo.EMVTVR[0],EMVTransInfo.EMVTVR[1],
													EMVTransInfo.EMVTVR[2],EMVTransInfo.EMVTVR[3],
													EMVTransInfo.EMVTVR[4]);
	util_Printf("\n| 	TSI: %02X %02X ",EMVTransInfo.EMVTSI[0],EMVTransInfo.EMVTSI[1]);
	util_Printf("\n| 	ATC: %02X %02X",G_NORMALTRNAS_ATC[0],G_NORMALTRNAS_ATC[1]);
	util_Printf("\n| 	CVM: %02X %02X %02X",EMVTransInfo.CVMResult.ucCVMCode,EMVTransInfo.CVMResult.ucConditionCode,EMVTransInfo.CVMResult.ucCVMResult);
	util_Printf("\n| 	App Label: %s",G_NORMALTRANS_aucAppLabel);
	util_Printf("\n| 	App Prefer Name: %s",G_NORMALTRANS_aucAppPreferName);
	util_Printf("\n|  APP uiSTLimit %d",G_NORMALTRANS_PECTransInfo.uiSTLimit);
	util_Printf("\n| 	App uiResetThreshold: %d",G_NORMALTRANS_PECTransInfo.uiResetThreshold);
	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x9F\x35",ucBuf,&uiLen);
	util_Printf("\n| 	TY: %02x %02x",ucBuf[0],ucBuf[1]);
	util_Printf("\n *********************************************** \n\n");

	if(DataSave0.ConstantParamData.uICCInfo=='0')
		return SUCCESS;

	//----- LCD DISPLAY -----
	//Os__clr_display(255);
	OSMMI_DisplayASCII(0x30,0,0, (uchar *)"--- IC Trans Info -----");

	memset( ucBuf,  0, sizeof(ucBuf));
	memcpy( ucBuf, "AID:",4 );
	hex_asc(&ucBuf[4], G_NORMALTRANS_aucAID, G_NORMALTRANS_ucAIDLen*2);
	OSMMI_DisplayASCII(0x30,1,0, ucBuf);

	memset( ucBuf,  0, sizeof(ucBuf));
	memcpy( ucBuf, "TVR:",4 );
	hex_asc(&ucBuf[4], EMVTransInfo.EMVTVR, 10);
	OSMMI_DisplayASCII(0x30,2,0, ucBuf);

	memset( ucBuf,  0, sizeof(ucBuf));
	memcpy( ucBuf, "TSI:",4 );
	hex_asc(&ucBuf[4], EMVTransInfo.EMVTSI, 4);
	OSMMI_DisplayASCII(0x30,3,0, ucBuf);

	memset( ucBuf,  0, sizeof(ucBuf));
	memcpy( ucBuf, "CVM:",4 );
	hex_asc(&ucBuf[4], EMVTransInfo.CVMResult.ucCVMCode, 2);
	hex_asc(&ucBuf[6], EMVTransInfo.CVMResult.ucConditionCode, 2);
	hex_asc(&ucBuf[8], EMVTransInfo.CVMResult.ucCVMResult, 2);
	OSMMI_DisplayASCII(0x30,4,0, ucBuf);

	memset( ucBuf,  0, sizeof(ucBuf));
	memcpy( ucBuf, "App Label:",9 );
	memcpy(&ucBuf[9] ,G_NORMALTRANS_aucAppLabel , 16);
	OSMMI_DisplayASCII(0x30,5,0, ucBuf);

	memset( ucBuf,  0, sizeof(ucBuf));
	memcpy( ucBuf, "App Prefer Name:",16 );
	OSMMI_DisplayASCII(0x30,6,0, (uchar *)"App Prefer Name:");
	memcpy(&ucBuf[0] ,G_NORMALTRANS_aucAppPreferName , 16);
	OSMMI_DisplayASCII(0x30,7,0, ucBuf);

	MSG_WaitKey(300);
	return SUCCESS;
#endif

}
#if SANDREADER
unsigned char  QMasApp_CopytEMVTransInfoToNormalTrans(void)
{
	unsigned char ucBuf[256];
	USHORT uiLen,ucI;
	unsigned char ucResult;

#if 1
	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	ucResult=QTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x9F\x5D",ucBuf,&uiLen);
	util_Printf("\nucResult=9F5D==%02x",ucResult);
	if(!ucResult)
	{
		G_NORMALTRANS_ucECTrans =TRANS_EC_PURCHASE;
	   	ECTransInfo.uiNBalance =CONV_BcdLong(ucBuf,12);
	   	memcpy((uchar *)&G_NORMALTRANS_PECTransInfo,(uchar *)&ECTransInfo,sizeof(ECTRANSINFO));
	}
	//---------- AC ------------
	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	if(QEMVTransInfo.enTransResult == ONLINEAPPROVED )
		QTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x9F\x26",ucBuf,&uiLen);//应用密码TC
	else
	if(QEMVTransInfo.enTransResult == OFFLINEAPPROVED)
		QTRANSTAG_GetTagValue(ALLPHASETAG ,(PUCHAR)"\x9F\x26",ucBuf,&uiLen);
	else
	if(QEMVTransInfo.enTransResult == UNABLEONLINE_OFFLINEAPPROVED)
		QTRANSTAG_GetTagValue(ALLPHASETAG ,(PUCHAR)"\x9F\x26",ucBuf,&uiLen);
	if(uiLen) memcpy( G_NORMALTRANS_AppCrypt, ucBuf, 8);
	//---------- AID -------------------
	uiLen = EMVAIDLEN-1;
	memset( ucBuf,  0, sizeof(ucBuf));
	QTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x4F",ucBuf,&uiLen);//AID应用
	G_NORMALTRANS_ucAIDLen = uiLen;
	if(uiLen) memcpy( G_NORMALTRANS_aucAID, ucBuf, G_NORMALTRANS_ucAIDLen);
	//---------- TVR ------------
	memcpy(G_NORMALTRANS_TVR , QEMVTransInfo.EMVTVR, 5);
	//---------- TSI ------------
	memcpy(G_NORMALTRANS_TSI ,QEMVTransInfo.EMVTSI, 5);
	//---------- ATC  ------------
	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	QTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x9F\x36",ucBuf,&uiLen);
	if(uiLen) memcpy( G_NORMALTRNAS_ATC, ucBuf, 2);
	//---------- CVM -------	-----
	memcpy(&G_NORMALTRNAS_CVM[0] , &QEMVTransInfo.CVMResult.ucCVMCode, sizeof(uchar));
	memcpy(&G_NORMALTRNAS_CVM[1] , &QEMVTransInfo.CVMResult.ucConditionCode, sizeof(uchar));
	memcpy(&G_NORMALTRNAS_CVM[2] , &QEMVTransInfo.CVMResult.ucCVMResult, sizeof(uchar));
	//---------- App Label ------------
	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	QTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x50",ucBuf,&uiLen);//应用标签
	G_NORMALTRANS_ucAppLabelLen = uiLen;
	if(uiLen) memcpy( G_NORMALTRANS_aucAppLabel, ucBuf, 16);
	//---------- Application Prefer Name  ------------
	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	QTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x9F\x12",ucBuf,&uiLen);//应用首选名
	G_NORMALTRANS_ucAppPreferNameLen = uiLen;
	if(uiLen) memcpy( G_NORMALTRANS_aucAppPreferName, ucBuf, 16);

	memset( ucBuf,  0, sizeof(ucBuf));
	uiLen=sizeof(ucBuf);
	QTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x89",ucBuf,&uiLen);//Auth  Code授权码

	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	QTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x5F\x24",ucBuf,&uiLen);//有效期
	if(uiLen)
	{
		if(!memcmp(G_NORMALTRANS_ucExpiredDate,"\x00\x00\x00\x00",4))
			memcpy( G_NORMALTRANS_ucExpiredDate, ucBuf, 2);
	}

	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
       ucResult=QTRANSTAG_GetTagValue(ALLPHASETAG ,(PUCHAR)"\x9F\x63",ucBuf,&uiLen);

       util_Printf("内核处理9f63返回码:%02x\n",ucResult);
       util_Printf("++++9f63 len=%d\n",uiLen);

       for(ucI=0;ucI<uiLen;ucI++)
           util_Printf("%02x ",ucBuf[ucI]);

       if ((ucBuf[15]&0x80)==0x80)
            G_RUNDATA_ucPTicketFlag=1;
        else
            G_RUNDATA_ucPTicketFlag=0;

	util_Printf("\n\n *********************************************** ");
	util_Printf("\n|   AUTH CODE: %02x",ucBuf[0]);
	util_Printf("\n| 	EMVTransInfo.enTransResult : %02X",QEMVTransInfo.enTransResult);
	memset( ucBuf,  0, sizeof(ucBuf));
	hex_asc(ucBuf,G_NORMALTRANS_AppCrypt,sizeof(G_NORMALTRANS_AppCrypt)*2);
	util_Printf("\n| 	TC:  %s ",ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	hex_asc(ucBuf,G_NORMALTRANS_aucAID,G_NORMALTRANS_ucAIDLen*2);
	util_Printf("\n| 	AID: %s",ucBuf);
	util_Printf("\n| 	TVR: %02X %02X %02X %02X %02X",QEMVTransInfo.EMVTVR[0],QEMVTransInfo.EMVTVR[1],
													QEMVTransInfo.EMVTVR[2],QEMVTransInfo.EMVTVR[3],
													QEMVTransInfo.EMVTVR[4]);
	util_Printf("\n| 	TSI: %02X %02X ",QEMVTransInfo.EMVTSI[0],QEMVTransInfo.EMVTSI[1]);
	util_Printf("\n| 	ATC: %02X %02X",G_NORMALTRNAS_ATC[0],G_NORMALTRNAS_ATC[1]);
	util_Printf("\n| 	CVM: %02X %02X %02X",QEMVTransInfo.CVMResult.ucCVMCode,QEMVTransInfo.CVMResult.ucConditionCode,QEMVTransInfo.CVMResult.ucCVMResult);
	util_Printf("\n| 	App Label: %s",G_NORMALTRANS_aucAppLabel);
	util_Printf("\n| 	App Prefer Name: %s",G_NORMALTRANS_aucAppPreferName);
	util_Printf("\n|  APP uiSTLimit %d",G_NORMALTRANS_PECTransInfo.uiSTLimit);
	util_Printf("\n| 	App uiResetThreshold: %d",G_NORMALTRANS_PECTransInfo.uiResetThreshold);
	uiLen = sizeof(ucBuf);
	memset( ucBuf,  0, sizeof(ucBuf));
	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x9F\x35",ucBuf,&uiLen);
	util_Printf("\n| 	TY: %02x %02x",ucBuf[0],ucBuf[1]);
	util_Printf("\n *********************************************** \n\n");

	if(DataSave0.ConstantParamData.uICCInfo=='0')
		return SUCCESS;

	//----- LCD DISPLAY -----
	//Os__clr_display(255);
	OSMMI_DisplayASCII(0x30,0,0, (uchar *)"--- IC Trans Info -----");

	memset( ucBuf,  0, sizeof(ucBuf));
	memcpy( ucBuf, "AID:",4 );
	hex_asc(&ucBuf[4], G_NORMALTRANS_aucAID, G_NORMALTRANS_ucAIDLen*2);
	OSMMI_DisplayASCII(0x30,1,0, ucBuf);

	memset( ucBuf,  0, sizeof(ucBuf));
	memcpy( ucBuf, "TVR:",4 );
	hex_asc(&ucBuf[4], QEMVTransInfo.EMVTVR, 10);
	OSMMI_DisplayASCII(0x30,2,0, ucBuf);

	memset( ucBuf,  0, sizeof(ucBuf));
	memcpy( ucBuf, "TSI:",4 );
	hex_asc(&ucBuf[4], QEMVTransInfo.EMVTSI, 4);
	OSMMI_DisplayASCII(0x30,3,0, ucBuf);

	memset( ucBuf,  0, sizeof(ucBuf));
	memcpy( ucBuf, "CVM:",4 );
	hex_asc(&ucBuf[4], QEMVTransInfo.CVMResult.ucCVMCode, 2);
	hex_asc(&ucBuf[6], QEMVTransInfo.CVMResult.ucConditionCode, 2);
	hex_asc(&ucBuf[8], QEMVTransInfo.CVMResult.ucCVMResult, 2);
	OSMMI_DisplayASCII(0x30,4,0, ucBuf);

	memset( ucBuf,  0, sizeof(ucBuf));
	memcpy( ucBuf, "App Label:",9 );
	memcpy(&ucBuf[9] ,G_NORMALTRANS_aucAppLabel , 16);
	OSMMI_DisplayASCII(0x30,5,0, ucBuf);

	memset( ucBuf,  0, sizeof(ucBuf));
	memcpy( ucBuf, "App Prefer Name:",16 );
	OSMMI_DisplayASCII(0x30,6,0, (uchar *)"App Prefer Name:");
	memcpy(&ucBuf[0] ,G_NORMALTRANS_aucAppPreferName , 16);
	OSMMI_DisplayASCII(0x30,7,0, ucBuf);

	MSG_WaitKey(300);
#endif

}
#endif
unsigned char MasApp_CARDPAN(unsigned char* pstrCardNo,unsigned char *pucCardNoLen)
{
	unsigned char aucBuff[20],aucPANBuf[10],ucI;
	unsigned short usPANBufLen;

	memset(aucBuff,0x00,sizeof(aucBuff));
	memset(aucPANBuf,0x00,sizeof(aucPANBuf));
	usPANBufLen = MAXPANDATALEN;

	EMVTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x5A",aucPANBuf,&usPANBufLen);
	util_Printf("unsigned short usPANBufLen = [%02x]\n",usPANBufLen);

	bcd_asc(aucBuff,aucPANBuf,usPANBufLen*2);
	for (ucI=0;ucI<usPANBufLen;ucI++)
	{
		util_Printf("[%02x] ",aucPANBuf[ucI]);
	}
	util_Printf("\n");

	for(ucI=0;ucI<usPANBufLen*2;ucI++)
	{
		util_Printf("[%02x] ",aucBuff[ucI]);
		if(aucBuff[ucI]==0x3F)
		{
			aucBuff[ucI]=0x00;
			break;
		}
	}
	util_Printf("\n");
	*pucCardNoLen=ucI;
	memcpy(pstrCardNo,aucBuff,ucI);
	//memcpy(pstrCardNo,aucBuff,*pucCardNoLen);
	return SUCCESS;
}
#if SANDREADER
unsigned char QMasApp_CARDPAN(unsigned char* pstrCardNo,unsigned char *pucCardNoLen)
{
	unsigned char aucBuff[20],aucPANBuf[10],ucI,ucResult;
	unsigned short usPANBufLen;

	memset(aucBuff,0x00,sizeof(aucBuff));
	memset(aucPANBuf,0x00,sizeof(aucPANBuf));
	usPANBufLen = MAXPANDATALEN;

	ucResult =QTRANSTAG_GetTagValue(ALLPHASETAG,(PUCHAR)"\x5A",aucPANBuf,&usPANBufLen);
	util_Printf("unsigned short usPANBufLen = [%02x]\n",usPANBufLen);
	if(ucResult)
		return SUCCESS;
	bcd_asc(aucBuff,aucPANBuf,usPANBufLen*2);
	for (ucI=0;ucI<usPANBufLen;ucI++)
	{
		util_Printf("[%02x] ",aucPANBuf[ucI]);
	}
	util_Printf("\n");

	for(ucI=0;ucI<usPANBufLen*2;ucI++)
	{
		util_Printf("[%02x] ",aucBuff[ucI]);
		if(aucBuff[ucI]==0x3F)
		{
			aucBuff[ucI]=0x00;
			break;
		}
	}
	util_Printf("\n");
	*pucCardNoLen=ucI;
	memcpy(pstrCardNo,aucBuff,ucI);
	//memcpy(pstrCardNo,aucBuff,*pucCardNoLen);
	return SUCCESS;
}
#endif
unsigned char EMV_DisplayAIDParam(void)
{
	unsigned char ucI,ucJ,aucBuff[50],aucAmount[15];

util_Printf("\n 1.masapp EMV_DisplayAIDParam...{0000}....Beign");

	if(!ucTermAIDNum)
	{
#ifdef GUI_PROJECT
		util_Printf("\n 1.masapp EMV_DisplayAIDParam...{0001}....POS Hava no AID Info.\n");
#else
		//Os__clr_display(255);
		//Os__GB2312_display(2,0,"POS没有AID信息");
		//Os__GB2312_display(3,0,"请手工下载!");
		Os__xget_key();
#endif		
	}
	else
	{
		for(ucI=0;ucI<ucTermAIDNum;ucI++)
		{
util_Printf("\n=========ucTermAIDNum[%d]========",ucI);
		
			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"AID:");
			hex_asc(aucBuff+strlen((char*)aucBuff),TermAID[ucI].aucAID,TermAID[ucI].ucAIDLen*2);
#ifndef GUI_PROJECT	
			//Os__clr_display(255);
			OSMMI_DisplayASCII(0x30,0,0,aucBuff);
#endif			
util_Printf("\n%s.",aucBuff);			
		    	memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"ASI:");
			char_asc(aucBuff+strlen((char*)aucBuff),2,&TermAID[ucI].ucASI);
#ifndef GUI_PROJECT	
			OSMMI_DisplayASCII(0x30,1,0,aucBuff);
#endif			
util_Printf("\n%s.",aucBuff);			
			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"APPVer:");
			hex_asc(aucBuff+strlen((char*)aucBuff),AIDParam[ucI].aucAppVer,sizeof(AIDParam[ucI].aucAppVer)*2);
#ifndef GUI_PROJECT	
			OSMMI_DisplayASCII(0x30,2,0,aucBuff);
#endif			
util_Printf("\n%s.",aucBuff);						
			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"ATCDefault:");
			hex_asc(aucBuff+strlen((char*)aucBuff),AIDParam[ucI].aucTACDefault,sizeof(AIDParam[ucI].aucTACDefault)*2);
#ifndef GUI_PROJECT	
			OSMMI_DisplayASCII(0x30,3,0,aucBuff);
#endif			
util_Printf("\n%s.",aucBuff);						
			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff," TACOnline:");
			hex_asc(aucBuff+strlen((char*)aucBuff),AIDParam[ucI].aucTACOnline,sizeof(AIDParam[ucI].aucTACOnline)*2);
#ifndef GUI_PROJECT	
			OSMMI_DisplayASCII(0x30,4,0,aucBuff);
#endif			
util_Printf("\n%s.",aucBuff);						
			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff," TACDenial:");
			hex_asc(aucBuff+strlen((char*)aucBuff),AIDParam[ucI].aucTACDenial,sizeof(AIDParam[ucI].aucTACDenial)*2);
#ifndef GUI_PROJECT	
			OSMMI_DisplayASCII(0x30,5,0,aucBuff);
#endif			
util_Printf("\n%s.",aucBuff);						
			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"Fllimit:");
			memset(aucAmount,0x00,sizeof(aucAmount));
			UTIL_Form_Montant(aucAmount,AIDParam[ucI].ulFloorLimit,2);
			ucJ=0;
			while(aucAmount[ucJ]==' ') ucJ++;
			memcpy(aucBuff+strlen((char*)aucBuff),aucAmount+ucJ,strlen((char*)aucAmount)-ucJ);
#ifndef GUI_PROJECT	
			OSMMI_DisplayASCII(0x30,6,0,aucBuff);
#endif			
util_Printf("\n%s.",aucBuff);						
			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"Thrhold:");
			memset(aucAmount,0x00,sizeof(aucAmount));
			UTIL_Form_Montant(aucAmount,AIDParam[ucI].ulThreshold,2);
			ucJ=0;
			while(aucAmount[ucJ]==' ') ucJ++;
			memcpy(aucBuff+strlen((char*)aucBuff),aucAmount+ucJ,strlen((char*)aucAmount)-ucJ);
#ifndef GUI_PROJECT	
			OSMMI_DisplayASCII(0x30,7,0,aucBuff);			
			Os__xget_key();
#endif			
util_Printf("\n%s.",aucBuff);			

			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"MaxPercent:");
			util_Printf("\nAIDParam[%d].ucMaxTargetPercent %d",ucI,AIDParam[ucI].ucMaxTargetPercent);
			bcd_asc(aucBuff+strlen((char*)aucBuff),&AIDParam[ucI].ucMaxTargetPercent,sizeof(AIDParam[ucI].ucMaxTargetPercent)*2);
			//char_asc(aucBuff+strlen((char*)aucBuff),2,&AIDParam[ucI].ucMaxTargetPercent);
#ifndef GUI_PROJECT	
			//Os__clr_display(255);
			OSMMI_DisplayASCII(0x30,0,0,aucBuff);
#endif			
util_Printf("\n%s.",aucBuff);						
			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"Percent:");
			bcd_asc(aucBuff+strlen((char*)aucBuff),&AIDParam[ucI].ucTargetPercent,sizeof(AIDParam[ucI].ucTargetPercent)*2);
			//char_asc(aucBuff+strlen((char*)aucBuff),2,&AIDParam[ucI].ucTargetPercent);
#ifndef GUI_PROJECT	
			OSMMI_DisplayASCII(0x30,1,0,aucBuff);
#endif			
util_Printf("\n%s.",aucBuff);				
			util_Printf("\n---AIDParam[ucI].ucTargetPercent %d",AIDParam[ucI].ucTargetPercent);
			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"DDOL:");
			hex_asc(aucBuff+strlen((char*)aucBuff),AIDParam[ucI].aucDDOL,AIDParam[ucI].ucDDOLLen*2);
#ifndef GUI_PROJECT	
			OSMMI_DisplayASCII(0x30,2,0,aucBuff);
#endif			
util_Printf("\n%s.",aucBuff);						
			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"PINCapab:");
			char_asc(aucBuff+strlen((char*)aucBuff),1,&AIDParam[ucI].ucOnlinePINCapab);
#ifndef GUI_PROJECT	
			OSMMI_DisplayASCII(0x30,3,0,aucBuff);
#endif			
util_Printf("\n%s.",aucBuff);			
	        ////2009-9-7 20:02cbf
			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"ECFloorLimit:");
			memset(aucAmount,0x00,sizeof(aucAmount));
			UTIL_Form_Montant(aucAmount,AIDParam[ucI].ulTelECFloorLimit,2);
			ucJ=0;
			while(aucAmount[ucJ]==' ') ucJ++;
			memcpy(aucBuff+strlen((char*)aucBuff),aucAmount+ucJ,strlen((char*)aucAmount)-ucJ);
#ifndef GUI_PROJECT	
			OSMMI_DisplayASCII(0x30,4,0,aucBuff);
#endif			
util_Printf("\n%s.",aucBuff);			
			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"UTFloorLimit:");
			memset(aucAmount,0x00,sizeof(aucAmount));
			UTIL_Form_Montant(aucAmount,AIDParam[ucI].ulUnTouchFloorLimit,2);
			ucJ=0;
			while(aucAmount[ucJ]==' ') ucJ++;
			memcpy(aucBuff+strlen((char*)aucBuff),aucAmount+ucJ,strlen((char*)aucAmount)-ucJ);
#ifndef GUI_PROJECT	
			OSMMI_DisplayASCII(0x30,5,0,aucBuff);
#endif			
util_Printf("\n%s.",aucBuff);			
			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"UTTransLimit:");
			memset(aucAmount,0x00,sizeof(aucAmount));
			UTIL_Form_Montant(aucAmount,AIDParam[ucI].ulUnTouchTransLimit,2);
			ucJ=0;
			while(aucAmount[ucJ]==' ') ucJ++;
			memcpy(aucBuff+strlen((char*)aucBuff),aucAmount+ucJ,strlen((char*)aucAmount)-ucJ);
#ifndef GUI_PROJECT	
			OSMMI_DisplayASCII(0x30,6,0,aucBuff);
#endif			
util_Printf("\n%s.",aucBuff);			
			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"CVMLimit:");
			memset(aucAmount,0x00,sizeof(aucAmount));
			UTIL_Form_Montant(aucAmount,AIDParam[ucI].ulTelCVMLimit,2);
			ucJ=0;
			while(aucAmount[ucJ]==' ') ucJ++;
			memcpy(aucBuff+strlen((char*)aucBuff),aucAmount+ucJ,strlen((char*)aucAmount)-ucJ);
#ifndef GUI_PROJECT	
			OSMMI_DisplayASCII(0x30,7,0,aucBuff);
	 		////End
			if(Os__xget_key()==KEY_CLEAR) break;
#endif			
util_Printf("\n%s.\n",aucBuff);

		}

util_Printf("\n=========ucTermAIDNum[%d]========End.\n",ucI);
		
	}
	return OK;
}


unsigned char EMV_DisplayPKInfo(void)
{
	unsigned char ucI,ucJ;
	unsigned char aucBuff[50];

util_Printf("\n 1.masapp EMV_DisplayPKInfo...{0000}....Beign");

	if(!ucTermCAPKNum)
	{
#ifdef GUI_PROJECT	
util_Printf("\n 1.masapp EMV_DisplayPKInfo...{0000}....POS hava no PubKey.\n");
#else
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,"POS没有公钥信息");
		//Os__GB2312_display(2,0,"请手工下载!");
		Os__xget_key();
#endif			
	
		return SUCCESS;
	}
	else
	{
		if(ucTermCAPKNum >=MAXTERMCAPKNUMS)
		{
#ifdef GUI_PROJECT	
util_Printf("\n 1.masapp EMV_DisplayPKInfo...{0000}....POS hava OverNo. PubKey.\n");
#else		
			//Os__clr_display(255);
			//Os__GB2312_display(1,0," 公钥数量过大");
			//Os__GB2312_display(2,0," 怀疑数据出错");
			Os__xget_key();
			return SUCCESS;
#endif
		}
		for(ucI=0;ucI<ucTermCAPKNum;ucI++)
		{
util_Printf("\n=========ucTermCAPKNum[%d]========Being.",ucI);

			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"RID:");
			hex_asc(aucBuff+strlen((char*)aucBuff),TermCAPK[ucI].aucRID,sizeof(TermCAPK[ucI].aucRID)*2);
#ifndef GUI_PROJECT	
			//Os__clr_display(255);
			OSMMI_DisplayASCII(0x30, 0,0, aucBuff);
#endif			
util_Printf("\n%s.",aucBuff);			


			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"CAPKI:");
			hex_asc(aucBuff+strlen((char*)aucBuff),&TermCAPK[ucI].ucCAPKI,sizeof(TermCAPK[ucI].ucCAPKI)*2);
#ifndef GUI_PROJECT	
			OSMMI_DisplayASCII(0x30,1,0,aucBuff);
#endif			
util_Printf("\n%s.",aucBuff);			

			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"EXP:");
			hex_asc(aucBuff+strlen((char*)aucBuff),TermCAPK[ucI].aucExponent,TermCAPK[ucI].ucExponentLen*2);
#ifndef GUI_PROJECT	
			OSMMI_DisplayASCII(0x30,2,0,aucBuff);
#endif			
util_Printf("\n%s.",aucBuff);			

			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"EXPIRE:");
			memcpy(aucBuff+strlen((char*)aucBuff),TermCAPK[ucI].aucExpireDate,sizeof(TermCAPK[ucI].aucExpireDate));
#ifndef GUI_PROJECT	
			OSMMI_DisplayASCII(0x30,3,0,aucBuff);
#endif			
util_Printf("\n%s.",aucBuff);			

			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"MOUDLEN:");
			char_asc(aucBuff+strlen((char*)aucBuff),3,(unsigned char*)&TermCAPK[ucI].ucModulLen);
#ifndef GUI_PROJECT	
			OSMMI_DisplayASCII(0x30,4,0,aucBuff);
#endif			
util_Printf("\n%s.",aucBuff);			

			for(ucJ=0;ucJ<3;ucJ++)
			{
				memset(aucBuff,0x00,sizeof(aucBuff));
				hex_asc(aucBuff,(unsigned char*)TermCAPK[ucI].aucModul+ucJ*10,20);
#ifndef GUI_PROJECT	
				OSMMI_DisplayASCII(0x30,5+ucJ,0,aucBuff);
#endif			
util_Printf("\n%s.",aucBuff);			

			}
util_Printf("\n");			

#ifndef GUI_PROJECT	
			if(Os__xget_key()==KEY_CLEAR) break;
#endif		
		}

util_Printf("\n=========ucTermCAPKNum[%d]========End.\n",ucI);		

	}
	return SUCCESS;

}
unsigned char ReadICBalance(void)
{
	DRV_OUT *pxIcc;
	DRV_OUT *pxKey;
	UCHAR	aucResetBuf[50],ucAppIndex,ucResult=SUCCESS,ucKey;
	USHORT	uiLen;
	UCHAR	bConfirm;
	UCHAR	aucBuff[30],ucICCardType,ucResetBufLen,ucBuf[16];
	unsigned char ucInputType;

	#if SANDREADER
	CANDIDATELIST	CandidateList;
	QCANDIDATELIST	QCandidateList;
	#endif
	UINT	uiAmount,Timeout;
	DRV_OUT  aucDRVOut;
	DRV_OUT *pxMifare;

	NEW_DRV_TYPE  new_drv;

	while(1)
	{
		util_Printf("\nstart balance=============");
		ucInputType=0x06;
		memset(&aucDRVOut,0,sizeof(DRV_OUT));

		if(ReaderSupport&&(ReaderType==READER_HONGBAO))
		{
			pxKey->gen_status =DRV_STOPPED;
			pxIcc->gen_status =DRV_STOPPED;
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"   (余额查询)");
			//Os__GB2312_display(2,0,"请插IC卡或挥卡");
			memset((unsigned char *)&InterfaceTransData,0,sizeof(InterfaceTransData));
			INTERFACE_TransToReader(0,&InterfaceTransData,&aucDRVOut,&ucInputType);
			util_Printf("\nRead返回刷卡模式ucInputType= %02x\n",ucInputType);

	    if(((ucInputType== 0x00)||(ucInputType== 0x04)||(ucInputType== 0x07))
	       &&(InterfaceTransData.unTransResult!=SUCCESS))//IC卡和磁卡不要判断
    	{
                ucResult = InterfaceTransData.unTransResult;
                util_Printf("非接卡器返回ucResult11= %02x\n",ucResult);
                //Os__clr_display(255);
                //Os__GB2312_display(0,0,(uchar *)"  (余额查询)");
                //Os__GB2312_display(2,0,(uchar *)"    读卡错！");
                INTERFACE_RefurbishReaderLCD(4);
                UTIL_WaitGetKey(3);
                return SUCCESS;
    	}
    	else
    	{
    		//BIT 0  IC;  BIT 1  mag ; BIT 2 interface
          	util_Printf("\nINTERFACE_TransToReader ucInputType=%02x",ucInputType);
          	if(ucInputType == 0x01)	     //IC卡
          	{
            	pxIcc = &aucDRVOut;
          		pxIcc->gen_status = DRV_ENDED;
          	}
          	else if(ucInputType == 0x04) //非接卡
          	{
                        memset(ucBuf,0,sizeof(ucBuf));
                        ucResult=INTERFACE_GetTransTagValue((PUCHAR)"\x9F\x5D",ucBuf,&uiLen);
    			util_Printf("\nucResult=9F5D==%d",ucResult);
    			if(!ucResult)
    			{
    			       //INTERFACE_RefurbishReaderLCD(4);//交易完成
    			   	uiAmount =CONV_BcdLong(ucBuf,12);
    			   	sprintf(aucBuff,"     %d.%02d",uiAmount/100,uiAmount%100);
    			   	if(DataSave0.ConstantParamData.Pinpadflag ==1)
    			   	{
    					//Os__clr_display(255);
    					//Os__GB2312_display(1,0,(unsigned char *)"    查询成功");
    					//Os__GB2312_display(2,0,(unsigned char *)" 请查看密码键盘");
    					Os__clr_display_pp(255);
    					Os__GB2312_display_pp(0,0,(uchar *)"余额(Balance):");
    					Os__display_pp(1,0,(unsigned char *)aucBuff);
    			   	}
    			   	else
    			   	{
    					//Os__clr_display(255);
    					//Os__GB2312_display(0,0,(uchar *)"  (余额查询)");
    					//Os__GB2312_display(1,0,"卡片余额:");
    					//Os__GB2312_display(2,0,aucBuff);
    		               }
    			        UTIL_WaitGetKey(5);
    				return SUCCESS;
    			}
    			else
    			{
    				//Os__clr_display(255);
    				//Os__GB2312_display(0,0,(uchar *)"  (余额查询)");
    				//Os__GB2312_display(2,0,(uchar *)"    读卡错！");
    				UTIL_WaitGetKey(3);
    				INTERFACE_RefurbishReaderLCD(4);//交易完成
    				return SUCCESS;
    			}
        	}
        	else
        	{
                	//Os__clr_display(255);
                	//Os__GB2312_display(0,0,(uchar *)"  (余额查询)");
                	//Os__GB2312_display(2,0,(uchar *)"    读卡错！");
                	UTIL_WaitGetKey(3);
                	INTERFACE_RefurbishReaderLCD(4);//交易完成
                	return SUCCESS;
        	}
   	  }
			INTERFACE_RefurbishReaderLCD(4);/*交易完成*/
		}
		#if SANDREADER
		else if(ReaderSupport&&(ReaderType==READER_SAND))
		{
			//pxIcc = Os__ICC_insert();
			//pxKey = Os__get_key();
			//pxMifare = Os__MIFARE_Polling();
		util_Printf("\n1.ReadICBalance Os_Wait_Event begin...");
			ucResult = Os_Wait_Event(KEY_DRV| ICC_DRV |MFR_DRV, &new_drv, 1000*60);
			//util_Printf("\n2. Os_Wait_Event:%02x",ret);
			//util_Printf("\n3. new_drv.drv_type:%02x",new_drv.drv_type);
			if(ucResult == 0)                                                              
			{                                                                         
				if(new_drv.drv_type == KEY_DRV)                                     
				{                                                                   
					if(new_drv.drv_data.gen_status== DRV_ENDED){
						pxKey =  (DRV_OUT *)&new_drv.drv_data;
					}
				}                                                                   
				else if(new_drv.drv_type == ICC_DRV)                                
				{                                                                   
					if(new_drv.drv_data.gen_status== DRV_ENDED)
					{
						pxIcc =  (DRV_OUT *)&new_drv.drv_data;
					}
				}                                                                   
				else if(new_drv.drv_type == MFR_DRV)                                
				{                                                                   
					if(new_drv.drv_data.gen_status== DRV_ENDED)
					{
						pxMifare =  (DRV_OUT *)&new_drv.drv_data;
					}
				}                                                                   
				else if(new_drv.drv_type == DRV_TIMEOUT)                            
				{    
					return ERR_CANCEL;			
				}	        
				util_Printf("\n OWE_NewDrvtts End.....\n");
				break;
			}       
						
			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"   (余额查询)");
			//Os__GB2312_display(2,0,"请插IC卡或挥卡");
			#if WKPINPAD
			while((pxMifare->gen_status!= DRV_ENDED)&&(pxIcc->gen_status != DRV_ENDED)&&(pxKey->gen_status != DRV_ENDED));
			#else
			ucResult =DIGITAL_OpenSerial();
			if(ucResult)
			{
				DIGITAL_PolingAbort();
				DIGITAL_ComClose();
				//Os__clr_display(255);
				//Os__GB2312_display(1,0,(uchar *)"打开串口错");
				Os__xget_key();
				return ERR_CANCEL;
			}
			DIGITAL_Poling();
			while(((ucResult=DIGITAL_ReceiveByte(&ucByte,1))!=SUCCESS)&&(pxIcc->gen_status != DRV_ENDED)&&(pxKey->gen_status != DRV_ENDED));
			#endif

			
		}
		#endif
		else
		{
			util_Printf("\nonly ICCCCCCCCCCCCC");

			/*
			Timeout=60*100;
			Os__light_on();
			Os__timer_start(&Timeout);
			pxIcc = Os__ICC_insert();
			pxKey = Os__get_key();

			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"   (余额查询)");
			//Os__GB2312_display(2,2,"请插IC卡");
			while((Timeout!=0)&&(pxIcc->gen_status != DRV_ENDED)
			      &&(pxKey->gen_status != DRV_ENDED));
			Os__timer_stop(&Timeout);
			if(Timeout==0)
			{
				Os__abort_drv(drv_icc);
				Os__abort_drv(drv_mmi);
				return ERR_CANCEL;
			}*/
			util_Printf("\n2.ReadICBalance Os_Wait_Event begin...");
			ucResult = Os_Wait_Event(KEY_DRV| ICC_DRV , &new_drv, 1000*60);
			//util_Printf("\n2. Os_Wait_Event:%02x",ret);
			//util_Printf("\n3. new_drv.drv_type:%02x",new_drv.drv_type);
			if(ucResult == 0)                                                              
			{                                                                         
				if(new_drv.drv_type == KEY_DRV)                                     
				{                                                                   
					if(new_drv.drv_data.gen_status== DRV_ENDED){
						pxKey = (DRV_OUT *)& new_drv.drv_data;
					}
				}                                                                   
				else if(new_drv.drv_type == ICC_DRV)                                
				{                                                                   
					if(new_drv.drv_data.gen_status== DRV_ENDED)
					{
						pxIcc = (DRV_OUT *)& new_drv.drv_data;
					}
				}                                                                   
				else if(new_drv.drv_type == DRV_TIMEOUT)                            
				{    
					return ERR_CANCEL;			
				}	        
				util_Printf("\n OWE_NewDrvtts End.....\n");
				break;
			}       
			
		}
		if(pxKey->gen_status==DRV_ENDED)
		{
			Os__abort_drv(drv_icc);
			ucKey = pxKey->xxdata[1];
			if(ucKey==KEY_CLEAR) return ERR_CANCEL;
		}
		if(pxIcc->gen_status== DRV_ENDED)
		{
			util_Printf("\nICCCCCCCCCProccess");
			Os__abort_drv(drv_mmi);
			ucICCardType =pxIcc->xxdata[1];
			ucResetBufLen=sizeof(aucResetBuf);
			if(ucICCardType==0x39)
				ucICCardType=SMART_WarmReset(0,0,aucResetBuf,(unsigned short*)&ucResetBufLen);

			if(ucICCardType!=ICC_ASY)
			{
				//Os__clr_display(255);
				//Os__GB2312_display(0,0,(uchar *)"  (余额查询)");
				//Os__GB2312_display(2,1,(unsigned char*)"卡片不支持");
				//Os__GB2312_display(3,2,(unsigned char*)"请拔出卡");
				Os__ICC_remove();
				return SUCCESS;
			}

			//Os__clr_display(255);
			//Os__GB2312_display(0,0,(uchar *)"   (余额查询)");
			//Os__GB2312_display(2,0," 读卡数据,请稍后");
			EMVConfig.ucCAPKNums =ucTermCAPKNum;
			EMVConfig.pTermSupportCAPK =TermCAPK;
			EMVConfig.ucTermSupportAppNum = ucTermAIDNum;
			EMVConfig.pTermSupportApp = TermAID;
			EMVConfig.ucIPKRevokeNum =ucIPKRevokeNum;
			EMVConfig.pIPKRevoke =IPKRevoke;
			EMVConfig.ucExceptFileNum = ucExceptFileNum;
			EMVConfig.pExceptPAN = ExceptFile;
			EMVConfig.pfnEMVICC = EMVICC;

			EMVDATA_TermInit(&EMVConfig);
			memset(&CandidateList,0,sizeof(CANDIDATELIST));	  //2009-9-10 Add By Wangan
			ucResult = APPSELECT_CreateAppCandidate(&CandidateList);

			bConfirm =FALSE;
			if(CandidateList.CandidateNums ==1 &&CandidateList.CandidateApp[0].bAppPriorityExist)
			{
				if(CandidateList.CandidateApp[0].ucAppPriorityIndicator &APPNEEDCONFIRM)
					bConfirm = TRUE;
			}
			else if (CandidateList.CandidateNums >1)
				bConfirm = TRUE;

			while(!ucResult && CandidateList.CandidateNums)
			{
				ucAppIndex = 0;

				if(bConfirm)
				{
					ucResult = CardHolderConfirmApp(&CandidateList,&ucAppIndex);
				}

				if(!ucResult)
				{
					ucResult = APPSELECT_FinalSelect(ucAppIndex,&CandidateList);
					if(EMVICC_CommandResult() == CARDSWDEF_INVALIDFILE)
					{
						ucResult =EMVERROR_SUCCESS;
						break;
					}
					if(ucResult == EMVERROR_RESELECT)
					{
						ucResult =EMVERROR_SUCCESS;
						continue;
					}
					else break;
				}
			}

			if(ucResult)
			{
				EMVERROR_DisplayMsg(ucResult);
				break;
			}
			util_Printf("\nECBalance111=%d",ucResult);
			if(ucResult) break;

			ucResult =EMVICC_GetData((PUCHAR)"\x9F\x79",2);
			if(ucResult)
				break;
			ucResult = EMVICC_CommandResult();
			if(ucResult)
				break;
			uiAmount =CONV_BcdLong(EMVIccOut.aucDataOut,EMVIccOut.uiLenOut*2);
			sprintf(aucBuff,"     %d.%02d",uiAmount/100,uiAmount%100);
                    if(DataSave0.ConstantParamData.Pinpadflag ==1)
                    {
                        //Os__clr_display(255);
                        //Os__GB2312_display(1,0,(unsigned char *)"    查询成功");
                        //Os__GB2312_display(2,0,(unsigned char *)" 请查看密码键盘");
                        Os__clr_display_pp(255);
                        Os__GB2312_display_pp(0,0,(uchar *)"余额(Balance):");
                        Os__display_pp(1,0,(unsigned char *)aucBuff);
                    }
                    else
                    {
        			//Os__clr_display(255);
        			//Os__GB2312_display(0,0,(uchar *)"  (余额查询)");
        			//Os__GB2312_display(1,0,"卡片余额:");
        			//Os__GB2312_display(2,0,aucBuff);
        			//Os__GB2312_display(3,0,(uchar *)"     请拔卡");
        			Os__ICC_remove();
		        }
		        UTIL_WaitGetKey(5);
			break;
		}
		#if SANDREADER
		if(pxMifare->gen_status == DRV_ENDED)
		{
			util_Printf("\nQQQQQQICCCCCCCCCProccess");
			Os__abort_drv(drv_mmi);
			Os__abort_drv(drv_icc);
			if(pxMifare->xxdata[0] ==0x00)
			{
				ucResult = Os__MIFARE_Active();
				util_Printf("\nucResult :=%d",ucResult);
				if(ucResult==SUCCESS)
				{
					//Os__clr_display(255);
					//Os__GB2312_display(0,0,(uchar *)"   (余额查询)");
					//Os__GB2312_display(2,0," 读卡数据,请稍后");
					QEMVConfig.ucCAPKNums =ucTermCAPKNum;
					QEMVConfig.pTermSupportCAPK =(PQCAPK)TermCAPK;
					QEMVConfig.ucTermSupportAppNum = ucTermAIDNum;
					QEMVConfig.pTermSupportApp = QTemAID;
					QEMVConfig.ucQIPKRevokeNum =ucIPKRevokeNum;
					QEMVConfig.pIPKRevoke =(PQIPKREVOKE)IPKRevoke;
					QEMVConfig.ucQExceptFileNum = ucExceptFileNum;
					QEMVConfig.pExceptPAN = (PQEXCEPTPAN)ExceptFile;
					#if WKPINPAD
					QEMVConfig.pfnQICC = QEMVICC;
					#else
					QEMVConfig.pfnQICC = QICC;
					#endif
					QDATA_TermInit(&QEMVConfig);
					memset(&CandidateList,0,sizeof(CANDIDATELIST));
					ucResult = QAPPSELECT_CreateAppCandidate(&QCandidateList);

					bConfirm =FALSE;
					if(QCandidateList.CandidateNums ==1 &&QCandidateList.CandidateApp[0].bAppPriorityExist)
					{
						if(QCandidateList.CandidateApp[0].ucAppPriorityIndicator &APPNEEDCONFIRM)
							bConfirm = TRUE;
					}
					else if (QCandidateList.CandidateNums >1)
						bConfirm = TRUE;

					while(!ucResult && QCandidateList.CandidateNums)
					{
						ucAppIndex = 0;

		//				if(bConfirm)
		//				{
		//					ucResult = QCardHolderConfirmApp(&QCandidateList,&ucAppIndex);
		//				}

						if(!ucResult)
						{
							ucResult = QAPPSELECT_FinalSelect(ucAppIndex,&QCandidateList);
		//					if(QICC_CommandResult() == CARDSWDEF_INVALIDFILE)
		//					{
		//						ucResult =EMVERROR_SUCCESS;
		//						break;
		//					}
							if(ucResult == EMVERROR_RESELECT)
							{
								ucResult =EMVERROR_SUCCESS;
								continue;
							}
							else break;
						}
					}

					if(ucResult)
					{
						//QERROR_DisplayMsg(ucResult);
						break;
					}
					else
					{
						/*
						EMVTAG_INIT(( DATAPROPERTY*)&ICCardProperty,FALSE);
						if(!ucResult)
							ucResult = EMVTAG_Unpack(EMVIccOut.aucDataOut,EMVIccOut.uiLenOut);
						if(!ucResult)
							ucResult = EMVTRANSTAG_CopyICDataInfo(&ICDataInfo,ADFPHASETAG);
						*/
					}
					util_Printf("\nECBalance111=%d",ucResult);
					if(ucResult) break;

					ucResult =QICC_GetData((PUCHAR)"\x9F\x79",2);
					if(ucResult)
						break;
					ucResult = QICC_CommandResult();
					if(ucResult)
						break;
					uiAmount =CONV_BcdLong(QIccOut.aucDataOut,QIccOut.uiLenOut*2);
					sprintf(aucBuff,"     %d.%02d",uiAmount/100,uiAmount%100);
					util_Printf("\nuiAmount::=%d",uiAmount);
					util_Printf("\nDataSave0.ConstantParamData.Pinpadflag:%d",DataSave0.ConstantParamData.Pinpadflag);
          if(DataSave0.ConstantParamData.Pinpadflag ==1)
          {
              //Os__clr_display(255);
              //Os__GB2312_display(1,0,(unsigned char *)"    查询成功");
              //Os__GB2312_display(2,0,(unsigned char *)" 请查看密码键盘");
              Os__clr_display_pp(255);
              Os__GB2312_display_pp(0,0,(uchar *)"余额(Balance):");
              Os__display_pp(1,0,(unsigned char *)aucBuff);
          }
          else
          {
      			//Os__clr_display(255);
      			//Os__GB2312_display(0,0,(uchar *)"  (余额查询)");
      			//Os__GB2312_display(1,0,"卡片余额:");
      			//Os__GB2312_display(2,0,aucBuff);
				  }
				  UTIL_WaitGetKey(5);
				}
				else continue;
				util_Printf("\nucResult:%d",ucResult);
				if( !ucResult )
				{
					//Os__clr_display(255);
					//Os__GB2312_display(2,2,(uchar *)"请移卡...");
					ucResult = Os__MIFARE_Remove();
				}
				util_Printf("\nucResult:%d",ucResult);
				Os__MIFARE_PowerRF(0);

			}
			else
				continue;

			break;
		}
		else
		{
			Os__abort_drv(drv_mmi);
			Os__abort_drv(drv_icc);
			return ucResult;
		}
		#endif
		break;
	}
	util_Printf("\n ========end balance=========:%d",ucResult);
	if(ucResult)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(0,0,(uchar *)"  (余额查询)");
		//Os__GB2312_display(2,0,(PUCHAR)"卡未被接收");
		MSG_WaitKey(3);
	}
	return SUCCESS;
}

unsigned char EMV_DisplayReaderPKInfo(void)
{
	unsigned char ucResult=SUCCESS,ucI,ucJ,ucDispLine;
	unsigned char aucBuff[50];
	unsigned char ucRIDNumber=0;
	INTERFACEPUBLICPKLIST ReaderRID[10];
 	ucDispLine =0;

	ucResult = INTERFACE_GetListPublicKeyProcess(&ucRIDNumber,ReaderRID);
	util_Printf("\nINTERFACE_GetListPublicKeyProcess ucRIDNumber=%d\n",ucRIDNumber);
	if( ucResult != SUCCESS)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,"取读卡器公钥失败");
		Os__xget_key();
		return	ucResult;
	}

	if(!ucRIDNumber)
	{
		//Os__clr_display(255);
		//Os__GB2312_display(2,0,"Reader没有公钥");
		//Os__GB2312_display(3,0,"信息!");
		Os__xget_key();
	}
	else
	{
		for(ucI=0;ucI<ucRIDNumber;ucI++)
		{
			//Os__clr_display(255);
			memset(aucBuff,0x00,sizeof(aucBuff));
			strcpy((char*)aucBuff,"RID:");
			hex_asc(aucBuff+strlen((char*)aucBuff),ReaderRID[ucI].aucRID,sizeof(ReaderRID[ucI].aucRID)*2);
			OSMMI_DisplayASCII(0x30, 0,0, aucBuff);

			if(ReaderRID[ucI].ucPKNumber<=7)
			{
				for(ucJ=0;ucJ<ReaderRID[ucI].ucPKNumber;ucJ++)
				{
					memset(aucBuff,0x00,sizeof(aucBuff));
					strcpy((char*)aucBuff,"CAPKI:");
					hex_asc(aucBuff+strlen((char*)aucBuff),&ReaderRID[ucI].aucPKIndex[ucJ],2);
					OSMMI_DisplayASCII(0x30,1+ucJ,0,aucBuff);
				}
			}
			else
			{
				for(ucJ=0;ucJ<ReaderRID[ucI].ucPKNumber;ucJ++)
				{
					memset(aucBuff,0x00,sizeof(aucBuff));
					strcpy((char*)aucBuff,"CAPKI:");
					hex_asc(aucBuff+strlen((char*)aucBuff),&ReaderRID[ucI].aucPKIndex[ucJ],2);
					if(ucJ%2)
					{
						OSMMI_DisplayASCII(0x30,1+ucDispLine,10,aucBuff);
						ucDispLine++;
					}
					else
						OSMMI_DisplayASCII(0x30,1+ucDispLine,0,aucBuff);
				}
			}
			if(Os__xget_key()==KEY_CLEAR)
				break;
		}
	}
	return ucResult;

}
unsigned char EMV_GetTagValueFromStr(uchar *aucTagStr,UINT uiTagStrLen,uchar *aucTag,uchar *aucTagValue,unsigned short* uiTagValueLen)
{
	uchar *pTag,ucTagLen,ucResult =ERR_END;

	pTag=(unsigned char *)aucTagStr;
	#if 0
	util_Printf("\nuiTagStrLen=%d\n",uiTagStrLen);
	for(i=0;i<uiTagStrLen;i++)
		 util_Printf("%02x ",*(aucTagStr+i));
	util_Printf("\naucTag1:%02x,%02x",*aucTag,*(aucTag+1));
	#endif
	while(pTag<aucTagStr+uiTagStrLen)
	{
		ucTagLen =TagByteNums(*pTag);
//		if((*(pTag+ucTagLen))&0x80)
//		{
//			ucLenLen =(*(pTag+ucTagLen))&0x7F;
//			*uiTagValueLen =tab_long(pTag+ucTagLen+1,ucLenLen);
//		}
//		else
//			*uiTagValueLen =*(pTag+ucTagLen);
		if(!memcmp(pTag,aucTag,ucTagLen))
		{
			*uiTagValueLen =*(pTag+ucTagLen);
			memcpy(aucTagValue,pTag+ucTagLen+1,*uiTagValueLen);
			ucResult =SUCCESS;
			break;
		}
		else
		{
			pTag+=*(pTag+ucTagLen)+ucTagLen+1;
		}
	}
	return ucResult;
}





