#ifndef __OS_MSG_H
#define __OS_MSG_H

#ifndef OSMSGDATA
#define OSMSGDATA     extern
#undef  OSMSGVALUE
#else
#define OSMSGVALUE
#define OSMSGDATA	
#endif

#define OSDISP_TITLEROW		0x00
#define OSDISP_MSG0ROW		0x00
#define OSDISP_MSG1ROW		0x01
#define OSDISP_MSG2ROW		0x02
#define OSDISP_MSG3ROW		0x03
#define OSDISP_MSG4ROW		0x04
#define OSDISP_MSG5ROW		0x05
#define OSDISP_MSG6ROW		0x06
#define OSDISP_MSG7ROW		0x07
#define OSDISP_ERRMSGROW	0x07



OSMSGDATA unsigned char OSMSG_OSVersion[]
#ifdef OSMSGVALUE
	   = "OS V3.00-            "
#endif
;

OSMSGDATA unsigned char OSMSG_CheckOS[]
#ifdef OSMSGVALUE
	   = "CHECK OS...         "
#endif
;

OSMSGDATA unsigned char OSMSG_CheckFont[]
#ifdef OSMSGVALUE
	   = "CHECK FONT...       "
#endif
;

OSMSGDATA unsigned char OSMSG_CheckAPP[]
#ifdef OSMSGVALUE
	   = "CHECK APP...         "
#endif
;

OSMSGDATA unsigned char OSMSG_MenuPassword[]
#ifdef OSMSGVALUE
	   = "ENTER PASSWORD:      "
#endif
;
OSMSGDATA unsigned char OSMSG_SRAMPassword[]
#ifdef OSMSGVALUE
	   = "SRAM SET PASSWORD:      "
#endif
;
OSMSGDATA unsigned char OSMSG_InputSRAMSize[]
#ifdef OSMSGVALUE
	   = "INPUT SRAM SIZE:     "
#endif
;

OSMSGDATA unsigned char OSMSG_Reboot[]
#ifdef OSMSGVALUE
	   = "SYSTEM REBOOT        "
#endif
;

OSMSGDATA unsigned char OSMSG_MountFlashFileSystem[]
#ifdef OSMSGVALUE
	   = "MOUNT FLASH FILESYS  "
#endif
;

OSMSGDATA unsigned char OSMSG_CreateFlashFileSystem[]
#ifdef OSMSGVALUE
	   = "CREATE FLASH FILESYS "
#endif
;

OSMSGDATA unsigned char OSMSG_MountSramFileSystem[]
#ifdef OSMSGVALUE
	   = "MOUNT SRAM FILESYS   "
#endif
;

OSMSGDATA unsigned char OSMSG_CreateSramFileSystem[]
#ifdef OSMSGVALUE
	   = "CREATE SRAM FILESYS  "
#endif
;

OSMSGDATA unsigned char OSMSG_Menu[]
#ifdef OSMSGVALUE
	   = "      MAIN MENU      "
#endif
;

OSMSGDATA unsigned char OSMSG_SystemConfig[]
#ifdef OSMSGVALUE
	   = "SYSTEM CONFIG        "
#endif
;

OSMSGDATA unsigned char OSMSG_SoftDownload[]
#ifdef OSMSGVALUE
	   = "SOFTWARE DOWNLOAD    "
#endif
;

OSMSGDATA unsigned char OSMSG_SoftDevelop[]
#ifdef OSMSGVALUE
	   = "SOFTWARE DEVELOP     "
#endif
;

OSMSGDATA unsigned char OSMSG_HardTest[]
#ifdef OSMSGVALUE
	   = "HARDWARE TEST        "
#endif
;

OSMSGDATA unsigned char OSMSG_SystemAdmin[]
#ifdef OSMSGVALUE
	   = "SYSTEM ADMIN         "
#endif
;

OSMSGDATA unsigned char OSMSG_MaintenceMenu[]
#ifdef OSMSGVALUE
	   = "   MAINTENCE MENU    "
#endif
;

OSMSGDATA unsigned char OSMSG_SysCfgMenu[]
#ifdef OSMSGVALUE
	   = "    CONFIG MENU      "
#endif
;

OSMSGDATA unsigned char OSMSG_PrintSetting[]
#ifdef OSMSGVALUE
	   = "PRINT SETTING        "
#endif
;

OSMSGDATA unsigned char OSMSG_DateTimeSetting[]
#ifdef OSMSGVALUE
	   = "DATE/TIME SETTING    "
#endif
;

OSMSGDATA unsigned char OSMSG_HardwareSetting[]
#ifdef OSMSGVALUE
	   = "HARDWARE SETTING     "
#endif
;

OSMSGDATA unsigned char OSMSG_BatterySetting[]
#ifdef OSMSGVALUE
	   = "BATTERY SETTING      "
#endif
;

OSMSGDATA unsigned char OSMSG_PrinterSetting[]
#ifdef OSMSGVALUE
	   = "PRINTER SETTING      "
#endif
;

OSMSGDATA unsigned char OSMSG_ModemSetting[]
#ifdef OSMSGVALUE
	   = "MODEM SETTING        "
#endif
;

OSMSGDATA unsigned char OSMSG_EthernetSetting[]
#ifdef OSMSGVALUE
	   = "ETHERNET SETTING     "
#endif
;

OSMSGDATA unsigned char OSMSG_EthSetting[]
#ifdef OSMSGVALUE
	   = "ETHERNET SETTING     "
#endif
;

OSMSGDATA unsigned char OSMSG_EthIPSetting[]
#ifdef OSMSGVALUE
	   = "ETH IP SETTING       "
#endif
;

OSMSGDATA unsigned char OSMSG_GsmSetting[]
#ifdef OSMSGVALUE
	   = "GSM SETTING          "
#endif
;
OSMSGDATA unsigned char OSMSG_GsmParamSetting[]
#ifdef OSMSGVALUE
	   = "GSM PARAM SETTING          "
#endif
;
OSMSGDATA unsigned char OSMSG_GsmBaudSetting[]
#ifdef OSMSGVALUE
	   = "GSM BUAD SETTING          "
#endif
;
OSMSGDATA unsigned char OSMSG_GsmDefaultAPN[]
#ifdef OSMSGVALUE
	   = "DEFAULT APN          "
#endif
;
OSMSGDATA unsigned char OSMSG_GsmChangeBand[]
#ifdef OSMSGVALUE
	   = "CHANGE BAND          "
#endif
;


OSMSGDATA unsigned char OSMSG_WirelessModule[]
#ifdef OSMSGVALUE
	   = "WIRELESS MODULE			 "
#endif
;

OSMSGDATA unsigned char OSMSG_ModuleCategory[]
#ifdef OSMSGVALUE
	   = "0-GPRS 1-CDMA             "
#endif
;

OSMSGDATA unsigned char OSMSG_ContactSetting[]
#ifdef OSMSGVALUE
	   = "CONTACT SETTING      "
#endif
;

OSMSGDATA unsigned char OSMSG_DebugSetting[]
#ifdef OSMSGVALUE
	   = "DEBUG SETTING        "
#endif
;

OSMSGDATA unsigned char OSMSG_1Y0N[]
#ifdef OSMSGVALUE
	   = "1-Y 0-N              "
#endif
;

OSMSGDATA unsigned char OSMSG_DATEYearMonthDay[]
#ifdef OSMSGVALUE
	   = "YY/MM/DD/HH/MM       "
