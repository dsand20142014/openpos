              bRadioOff;
    BOOL                    bEnablePSMode;
    WORD                    wListenInterval;
    BOOL                    bPWBitOn;
    WMAC_POWER_MODE         ePSMode;


    // GPIO Radio Control
    BYTE                    byRadioCtl;
    BYTE                    byGPIO;
    BOOL                    bHWRadioOff;
    BOOL                    bPrvActive4RadioOFF;
    BOOL                    bGPIOBlockRead;

    // Beacon releated
    WORD                    wSeqCounter;
    WORD                    wBCNBufLen;
    BOOL                    bBeaconBufReady;
    BOOL                    bBeaconSent;
    BOOL                    bIsBeaconBufReadySet;
    UINT                    cbBeaconBufReadySetCnt;
    BOOL                    bFixRate;
    BYTE                    byCurrentCh;
    UINT                    uScanTime;

    CMD_STATE               eCommandState;

    CMD_CODE                eCommand;
    BOOL                    bBeaconTx;

    BOOL                    bStopBeacon;
    BOOL                    bStopDataPkt;
    BOOL                    bStopTx0Pkt;
    UINT                    uAutoReConnectTime;

    // 802.11 counter

    CMD_ITEM                eCmdQueue[CMD_Q_SIZE];
    UINT                    uCmdDequeueIdx;
    UINT                    uCmdEnqueueIdx;
    UINT                    cbFreeCmdQueue;
    BOOL                    bCmdRunning;
    BOOL                    bCmdClear;



    BOOL                    bRoaming;
    //WOW
    BYTE                    abyIPAddr[4];

    ULONG                   ulTxPower;
    NDIS_802_11_WEP_STATUS  eEncryptionStatus;
    BOOL                    bTransmitKey;
//2007-0925-01<Add>by MikeLiu
//mike add :save old Encryption
    NDIS_802_11_WEP_STATUS  eOldEncryptionStatus;
    SKeyManagement          sKey;
    DWORD                   dwIVCounter;

    QWORD                   qwPacketNumber; //For CCMP and TKIP as TSC(6 bytes)
    UINT                    uCurrentWEPMode;

    RC4Ext                  SBox;
    BYTE                    abyPRNG[WLAN_WEPMAX_KEYLEN+3];

    BYTE                    byKeyIndex;
    UINT                    uKeyLength;
    BYTE                    abyKey[WLAN_WEP232_KEYLEN];

    BOOL                    bAES;
    BYTE                    byCntMeasure;

    // for AP mode
    UINT                    uAssocCount;
    BOOL                    bMoreData;

    // QoS
    BOOL                    bGrpAckPolicy;

    // for OID_802_11_ASSOCIATION_INFORMATION
    BOOL                    bAssocInfoSet;


    BYTE                    byAutoFBCtrl;

    BOOL                    bTxMICFail;
    BOOL                    bRxMICFail;


    UINT                    uRATEIdx;


    // For Update BaseBand VGA Gain Offset
    BOOL                    bUpdateBBVGA;
    UINT                    uBBVGADiffCount;
    BYTE                    byBBVGANew;
    BYTE                    byBBVGACurrent;
    BYTE                    abyBBVGA[BB_VGA_LEVEL];
    LONG                    ldBmThreshold[BB_VGA_LEVEL];

    BYTE                    byBBPreEDRSSI;
    BYTE                    byBBPreEDIndex;

    BOOL                    bRadioCmd;
    DWORD                   dwDiagRefCount;

    // For FOE Tuning
    BYTE                    byFOETuning;

    // For Auto Power Tunning

    BYTE                    byAutoPwrTunning;
    SHORT                   sPSetPointCCK;
    SHORT                   sPSetPointOFDMG;
    SHORT                   sPSetPointOFDMA;
    LONG                    lPFormulaOffset;
    SHORT                   sPThreshold;
    CHAR                    cAdjustStep;
    CHAR                    cMinTxAGC;

    // For RF Power table
    BYTE                    byCCKPwr;
    BYTE                    byOFDMPwrG;
    BYTE                    byCurPwr;
    I8                      byCurPwrdBm;
    BYTE                    abyCCKPwrTbl[CB_MAX_CHANNEL_24G+1];
    BYTE                    abyOFDMPwrTbl[CB_MAX_CHANNEL+1];
    I8                      abyCCKDefaultPwr[CB_MAX_CHANNEL_24G+1];
    I8                      abyOFDMDefaultPwr[CB_MAX_CHANNEL+1];
    I8                      abyRegPwr[CB_MAX_CHANNEL+1];
    I8                      abyLocalPwr[CB_MAX_CHANNEL+1];


    // BaseBand Loopback Use
    BYTE                    byBBCR4d;
    BYTE                    byBBCRc9;
    BYTE                    byBBCR88;
    BYTE                    byBBCR09;

    // command timer
    struct timer_list       sTimerCommand;
#ifdef TxInSleep
     struct timer_list       sTimerTxData;
     ULONG                       nTxDataTimeCout;
     BOOL  fTxDataInSleep;
     BOOL  IsTxDataTrigger;
#endif

#ifdef WPA_SM_Transtatus
    BOOL  fWPA_Authened;           //is WPA/WPA-PSK or WPA2/WPA2-PSK authen??
#endif
    BYTE            byReAssocCount;   //mike add:re-association retry times!
    BYTE            byLinkWaitCount;


    BYTE                    abyNodeName[17];

    BOOL                    bDiversityRegCtlON;
    BOOL                    bDiversityEnable;
    ULONG                   ulDiversityNValue;
    ULONG               