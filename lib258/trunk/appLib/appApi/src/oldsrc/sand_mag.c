/*
s3c2410 mag API实现
0731 支持mmap映射及直接访问方式，对应驱动版本为v1.1

*/
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <asm/fcntl.h>
#include <linux/ioctl.h>
#include <string.h>
#include "osdriver.h"
#include <sys/mman.h>

#include "s3c2410_mag.h"

int fd_mag;

#if 1
#define USE_MMAP
DRV_OUT *magdata;

#else
#define USE_DIRECT_WR
#define MAG_DIRACESS _IOWR(S3C2410_MAG_IOCTL_BASE, 3, DRV_OUT) //读取刷卡状态
DRV_OUT magdata;

#endif


int Os_open_mag_fd()
{
        fd_mag = open("/dev/s3c2410_mag", O_RDWR);
        if (fd_mag<0) {
                Uart_Printf("open s3c2410_mag failed!\n");
                return -1;
        }
        return fd_mag;
}

int Os_close_mag_fd()
{
        if (fd_mag) close(fd_mag);
        return 0;
}


/*
说明：本函数不阻塞，在获取磁卡状态后即退出

struct  sdrv_out
{
    unsigned char   reqnr;
    volatile unsigned char   gen_status;
    unsigned char   drv_status;
    unsigned char   xxdata[255];
    unsigned char   length;
};
typedef struct sdrv_out DRV_OUT;


Action	: Activates the reading of a magnetic track card.

Syntax	: DRV_OUT * Os__mag_read (void) ;

Description	: Activates the magnetic card driver for reading a magnetic track card.

Returned value	: A pointer to the DRV_OUT structure :
	reqnr	: not used.
	gen_status	: general execution status.
	drv_status	: particular status about the ISO2 track reading.

	xxdata[0]	: ISO2 track length.
	xxdata[1..length(ISO2)]	: ISO2 track.
	xxdata[length(ISO2)+1]	: particular status about is ISO1 or ISO3 track reading (according  to the EFT terminal configuration).
	xxdata[length(ISO2)+2]	: ISO1 or ISO3 track length.
	xxdata[length(ISO2)+3 ..]	: ISO1 or ISO3 track.

Remark : The 'xxdata' item has a maximum length of 183 characters.
*/

DRV_OUT * Os__mag_read (void)
{
        DRV_OUT *tmp;
        int i;



        Os_close_mag_fd();
        Os_open_mag_fd();

        munmap(magdata, 1024);
        magdata = (DRV_OUT *)mmap(0, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mag, 0);
        if (magdata == MAP_FAILED) {
                perror("!!!!!!!Os__mag_read mmap");
                goto out;
        }


        /*be sure to have new data, not last striped ones*/
        memset(magdata, 0, sizeof(DRV_OUT));
        tmp = (DRV_OUT *)magdata;




#if 1
        printf("************* %s ********************\n", __func__);
        printf("gen_status=%x %p\n", tmp->gen_status, tmp->gen_status);
        printf("ISO2: ucTKLen=%d\n", tmp->xxdata[0]);
        for (i=0; i< tmp->xxdata[0]; i++)
                printf("%c ", tmp->xxdata[i+1]);
        printf("\n");
        tmp->gen_status = 0;
        printf("************* %s end********************\n\n\n", __func__);
#endif


        return (DRV_OUT *)magdata;

out:
        return NULL;
}


void OSCFG_MAGSettingLoad(OSMAGCFG  *pOsMagCfg)
{
        int fd;

        fd=open("/dev/s3c2410_mag", O_RDWR);
        if (fd<0) {
                Uart_Printf("open failed!\n");
                return;
        }

        ioctl(fd, MAG_SET_CFG, (OSMAGCFG *)pOsMagCfg);

        close(fd);
}





