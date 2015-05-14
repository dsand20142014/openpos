#define		INTERFACEPARAM_DATA
#include  <interfaceparam.h>
#undef		INTERFACEPARAM_DATA




const BASICCONFIG BasicConfigDef =
  {      
         0x01,  //ulCOMType
         38400, //ulBaudRate
         0x01,  //ucDebugMode
         0x00,   //ucPowerUpFlag    
         "\x0B\x01",
         "\x0E\x01" ,
         0x00,
         0x00
  };




const TAGDEFINFO  TagInfoDef[]=
{
        {"\x9F\x35",  1,"\x00"},  	 //   Terminal Type  00 or not present support international contactless transaction  01 (Default value) support domestic contactless transaction only
        {"\xDF\x04", 1,"\x01"},  	 //   CVM Requirements  01: Signature 02: Online PIN     (这里是要求支持CVM，并不是要求READER有这个能力)
        {"\x9F\x1A", 2,"\x01\x56"},  //   Terminal Country Code
        {"\x5F\x2A", 2,"\x01\x56"},  //   Transaction Currency
        {"\x9C", 1,"\x00"},          //   Transaction Type   00 means purchase  
        {"\xDF\x00", 6,"\x00\x00\x00\x05\x00\x00"},   //  Reader Contactless Transaction limit   >=  the reader shall prompt the cardholder to use another interface
        {"\xDF\x01", 6,"\x00\x00\x00\x02\x50\x00"},   //  Reader CVM Required limit          <  CVM is required
        {"\xDF\x02", 6,"\x00\x00\x00\x00\x50\x00"},   //  Reader Contactless Floor Limit     <  offline transaction is allowed
        {"\xDF\x03", 1,"\x00"},                       //  Enhanced DDA version number   0 the reader should be able to support offline transactions for all types of DDA cards.
        {"\xDF\x05", 1,"\x00"},                       //  Display offline funds  indicator 00 means not required to display the offline available funds 01 means display the offline available funds
        {"\x9F\x66", 4,"\xA6\x00\x00\x00"},           //Terminal Transaction Qualifiers  B1 : 8  MSD ,7 Contactless VSDC ,6 Contactless qVSDC,5 Contact VSDC,4 Reader is Offline Only,3 Online PIN,2 Signature                                                      // B2 : 8 Online cryptogram, 7 CVM 
#if 0
        {"\xDF\x61", 6,"\x00\x00\x00\x02\x50\x00"},   //  qpboc  Reader CVM Required limit   
        {"\xDF\x62", 6,"\x00\x00\x00\x00\x50\x00"},   // qpboc    Reader Contactless Floor Limit 
        {"\xDF\x63", 1,"\x00"},                       //  qpboc     Enhanced DDA version number 
        {"\xDF\x64", 1,"\x01"},  //   qpboc       CVM Requirements
        {"\xDF\x65", 1,"\x00"},                       //  qpboc   Display offline funds  
        {"\xDF\x66",  1,"\x00"},    //  qpboc   Terminal Type 
        {"\xDF\x67", 4,"\xA6\x00\x00\x00"},           //qpboc   Terminal Transaction Qualifiers 
 #endif
        {0}
};


