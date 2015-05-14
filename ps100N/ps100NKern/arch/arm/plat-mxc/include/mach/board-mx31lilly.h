 	0x40
#define		ENLIQABORT      	0x20
#define		ENLQICRCI_LQ    	0x10
#define		ENLQICRCI_NLQ   	0x08
#define		ENLQIBADLQI     	0x04
#define		ENLQIOVERI_LQ   	0x02
#define		ENLQIOVERI_NLQ  	0x01

#define	LQISTAT1        		0x51
#define		LQIPHASE_LQ     	0x80
#define		LQIPHASE_NLQ    	0x40
#define		LQIABORT        	0x20
#define		LQICRCI_LQ      	0x10
#define		LQICRCI_NLQ     	0x08
#define		LQIBADLQI       	0x04
#define		LQIOVERI_LQ     	0x02
#define		LQIOVERI_NLQ    	0x01

#define	CLRLQIINT1      		0x51
#define		CLRLQIPHASE_LQ  	0x80
#define		CLRLQIPHASE_NLQ 	0x40
#define		CLRLIQABORT     	0x20
#define		CLRLQICRCI_LQ   	0x10
#define		CLRLQICRCI_NLQ  	0x08
#define		CLRLQIBADLQI    	0x04
#define		CLRLQIOVERI_LQ  	0x02
#define		CLRLQIOVERI_NLQ 	0x01

#define	LQISTAT2        		0x52
#define		PACKETIZED      	0x80
#define		LQIPHASE_OUTPKT 	0x40
#define		LQIWORKONLQ     	0x20
#define		LQIWAITFIFO     	0x10
#define		LQISTOPPKT      	0x08
#define		LQISTOPLQ       	0x04
#define		LQISTOPCMD      	0x02
#define		LQIGSAVAIL      	0x01

#define	SSTAT3          		0x53
#define		NTRAMPERR       	0x02
#define		OSRAMPERR       	0x01

#define	SIMODE3         		0x53
#define		ENNTRAMPERR     	0x02
#define		ENOSRAMPERR     	0x01

#define	CLRSINT3        		0x53
#define		CLRNTRAMPERR    	0x02
#define		CLROSRAMPERR    	0x01

#define	LQOSTAT0        		0x54
#define		LQOTARGSCBPERR  	0x10
#define		LQOSTOPT2       	0x08
#define		LQOATNLQ    