//zy start_303
int mag_te1st()
{
        DRV_OUT *data;
        DRV_OUT *p;
        OSMAGCFG magcfg;

        int i=5, len=0, len1=0, idx3=0, idx1=0;

        while (i--) {

                Uart_Printf("%s fd_mag1 =%d\n", __func__,fd_mag);
                data=Os__mag_read();
                if (data==NULL) {
                        Uart_Printf("%s data=NULL\n", __func__);
                        return -1;
                }
                /*
                    Uart_Printf("data=%p, magdata=%p\n", data, magdata);
                    Uart_Printf("data gen_status=%p magdata=%p\n", data->gen_status, magdata->gen_status);
                */
                while (data->gen_status!=DRV_ENDED);

                Uart_Printf("************* %s getpid=%d********************\n", __func__, getpid());

                len =  data->xxdata[0];
                Uart_Printf("ISO2: ucTKLen=%d\n", len);
                for (i=1; i<= len; i++) {
                        Uart_Printf("%c ", data->xxdata[i]);
                        if ((i+1)%20==0) Uart_Printf("\n");
                }
                Uart_Printf("\n");


                idx3 = len+1;
                len = data->xxdata[idx3+1];
                Uart_Printf("ISO3:status=%02x ucTKLen=%d, sidx=%d\n", data->xxdata[idx3], data->xxdata[idx3+1]);
                for (i=idx3+2; i<=idx3+2+len; i++) {
                        Uart_Printf("%c ", data->xxdata[i]);
                        if ((i+1)%20==0) Uart_Printf("\n");
                }
                Uart_Printf("\n");


                idx1 = 1+data->xxdata[0] + 2 +len;
                len = data->xxdata[idx1+1];

                Uart_Printf("ISO1:status=%02x ucTKLen=%d\n", data->xxdata[idx1], data->xxdata[idx1+1]);
                for (i=idx1+2; i<=idx1+2+len; i++) {
                        Uart_Printf("%c ", data->xxdata[i]);
                        if ((i+1)%20==0) Uart_Printf("\n");
                }
                Uart_Printf("\n");


        }

#if 0
        /*test for parameters setting*/
        magcfg.ucMagDebug = 1;
        OSCFG_MAGSettingLoad(&magcfg);
        Uart_Printf("ver=%s\n", magcfg.ucMagVer);
#endif

out:
        return 0;
}

int read_without_mmap()
{
        int fd, ret;
        DRV_OUT *data;
        int i=5, len=0, len1=0, idx3=0, idx1=0, uiOffset=0;

        fd = Os_open_mag_fd();
        if (fd<0) {
                Uart_Printf("open failed\n");
                return -1;
        }

        data=malloc(sizeof(DRV_OUT));
        if (!data) {
                Uart_Printf("malloc failed!\n");
                return -1;
        }

        ret= read(fd, (char *)data, sizeof(DRV_OUT));
        if (ret) {

                Uart_Printf("APP************* %s getpid=%d********************\n", __func__, getpid());

                uiOffset=0;
                len= data->xxdata[uiOffset++];
                Uart_Printf("APP************ TRACK 2(%d, uiOffset=%d)***************\n",len, uiOffset);
                for (i = 1; i <=len ; i++) {
                        Uart_Printf("%c ", data->xxdata[uiOffset++]);
                        if ((i+1)%20==0) Uart_Printf("\n");
                }
                Uart_Printf("\n");

                Uart_Printf("APP************ TRACK 3( x uiOffset=%d err:%d, len:%d, )***************\n",
                            uiOffset, data->xxdata[uiOffset++], data->xxdata[uiOffset++]);
                len= data->xxdata[uiOffset-1];

                for (i = 1; i <= len; i++) {
                        Uart_Printf("%c ", data->xxdata[uiOffset++]);
                        if ((i+1)%20==0) Uart_Printf("\n");
                }
                Uart_Printf("\n");


                Uart_Printf("APP************ TRACK 1(uiOffset=%d err:%d,len: %d, )***************\n",
                            uiOffset, data->xxdata[uiOffset++], data->xxdata[uiOffset++]);
                len= data->xxdata[uiOffset-1];
                for (i = 1; i <= len; i++) {
                        Uart_Printf("%c ", data->xxdata[uiOffset++]);
                        if ((i+1)%20==0) Uart_Printf("\n");
                }
                Uart_Printf("\n");


        }
        Uart_Printf("read len=%d, data->gen_status=%x, sizeof(DRV_OUT)=%x\n", ret, data->gen_status, sizeof(DRV_OUT));


        close(fd);
}