const INTERFACEPARAMDEF InterfaceParamDef[] =
{
	{P_MSG_TIMEOUT, 	INTERFACE_USHORT_TYPE,2,"\x01\xF4"},//500milliseconds ,"P_MSG_TIMEOUT(m)","通讯超时(毫秒)"
	{P_SALE_TIMEOUT, 	INTERFACE_USHORT_TYPE,2,"\x75\x30"},//30000milliseconds ,"P_SALE_TIMEOUT(m)","交易时间(毫秒)"
	{P_POLL_MSG,      	INTERFACE_USHORT_TYPE,2,"\x00\x1E"},//30seconds ,"P_POLL_MSG(s)","握手时间(秒)"
	{P_BUF_TIMEOUT,   	INTERFACE_USHORT_TYPE,2,"\x13\x88"},//5,000 milliseconds ,"P_BUF_TIMEOUT(m)","读卡器清空(毫秒)"
	{P_ENCRYPTION, 		INTERFACE_UCAHR_TYPE,1,"\x01"}, //0x01 – TDEA is enabled ,"P_ENCRYPTION","通讯数据加密"
	{P_DISPLAY,			INTERFACE_UCAHR_TYPE,1,"\x01"},// 0x00 display all the response code  0x01convert the response code into meaning ,"P_DISPLAY","信息显示方式"
	{P_MAX_BUF_SIZE,	INTERFACE_USHORT_TYPE,2,"\x04\x00"},// 1024 bytes ,"P_MAX_BUF_SIZE(b)","通讯最大字节"
	{P_DOUBLE_DIP,		INTERFACE_USHORT_TYPE,2,"\x13\x88"},//5,000 milliseconds ,"P_DOUBLE_DIP(m)","同卡受理(毫秒)"
	{P_READER_INDEX,	INTERFACE_UCAHR_TYPE,2,"\x0E\x01"},  // "P_READER_INDEX","读卡器索引"
	{P_LANGUAGE,		INTERFACE_UCAHR_TYPE,9,"\x5F\x2D\x06\x7A\x68\x01\x65\x6E\x00"},//(Chinese \x7A\x68, English65\x6E) 01 enabled  ,"P_LAUGUAGE","读卡器支持语言"
	{P_DISPLAY_S_MSG,	INTERFACE_USHORT_TYPE,2,"\x07\xD0"},//  2,000 milliseconds ,"P_DISPLAY_S_MSG(m)","简单显示(毫秒)"
	{P_DISPLAY_L_MSG,	INTERFACE_USHORT_TYPE,2,"\x13\x88"},  //  5,000 milliseconds ,"P_DISPLAY_L_MSG(m)","复杂显示(毫秒)"
	{P_DISPLAY_SS_MSG,	INTERFACE_USHORT_TYPE,2,"\x27\x10"},  //10,000 milliseconds ,"P_DISPLAY_SS_MSG m","屏幕签名(毫秒)"
	{P_DISPLAY_SR_MSG,	INTERFACE_USHORT_TYPE,2,"\x13\x88"},// 5,000 milliseconds ,"P_DISPLAY_SR_MSG m","单据签名(毫秒)"
	{P_DISPLAY_PIN_MSG,	INTERFACE_USHORT_TYPE,2,"\x27\x10"},  // 10,000 milliseconds  ,"P_DISPLAY_PIN_MSGm","输密码时间(毫秒)"
	{P_DISPLAY_E_MSG,	INTERFACE_USHORT_TYPE,2,"\x0B\xB8"},//  3,000 milliseconds  ,"P_DISPLAY_E_MSG(m)","错误显示(毫秒)"
	{0}
};



const CVMINFO  CVMInfoDef[]
	={
		 {"Sign with touchscr",0x10,0x00},
		 {"Online PIN",0x11,0x00},
          // {"Don't support CVM",0x00,0x00},		 
	   // {"Offline PIN",0x12,0x00},
		 {0}
	};




const READER_CAPAB  ReaderCapab[]=   // 这个上面的9F66内容一样，所以直接用9F66来控制就好了
 { 
 	   {VISA_WAVE3,"VISA_WAVE3"}, 
	 //  {VISA_MSD20,"VISA_MSD2.0"},
 	// {VISA_MSI,"VISA_MSI"},
  // {VISA_DDA_MSI,"VISA_DDA_MSI"},
  // {VISA_CDA,"VISA_CDA"},
  // {VISA_DCVV,"VISA_DCVV"}, 
  // {VISA_VSDC,"VISA_VSDC"},
	// {VISA_FDDA_VLP,"VISA_FDDA_VLP"},
  // {VISA_WAVE2,"VISA_WAVE2"},     
	// {JCB_WAVE1,"JCB_WAVE1"},
	// {JCB_WAVE2,"JCB_WAVE2"}, 
	// {JCB_WAVE3,"JCB_WAVE3"}, 
	 {0}
  };      





const MESSAGE  MessageData[]
	={
#if 1
          	  {1,"欢迎使用"},  //Welcome
		      {2,"谢谢惠顾"}, //Thank You
		      {3,"谢谢"},//Thank You, tag 5F20 tag 5F20 means cardholder name
		      {4,"交易完成"},// Transaction Completed
		      {5,"请使用其它卡"},//Please Use Other Card
		      {6,"请插卡"},//Please Insert Card
		      {7,"请选择其中一张卡"},//Please Select 1 Card     indicate that there are more than 1 cards in the field.
		      {8,"国际卡,请插卡"},//International Card Please Insert
		      {9,"请再试一次"},//Please Try Again
		      {10,"国际卡,请刷卡"},//International Card Please Swipe
		      {11,"请在触摸屏上签名"},//Please Sign on The Screen
		      {12,"请在单据上签名"},//Please Sign on The receipt
		      {13,"请输入密码"},//Please Enter PIN
		      {14,"可用脱机金额"},//Offline Available Funds
		      {15,"交易未完成,请输密码"},//PIN Entry Required.Transaction Not Completed
		      {16,"交易未完成,请签名"},//Signature Required.Transaction Not Completed
		      {17,"对不起,不在使用中"},//Sorry Not in Use
		      {18,"请放卡"},//Present Card
		      {19,"移开卡,请等待"},//Remove Card. Please Wait
		      {20,"处理中..."},//Processing...
#else

          {1,"Welcome"},
		      {2,"Thank You"},
		      {3,"Thank You, tag 5F20"},
		      {4,"Transaction Completed"},
		      {5,"Please Use Other Card"},
		      {6,"Please Insert Card"},
		      {7,"Please Select 1 Card"},
		      {8,"International Card Please Insert"},
		      {9,"Please Try Again"},
		      {10,"International Card Please Swipe"},
		      {11,"Please Sign on The Screen"},
		      {12,"Please Sign on The receipt"},
		      {13,"Please Enter PIN"},
		      {14,"Offline Available Funds"},
		      {15,"PIN Entry Required.Transaction Not Completed"},
		      {16,"Signature Required.Transaction Not Completed"},
		      {17,"Sorry Not in Use"},
		      {18,"Present Card"},
		      {19,"Remove Card. Please Wait"},
		      {20,"Processing..."},

#endif
		      {0}
          };

