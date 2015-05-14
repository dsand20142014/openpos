
/************************************/
/*		EMV Kernel Define File      */
/*           Ver4.1 by Yangjun      */
/************************************/

#ifndef		_QDEF_H
#define		_QDEF_H

#define  QRightRotate(x)		(0x01<<x)

/**********Terminal Capabilities********/
#define  QTERMCAPAB_MANUALKEY		   QRightRotate(7)
#define  QTERMCAPAB_MAGSTRIPE		   QRightRotate(6)   
#define  QTERMCAPAB_IC				   QRightRotate(5)   

#define  QTERMCAPAB_PLAINTEXTPIN	   QRightRotate(7)   
#define  QTERMCAPAB_ENCIPHEREDONLINEPIN QRightRotate(6)   
#define  QTERMCAPAB_SIGNATURE		   QRightRotate(5) 
#define  QTERMCAPAB_ENCIPHEREDOFFLINEPIN QRightRotate(4)   
#define  QTERMCAPAB_NOCVM			   QRightRotate(3)   
#define  QTERMCAPAB_IDCARD			   QRightRotate(0)  

#define  QTERMCAPAB_SDA			   	   QRightRotate(7)     
#define  QTERMCAPAB_DDA			   	   QRightRotate(6)     
#define  QTERMCAPAB_CARDCAPTURE	   QRightRotate(5)
#define  QTERMCAPAB_CDA			   	   QRightRotate(3)          

/**********Additional Terminal Capabilities********/
#define	 QTERMADDCAPAB_CASH			   QRightRotate(7) 
#define	 QTERMADDCAPAB_GOODS			   QRightRotate(6) 
#define	 QTERMADDCAPAB_SERVICES		   QRightRotate(5) 
#define	 QTERMADDCAPAB_CASHBACK		   QRightRotate(4) 
#define	 QTERMADDCAPAB_INQUIRY		   	   QRightRotate(3) 
#define	 QTERMADDCAPAB_TRANSFER		   QRightRotate(2) 
#define	 QTERMADDCAPAB_PAYMENT		   	   QRightRotate(1) 
#define	 QTERMADDCAPAB_ADMIN			   QRightRotate(0) 

#define	 QTERMADDCAPAB_CASHDEPOSIT	   QRightRotate(7) 

#define	 QTERMADDCAPAB_CASHDEPOSIT	   QRightRotate(7) 

#define	 QTERMADDCAPAB_NUMERICKEY	   		QRightRotate(7)
#define	 QTERMADDCAPAB_ALPHAKEY		   	QRightRotate(6)
#define	 QTERMADDCAPAB_COMMANDKEY	   		QRightRotate(5)
#define	 QTERMADDCAPAB_FUNCTIONKEY	   	QRightRotate(4)

#define	 QTERMADDCAPAB_PRINTATTEND	  	QRightRotate(7)
#define	 QTERMADDCAPAB_PRINTCARDHOLDER  	QRightRotate(6)
#define	 QTERMADDCAPAB_DISPATTEND	   		QRightRotate(5)
#define	 QTERMADDCAPAB_DISPCARDHOLDER   	QRightRotate(4)
#define	 QTERMADDCAPAB_CODETABLE10	   	QRightRotate(1)
#define	 QTERMADDCAPAB_CODETABLE9	   		QRightRotate(0)

#define	 QTERMADDCAPAB_CODETABLE8	   QRightRotate(7)
#define	 QTERMADDCAPAB_CODETABLE7	   QRightRotate(6)
#define	 QTERMADDCAPAB_CODETABLE6	   QRightRotate(5)
#define	 QTERMADDCAPAB_CODETABLE5	   QRightRotate(4)
#define	 QTERMADDCAPAB_CODETABLE4	   QRightRotate(3)
#define	 QTERMADDCAPAB_CODETABLE3	   QRightRotate(2)
#define	 QTERMADDCAPAB_CODETABLE2	   QRightRotate(1)
#define	 QTERMADDCAPAB_CODETABLE1	   QRightRotate(0)


#define	  QAIP_SDA						QRightRotate(6)
#define	  QAIP_DDA						QRightRotate(5)
#define	  QAIP_CARDHOLDERVERIFY		QRightRotate(4)
#define	  QAIP_TERMRISKMANAGE			QRightRotate(3)
#define	  QAIP_ISSUERAUTH				QRightRotate(2)
#define	  QAIP_CDA						QRightRotate(1)

#define	  QAIP_MSD						QRightRotate(7)


#define	  QTSI_OFFLINEDATAAUTH			QRightRotate(7)
#define	  QTSI_CARDHOLDERVERIFY		QRightRotate(6)
#define	  QTSI_CARDRISKMANAGE			QRightRotate(5)
#define	  QTSI_ISSUERAUTH				QRightRotate(4)
#define	  QTSI_TERMRISKMANAGE			QRightRotate(3)
#define	  QTSI_SCRIPTPROCESS			QRightRotate(2)