#endif
;

OSMSGDATA unsigned char OSMSG_DATEHourMinSec[]
#ifdef OSMSGVALUE
	   = "HH:MM                "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicModel[]
#ifdef OSMSGVALUE
	   = "MODEL                "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicModel1[]
#ifdef OSMSGVALUE
	   = "1-PS100    2-PS200   "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicModel2[]
#ifdef OSMSGVALUE
	   = "3-PS300    4-PS400   "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicMainVer[]
#ifdef OSMSGVALUE
	   = "MAIN VER             "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicSubVer[]
#ifdef OSMSGVALUE
	   = "SUB VER              "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicSerialNo[]
#ifdef OSMSGVALUE
	   = "SERIAL NO:           "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicPADSerialNo[]
#ifdef OSMSGVALUE
	   = "PINPAD SERIAL NO:    "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicModelPS100[]
#ifdef OSMSGVALUE
	   = "MODEL PS100          "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicModelPS200[]
#ifdef OSMSGVALUE
	   = "MODEL PS200          "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicModelPS300[]
#ifdef OSMSGVALUE
	   = "MODEL PS300          "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicModelPS400[]
#ifdef OSMSGVALUE
	   = "MODEL PS400          "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicModelUnknown[]
#ifdef OSMSGVALUE
	   = "MODEL UNKNOWN        "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicModemExistAsk[]
#ifdef OSMSGVALUE
	   = "MODEM EXIST?         "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicICCExistAsk[]
#ifdef OSMSGVALUE
	   = "ICC READER EXIST?    "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicETHExistAsk[]
#ifdef OSMSGVALUE
	   = "ETHERNET EXIST?      "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicGSMExistAsk[]
#ifdef OSMSGVALUE
	   = "GSM EXIST?           "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicVer[]
#ifdef OSMSGVALUE
	   = "VER                  "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicICCExist[]
#ifdef OSMSGVALUE
	   = "ICC       EXIST      "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicICCNotExist[]
#ifdef OSMSGVALUE
	   = "ICC       NOT EXIST  "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicETHExist[]
#ifdef OSMSGVALUE
	   = "ETHERNET  EXIST      "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicETHNotExist[]
#ifdef OSMSGVALUE
	   = "ETHERNET  NOT EXIST  "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicGSMExist[]
#ifdef OSMSGVALUE
	   = "GSM       EXIST      "
#endif
;

OSMSGDATA unsigned char OSMSG_BasicGSMNotExist[]
#ifdef OSMSGVALUE
	   = "GSM       NOT EXIST  "
#endif
;

OSMSGDATA unsigned char OSMSG_BatVoltage[]
#ifdef OSMSGVALUE
	   = "VOLTAGE              "
#endif
;

OSMSGDATA unsigned char OSMSG_PrinterNotNeedConfig[]
#ifdef OSMSGVALUE
	   = "NOT NEED CONFIG      "
#endif
;

OSMSGDATA unsigned char OSMSG_PrinterThermalVoltage[]
#ifdef OSMSGVALUE
	   = "VOLTAGE(V)           "
#endif
;

OSMSGDATA unsigned char OSMSG_PrinterThermalSpeed[]
#ifdef OSMSGVALUE
	   = "STEP SPEED(us)       "
#endif
;

OSMSGDATA unsigned char OSMSG_PrinterThermalStrobeNB[]
#ifdef OSMSGVALUE
	   = "STROBE NUMBER        "
#endif
;
OSMSGDATA unsigned char OSMSG_PrinterThermalAdjustTime[]
#ifdef OSMSGVALUE
	   = "ADJUST TIME          "
#endif
;



OSMSGDATA unsigned char OSMSG_PrinterSprocketSync[]
#ifdef OSMSGVALUE
	   = "SYNC ADJUST          "
#endif
;

OSMSGDATA unsigned char OSMSG_ModemModel[]
#ifdef OSMSGVALUE
	   = "MODEM MODEL          "
#endif
;

OSMSGDATA unsigned char OSMSG_ModemModel1[]
#ifdef OSMSGVALUE
	   = "1-TDK73K   2-SILICON "
#endif
;

OSMSGDATA unsigned char OSMSG_ModemModel2[]
#ifdef OSMSGVALUE
	   = "3-CX86500  4-CMX868  "
#endif
;

OSMSGDATA unsigned char OSMSG_ModemModel3[]
#ifdef OSMSGVALUE
	   = "5-CMX866             "
#endif
;

OSMSGDATA unsigned char OSMSG_ModemNone[]
#ifdef OSMSGVALUE
	   = "MODEM MODEL          "
#endif
;

OSMSGDATA unsigned char OSMSG_ModemTDK73K[]
#ifdef OSMSGVALUE
	   = "MODEM TDK73K         "
#endif
;

OSMSGDATA unsigned char OSMSG_ModemSILICON[]
#ifdef OSMSGVALUE
	   = "MODEM SILICON        "
#endif
;

OSMSGDATA unsigned char OSMSG_ModemCX86500[]
#ifdef OSMSGVALUE
	   = "MODEM CX86500        "
#endif
;

OSMSGDATA unsigned char OSMSG_ModemCMX868[]
#ifdef OSMSGVALUE
	   = "MODEM CMX868         "
#endif
;

OSMSGDATA unsigned char OSMSG_ModemCMX866[]
#ifdef OSMSGVALUE
	   = "MODEM CMX866         "
#endif
;

OSMSGDATA unsigned char OSMSG_ModemUnknown[]
#ifdef OSMSGVALUE
	   = "MODEM UNKNOWN        "
#endif
;

OSMSGDATA unsigned char OSMSG_ModemDebug[]
#ifdef OSMSGVALUE
	   = "MODEM DEBUG OPTION         "
#endif
;

OSMSGDATA unsigned char OSMSG_EthMAC[]
#ifdef OSMSGVALUE
	   = "MAC                  "
#endif
;

OSMSGDATA unsigned char OSMSG_IPDHCP[]
#ifdef OSMSGVALUE
	   = "DHCP ENABLE?         "
#endif
;
OSMSGDATA unsigned char OSMSG_IPBOARD[]
#ifdef OSMSGVALUE
	   = "BOARDCAST ENABLE?         "
#endif
;
OSMSGDATA unsigned char OSMSG_IPDISBOARD[]
#ifdef OSMSGVALUE
	   = "BOARDCAST DISABLE?         "
#endif
;

OSMSGDATA unsigned char OSMSG_IPAddr[]
#ifdef OSMSGVALUE
	   = "IP ADDRESS           "
#endif
;

OSMSGDATA unsigned char OSMSG_IPNetMask[]
#ifdef OSMSGVALUE
	   = "NETWORK MASK         "
#endif
;

OSMSGDATA unsigned char OSMSG_IPGatewayAddr[]
#ifdef OSMSGVALUE
	   = "GATEWAY ADDRESS      "
#endif
;

OSMSGDATA unsigned char OSMSG_IPDNSAddr[]
#ifdef OSMSGVALUE
	   = "DNS ADDRESS          "
#endif
;

OSMSGDATA unsigned char OSMSG_IPDHCPEnable[]
#ifdef OSMSGVALUE
	   = "DHCP ENABLE          "
#endif
;

OSMSGDATA unsigned char OSMSG_IPDHCPDisable[]
#ifdef OSMSGVALUE
	   = "DHCP DISABLE         "
