
/************************************/
/*		EMV Kernel Define File      */
/*           Ver4.1 by Yangjun      */
/************************************/

#ifndef		_EMVDEF_H
#define		_EMVDEF_H

#define  RightRotate(x)		(0x01<<x)

/**********Terminal Capabilities********/
#define  TERMCAPAB_MANUALKEY		   RightRotate(7)
#define  TERMCAPAB_MAGSTRIPE		   RightRotate(6)   
#define  TERMCAPAB_IC				   RightRotate(5)   

#define  TERMCAPAB_PLAINTEXTPIN	   RightRotate(7)   
#define  TERMCAPAB_ENCIPHEREDONLINEPIN RightRotate(6)   
#define  TERMCAPAB_SIGNATURE		   RightRotate(5) 
#define  TERMCAPAB_ENCIPHEREDOFFLINEPIN RightRotate(4)   
#define  TERMCAPAB_NOCVM			   RightRotate(3)   
#define  TERMCAPAB_IDCARD			   RightRotate(0)  

#define  TERMCAPAB_SDA			   	   RightRotate(7)     
#define  TERMCAPAB_DDA			   	   RightRotate(6)     
#define  TERMCAPAB_CARDCAPTURE	   RightRotate(5)
#define  TERMCAPAB_CDA			   	   RightRotate(3)          

/**********Additional Terminal Capabilities********/
#define	 TERMADDCAPAB_CASH			   RightRotate(7) 
#define	 TERMADDCAPAB_GOODS			   RightRotate(6) 
#define	 TERMADDCAPAB_SERVICES		   RightRotate(5) 
#define	 TERMADDCAPAB_CASHBACK		   RightRotate(4) 
#define	 TERMADDCAPAB_INQUIRY		   	   RightRotate(3) 
#define	 TERMADDCAPAB_TRANSFER		   RightRotate(2) 
#define	 TERMADDCAPAB_PAYMENT		   	   RightRotate(1) 
#define	 TERMADDCAPAB_ADMIN			   RightRotate(0) 

#define	 TERMADDCAPAB_CASHDEPOSIT	   RightRotate(7) 

#define	 TERMADDCAPAB_CASHDEPOSIT	   RightRotate(7) 

#define	 TERMADDCAPAB_NUMERICKEY	   		RightRotate(7)
#define	 TERMADDCAPAB_ALPHAKEY		   	RightRotate(6)
#define	 TERMADDCAPAB_COMMANDKEY	   		RightRotate(5)
#define	 TERMADDCAPAB_FUNCTIONKEY	   	RightRotate(4)

#define	 TERMADDCAPAB_PRINTATTEND	  	RightRotate(7)
#define	 TERMADDCAPAB_PRINTCARDHOLDER  	RightRotate(6)
#define	 TERMADDCAPAB_DISPATTEND	   		RightRotate(5)
#define	 TERMADDCAPAB_DISPCARDHOLDER   	RightRotate(4)
#define	 TERMADDCAPAB_CODETABLE10	   	RightRotate(1)
#define	 TERMADDCAPAB_CODETABLE9	   		RightRotate(0)

#define	 TERMADDCAPAB_CODETABLE8	   RightRotate(7)
#define	 TERMADDCAPAB_CODETABLE7	   RightRotate(6)
#define	 TERMADDCAPAB_CODETABLE6	   RightRotate(5)
#define	 TERMADDCAPAB_CODETABLE5	   RightRotate(4)
#define	 TERMADDCAPAB_CODETABLE4	   RightRotate(3)
#define	 TERMADDCAPAB_CODETABLE3	   RightRotate(2)
#define	 TERMADDCAPAB_CODETABLE2	   RightRotate(1)
#define	 TERMADDCAPAB_CODETABLE1	   RightRotate(0)


#define	  AIP_SDA						RightRotate(6)
#define	  AIP_DDA						RightRotate(5)
#define	  AIP_CARDHOLDERVERIFY		RightRotate(4)
#define	  AIP_TERMRISKMANAGE			RightRotate(3)
#define	  AIP_ISSUERAUTH				RightRotate(2)
#define	  AIP_CDA						RightRotate(0)


#define	  TSI_OFFLINEDATAAUTH			RightRotate(7)
#define	  TSI_CARDHOLDERVERIFY		RightRotate(6)
#define	  TSI_CARDRISKMANAGE			RightRotate(5)
#define	  TSI_ISSUERAUTH				RightRotate(4)
#define	  TSI_TERMRISKMANAGE			RightRotate(3)
#define	  TSI_SCRIPTPROCESS			RightRotate(2)

