
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
//#include <linux/ioctl.h>
//#include "classic.h"
#include <string.h>
#include "wjc.h"

int rf_test (int argc,char *argv[])
{

    int fd,status,val;

    if (argc <2)
    {
        printf ("rc531: input error.\n");
        return -1 ;
    }
    status = atoi(argv[1]);
    printf("status = %d\n",status);

    fd = open("/dev/rc531", O_RDWR) ;
    if (fd < 0 )
    {
        printf ("open error.\n") ;
        return -1 ;
    }

    val = 100;
    //ioctl(fd,PCD_IOC_INIT,&val) ;

    switch (status)
    {
    case 0:
    {

        unsigned char ucCardType,aucUid[32],ucUidLen,aucAtq[2],ucAtqLen,ucI,ucSak;
        unsigned char aucData[32];
        int ucStatus;
        struct card_attr_info cdAttr;

        ucUidLen = sizeof(aucUid);
        ucAtqLen = sizeof(aucAtq);

        do
        {
            ucStatus = CLASSIC_PollCard(fd,&ucCardType,aucUid,&ucUidLen,aucAtq,&ucAtqLen );
            if ( ucStatus!=PCD_ERR_SUCCESS ) CLASSIC_Reset(fd,1000);
        }
        while (ucStatus);

        if ( ucStatus==PCD_ERR_SUCCESS )
        {
            ucStatus = CLASSIC_Wupa(fd, aucAtq,&ucAtqLen );
            printf("CLASSIC_Wupa ucStatus:%d\n",ucStatus);
        }
        if ( ucStatus==PCD_ERR_SUCCESS )
        {
            ucStatus = CLASSIC_Select(fd, aucUid,ucUidLen,&ucSak );
            printf("CLASSIC_Select ucStatus:%d\n",ucStatus);
        }
        if ( ucStatus==PCD_ERR_SUCCESS )
        {
            ucStatus = CLASSIC_AuthBlock(fd, 0x01,7,0x01,(unsigned char *)"\xff\xff\xff\xff\xff\xff" );
            printf("CLASSIC_AuthBlock ucStatus:%d\n",ucStatus);
        }
        if ( ucStatus==PCD_ERR_SUCCESS )
        {
            int i ;
            ucStatus = CLASSIC_ReadBlock(fd, 28,aucData );
            printf("CLASSIC_ReadBlock 28 ucStatus:%d\n",ucStatus);
            for (i=0; i<16; i++)
            {
                printf("%#x(%d) ",aucData[i],i);
            }
        }
        if ( ucStatus==PCD_ERR_SUCCESS )
        {
            memset(aucData,0x55,32);
            ucStatus = CLASSIC_WriteBlock(fd, 29,aucData );
            printf("\nCLASSIC_WriteBlock 29 ucStatus:%d\n",ucStatus);
        }

        if ( ucStatus==PCD_ERR_SUCCESS )
        {
            int i ;
            unsigned char xaucData[32];
            ucStatus = CLASSIC_ReadBlock(fd, 29,xaucData );

            printf("CLASSIC_ReadBlock 29 ucStatus:%d\n",ucStatus);
            for (i=0; i<16; i++)
            {
                printf("%#x(%d) ",xaucData[i],i);
            }
        }
        if ( ucStatus==PCD_ERR_SUCCESS )	 printf("\n test ok.\n");

        break;
    }
    case 1:
    {
        unsigned char test_dat =0x05 ;
        int rt =-1;
        rt = ioctl(fd,PCD_IOC_TEST,&test_dat) ;
        break;
    }
    case 2: //use card form kernel.
    {
        int rt =-1;
        int bb = 0;
        rt = ioctl(fd,PCD_IOC_USE_CARD,&bb) ;
        break;
    }
    case 3: //init.
    {
        int rt =-1;
        rt = ioctl(fd,PCD_IOC_INIT);
        printf("rt =%d\n",rt);
        break;
    }
    case 4: //auth
    {
        int rt =-1;
        //struct auth_data_info data;
        //data.ucAuthType = 0x01 ;
        //data.ucSectorIndex = 0;
        //data.ucKeyType = 0x01 ;
        //memcpy (data.pucKey , (unsigned char *)"\xfc\x61\xcb\xb5\x37\x51",sizeof(data.pucKey));
        rt = CLASSIC_AuthBlock(fd, 0x01,0,0x01,(unsigned char *)"\xff\xff\xff\xff\xff\xff" );

        //	rt = ioctl(fd,PCD_IOC_AUTH,&data);
        break;
    }
    case 5: //reset
    {
        int cret;
        cret = CLASSIC_Reset(fd,1000);
        printf("cret:%#x\n");
        break;
    }
    case 6: //wake up type A ??
    {
        unsigned char aucAtq[2],atqlen;
        int cret;
        aucAtq[0] = 0x55;
        aucAtq[1] = 0xaa;
        atqlen = sizeof(aucAtq);
        cret =  CLASSIC_Wupa(fd,aucAtq,&atqlen);
        printf("6. cret:%#x\n",cret);
        break;
    }
    case 7: //PollCard.
    {
        int cret;
        unsigned char type,uid[32],uidlen,atq[2],atqlen;
        uidlen = sizeof(uid);
        atqlen = sizeof(atq);
        do
        {
            cret =  CLASSIC_PollCard(fd,&type,uid,&uidlen,atq,&atqlen);
            printf("type:%#x,uidlen:%#x,atqlen:%#x cret:%#x\n",type,uidlen,atqlen,cret);
            if ( cret!=PCD_ERR_SUCCESS ) CLASSIC_Reset(fd,1000);
        }
        while (cret);

        break;
    }
    case 8: //select card.
    {
        unsigned char Uid[4] = {0xa2,0xae,0x3b,0xd3};
        unsigned char uidLen, ucSak;
        int cret;
        uidLen = sizeof(Uid);
        cret = CLASSIC_Select(fd,Uid,uidLen,&ucSak);
        printf("8.cret:%#x\n",cret);

        break;
    }

    default:
        break;
    }

    close(fd);
    return 0;
}