#endif
;

OSMSGDATA unsigned char OSMSG_IP[]
#ifdef OSMSGVALUE
	   = "IP                   "
#endif
;

OSMSGDATA unsigned char OSMSG_IPMask[]
#ifdef OSMSGVALUE
	   = "MASK                 "
#endif
;

OSMSGDATA unsigned char OSMSG_IPGateway[]
#ifdef OSMSGVALUE
	   = "GATE                 "
#endif
;

OSMSGDATA unsigned char OSMSG_IPDNS[]
#ifdef OSMSGVALUE
	   = "DNS                  "
#endif
;

OSMSGDATA unsigned char OSMSG_ModuleDebug[]
#ifdef OSMSGVALUE
	   = "DEBUG OPTION   "
#endif
;

OSMSGDATA unsigned char OSMSG_ModuleSpeed[]
#ifdef OSMSGVALUE
	   = "MODULE SPEED         "
#endif
;
OSMSGDATA unsigned char OSMSG_GsmOldModuleSpeed[]
#ifdef OSMSGVALUE
	   = "MODULE OLD SPEED         "
#endif
;
OSMSGDATA unsigned char OSMSG_GsmNewModuleSpeed[]
#ifdef OSMSGVALUE
	   = "MODULE NEW SPEED         "
#endif
;
OSMSGDATA unsigned char OSMSG_ModuleAlwaysOnline[]
#ifdef OSMSGVALUE
	   = "MODULE ALWAYS ONLINE     "
#endif
;

OSMSGDATA unsigned char OSMSG_GprsAPN[]
#ifdef OSMSGVALUE
	   = "APN                  "
#endif
;

OSMSGDATA unsigned char OSMSG_GprsUsername[]
#ifdef OSMSGVALUE
	   = "USERNAME             "
#endif
;

OSMSGDATA unsigned char OSMSG_GprsPassword[]
#ifdef OSMSGVALUE
	   = "PASSWORD             "
#endif
;

OSMSGDATA unsigned char OSMSG_GsmSave[]
#ifdef OSMSGVALUE
	   = "SAVE TO MODULE...    "
#endif
;

OSMSGDATA unsigned char OSMSG_ContactHelpTele[]
#ifdef OSMSGVALUE
	   = "HELP TELEPHONE:      "
#endif
;

OSMSGDATA unsigned char OSMSG_OsOsDebugEnable[]
#ifdef OSMSGVALUE
	   = "OS DEBUG ENABLE?     "
#endif
;

OSMSGDATA unsigned char OSMSG_OsAppDebugEnable[]
#ifdef OSMSGVALUE
	   = "APP DEBUG ENABLE?    "
#endif
;

OSMSGDATA unsigned char OSMSG_PrintLine[]
#ifdef OSMSGVALUE
	   = "---------------------"
#endif
;

OSMSGDATA unsigned char OSMSG_DownloadMenu[]
#ifdef OSMSGVALUE
	   = "   DOWNLOAD MENU     "
#endif
;

OSMSGDATA unsigned char OSMSG_DownloadROM[]
#ifdef OSMSGVALUE
	   = "DOWNLOAD ROM         "
#endif
;

OSMSGDATA unsigned char OSMSG_DownloadOS[]
#ifdef OSMSGVALUE
	   = "DOWNLOAD OS          "
#endif
;
OSMSGDATA unsigned char OSMSG_Sequence[]
#ifdef OSMSGVALUE
	   = "APP ADJUST ORDER     "
#endif
;
OSMSGDATA unsigned char OSMSG_Invisible[]
#ifdef OSMSGVALUE
	   = "APP INVISIBLE     "
#endif
;
OSMSGDATA unsigned char OSMSG_DownloadFont[]
#ifdef OSMSGVALUE
	   = "DOWNLOAD FONT        "
#endif
;

OSMSGDATA unsigned char OSMSG_AppMenu[]
#ifdef OSMSGVALUE
	   = "  APPLICATION MENU   "
#endif
;

OSMSGDATA unsigned char OSMSG_AppMaintence[]
#ifdef OSMSGVALUE
	   = "APP MAINTENCE        "
#endif
;

OSMSGDATA unsigned char OSMSG_AppDownload[]
#ifdef OSMSGVALUE
	   = "APP DOWNLOAD         "
#endif
;

OSMSGDATA unsigned char OSMSG_AppDelete[]
#ifdef OSMSGVALUE
	   = "APP DELETE           "
#endif
;

OSMSGDATA unsigned char OSMSG_AppDeleteOK[]
#ifdef OSMSGVALUE
	   = "DELETE SCCESS        "
#endif
;


OSMSGDATA unsigned char OSMSG_WaitDownload[]
#ifdef OSMSGVALUE
	   = "WAIT DOWNLOAD        "
#endif
;
OSMSGDATA unsigned char OSMSG_Downloading[]
#ifdef OSMSGVALUE
	   = "DOWNLOADING...       "
#endif
;

OSMSGDATA unsigned char OSMSG_AskSure[]
#ifdef OSMSGVALUE
	   = "ARE YOU SURE?        "
#endif
;

OSMSGDATA unsigned char OSMSG_AskSave[]
#ifdef OSMSGVALUE
	   = "SAVE?                "
#endif
;

OSMSGDATA unsigned char OSMSG_Saving[]
#ifdef OSMSGVALUE
	   = "SAVING               "
#endif
;

OSMSGDATA unsigned char OSMSG_AskDelete[]
#ifdef OSMSGVALUE
	   = "DELETE?              "
#endif
;

OSMSGDATA unsigned char OSMSG_AppExist[]
#ifdef OSMSGVALUE
	   = "APP EXIST            "
#endif
;

OSMSGDATA unsigned char OSMSG_1U2O[]
#ifdef OSMSGVALUE
	   = "1-UPDATE 2-OVERWRITE "
#endif
;

OSMSGDATA unsigned char OSMSG_DevelopMenu[]
#ifdef OSMSGVALUE
	   = "    DEVELOP MENU     "
#endif
;

OSMSGDATA unsigned char OSMSG_DevelopNormal[]
#ifdef OSMSGVALUE
	   = "START NORMAL DEVELOP "
#endif
;

OSMSGDATA unsigned char OSMSG_DevelopAngel[]
#ifdef OSMSGVALUE
	   = "START ANGEL          "
#endif
;

OSMSGDATA unsigned char OSMSG_DevelopLoadDriver[]
#ifdef OSMSGVALUE
	   = "LOAD DRIVER...       "
#endif
;

OSMSGDATA unsigned char OSMSG_DevelopWait[]
#ifdef OSMSGVALUE
	   = "PLS START DEBUG IN PC"
#endif
;

OSMSGDATA unsigned char OSMSG_UnderDevelop[]
#ifdef OSMSGVALUE
	   = "UNDER DEVELOP...     "
#endif
;

OSMSGDATA unsigned char OSMSG_WaitDevelop[]
#ifdef OSMSGVALUE
	   = "PLEASE WAIT :-)      "
#endif
;

OSMSGDATA unsigned char OSMSG_TestMenu[]
#ifdef OSMSGVALUE
	   = "      TEST MENU      "
#endif
;

OSMSGDATA unsigned char OSMSG_TestMMI[]
#ifdef OSMSGVALUE
	   = "MMI                  "
#endif
;