//zy start_303
int mag_test_print()
{
        DRV_OUT *data;
        DRV_OUT *p;
        OSMAGCFG magcfg;

        int i=5, len=0, len1=0, idx3=0, idx1=0, uiOffset=0;


        while (1) {

                data=(DRV_OUT *)Os__mag_read();
                if (data==NULL) {
                        Uart_Printf("%s data=NULL\n", __func__);
                        return -1;
                }


                /*
                    Uart_Printf("data=%p, magdata=%p\n", data, magdata);
                    Uart_Printf("data gen_status=%p magdata=%p\n", data->gen_status, magdata->gen_status);
                */
                do {
                        //Uart_Printf("data gen_status=%x\n", data->gen_status);
                } while (data->gen_status!=DRV_ENDED);

#if 0
                Uart_Printf("************* %s getpid=%d********************\n", __func__, getpid());

                uiOffset=0;
                len= data->xxdata[uiOffset++];
                Uart_Printf("************ %s TRACK 2(%d, uiOffset=%d)***************\n",__func__, len, uiOffset);
                for (i = 1; i <=len ; i++) {
                        Uart_Printf("%c ", data->xxdata[uiOffset++]);
                        if ((i+1)%20==0) Uart_Printf("\n");
                }
                Uart_Printf("\n");

                Uart_Printf("************ %s TRACK 3( x uiOffset=%d err:%d, len:%d, )***************\n",
                            __func__, uiOffset, data->xxdata[uiOffset++], data->xxdata[uiOffset++]);
                len= data->xxdata[uiOffset-1];

                for (i = 1; i <= len; i++) {
                        Uart_Printf("%c ", data->xxdata[uiOffset++]);
                        if ((i+1)%20==0) Uart_Printf("\n");
                }
                Uart_Printf("\n");


                Uart_Printf("************ TRACK 1(uiOffset=%d err:%d,len: %d, )***************\n",
                            uiOffset, data->xxdata[uiOffset++], data->xxdata[uiOffset++]);
                len= data->xxdata[uiOffset-1];
                for (i = 1; i <= len; i++) {
                        Uart_Printf("%c ", data->xxdata[uiOffset++]);
                        if ((i+1)%20==0) Uart_Printf("\n");
                }
                Uart_Printf("\n");
#endif

        }

#if 0
        /*test for parameters setting*/
        magcfg.ucMagDebug = 1;
        OSCFG_MAGSettingLoad(&magcfg);
        Uart_Printf("ver=%s\n", magcfg.ucMagVer);
#endif

out:
        return 0;
}


unsigned char COMMS_SendData(const unsigned char *pucInData,unsigned short uiInLen )
{
        unsigned int  uiI;
        unsigned char aucSndBuf[800];

        //闀垮害瓒呴檺
        memset(aucSndBuf,0,sizeof(aucSndBuf));
        if (uiInLen <= sizeof(aucSndBuf))
                memcpy(aucSndBuf,pucInData,uiInLen);
        else
                return -1;

        for (uiI=0; uiI<uiInLen; uiI++) {
                Os__txcar3(aucSndBuf[uiI]);
        }

        return 0;
}