const INTERFACESUPPORTAPP InterfaceSupportAppDefault[]=
	{
		{7,"\xA0\x00\x00\x00\x03\x10\x10",0},			
		{7,"\xA0\x00\x00\x03\x33\x01\x01",0},
		{0}
	};


const   VISAPK   VISAPKDef[]
   ={
                  {
                        "\xA0\x00\x00\x03\x33",
                        0x80,
                        0x01,
                        0x01,
                        0x80,
                        "\xCC\xDB\xA6\x86\xE2\xEF\xB8\x4C\xE2\xEA\x01\x20\x9E\xEB\x53\xBE\xF2\x1A\xB6\xD3\x53\x27\x4F\xF8\x39\x1D\x70\x35\xD7\x6E\x21\x56\xCA\xED\xD0\x75\x10\xE0\x7D\xAF\xCA\xCA\xBB\x7C\xCB\x09\x50\xBA\x2F\x0A\x3C\xEC\x31\x3C\x52\xEE\x6C\xD0\x9E\xF0\x04\x01\xA3\xD6\xCC\x5F\x68\xCA\x5F\xCD\x0A\xC6\x13\x21\x41\xFA\xFD\x1C\xFA\x36\xA2\x69\x2D\x02\xDD\xC2\x7E\xDA\x4C\xD5\xBE\xA6\xFF\x21\x91\x3B\x51\x3C\xE7\x8B\xF3\x3E\x68\x77\xAA\x5B\x60\x5B\xC6\x9A\x53\x4F\x37\x77\xCB\xED\x63\x76\xBA\x64\x9C\x72\x51\x6A\x7E\x16\xAF\x85",
                        0x03,
                        "\x01\x00\x01",
                        "\xa5\xe4\x4b\xb0\xe1\xfa\x4f\x96\xa1\x17\x09\x18\x66\x70\xd0\x83\x50\x57\xd3\x5e"
                  },
                  {
                        "\xA0\x00\x00\x00\x03",
                        0x95,
                        0x01,
                        0x01,
                        0x90,
                        "\xBE\x9E\x1F\xA5\xE9\xA8\x03\x85\x29\x99\xC4\xAB\x43\x2D\xB2\x86\x00\xDC\xD9\xDA\xB7\x6D\xFA\xAA\x47\x35\x5A\x0F\xE3\x7B\x15\x08\xAC\x6B\xF3\x88\x60\xD3\xC6\xC2\xE5\xB1\x2A\x3C\xAA\xF2\xA7\x00\x5A\x72\x41\xEB\xAA\x77\x71\x11\x2C\x74\xCF\x9A\x06\x34\x65\x2F\xBC\xA0\xE5\x98\x0C\x54\xA6\x47\x61\xEA\x10\x1A\x11\x4E\x0F\x0B\x55\x72\xAD\xD5\x7D\x01\x0B\x7C\x9C\x88\x7E\x10\x4C\xA4\xEE\x12\x72\xDA\x66\xD9\x97\xB9\xA9\x0B\x5A\x6D\x62\x4A\xB6\xC5\x7E\x73\xC8\xF9\x19\x00\x0E\xB5\xF6\x84\x89\x8E\xF8\xC3\xDB\xEF\xB3\x30\xC6\x26\x60\xBE\xD8\x8E\xA7\x8E\x90\x9A\xFF\x05\xF6\xDA\x62\x7B",
                        0x01,
                        "\x03",
                        "\xEE\x15\x11\xCE\xC7\x10\x20\xA9\xB9\x04\x43\xB3\x7B\x1D\x5F\x6E\x70\x30\x30\xF6"
                  },


                  {//  1
                        "\xA0\x00\x00\x00\x03",
                        0x01,
                        0x01,
                        0x01,
                        0x80,
                        "\xC6\x96\x03\x42\x13\xD7\xD8\x54\x69\x84\x57\x9D\x1D\x0F\x0E\xA5\x19\xCF\xF8\xDE\xFF\xC4\x29\x35\x4C\xF3\xA8\x71\xA6\xF7\x18\x3F\x12\x28\xDA\x5C\x74\x70\xC0\x55\x38\x71\x00\xCB\x93\x5A\x71\x2C\x4E\x28\x64\xDF\x5D\x64\xBA\x93\xFE\x7E\x63\xE7\x1F\x25\xB1\xE5\xF5\x29\x85\x75\xEB\xE1\xC6\x3A\xA6\x17\x70\x69\x17\x91\x1D\xC2\xA7\x5A\xC2\x8B\x25\x1C\x7E\xF4\x0F\x23\x65\x91\x24\x90\xB9\x39\xBC\xA2\x12\x4A\x30\xA2\x8F\x54\x40\x2C\x34\xAE\xCA\x33\x1A\xB6\x7E\x1E\x79\xB2\x85\xDD\x57\x71\xB5\xD9\xFF\x79\xEA\x63\x0B\x75",
                        0x01,
                        "\x03",
                        "\xD3\x4A\x6A\x77\x60\x11\xC7\xE7\xCE\x3A\xEC\x5F\x03\xAD\x2F\x8C\xFC\x55\x03\xCC"
                  },

                   {  //  2
                        "\xA0\x00\x00\x00\x03",
                        0x07,
                        0x01,
                        0x01,
                        0x90,
                        "\xA8\x9F\x25\xA5\x6F\xA6\xDA\x25\x8C\x8C\xA8\xB4\x04\x27\xD9\x27\xB4\xA1\xEB\x4D\x7E\xA3\x26\xBB\xB1\x2F\x97\xDE\xD7\x0A\xE5\xE4\x48\x0F\xC9\xC5\xE8\xA9\x72\x17\x71\x10\xA1\xCC\x31\x8D\x06\xD2\xF8\xF5\xC4\x84\x4A\xC5\xFA\x79\xA4\xDC\x47\x0B\xB1\x1E\xD6\x35\x69\x9C\x17\x08\x1B\x90\xF1\xB9\x84\xF1\x2E\x92\xC1\xC5\x29\x27\x6D\x8A\xF8\xEC\x7F\x28\x49\x20\x97\xD8\xCD\x5B\xEC\xEA\x16\xFE\x40\x88\xF6\xCF\xAB\x4A\x1B\x42\x32\x8A\x1B\x99\x6F\x92\x78\xB0\xB7\xE3\x31\x1C\xA5\xEF\x85\x6C\x2F\x88\x84\x74\xB8\x36\x12\xA8\x2E\x4E\x00\xD0\xCD\x40\x69\xA6\x78\x31\x40\x43\x3D\x50\x72\x5F",
                        0x01,
                        "\x03",
                        "\xB4\xBC\x56\xCC\x4E\x88\x32\x49\x32\xCB\xC6\x43\xD6\x89\x8F\x6F\xE5\x93\xB1\x72"
                  },
  #if 0  
                   {
                        "\xA0\x00\x00\x00\x03",
                        0x08,
                        0x01,
                        0x01,
                        0xB0,
                        "\xD9\xFD\x6E\xD7\x5D\x51\xD0\xE3\x06\x64\xBD\x15\x70\x23\xEA\xA1\xFF\xA8\x71\xE4\xDA\x65\x67\x2B\x86\x3D\x25\x5E\x81\xE1\x37\xA5\x1D\xE4\xF7\x2B\xCC\x9E\x44\xAC\xE1\x21\x27\xF8\x7E\x26\x3D\x3A\xF9\xDD\x9C\xF3\x5C\xA4\xA7\xB0\x1E\x90\x70\x00\xBA\x85\xD2\x49\x54\xC2\xFC\xA3\x07\x48\x25\xDD\xD4\xC0\xC8\xF1\x86\xCB\x02\x0F\x68\x3E\x02\xF2\xDE\xAD\x39\x69\x13\x3F\x06\xF7\x84\x51\x66\xAC\xEB\x57\xCA\x0F\xC2\x60\x34\x45\x46\x98\x11\xD2\x93\xBF\xEF\xBA\xFA\xB5\x76\x31\xB3\xDD\x91\xE7\x96\xBF\x85\x0A\x25\x01\x2F\x1A\xE3\x8F\x05\xAA\x5C\x4D\x6D\x03\xB1\xDC\x2E\x56\x86\x12\x78\x59\x38\xBB\xC9\xB3\xCD\x3A\x91\x0C\x1D\xA5\x5A\x5A\x92\x18\xAC\xE0\xF7\xA2\x12\x87\x75\x26\x82\xF1\x58\x32\xA6\x78\xD6\xE1\xED\x0B",
                        0x01,
                        "\x03",
                        "\x20\xD2\x13\x12\x69\x55\xDE\x20\x5A\xDC\x2F\xD2\x82\x2B\xD2\x2D\xE2\x1C\xF9\xA8"
                  },
                 

                  {
                        0x09,
                        0x01,
                        0x01,
                        0xF8,
                        "\x9D\x91\x22\x48\xDE\x0A\x4E\x39\xC1\xA7\xDD\xE3\xF6\xD2\x58\x89\x92\xC1\xA4\x09\x5A\xFB\xD1\x82\x4D\x1B\xA7\x48\x47\xF2\xBC\x49\x26\xD2\xEF\xD9\x04\xB4\xB5\x49\x54\xCD\x18\x9A\x54\xC5\xD1\x17\x96\x54\xF8\xF9\xB0\xD2\xAB\x5F\x03\x57\xEB\x64\x2F\xED\xA9\x5D\x39\x12\xC6\x57\x69\x45\xFA\xB8\x97\xE7\x06\x2C\xAA\x44\xA4\xAA\x06\xB8\xFE\x6E\x3D\xBA\x18\xAF\x6A\xE3\x73\x8E\x30\x42\x9E\xE9\xBE\x03\x42\x7C\x9D\x64\xF6\x95\xFA\x8C\xAB\x4B\xFE\x37\x68\x53\xEA\x34\xAD\x1D\x76\xBF\xCA\xD1\x59\x08\xC0\x77\xFF\xE6\xDC\x55\x21\xEC\xEF\x5D\x27\x8A\x96\xE2\x6F\x57\x35\x9F\xFA\xED\xA1\x94\x34\xB9\x37\xF1\xAD\x99\x9D\xC5\xC4\x1E\xB1\x19\x35\xB4\x4C\x18\x10\x0E\x85\x7F\x43\x1A\x4A\x5A\x6B\xB6\x51\x14\xF1\x74\xC2\xD7\xB5\x9F\xDF\x23\x7D\x6B\xB1\xDD\x09\x16\xE6\x44\xD7\x09\xDE\xD5\x64\x81\x47\x7C\x75\xD9\x5C\xDD\x68\x25\x46\x15\xF7\x74\x0E\xC0\x7F\x33\x0A\xC5\xD6\x7B\xCD\x75\xBF\x23\xD2\x8A\x14\x08\x26\xC0\x26\xDB\xDE\x97\x1A\x37\xCD\x3E\xF9\xB8\xDF\x64\x4A\xC3\x85\x01\x05\x01\xEF\xC6\x50\x9D\x7A\x41",
                        0x01,
                        "\x03",
                        "\x1F\xF8\x0A\x40\x17\x3F\x52\xD7\xD2\x7E\x0F\x26\xA1\x46\xA1\xC8\xCC\xB2\x90\x46"
                  },

                  {  // 5
                        0x99,
                        0x01,
                        0x01,
                        0x80,
                        "\xAB\x79\xFC\xC9\x52\x08\x96\x96\x7E\x77\x6E\x64\x44\x4E\x5D\xCD\xD6\xE1\x36\x11\x87\x4F\x39\x85\x72\x25\x20\x42\x52\x95\xEE\xA4\xBD\x0C\x27\x81\xDE\x7F\x31\xCD\x3D\x04\x1F\x56\x5F\x74\x73\x06\xEE\xD6\x29\x54\xB1\x7E\xDA\xBA\x3A\x6C\x5B\x85\xA1\xDE\x1B\xEB\x9A\x34\x14\x1A\xF3\x8F\xCF\x82\x79\xC9\xDE\xA0\xD5\xA6\x71\x0D\x08\xDB\x41\x24\xF0\x41\x94\x55\x87\xE2\x03\x59\xBA\xB4\x7B\x75\x75\xAD\x94\x26\x2D\x4B\x25\xF2\x64\xAF\x33\xDE\xDC\xF2\x8E\x09\x61\x5E\x93\x7D\xE3\x2E\xDC\x03\xC5\x44\x45\xFE\x7E\x38\x27\x77",
                        0x01,
                        "\x03",
                        "\x4A\xBF\xFD\x6B\x1C\x51\x21\x2D\x05\x55\x2E\x43\x1C\x5B\x17\x00\x7D\x2F\x5E\x6D"
                  },

                  {
                        0x95,
                        0x01,
                        0x01,
                        0x90,
                        "\xBE\x9E\x1F\xA5\xE9\xA8\x03\x85\x29\x99\xC4\xAB\x43\x2D\xB2\x86\x00\xDC\xD9\xDA\xB7\x6D\xFA\xAA\x47\x35\x5A\x0F\xE3\x7B\x15\x08\xAC\x6B\xF3\x88\x60\xD3\xC6\xC2\xE5\xB1\x2A\x3C\xAA\xF2\xA7\x00\x5A\x72\x41\xEB\xAA\x77\x71\x11\x2C\x74\xCF\x9A\x06\x34\x65\x2F\xBC\xA0\xE5\x98\x0C\x54\xA6\x47\x61\xEA\x10\x1A\x11\x4E\x0F\x0B\x55\x72\xAD\xD5\x7D\x01\x0B\x7C\x9C\x88\x7E\x10\x4C\xA4\xEE\x12\x72\xDA\x66\xD9\x97\xB9\xA9\x0B\x5A\x6D\x62\x4A\xB6\xC5\x7E\x73\xC8\xF9\x19\x00\x0E\xB5\xF6\x84\x89\x8E\xF8\xC3\xDB\xEF\xB3\x30\xC6\x26\x60\xBE\xD8\x8E\xA7\x8E\x90\x9A\xFF\x05\xF6\xDA\x62\x7B",
                        0x01,
                        "\x03",
                        "\xEE\x15\x11\xCE\xC7\x10\x20\xA9\xB9\x04\x43\xB3\x7B\x1D\x5F\x6E\x70\x30\x30\xF6"
                  },

                   {
                        0x92,
                        0x01,
                        0x01,
                        0xB0,
                        "\x99\x6A\xF5\x6F\x56\x91\x87\xD0\x92\x93\xC1\x48\x10\x45\x0E\xD8\xEE\x33\x57\x39\x7B\x18\xA2\x45\x8E\xFA\xA9\x2D\xA3\xB6\xDF\x65\x14\xEC\x06\x01\x95\x31\x8F\xD4\x3B\xE9\xB8\xF0\xCC\x66\x9E\x3F\x84\x40\x57\xCB\xDD\xF8\xBD\xA1\x91\xBB\x64\x47\x3B\xC8\xDC\x9A\x73\x0D\xB8\xF6\xB4\xED\xE3\x92\x41\x86\xFF\xD9\xB8\xC7\x73\x57\x89\xC2\x3A\x36\xBA\x0B\x8A\xF6\x53\x72\xEB\x57\xEA\x5D\x89\xE7\xD1\x4E\x9C\x7B\x6B\x55\x74\x60\xF1\x08\x85\xDA\x16\xAC\x92\x3F\x15\xAF\x37\x58\xF0\xF0\x3E\xBD\x3C\x5C\x2C\x94\x9C\xBA\x30\x6D\xB4\x4E\x6A\x2C\x07\x6C\x5F\x67\xE2\x81\xD7\xEF\x56\x78\x5D\xC4\xD7\x59\x45\xE4\x91\xF0\x19\x18\x80\x0A\x9E\x2D\xC6\x6F\x60\x08\x05\x66\xCE\x0D\xAF\x8D\x17\xEA\xD4\x6A\xD8\xE3\x0A\x24\x7C\x9F",
                        0x01,
                        "\x03",
                        "\x42\x9C\x95\x4A\x38\x59\xCE\xF9\x12\x95\xF6\x63\xC9\x63\xE5\x82\xED\x6E\xB2\x53"
                  },

                  {
                        0x51,
                        0x01,
                        0x01,
                        0x90,
                        "\xDB\x5F\xA2\x9D\x1F\xDA\x8C\x16\x34\xB0\x4D\xCC\xFF\x14\x8A\xBE\xE6\x3C\x77\x20\x35\xC7\x98\x51\xD3\x51\x21\x07\x58\x6E\x02\xA9\x17\xF7\xC7\xE8\x85\xE7\xC4\xA7\xD5\x29\x71\x0A\x14\x53\x34\xCE\x67\xDC\x41\x2C\xB1\x59\x7B\x77\xAA\x25\x43\xB9\x8D\x19\xCF\x2C\xB8\x0C\x52\x2B\xDB\xEA\x0F\x1B\x11\x3F\xA2\xC8\x62\x16\xC8\xC6\x10\xA2\xD5\x8F\x29\xCF\x33\x55\xCE\xB1\xBD\x3E\xF4\x10\xD1\xED\xD1\xF7\xAE\x0F\x16\x89\x79\x79\xDE\x28\xC6\xEF\x29\x3E\x0A\x19\x28\x2B\xD1\xD7\x93\xF1\x33\x15\x23\xFC\x71\xA2\x28\x80\x04\x68\xC0\x1A\x36\x53\xD1\x4C\x6B\x48\x51\xA5\xC0\x29\x47\x8E\x75\x7F",
                        0x01,
                        "\x03",
                        "\xB9\xD2\x48\x07\x5A\x3F\x23\xB5\x22\xFE\x45\x57\x3E\x04\x37\x4D\xC4\x99\x5D\x71"
                  },
#endif
                
                  {
                        0
                  }
   };
   