OSMSGDATA unsigned char OSMSG_TestPrinter[]
#ifdef OSMSGVALUE
	   = "PRINTER              "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKeyboard[]
#ifdef OSMSGVALUE
	   = "KEYBOARD             "
#endif
;

OSMSGDATA unsigned char OSMSG_TestMagnetic[]
#ifdef OSMSGVALUE
	   = "MAGNETIC             "
#endif
;

OSMSGDATA unsigned char OSMSG_TestICC[]
#ifdef OSMSGVALUE
	   = "ICC                  "
#endif
;

OSMSGDATA unsigned char OSMSG_TestSAM[]
#ifdef OSMSGVALUE
	   = "SAM                  "
#endif
;

OSMSGDATA unsigned char OSMSG_TestUART[]
#ifdef OSMSGVALUE
	   = "UART                 "
#endif
;

OSMSGDATA unsigned char OSMSG_TestPinpad[]
#ifdef OSMSGVALUE
	   = "PINPAD               "
#endif
;

OSMSGDATA unsigned char OSMSG_TestModem[]
#ifdef OSMSGVALUE
	   = "MODEM                "
#endif
;

OSMSGDATA unsigned char OSMSG_TestGSM_Menu[]
#ifdef OSMSGVALUE
	   = "   GSM TEST MENU     "
#endif
;

OSMSGDATA unsigned char OSMSG_TestEthernet_Menu[]
#ifdef OSMSGVALUE
	   = " ETHERNET TEST MENU  "
#endif
;

OSMSGDATA unsigned char OSMSG_TestMemory[]
#ifdef OSMSGVALUE
	   = "MEMORY               "
#endif
;
OSMSGDATA unsigned char OSMSG_TestBarCode[]
#ifdef OSMSGVALUE
	   = "BARCODE PRINT        "
#endif
;
OSMSGDATA unsigned char OSMSG_TestAuto[]
#ifdef OSMSGVALUE
	   = "AUTOMATIC TEST"
#endif
;

OSMSGDATA unsigned char OSMSG_TestManual[]
#ifdef OSMSGVALUE
	   = "MANUAL TEST"
#endif
;
OSMSGDATA unsigned char OSMSG_TestMMITime[]
#ifdef OSMSGVALUE
	   = "TIME TEST"
#endif
;
OSMSGDATA unsigned char OSMSG_TestMMICurrentTime[]
#ifdef OSMSGVALUE
	   = "CURRENT TIME:"
#endif
;
OSMSGDATA unsigned char OSMSG_TestPressKeyContinue[]
#ifdef OSMSGVALUE
	   = "PRESS KEY TO CONTINUE"
#endif
;

OSMSGDATA unsigned char OSMSG_TestPressKeyCount[]
#ifdef OSMSGVALUE
	   = "REPEAT KEY COUNT"
#endif
;

OSMSGDATA unsigned char OSMSG_TestPressKeyReturn[]
#ifdef OSMSGVALUE
	   = "PRESS KEY TO RETURN  "
#endif
;

OSMSGDATA unsigned char OSMSG_TestEnd[]
#ifdef OSMSGVALUE
	   = "TEST END             "
#endif
;

OSMSGDATA unsigned char OSMSG_TestOK[]
#ifdef OSMSGVALUE
	   = "TEST OK!             "
#endif
;

OSMSGDATA unsigned char OSMSG_TestFail[]
#ifdef OSMSGVALUE
	   = "TEST FAIL!           "
#endif
;

OSMSGDATA unsigned char OSMSG_TestMMIMenu[]
#ifdef OSMSGVALUE
	   = "    MMI TEST MENU    "
#endif
;

OSMSGDATA unsigned char OSMSG_TestMMIBeep[]
#ifdef OSMSGVALUE
	   = "BEEP                 "
#endif
;

OSMSGDATA unsigned char OSMSG_TestMMIBacklightOn[]
#ifdef OSMSGVALUE
	   = "LCD BACKLIGHT IS ON  "
#endif
;

OSMSGDATA unsigned char OSMSG_TestMMIBacklightOff[]
#ifdef OSMSGVALUE
	   = "LCD BACKLIGHT IS OFF "
#endif
;

OSMSGDATA unsigned char OSMSG_TestMMIDisplayGB2312[]
#ifdef OSMSGVALUE
	   = "杉德公司测试"
#endif
;

OSMSGDATA unsigned char OSMSG_TestPRNTET[]
#ifdef OSMSGVALUE
	   = "PRINTER TEST        "
#endif
;

OSMSGDATA unsigned char OSMSG_TestPRN_FEED[]
#ifdef OSMSGVALUE
	   = "PRINTER LINE FEED   "
#endif
;

OSMSGDATA unsigned char OSMSG_TestPRNEL_FEED[]
#ifdef OSMSGVALUE
	   = "ENDLESS LOOP        "
#endif
;

OSMSGDATA unsigned char OSMSG_TestPRNTETCheckPaper[]
#ifdef OSMSGVALUE
	   = "CHECK PAPER TEST        "
#endif
;

OSMSGDATA unsigned char OSMSG_TestPRNMenu[]
#ifdef OSMSGVALUE
	   = " PRINTER TEST MENU"
#endif
;
OSMSGDATA unsigned char OSMSG_TestPRNCounts[]
#ifdef OSMSGVALUE
	   = "PRN TEST COUNTS:     "
#endif
;
OSMSGDATA unsigned char OSMSG_TestPRNInterval[]
#ifdef OSMSGVALUE
	   = "PRN INTERVAL(s):     "
#endif
;
OSMSGDATA unsigned char OSMSG_TestPRNCheckResult[]
#ifdef OSMSGVALUE
	   = "CHECK PRINTER PAPER  "
#endif
;


OSMSGDATA unsigned char OSMSG_TestKBDMenu[]
#ifdef OSMSGVALUE
	   = " KEYBOARD TEST MENU "
#endif
;
OSMSGDATA unsigned char OSMSG_TestKBDPOS[]
#ifdef OSMSGVALUE
	   = "POS KEY TEST        "
#endif
;
OSMSGDATA unsigned char OSMSG_TestKBDPS2[]
#ifdef OSMSGVALUE
	   = "PS2 KEY TEST        "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDValuePG[]
#ifdef OSMSGVALUE
	   = "CAPTURE           "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDValue0x30[]
#ifdef OSMSGVALUE
	   = "0                    "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDValue0x31[]
#ifdef OSMSGVALUE
	   = "1                    "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDValue0x32[]
#ifdef OSMSGVALUE
	   = "2                    "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDValue0x33[]
#ifdef OSMSGVALUE
	   = "3                    "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDValue0x34[]
#ifdef OSMSGVALUE
	   = "4                    "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDValue0x35[]
#ifdef OSMSGVALUE
	   = "5                    "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDValue0x36[]
#ifdef OSMSGVALUE
	   = "6                    "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDValue0x37[]
#ifdef OSMSGVALUE
	   = "7                    "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDValue0x38[]
#ifdef OSMSGVALUE
	   = "8                    "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDValue0x39[]
#ifdef OSMSGVALUE
	   = "9                    "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDValue0x3A[]
#ifdef OSMSGVALUE
	   = "ENTER                "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDValue0x3B[]
#ifdef OSMSGVALUE
	   = "CANCEL               "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDValue0x3C[]
#ifdef OSMSGVALUE
	   = "00                   "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDValue0x3D[]