//串口打印
int mag_test()
{
        DRV_OUT *pMMIDrvOut;
        DRV_OUT *pMAGDrvOut;
        DRV_OUT *p;
        OSMAGCFG magcfg;
        unsigned char ucI;
        unsigned char ucTrack1Status;
        unsigned long ucTrack1Len;
        unsigned char aucTrack1[77];
        unsigned char ucTrack2Status;
        unsigned long ucTrack2Len;
        unsigned char aucTrack2[38];
        unsigned char ucTrack3Status;
        unsigned long ucTrack3Len;
        unsigned char aucTrack3[105];
        unsigned int  uiTimeOut;
        unsigned char aucOutBuf[256];
        unsigned char aucSndBuf[1024];
        unsigned int  uiOffset;
        unsigned int  uiDataLen,uiJ;
        unsigned char ucResult = 0;
        unsigned char CRC;

        unsigned long uiLen=0;

        int i=5, len=0, len1=0, idx3=0, idx1=0;

        Os__init_com3(0x0303, 0x1400, 0x14);
//        OSMMI_DisplayASCII(0x30,0,0,(unsigned char *)"SWIPE MAGNETIC CARD");
//        OSMMI_DisplayASCII(0x30,7,0,(unsigned char *)"PRESS CANCEL TO EXIT");

        while (1) {
                pMMIDrvOut = Os__get_key();
                pMAGDrvOut=(DRV_OUT *)Os__mag_read();

                if (pMAGDrvOut==NULL) {
                        Uart_Printf("%s data=NULL\n", __func__);
                        return -1;
                }


                do {
                        //Uart_Printf("data gen_status=%x\n", data->gen_status);
                } while (pMAGDrvOut->gen_status!=DRV_ENDED
                                &&(pMMIDrvOut->gen_status  == DRV_RUNNING));

                COMMS_SendData("helloworld",10);

                if (pMMIDrvOut->gen_status  != DRV_RUNNING) {
	                COMMS_SendData("close com3",strlen("close com3"));
                        Os__end_com3();
                        return 0;
                }




                uiOffset = 0;
                memset(aucTrack1,0,sizeof(aucTrack1));
                memset(aucTrack2,0,sizeof(aucTrack2));
                memset(aucTrack3,0,sizeof(aucTrack3));
                //.ISO2.
                ucTrack2Status = pMAGDrvOut->drv_status | '0';
                ucTrack2Len = pMAGDrvOut->xxdata[uiOffset];
                uiOffset ++;
                for (ucI=0; ucI<ucTrack2Len; ucI++,uiOffset++) {
                        aucTrack2[ucI] = pMAGDrvOut->xxdata[uiOffset] | '0';
                }
                //.ISO3.
                ucTrack3Status = pMAGDrvOut->xxdata[uiOffset] | '0';
                uiOffset ++;
                ucTrack3Len = pMAGDrvOut->xxdata[uiOffset];
                uiOffset ++;
                for (ucI=0; ucI<ucTrack3Len; ucI++,uiOffset++) {
                        aucTrack3[ucI] = pMAGDrvOut->xxdata[uiOffset] | '0';
                }
                //.ISO1.
                ucTrack1Status = pMAGDrvOut->xxdata[uiOffset] | '0';
                uiOffset ++;
                ucTrack1Len = pMAGDrvOut->xxdata[uiOffset];
                uiOffset ++;
                for (ucI=0; ucI<ucTrack1Len; ucI++,uiOffset++) {
                        aucTrack1[ucI] = pMAGDrvOut->xxdata[uiOffset] + 0x20;
                }



                // head
                uiOffset = 0;
                memset(aucSndBuf,0,sizeof(aucSndBuf));
                aucSndBuf[uiOffset++] = 0x02;
                aucSndBuf[uiOffset++] = 0x00;
                aucSndBuf[uiOffset++] = 0x00;
                //-iso1 status lenlenlen data...
                aucSndBuf[uiOffset++] = 0xA1;
                uiLen = 2+1+3+1+ucTrack1Len;
                long_tab(&aucSndBuf[uiOffset],2,&uiLen);
                //printf("%s:%d, middle of code\n", __func__, __LINE__);

                uiOffset+=2;

                hex_asc(&aucSndBuf[uiOffset],&ucTrack1Status,2);
                //printf("%s:%d, middle of code\n", __func__, __LINE__);

                uiOffset+=2;
                aucSndBuf[uiOffset++] = 0x20;
                long_asc(&aucSndBuf[uiOffset],3,&ucTrack1Len);
               // printf("%s:%d, middle of code\n", __func__, __LINE__);

                uiOffset+=3;
                aucSndBuf[uiOffset++] = 0x20;
                if (ucTrack1Len)
                        memcpy(&aucSndBuf[uiOffset],aucTrack1,ucTrack1Len);
                uiOffset+=ucTrack1Len;
                //-iso2 status lenlenlen data...
                aucSndBuf[uiOffset++] = 0xA2;
                uiLen = 2+1+3+1+ucTrack2Len;
                long_tab(&aucSndBuf[uiOffset],2,&uiLen);
                //printf("%s:%d, middle of code\n", __func__, __LINE__);

                uiOffset+=2;

                hex_asc(&aucSndBuf[uiOffset],&ucTrack2Status,2);
                //printf("%s:%d, middle of code\n", __func__, __LINE__);

                uiOffset+=2;
                aucSndBuf[uiOffset++] = 0x20;
                long_asc(&aucSndBuf[uiOffset],3,&ucTrack2Len);
                //printf("%s:%d, middle of code\n", __func__, __LINE__);

                uiOffset+=3;
                aucSndBuf[uiOffset++] = 0x20;
                if (ucTrack2Len)
                        memcpy(&aucSndBuf[uiOffset],aucTrack2,ucTrack2Len);
                uiOffset+=ucTrack2Len;
                //-iso3
                aucSndBuf[uiOffset++] = 0xA3;
                uiLen = 2+1+3+1+ucTrack3Len;
                long_tab(&aucSndBuf[uiOffset],2,&uiLen);
                uiOffset+=2;
                //printf("%s:%d, middle of code\n", __func__, __LINE__);

                hex_asc(&aucSndBuf[uiOffset],&ucTrack3Status,2);
                uiOffset+=2;
                aucSndBuf[uiOffset++] = 0x20;
                //printf("%s:%d, middle of code\n", __func__, __LINE__);

                long_asc(&aucSndBuf[uiOffset],3,&ucTrack3Len);
//                printf("%s:%d, middle of code\n", __func__, __LINE__);

                uiOffset+=3;
                aucSndBuf[uiOffset++] = 0x20;
                if (ucTrack3Len)
                        memcpy(&aucSndBuf[uiOffset],aucTrack3,ucTrack3Len);
                uiOffset+=ucTrack3Len;

                aucSndBuf[uiOffset++] = 0x03;



                uiLen = uiOffset-3-1;
                long_tab(&aucSndBuf[1],2,&uiLen);
                //printf("%s:%d, middle of code\n", __func__, __LINE__);


                CRC = 0x00;
                for (uiJ = 1; uiJ < uiOffset ; uiJ++) {
                        CRC ^= aucSndBuf[uiJ];
                }
                aucSndBuf[uiOffset++] = CRC;

                //printf("%s:%d, todo COMMS_SendData\n", __func__, __LINE__);
                ucResult = COMMS_SendData(aucSndBuf,uiOffset);
                //printf("%s:%d, after COMMS_SendData\n", __func__, __LINE__);



        }


out:
        return 0;
}