#define		QTVR_OFFAUTHNOTPERFORM		QRightRotate(7)
#define		QTVR_SDAFAILED				QRightRotate(6)
#define		QTVR_ICCDATAMISSING		QRightRotate(5)
#define		QTVR_CARDEXCEPTION			QRightRotate(4)
#define		QTVR_DDAFAILED				QRightRotate(3)
#define		QTVR_CDAFAILED				QRightRotate(2)

#define		QTVR_APPVERDIFF				QRightRotate(7)
#define		QTVR_EXPRIEDAPP				QRightRotate(6)
#define		QTVR_NOTEFFECTAPP			QRightRotate(5)
#define		QTVR_INVALIDSERVICE		QRightRotate(4)
#define		QTVR_NEWCARD				QRightRotate(3)

#define		QTVR_VERIFYFAILED			QRightRotate(7)
#define		QTVR_UNRECOGNISED			QRightRotate(6)
#define		QTVR_PINLIMITEXCEEDED		QRightRotate(5)
#define		QTVR_PINNOTPRESENT			QRightRotate(4)
#define		QTVR_PINBYPASS				QRightRotate(3)
#define		QTVR_ONLINEPIENTERED		QRightRotate(2)

#define		QTVR_FLOORLIMIT				QRightRotate(7)
#define		QTVR_LOWEREXCEED			QRightRotate(6)
#define		QTVR_UPPEREXCEEDED			QRightRotate(5)
#define		QTVR_RANDOMSELECT			QRightRotate(4)
#define		QTVR_FORCEONLINE			QRightRotate(3)

#define		QTVR_DEFAULTTDOL			QRightRotate(7)
#define		QTVR_ISSUERAUTHFAILED		QRightRotate(6)
#define		QTVR_SCRIPTFAILEDBEFORE	QRightRotate(5)
#define		QTVR_SCRIPTFAILEDAFTER		QRightRotate(4)





#define		QMAXAIDLEN					16
#define		QMAXLOACALNAMELEN			16
#define		QMAXCANDIDATEAPPNUMS			16
#define		QMAXTERMSUPPORTAPPNUM  		16
#define		QMAXDDFNAMELEN				16
#define		QMAXADFNAMELEN				16
#define		QMAXAPPLABELLEN				16
#define     QMAXAPPPREFERNAMELEN   		16
#define		PSEFILE                "1PAY.SYS.DDF01"
#define		PPSEFILE                "2PAY.SYS.DDF01"

#define		QMAXPDOLLEN					255
#define		QLOGENTRYLEN				2
#define		QMAXDEFAULTDOLLEN			255	
#define		QMAXDEFAULTTDOLLEN			252
#define		QMAXAUTHDATALEN				255
#define		QTERMCAPABLEN				3
#define		QTERMADDCAPABLEN			5
#define		QTERMCOUNTRYCODELEN			2
#define		QTRANSCURRENCYCODELEN		2
#define		QAPPVERNUMBERLEN			2


#define 	QPARTIAL_MATCH  				0
#define 	QEXACT_MATCH    				1

#define		QRIDDATALEN					5
#define		QCERTSERIALLEN				3
#define		QMAXMODULLEN				248
#define		QMAXEXPONENTLEN				3
#define		QHASHDATALEN				20
#define		QISSUERIDLEN				4
#define		QTRANSDATELEN				8
#define		QTRANSTIMELEN				6
#define		QMAXCDOLDATALEN				255
#define		QMAXGACDATALEN				255
#define		QMAXSCRIPTRESULTNUMS		32
#define		QMAXSCRIPTDATALEN			255
#define		QMAXSCRIPTNUMS				32
#define		QSCRIPTIDLEN				4
#define		QAPPNEEDCONFIRM				0x80
#define		QMAXTERMCAPKNUMS			60
#define		QMAXTERMAIDNUMS				16
#define		QMAXIPKREVOKENUMS			40
#define		QMAXEXCEPTFILENUMS			16

#define		QMAXPANDATALEN				10
#define		QMAXTRACK1LEN				80
#define		QMAXTRACK2LEN				38
#define		QMAXTRACK3LEN				105


#define			QTERMINALIDLEN		8
#define			QMERCHANTIDLEN		15
#define			QMERCHANTNAMELEN		40
#define			QAUTHCODELEN			6
typedef		enum { QCASH,QGOODS,QSERVICES,QCASHBACK,QINQUIRY,QTRANSFER,QPAYMENT,QADMIN,QDEPOSIT,QPREAUTHOR}QTRANSTYPE;
#endif