#ifdef OSMSGVALUE
	   = "CLEAR                "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDValue0x3E[]
#ifdef OSMSGVALUE
	   = "FEED PAPER           "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDValue0x40[]
#ifdef OSMSGVALUE
	   = "F1                   "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDValue0x41[]
#ifdef OSMSGVALUE
	   = "F2                   "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDValue0x42[]
#ifdef OSMSGVALUE
	   = "F3                   "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDValue0x43[]
#ifdef OSMSGVALUE
	   = "F4                   "
#endif
;
OSMSGDATA unsigned char OSMSG_TestKBDValue0x44[]
#ifdef OSMSGVALUE
	   = "PAPER UP             "
#endif
;
OSMSGDATA unsigned char OSMSG_TestKBDValue0x4B[]
#ifdef OSMSGVALUE
	   = "PAPER DOWN           "
#endif
;

OSMSGDATA unsigned char OSMSG_TestKBDExit[]
#ifdef OSMSGVALUE
	   = "TWO CANCEL TO EXIT   "
#endif
;

OSMSGDATA unsigned char OSMSG_TestMAGMenu[]
#ifdef OSMSGVALUE
	   = "SWIPE MAGNETIC CARD "
#endif
;

OSMSGDATA unsigned char OSMSG_TestMAGTrack1[]
#ifdef OSMSGVALUE
	   = "TRACK 1 "
#endif
;

OSMSGDATA unsigned char OSMSG_TestMAGTrack2[]
#ifdef OSMSGVALUE
	   = "TRACK 2 "
#endif
;

OSMSGDATA unsigned char OSMSG_TestMAGTrack3[]
#ifdef OSMSGVALUE
	   = "TRACK 3 "
#endif
;




OSMSGDATA unsigned char OSMSG_TestMAGExit[]
#ifdef OSMSGVALUE
	   = "PRESS CANCEL TO EXIT"
#endif
;

OSMSGDATA unsigned char OSMSG_TestICCMenu[]
#ifdef OSMSGVALUE
	   = "   ICC TEST MENU    "
#endif
;

OSMSGDATA unsigned char OSMSG_TestICCAsync[]
#ifdef OSMSGVALUE
	   = "ASYNC CARD TEST     "
#endif
;

OSMSGDATA unsigned char OSMSG_TestICC4442[]
#ifdef OSMSGVALUE
	   = "4442 CARD TEST      "
#endif
;
OSMSGDATA unsigned char OSMSG_TestICC4428[]
#ifdef OSMSGVALUE
	   = "4428 CARD TEST      "
#endif
;
OSMSGDATA unsigned char OSMSG_TestICCInsertAsync[]
#ifdef OSMSGVALUE
	   = "INSERT PBOC CARD    "
#endif
;

OSMSGDATA unsigned char OSMSG_TestICCInsert4442[]
#ifdef OSMSGVALUE
	   = "INSERT 4442 CARD    "
#endif
;
OSMSGDATA unsigned char OSMSG_TestICCInsert4428[]
#ifdef OSMSGVALUE
	   = "INSERT 4428 CARD    "
#endif
;
OSMSGDATA unsigned char OSMSG_TestPutMFCCard[]
#ifdef OSMSGVALUE
	   = "PUT MIFARE CARD   "
#endif
;
OSMSGDATA unsigned char OSMSG_TestICCATR[]
#ifdef OSMSGVALUE
	   = "ANSWER TO RESET:"
#endif
;

OSMSGDATA unsigned char OSMSG_TestICCSelectFile[]
#ifdef OSMSGVALUE
	   = "SELECT 1PAY.SYS.DDF01"
#endif
;

OSMSGDATA unsigned char OSMSG_TestICCSyncData[]
#ifdef OSMSGVALUE
	   = "FIRST 32 BYTES DATA  "
#endif
;

OSMSGDATA unsigned char OSMSG_TestICCRemoveCard[]
#ifdef OSMSGVALUE
	   = "REMOVE CARD          "
#endif
;

OSMSGDATA unsigned char OSMSG_TestICCExit[]
#ifdef OSMSGVALUE
	   = "PRESS CANCEL TO EXIT "
#endif
;

OSMSGDATA unsigned char OSMSG_TestSAMMenu[]
#ifdef OSMSGVALUE
	   = "SAM NUMBER:          "
#endif
;

OSMSGDATA unsigned char OSMSG_TestSAMSelectFile[]
#ifdef OSMSGVALUE
	   = "SELECT 1PAY.SYS.DDF01"
#endif
;

OSMSGDATA unsigned char OSMSG_TestUARTMenu[]
#ifdef OSMSGVALUE
	   = "   UART TEST MENU    "
#endif
;

OSMSGDATA unsigned char OSMSG_TestUARTCom12[]
#ifdef OSMSGVALUE
	   = "COM1&2 TEST          "
#endif
;

OSMSGDATA unsigned char OSMSG_TestUARTCom1[]
#ifdef OSMSGVALUE
	   = "COM2 RECE TEST            "
#endif
;


OSMSGDATA unsigned char OSMSG_TestUARTCom2[]
#ifdef OSMSGVALUE
	   = "COM2 TEST            "
#endif
;

OSMSGDATA unsigned char OSMSG_TestUARTUsb[]
#ifdef OSMSGVALUE
	   = "USB TEST            "
#endif
;


OSMSGDATA unsigned char OSMSG_TestUARTPressChar[]
#ifdef OSMSGVALUE
	   = "PRESS CHAR IN PC...  "
#endif
;

OSMSGDATA unsigned char OSMSG_TestUARTExit[]
#ifdef OSMSGVALUE
	   = "PRESS CANCEL TO EXIT "
#endif
;

OSMSGDATA unsigned char OSMSG_TestPADMenu[]
#ifdef OSMSGVALUE
	   = "  PINPAD TEST MENU   "
#endif
;

OSMSGDATA unsigned char OSMSG_TestPADDisplay[]
#ifdef OSMSGVALUE
	   = "PINPAD DISPLAY       "
#endif
;
OSMSGDATA unsigned char OSMSG_TestPADEncrypt[]
#ifdef OSMSGVALUE
	   = "ENCRYPT AND CRYPT    "
#endif
;
OSMSGDATA unsigned char OSMSG_TestMFCCard[]
#ifdef OSMSGVALUE
	   = "MFC CARD DETECT	      "
#endif
;
OSMSGDATA unsigned char OSMSG_TestEncryptChoiceInfo[]
#ifdef OSMSGVALUE
	   = "ENCRYPT OPTION       "
#endif
;
OSMSGDATA unsigned char OSMSG_TestEncryptOption[]
#ifdef OSMSGVALUE
	   = "(0.PAD    1.MMI)     "
#endif
;

OSMSGDATA unsigned char OSMSG_TestPADLoadKeyOK[]
#ifdef OSMSGVALUE
	   = "PINPAD LOAD KEY OK   "
#endif
;
OSMSGDATA unsigned char OSMSG_TestPADLoadKeyFail[]
#ifdef OSMSGVALUE
	   = "PINPAD LOAD KEY FAIL "
#endif
;
OSMSGDATA unsigned char OSMSG_TestPADEncryptOK[]
#ifdef OSMSGVALUE
	   = "ENCRYPT OK           "
#endif
;
OSMSGDATA unsigned char OSMSG_TestPADEncryptFail[]
#ifdef OSMSGVALUE
	   = "ENCRYPT FAIL         "