#define		TVR_OFFAUTHNOTPERFORM		RightRotate(7)
#define		TVR_SDAFAILED				RightRotate(6)
#define		TVR_ICCDATAMISSING		RightRotate(5)
#define		TVR_CARDEXCEPTION			RightRotate(4)
#define		TVR_DDAFAILED				RightRotate(3)
#define		TVR_CDAFAILED				RightRotate(2)

#define		TVR_APPVERDIFF				RightRotate(7)
#define		TVR_EXPRIEDAPP				RightRotate(6)
#define		TVR_NOTEFFECTAPP			RightRotate(5)
#define		TVR_INVALIDSERVICE		RightRotate(4)
#define		TVR_NEWCARD				RightRotate(3)

#define		TVR_VERIFYFAILED			RightRotate(7)
#define		TVR_UNRECOGNISED			RightRotate(6)
#define		TVR_PINLIMITEXCEEDED		RightRotate(5)
#define		TVR_PINNOTPRESENT			RightRotate(4)
#define		TVR_PINBYPASS				RightRotate(3)
#define		TVR_ONLINEPIENTERED		RightRotate(2)

#define		TVR_FLOORLIMIT				RightRotate(7)
#define		TVR_LOWEREXCEED			RightRotate(6)
#define		TVR_UPPEREXCEEDED			RightRotate(5)
#define		TVR_RANDOMSELECT			RightRotate(4)
#define		TVR_FORCEONLINE			RightRotate(3)

#define		TVR_DEFAULTTDOL			RightRotate(7)
#define		TVR_ISSUERAUTHFAILED		RightRotate(6)
#define		TVR_SCRIPTFAILEDBEFORE	RightRotate(5)
#define		TVR_SCRIPTFAILEDAFTER		RightRotate(4)





#define		MAXAIDLEN					16
#define		MAXLOACALNAMELEN			16
#define		MAXCANDIDATEAPPNUMS			16
#define		MAXTERMSUPPORTAPPNUM  		16
#define		MAXDDFNAMELEN				16
#define		MAXADFNAMELEN				16
#define		MAXAPPLABELLEN				16
#define     MAXAPPPREFERNAMELEN   		17
#define		PSEFILE                "1PAY.SYS.DDF01"
#define		MAXPDOLLEN					255
#define		LOGENTRYLEN				2
#define		MAXDEFAULTDOLLEN			255	
#define		MAXDEFAULTTDOLLEN			252
#define		MAXAUTHDATALEN				2048
#define		TERMCAPABLEN				3
#define		TERMADDCAPABLEN			5
#define		TERMCOUNTRYCODELEN			2
#define		TRANSCURRENCYCODELEN		2
#define		APPVERNUMBERLEN			2
#define		LANGUAGEPREFERMAXLEN		8

#define 	PARTIAL_MATCH  				0
#define 	EXACT_MATCH    				1

#define		RIDDATALEN					5
#define		CERTSERIALLEN				3
#define		MAXMODULLEN				248
#define		MAXEXPONENTLEN				3
#define		HASHDATALEN				20
#define		EMVISSUERIDLEN				4
#define		TRANSDATELEN				8
#define		TRANSTIMELEN				6
#define		MAXCDOLDATALEN				255
#define		MAXGACDATALEN				255
#define		MAXSCRIPTRESULTNUMS		32
#define		MAXSCRIPTDATALEN			128
#define		MAXSCRIPTNUMS				32
#define		SCRIPTIDLEN				4
#define		APPNEEDCONFIRM				0x80
#define		MAXTERMCAPKNUMS			60
#define		MAXTERMAIDNUMS				16
#define		MAXIPKREVOKENUMS			40
#define		MAXEXCEPTFILENUMS			16
#define		ECISSUERCODELEN				6
#define		MAXIFDLEN					8

#define		MAXPANDATALEN				10

#define			TERMINALIDLEN		8
#define			MERCHANTIDLEN		15
#define			MERCHANTNAMELEN		40
#define			AUTHCODELEN			6
typedef		enum { CASH,GOODS,SERVICES,CASHBACK,INQUIRY,TRANSFER,PAYMENT,ADMIN,DEPOSIT,PREAUTHOR,DEPOSIT_CASH,DEPOSIT_SPECACCOUNT}TRANSTYPE;
#endif