const EERRMSG TerminalErrMsg[]
	={
#if 1
			{'0',0x00,"交易成功"},
			{'0',0x01,"交易结束"},
			{'0',0x02,"交易取消"},
			{'0',0x03,"数据发送方出错"},
			{'0',0x04,"数据接收方出错"},
			{'0',0x05,"数据序列号出错"},
			{'0',0x06,"序列号相同"},
			{'0',0x07,"接收数据长度出错"},
			{'0',0x08,"回波测试错误"},
			{'0',0x09,"硬件不支持"},
			{'0',0x0A,"密钥类型不一致"},
			{'0',0x0B,"密钥索引不一致"},
			{'0',0x0C,"初始化通讯密钥错"},
			{'0',0x0D,"错误的密钥类型"},
			{'0',0x10,"数据发送出错"},
			{'0',0x11,"数据接收出错"},
			{'0',0x12,"协议出错"},
			{'0',0x13,"接收数据超限"},
			{'0',0x14,"接收超时"},
			{'0',0x15,"包校验位错"},
			{'0',0x16,"串口初始化失败"},
			{'0',0x17,"错误串口类型"},
			{'0',0x18,"包长度错"},
			{'0',0x19,"交易未处理"},
			{'0',0x1A,"非法波特率"},
			{'0',0x20,"密码错误"},
			{'0',0x21,"密码输入不一致"},
			{'0',0x22,"当前模式不支持"},
			{'0',0x23,"系数长度出错"},
			{'0',0x24,"指数长度出错"},
			{'0',0x25,"VISAPK长度出错"},
			{'0',0x26,"消息个数超限"},
			{'0',0x27,"消息长度出错"},
			{'0',0x28,"消息长度超限"},
			{'0',0x29,"消息类型不存在"},
			{'0',0x2A,"时间超时"},
			{'0',0x30,"CVM CAPAB超限"},
			{'0',0x31,"输入超限"},
			{'0',0x32,"TAG个数超限"},
			{'0',0x33,"TAG BUF超限"},
			{'0',0x34,"TAG 数据出错"},
			{'0',0x35,"公钥标志位出错"},
			{'0',0x36,"公钥索引不存在"},
			{'0',0x37,"MSGID非法"},
			{'0',0x38,"请选择接触卡交易"},
			{'0',0x39,"日期输入出错"},
			{'0',0x3A,"时间输入出错"},
			{'0',0x3B,"波特率输入出错"},
			{'0',0x3C,"读卡器性能输入错"},
			{'0',0x3D,"消息ID输入出错"},
			{'0',0x3E,"CVM ID输入出错"},
			{'0',0x3F,"TAG参数出错"},
			{'0',0x40,"TAG已存在"},
			{'0',0x41,"TAG不存在"},
			{'0',0x42,"接收数据超限"},
			{'0',0xFF,"未知错误"}
#else
      {'0',0x00,"SUCCESS"},
			{'0',0x01,"ONLINE DECLINE"},
			{'0',0x02,"CANCEL"},
			{'0',0x03,"ERR SENDERINDEX"},
			{'0',0x04,"ERR RECEIVERINDEX"},
			{'0',0x05,"ERR SEQUENCE "},
			{'0',0x06,"SAME SEQUENCE"},
			{'0',0x07,"ERR RECEIVERLEN"},
			{'0',0x08,"ERR ECHO"},
			{'0',0x09,"ERR LOAD KEY"},
			{'0',0x0A,"DIFFER KEY TYPE"},
			{'0',0x0B,"DIFFER KEY INDEX"},
			{'0',0x0C,"ERR INITIALIZEKEY"},
			{'0',0x0D,"ERR KEY TYPE"},
			{'0',0x10,"ERR SEND"},
			{'0',0x11,"ERR RECEIVE"},
			{'0',0x12,"ERR PROTOCOL"},
			{'0',0x14,"RECEIVE TIMEOUT"},
			{'0',0x15,"ERR CRC"},
			{'0',0x16,"ERR INIT SER PORT"},
			{'0',0x17,"ERR SERIAL TYPE"},
			{'0',0x18,"ERR PACKLEN"},
			{'0',0x1A,"ERR BAUD RATE"},
			{'0',0x20,"ERR PASSWORD"},
			{'0',0x21,"DIFFER PASSWORD"},
			{'0',0x22,"NOSUPPORT MODE"},
			{'0',0x23,"ERR MODULUSLEN"},
			{'0',0x24,"ERR EXPONENTLEN"},
			{'0',0x25,"ERR PK LEN"},
			{'0',0x26,"ERR MESSAGE NUM"},
			{'0',0x27,"ERR MESSAGE LEN"},
			{'0',0x28,"MSG OVERTAKE"},
			{'0',0x29,"NO MSG ID"},
			{'0',0x2A,"TIMEOUT"},
			{'0',0x30,"CVMCAPAB OVERTAKE"},
			{'0',0x31,"INPUT OVER MAX"},
			{'0',0x32,"OVER MAXTAGNUM"},
			{'0',0x33,"OVER MAXTAGBUF"},
			{'0',0x34,"ERR TAGDATA"},
			{'0',0x35,"ERR ADDDELFLAG"},
			{'0',0x36,"PKINDEX NOT EXIST"},
			{'0',0x37,"INVALID MSGID"},
			{'0',0x38,"CHANGE TO CONTACT"},
			{'0',0x39,"INPUT DATE ERR"},
			{'0',0x3A,"INPUT TIME ERR"},
			{'0',0x3B,"INPUT BAUD RATE ERR"},
			{'0',0x3C,"INPUT CAPABILITY ERR"},
			{'0',0x3D,"INPUT MESSAGE ID ERR"},
			{'0',0x3E,"INPUT CVM ID ERR"},
			{'0',0x3F,"TAGPARAM ERR"},
			{'0',0x40,"TAG EXIST"},
			{'0',0x41,"TAG NOT EXIST"},
			 {'0',0x42,"RECEIVER OVER LEN"},
			{'0',0xFF,"UNKNOWN ERR"}
#endif
       };