#endif
;
OSMSGDATA unsigned char OSMSG_TestPADEncryptValueOK[]
#ifdef OSMSGVALUE
	   = "ENCRYPT VALUE OK     "
#endif
;
OSMSGDATA unsigned char OSMSG_TestPADEncryptValueFail[]
#ifdef OSMSGVALUE
	   = "ENCRYPT VALUE FAIL   "
#endif
;
OSMSGDATA unsigned char OSMSG_TestPADCryptOK[]
#ifdef OSMSGVALUE
	   = "CRYPT OK           "
#endif
;
OSMSGDATA unsigned char OSMSG_TestPADCryptFail[]
#ifdef OSMSGVALUE
	   = "CRYPT FAIL         "
#endif
;
OSMSGDATA unsigned char OSMSG_TestPADCryptValueOK[]
#ifdef OSMSGVALUE
	   = "CRYPT VALUE OK     "
#endif
;
OSMSGDATA unsigned char OSMSG_TestPADCryptValueFail[]
#ifdef OSMSGVALUE
	   = "CRYPT VALUE FAIL   "
#endif
;


OSMSGDATA unsigned char OSMSG_TestPADBeep[]
#ifdef OSMSGVALUE
	   = "PINPAD BEEP          "
#endif
;

OSMSGDATA unsigned char OSMSG_TestPADBacklightOn[]
#ifdef OSMSGVALUE
	   = "LCD BACKLIGHT IS ON  "
#endif
;

OSMSGDATA unsigned char OSMSG_TestPADBacklightOff[]
#ifdef OSMSGVALUE
	   = "LCD BACKLIGHT IS OFF"
#endif
;

OSMSGDATA unsigned char OSMSG_TestPADDisplayGB2312[]
#ifdef OSMSGVALUE
	   = "杉德公司测试"
#endif
;

OSMSGDATA unsigned char OSMSG_TestPADKeyboard[]
#ifdef OSMSGVALUE
	   = "PINPAD KEYBOARD      "
#endif
;

OSMSGDATA unsigned char OSMSG_TestPADExit[]
#ifdef OSMSGVALUE
	   = "PRESS CANCEL TO EXIT "
#endif
;

OSMSGDATA unsigned char OSMSG_TestMODMenu[]
#ifdef OSMSGVALUE
	   = "   MODEM TEST MENU   "
#endif
;

OSMSGDATA unsigned char OSMSG_TestModemAsync[]
#ifdef OSMSGVALUE
	   = "ASYNC MODEM TEST     "
#endif
;

OSMSGDATA unsigned char OSMSG_TestModemHDLC[]
#ifdef OSMSGVALUE
	   = "HDLC MODEM TEST      "
#endif
;

OSMSGDATA unsigned char OSMSG_TestModemInputPhoneNB[]
#ifdef OSMSGVALUE
	   = "INPUT PHONE NUMBER:  "
#endif
;

OSMSGDATA unsigned char OSMSG_TestModemInputTestNB[]
#ifdef OSMSGVALUE
	   = "INPUT TEST NUMBER:   "
#endif
;

OSMSGDATA unsigned char OSMSG_TestModemInputCharNB[]
#ifdef OSMSGVALUE
	   = "INPUT CHARACTERS NUM:"
#endif
;

OSMSGDATA unsigned char OSMSG_TestModemDial[]
#ifdef OSMSGVALUE
	   = "DIAL...              "
#endif
;

OSMSGDATA unsigned char OSMSG_TestModemSend[]
#ifdef OSMSGVALUE
	   = "SENDING...           "
#endif
;

OSMSGDATA unsigned char OSMSG_TestModemRecv[]
#ifdef OSMSGVALUE
	   = "RECEIVING...         "
#endif
;

OSMSGDATA unsigned char OSMSG_TestGprsDial[]
#ifdef OSMSGVALUE
	   = "GPRS DIALING...      "
#endif
;

OSMSGDATA unsigned char OSMSG_TestGprsCheckDial[]
#ifdef OSMSGVALUE
	   = "GPRS DIAL CHECKING.. "
#endif
;

OSMSGDATA unsigned char OSMSG_TestMEMMenu[]
#ifdef OSMSGVALUE
	   = "  MEMORY TEST MENU   "
#endif
;

OSMSGDATA unsigned char OSMSG_TestMEMCache[]
#ifdef OSMSGVALUE
	   = "CACHE TEST           "
#endif
;

OSMSGDATA unsigned char OSMSG_TestMEMSDRAM[]
#ifdef OSMSGVALUE
	   = "SDRAM TEST           "
#endif
;
OSMSGDATA unsigned char OSMSG_TestSRAMSIZE[]
#ifdef OSMSGVALUE
	   = "SRAM SIZE           "
#endif
;
OSMSGDATA unsigned char OSMSG_TestMEMSRAM[]
#ifdef OSMSGVALUE
	   = "SRAM TEST            "
#endif
;

OSMSGDATA unsigned char OSMSG_TestInputBarCode[]
#ifdef OSMSGVALUE
	   = "INPUT BARCODE       "
#endif
;

OSMSGDATA unsigned char OSMSG_AdminMenu[]
#ifdef OSMSGVALUE
	   = "     ADMIN MENU      "
#endif
;

OSMSGDATA unsigned char OSMSG_AdminDataToPC[]
#ifdef OSMSGVALUE
	   = "DATA TO PC           "
#endif
;

OSMSGDATA unsigned char OSMSG_AdminDataFromPC[]
#ifdef OSMSGVALUE
	   = "DATA FROM PC         "
#endif
;

OSMSGDATA unsigned char OSMSG_AdminRebuildFileSystem[]
#ifdef OSMSGVALUE
	   = "REBUILD FILE SYSTEM  "
#endif
;





OSMSGDATA unsigned char OSTESTMSG_Startup[]
#ifdef OSMSGVALUE
	   = "SYSTEM SELFTEST...   "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_Config[]
#ifdef OSMSGVALUE
	   = "CONFIG TEST...       "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_ConfigError[]
#ifdef OSMSGVALUE
	   = "CONFIG ERROR!!!      "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_Cache[]
#ifdef OSMSGVALUE
	   = "CACHE TEST...        "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_CacheError[]
#ifdef OSMSGVALUE
	   = "CACHE ERROR!!!       "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_Battery[]
#ifdef OSMSGVALUE
	   = "BATTERY TEST...      "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_BatteryError[]
#ifdef OSMSGVALUE
	   = "BATTERY ERROR!!!     "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_Ethernet[]
#ifdef OSMSGVALUE
	   = "ETHERNET TEST...     "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_EthernetError[]
#ifdef OSMSGVALUE
	   = "ETHERNET ERROR!!!    "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_ICC[]
#ifdef OSMSGVALUE
	   = "ICC TEST...          "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_ICCError[]
#ifdef OSMSGVALUE
	   = "ICC ERROR!!!         "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_Printer[]
#ifdef OSMSGVALUE
	   = "PRINTER TEST...      "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_PrinterError[]
#ifdef OSMSGVALUE
	   = "PRINTER ERROR!!!     "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_PrinteDotError[]
#ifdef OSMSGVALUE
	   = "PRINTEDOT ERROR!!!   "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_Modem[]
#ifdef OSMSGVALUE
	   = "MODEM TEST...        "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_ModemError[]
