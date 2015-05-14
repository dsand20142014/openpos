

#define SHA1                               0x01
#define SHA256                           0x02


#define RTIC_BASE                                    0x53F00000+ 0x000EC000
#define RTIC_STATUS                                 0x000
#define RTIC_CMD                                       0x004
#define RTIC_CTRL                                      0x008
#define RTIC_DMATHROTTLE                     0x00C
#define RTIC_MEMBLOCKA1                      0x010
#define RTIC_MEMLENA1                           0x014
#define RTIC_MEMBLOCKA2                      0x018
#define RTIC_MEMLENA2                           0x01C
#define RTIC_MEMBLOCKB1                      0x030 
#define RTIC_MEMLENB1                           0x034
#define RTIC_MEMBLOCKB2                      0x038
#define RTIC_MEMLENB2                           0x03C
#define RTIC_MEMBLOCKC1                      0x050 
#define RTIC_MEMLENC1                           0x054
#define RTIC_MEMBLOCKC2                      0x058
#define RTIC_MEMLENC2                           0x05C
#define RTIC_MEMBLOCKD1                      0x070 
#define RTIC_MEMLEND1                           0x074
#define RTIC_MEMBLOCKD2                      0x078
#define RTIC_MEMLEND2                           0x07C
#define RTIC_FAULT                                    0x090 
#define RTIC_WDOG                                    0x094
#define RTIC_VERID                                    0x098
#define RTIC_HASHA_A                             0x0A0 
#define RTIC_HASHA_B                             0x0A4
#define RTIC_HASHA_C                             0x0A8
#define RTIC_HASHA_D                             0x0AC
#define RTIC_HASHA_E                              0x0B0 
#define RTIC_HASHA_F                              0x0B4
#define RTIC_HASHA_G                              0x0B8
#define RTIC_HASHA_H                              0x0BC
#define RTIC_HASHB_A                              0x0C0 
#define RTIC_HASHB_B                             0x0C4
#define RTIC_HASHB_C                             0x0C8
#define RTIC_HASHB_D                             0x0CC
#define RTIC_HASHB_E                              0x0D0 
#define RTIC_HASHB_F                              0x0D4
#define RTIC_HASHB_G                              0x0D8
#define RTIC_HASHB_H                              0x0DC
#define RTIC_HASHC_A                              0x0E0 
#define RTIC_HASHC_B                              0x0E4
#define RTIC_HASHC_C                              0x0E8
#define RTIC_HASHC_D                              0x0EC
#define RTIC_HASHC_E                              0x0F0 
#define RTIC_HASHC_F                              0x0F4
#define RTIC_HASHC_G                              0x0F8
#define RTIC_HASHC_H                              0x0FC
#define RTIC_HASHD_A                              0x100 
#define RTIC_HASHD_B                              0x104
#define RTIC_HASHD_C                              0x108
#define RTIC_HASHD_D                              0x10C
#define RTIC_HASHD_E                              0x110 
#define RTIC_HASHD_F                              0x114
#define RTIC_HASHD_G                              0x118
#define RTIC_HASHD_H                              0x11C





#define RTIC_WRITE_REGISTER(offset,value)   (*((volatile unsigned int *)(RTIC_BASE+offset)))=value
#define RTIC_READ_REGISTER(offset)                (*((volatile unsigned int *)(RTIC_BASE+offset))) 




int imx_hash_compute(unsigned char hashtype,unsigned int pinput, unsigned int uilen, unsigned int *output )
{
   unsigned int status;
   unsigned int i;

     if(pinput % 4 ||uilen % 4)
	   return -1;
     
     status = RTIC_READ_REGISTER(RTIC_STATUS);
	//printf("rtic_status=%08x",status);
     if(!(status & 0x01))
     	{             
             RTIC_WRITE_REGISTER(RTIC_MEMBLOCKC1,pinput);  //write the start address             
             RTIC_WRITE_REGISTER(RTIC_MEMLENC1,uilen);	     
	     RTIC_WRITE_REGISTER(RTIC_MEMLENC2,0);     //skip Memory C2
	     if(hashtype == SHA256)	        	  
	          RTIC_WRITE_REGISTER(RTIC_CTRL,0x4040 );   //enable Memory C hash256 once 
	      else
		   RTIC_WRITE_REGISTER(RTIC_CTRL,0x40 );  //enable Memory C hash1 once 
	     RTIC_WRITE_REGISTER(RTIC_CMD,0x04);
		 while(!(RTIC_READ_REGISTER(RTIC_STATUS)&0x02));
	       
            if(hashtype == SHA256)
             {
                for(i=0;i<8;i++)
                {
		    output[i] = RTIC_READ_REGISTER(RTIC_HASHC_A+(4*i));
		    //printf("out[%d]=%02x/",i,output[i]);
                }
              }
	     else
	     {
              for(i=0;i<5;i++)//5
              	{
	            output[i] = RTIC_READ_REGISTER(RTIC_HASHC_A+(4*i));		
		   // printf("out[%d]=%02x/",i,output[i]);
              	}
	     }
     	  
	RTIC_WRITE_REGISTER(RTIC_CMD,0x03);
	return 0;
     }
	 else
	 	return -1;
}



