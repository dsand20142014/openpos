
const unsigned char  AIPINFO[16][30]=
{
      	" ",
	"SDA",
      	"DDA",
	"Card Holder Verify",
      	"Terminal risk manage",
      	"Issuer auth",
      	" ",
      	"CDA",
      	" ",
      	" ",
      	" ",
      	" ",
      	" ",
      	" ",
      	" ",
      	" ",
};


const unsigned char  TVRINFO[40][30]=
{
      	"OFFLINE Auth not Perform",
      	"SDA Failed",
	"ICC Data Missing",
      	"Card Exception",
      	"DDA Failed",
      	"CDA Failed",
      	" ",
      	" ",
      	"App Ver Different",
      	"Expried App",
      	"Not effect App",
      	"Invalid Service",
      	"New Card",
      	" ",
      	" ",
      	" ",
      	"Verify Failed",
      	"CVM Unrecognised",
      	"Pin Limit Exceeded",
      	"Pin Not Present",
      	"Pin Bypass",
      	"Online EP Entered",
      	" ",
      	" ",
      	"Floor Limit",
	"Lower Exceed",
	"Upper Exceed",
	"Random Select",
	"Force Online",
      	" ",
      	" ",
      	" ",
	"Default TDOL",
	"Issuer Auth Failed",
	"Script Failed Before",
	"Script Failed After",
      	" ",
      	" ",
      	" ",
      	" "
};

const unsigned char  TSIINFO[16][30]=
{
      	"OFFLINE Data Auth",
	"Card Holder Verify",
      	"Card Risk Manage",
	"Issuer Auth",
      	"Term Risk Manage",
      	"Script Process",
      	" ",
      	" ",
      	" ",
      	" ",
      	" ",
      	" ",
      	" ",
      	" ",
      	" ",
      	" ",
};


void EMVTrans_Infor_Display(void);
void Analyze_TransType_Display(void);
void Analyze_TransResult_Display(void);
void Analyze_AIP_Display(void);
void Analyze_TVR_Display(void);
void Analyze_TSI_Display(void);
void Analyze_AuthStyle_Display(void);
void Tag_Display(PUCHAR pucTag,USHORT pucTagLen,PUCHAR pucBuff,USHORT uiTagLen);
void Analyze_CVMMethod_code_Display(UCHAR ucCVMCode);
void Analyze_CVM_Condition_Display(UCHAR ucCondition);