#ifdef OSMSGVALUE
	   = "MODEM ERROR!!!       "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_GBDot[]
#ifdef OSMSGVALUE
	   = "GB2312 DOT TEST...   "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_GBDotError[]
#ifdef OSMSGVALUE
	   = "GB2312 DOT ERROR!!!  "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_Memory[]
#ifdef OSMSGVALUE
	   = "MEMORY TEST...       "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_MemoryError[]
#ifdef OSMSGVALUE
	   = "SDRAM ERROR!!!       "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_OK[]
#ifdef OSMSGVALUE
	   = "SELFTEST OK:-)       "
#endif
;

OSMSGDATA unsigned char OSTESTMSG_Error[]
#ifdef OSMSGVALUE
	   = "TEST ERROR!!!        "
#endif
;




OSMSGDATA unsigned char OSERRMSG_KeyboardError[]
#ifdef OSMSGVALUE
	   = "ERR: KEYBOARD ERROR! "
#endif
;

OSMSGDATA unsigned char OSERRMSG_WrongMenuPassword[]
#ifdef OSMSGVALUE
	   = "ERR: WRONG PASSWORD! "
#endif
;

OSMSGDATA unsigned char OSERRMSG_NotSetting[]
#ifdef OSMSGVALUE
	   = "NO SETTING!!!        "
#endif
;

OSMSGDATA unsigned char OSERRMSG_WrongDate[]
#ifdef OSMSGVALUE
	   = "ERR: WRONG DATE!!!   "
#endif
;

OSMSGDATA unsigned char OSERRMSG_WrongTime[]
#ifdef OSMSGVALUE
	   = "ERR: WRONG TIME!!!   "
#endif
;
OSMSGDATA unsigned char OSERRMSG_WrongOS[]
#ifdef OSMSGVALUE
	   = "ERR: WRONG OS!!!   "
#endif
;
OSMSGDATA unsigned char OSERRMSG_DownloadOK[]
#ifdef OSMSGVALUE
	   = "DOWNLOAD OK!         "
#endif
;

OSMSGDATA unsigned char OSERRMSG_DownloadTimeout[]
#ifdef OSMSGVALUE
	   = "ERR: DOWNLOAD TIMEOUT"
#endif
;

OSMSGDATA unsigned char OSERRMSG_OSBadSize[]
#ifdef OSMSGVALUE
	   = "ERR: OS BAD SIZE     "
#endif
;

OSMSGDATA unsigned char OSERRMSG_OSBadChecksum[]
#ifdef OSMSGVALUE
	   = "ERR: OS BAD CHECKSUM "
#endif
;

OSMSGDATA unsigned char OSERRMSG_FontBadSize[]
#ifdef OSMSGVALUE
	   = "ERR: FONT BAD SIZE   "
#endif
;

OSMSGDATA unsigned char OSERRMSG_FontBadChecksum[]
#ifdef OSMSGVALUE
	   = "ERR: FONT BAD CHK    "
#endif
;

OSMSGDATA unsigned char OSERRMSG_APPNotExist[]
#ifdef OSMSGVALUE
	   = "ERR: APP NOT EXIST   "
#endif
;

OSMSGDATA unsigned char OSERRMSG_APPBadSize[]
#ifdef OSMSGVALUE
	   = "ERR: APP BAD SIZE    "
#endif
;

OSMSGDATA unsigned char OSERRMSG_APPBadChecksum[]
#ifdef OSMSGVALUE
	   = "ERR: APP BAD CHECKSUM"
#endif
;

OSMSGDATA unsigned char OSERRMSG_SavedOK[]
#ifdef OSMSGVALUE
	   = "SAVED OK!            "
#endif
;

OSMSGDATA unsigned char OSERRMSG_SavedError[]
#ifdef OSMSGVALUE
	   = "ERR: WRITE FLASH     "
#endif
;

OSMSGDATA unsigned char OSERRMSG_SaveGSMModuleError[]
#ifdef OSMSGVALUE
	   = "ERR: SAVE TO GSM     "
#endif
;

OSMSGDATA unsigned char OSERRMSG_PRNUnknown[]
#ifdef OSMSGVALUE
	   = "ERR: UNKNOWN PRINTER "
#endif
;

OSMSGDATA unsigned char OSERRMSG_ICCDriver[]
#ifdef OSMSGVALUE
	   = "ICC DRIVER ERROR!    "
#endif
;

OSMSGDATA unsigned char OSERRMSG_ICCAsyncReset[]
#ifdef OSMSGVALUE
	   = "ERR: RESET RESULT    "
#endif
;

OSMSGDATA unsigned char OSERRMSG_ICCSyncReset[]
#ifdef OSMSGVALUE
	   = "ERR: 4442 RESET      "
#endif
;

OSMSGDATA unsigned char OSERRMSG_ICCSyncRead[]
#ifdef OSMSGVALUE
	   = "ERR: 4442 READ       "
#endif
;

OSMSGDATA unsigned char OSERRMSG_SAMSelectFile[]
#ifdef OSMSGVALUE
	   = "ERR: SAM SELECT FILE "
#endif
;

OSMSGDATA unsigned char OSERRMSG_UARTTestOK[]
#ifdef OSMSGVALUE
	   = "COM1&2 TEST OK!      "
#endif
;

OSMSGDATA unsigned char OSERRMSG_InitUART[]
#ifdef OSMSGVALUE
	   = "ERR: UART INIT       "
#endif
;

OSMSGDATA unsigned char OSERRMSG_UARTSendRecv[]
#ifdef OSMSGVALUE
	   = "ERR: UART SEND&RECV  "
#endif
;

OSMSGDATA unsigned char OSERRMSG_ModemDial[]
#ifdef OSMSGVALUE
	   = "ERR: DIAL ERROR!!    "
#endif
;

OSMSGDATA unsigned char OSERRMSG_ModemHDLCTestOK[]
#ifdef OSMSGVALUE
	   = "MODEM HDLC TEST OK!! "
#endif
;

OSMSGDATA unsigned char OSERRMSG_ModemASYNCTestOK[]
#ifdef OSMSGVALUE
	   = "MODEM ASYNC TEST OK!!"
#endif
;

OSMSGDATA unsigned char OSERRMSG_ModemHDLCRecvTimeout[]
#ifdef OSMSGVALUE
	   = "ERR: RECV DATA TIMEOU"
#endif
;

OSMSGDATA unsigned char OSERRMSG_ModemHDLCSendData[]
#ifdef OSMSGVALUE
	   = "ERR: SEND DATA       "
#endif
;

OSMSGDATA unsigned char OSERRMSG_ModemHDLCRecvData[]
#ifdef OSMSGVALUE
	   = "ERR: BAD RECV DATA   "
#endif
;

OSMSGDATA unsigned char OSERRMSG_ModemNotExist[]
#ifdef OSMSGVALUE
	   = "ERR: MODEM NOT EXIST "
#endif
;

OSMSGDATA unsigned char OSERRMSG_EthNotExist[]
#ifdef OSMSGVALUE
	   = "ERR: ETH NOT EXIST   "
#endif
;

OSMSGDATA unsigned char OSERRMSG_EthConfig[]
#ifdef OSMSGVALUE
	   = "ERR: ETH NOT CONFIG  "
#endif
;

OSMSGDATA unsigned char OSERRMSG_ModuleNotExist[]
#ifdef OSMSGVALUE
	   = "ERR: MODULE NOT EXIST"