const EERRMSG ReaderErrMsg[]
	={
		
#if 1		
			{'0',0x00,"交易成功"},
			{'0',0x01,"卡数据"},
			{'0',0x02,"验证成功的握手"},
			{'0',0x03,"没有验证的握手"},
			{'0',0x04,"设备"},
			{'0',0x05,"签名"},
			{'0',0x06,"联机密码"},
			{'0',0x07,"脱机密码"},
			{'0',0x08,"第二个应用"},
			{'0',0xEA,"JCB公钥索引错误"},
			{'0',0xEB,"未签名"},
			{'0',0xEC,"未输密码"},
			{'0',0xED,"美国卡"},
			{'0',0xEE,"其它应用卡"},
			{'0',0xEF,"握手失败"},
			{'0',0xF0,"无此参数定义"},
			{'0',0xF1,"无此TAG"},
			{'0',0xF2,"无卡"},
			{'0',0xF3,"多卡"},
			{'0',0xF4,"VISA公钥索引错误"},
			{'0',0xF5,"无设备"},
			{'0',0xF6,"错误密钥索引"},
			{'0',0xF7,"无此参数"},
			{'0',0xF8,"不正确的数据"},
			{'0',0xF9,"无此消息ID"},
			{'0',0xFA,"卡处理失败"},
			{'0',0xFB,"无相互验证"},
			{'0',0xFC,"相互验证失败"},
			{'0',0xFD,"导入失败"},
			{'0',0xFE,"无导入"},
			{'0',0xFF,"失败"}
			
#else			
			{'0',0x00,"SUCCESS"},
			{'0',0x01,"CARD DATA"},
			{'0',0x02,"POLL A"},
			{'0',0x03,"POLL P"},
			{'0',0x04,"SCHEME"},
			{'0',0x05,"SIGNATURE"},
			{'0',0x06,"PIN_ONLINE"},
			{'0',0x07,"PIN_OFFLINE"},
			{'0',0x08,"SECOND_APP"},
			{'0',0xEA,"Bad JCB CA KEYID"},
			{'0',0xEB,"No Signature"},
			{'0',0xEC,"No PIN"},
			{'0',0xED,"US Cards"},
			{'0',0xEE,"Other AP Cards"},
			{'0',0xEF,"POLL N"},
			{'0',0xF0,"NO PARA"},
			{'0',0xF1,"NO TAGS"},
			{'0',0xF2,"NO CARD"},
			{'0',0xF3,"MORE THAN 1 CARD"},
			{'0',0xF4,"BAD Visa CA KEYID"},
			{'0',0xF5,"NO SCHEME"},
			{'0',0xF6,"BAD KEYID"},
			{'0',0xF7,"NO PARA"},
			{'0',0xF8,"DATA INCORRECT"},
			{'0',0xF9,"NO MSG ID"},
			{'0',0xFA,"CARD FAIL"},
			{'0',0xFB,"NO AUTH"},
			{'0',0xFC,"FAIL AUTH"},
			{'0',0xFD,"FAIL ACCESS"},
			{'0',0xFE,"NO ACCESS"},
			{'0',0xFF,"FAILURE"}
			
#endif			
       };
    





//9F74  Authorization Code data
//Tag 9F4B  an offline cryptogram and DDA





	