#endif
;

OSMSGDATA unsigned char OSERRMSG_MEMTestOK[]
#ifdef OSMSGVALUE
	   = "MEMORY TEST OK!     "
#endif
;

OSMSGDATA unsigned char OSERRMSG_MEMCache[]
#ifdef OSMSGVALUE
	   = "ERR: CACHE TEST FAIL!"
#endif
;

OSMSGDATA unsigned char OSERRMSG_MEMSDRAM[]
#ifdef OSMSGVALUE
	   = "ERR: SDRAM TEST FAIL!"
#endif
;

OSMSGDATA unsigned char OSERRMSG_MEMSRAM[]
#ifdef OSMSGVALUE
	   = "ERR: SRAM TEST FAIL! "
#endif
;
OSMSGDATA unsigned char OSERRMSG_SRAMSIZE[]
#ifdef OSMSGVALUE
	   = "ERR: SRAM SIZE ERROR! "
#endif
;
OSMSGDATA unsigned char OSERRMSG_MEMCannotTest[]
#ifdef OSMSGVALUE
	   = "ERR: CANNOT DO TEST!"
#endif
;
OSMSGDATA unsigned char OSMSG_HOSTIPADDR[]
#ifdef OSMSGVALUE
	   = "HOST IP              "
#endif
;

OSMSGDATA unsigned char OSMSG_PORT[]
#ifdef OSMSGVALUE
	   = "PORT                 "
#endif
;

OSMSGDATA unsigned char OSMSG_TestFactory[]
#ifdef OSMSGVALUE
	   = "CONNECT CHECK        "
#endif
;

OSMSGDATA unsigned char OSMSG_TestConnect[]
#ifdef OSMSGVALUE
	   = "CONNECT TEST         "
#endif
;

OSMSGDATA unsigned char OSMSG_Answer_Calling[]
#ifdef OSMSGVALUE
	   = "GSM INCOMING CALL   "
#endif
;

OSMSGDATA unsigned char OSMSG_ConnectTest[]
#ifdef OSMSGVALUE
	   = "GSM CONNECT TEST   "
#endif
;

OSMSGDATA unsigned char OSMSG_TEST_SET_FREQ[]
#ifdef OSMSGVALUE
	   = "GSM BANDS          "
#endif
;

OSMSGDATA unsigned char OSMSG_TestDNS[]
#ifdef OSMSGVALUE
	   = "DNS                "
#endif
;
OSMSGDATA unsigned char OSMSG_DOMAINNAME[]
#ifdef OSMSGVALUE
	   = "DOMAIN NAME          "
#endif
;
OSMSGDATA unsigned char OSMSG_Connecting[]
#ifdef OSMSGVALUE
	   = "Connecting...        "
#endif
;
OSMSGDATA unsigned char OSMSG_BaseUSBNotExist[]
#ifdef OSMSGVALUE
	   = "USB       NOT EXIST  "
#endif
;
OSMSGDATA unsigned char OSMSG_BaseUSBExistAsk[]
#ifdef OSMSGVALUE
	   = "USB EXIST?           "
#endif
;
OSMSGDATA unsigned char OSMSG_BaseUSBExist[]
#ifdef OSMSGVALUE
	   = "USB       EXIST  "
#endif
;
OSMSGDATA unsigned char OSTESTMSG_USBError[]
#ifdef OSMSGVALUE
	   = "USB ERROR!!!   "
#endif
;

OSMSGDATA unsigned char OSERRMSG_USBConfig[]
#ifdef OSMSGVALUE
	   = "ERR: USB NOT CONFIG  "
#endif
;

OSMSGDATA unsigned char OSMSG_ProgramDownload[]  
#ifdef OSMSGVALUE
		= "PROGRAM DOWNLOAD...     ";
#endif
;

OSMSGDATA unsigned char OSMSG_ConfigDownload[]  
#ifdef OSMSGVALUE
		= "CONFIG DOWNLOAD...     ";
#endif
;

OSMSGDATA unsigned char OSMSG_CoverProgram[]  
#ifdef OSMSGVALUE
		= "COVER PROGRAM...     ";
#endif
;

OSMSGDATA unsigned char OSMSG_Configupload[]  
#ifdef OSMSGVALUE
		= "CONFIG UPLOAD...     ";
#endif
;

OSMSGDATA unsigned char OSMSG_AllDownload[]  
#ifdef OSMSGVALUE
		= "All DOWNLOAD...     ";
#endif
;
OSMSGDATA unsigned char OSMSG_Updating[]  	 
#ifdef OSMSGVALUE
		= "UPDATING ...        ";
#endif
;
OSMSGDATA unsigned char OSMSG_Overwriting[]  
#ifdef OSMSGVALUE
		= "OVERWRITING ...     ";
#endif
;
OSMSGDATA unsigned char OSMSG_AllLoadOK[]  	 
#ifdef OSMSGVALUE
		= "LOAD AND SAVED OK   ";
#endif
;
OSMSGDATA unsigned char OSMSG_SystemReboot[]
#ifdef OSMSGVALUE
		= "SYSTEM WILL REBOOT  ";
#endif
;
OSMSGDATA unsigned char OSMSG_PressAnyKey[]  
#ifdef OSMSGVALUE
		= "PRESS ANY KEY FINISH";
#endif
;
OSMSGDATA unsigned char OSMSG_UpdateOverwrite[] 
#ifdef OSMSGVALUE
		= "1.UPDATE 2.OVERWRITE";
#endif
;
OSMSGDATA unsigned char OSMSG_FileDeleting[] 
#ifdef OSMSGVALUE
		= "FILE DELETING ...   ";
#endif
;
OSMSGDATA unsigned char OSMSG_FileDelete[]   
#ifdef OSMSGVALUE
		= "1.DELETE 2.CANCEL   ";
#endif
;



OSMSGDATA unsigned char OSMSG_GPRS_Dial[]   
#ifdef OSMSGVALUE
		= "GPRS-Dial  ";
#endif
;
OSMSGDATA unsigned char OSMSG_Close_GPRS[]   
#ifdef OSMSGVALUE
		= "Close-GPRS  ";
#endif
;
OSMSGDATA unsigned char OSMSG_GPRS_Set_APN[]   
#ifdef OSMSGVALUE
		= "GPRS-Set-APN";
#endif
;

OSMSGDATA unsigned char OSMSG_GPRS_Check_Dial[]   
#ifdef OSMSGVALUE
		= "GPRS-Check-Dial";
#endif
;

OSMSGDATA unsigned char OSMSG_GPRS_PowerOn[]   
#ifdef OSMSGVALUE
		= "GPRS-PowerOn";
#endif
;

OSMSGDATA unsigned char OSMSG_GPRS_Reset[]   
#ifdef OSMSGVALUE
		= "GPRS-Reset";
#endif
;

OSMSGDATA unsigned char OSMSG_GPRS_HW_Reset[]   
#ifdef OSMSGVALUE
		= "GPRS-HW-Reset";
#endif
;

#if 0
OSMSGDATA unsigned char OSMSG_GPRS_AT_CMD[]   
#ifdef OSMSGVALUE
		= "AT-CMD";
#endif
;
#endif

OSMSGDATA unsigned char OSMSG_GPRS_SMART_DIAl[]   
#ifdef OSMSGVALUE
		= "GPRS-SMART-DIAL";
#endif
;



#endif
