#include "keytest.h"
//#include "ui_keytest.h"

extern "C"{
#include "encrypt.h"
//#include "applist.h"

}

extern void _print(char *fmt,...);

KeyTest::KeyTest(QWidget *parent) :
    BaseForm(tr("KEY TEST"),parent)
{
    btndes = new Button(tr("  1.DES TEST  "),9,this);
    btn3des = new Button(tr("  2.3DES TEST  "),9,this);

    QTextEdit *textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(btndes);
    vbox->setSpacing(40);
    vbox->addWidget(btn3des);
    vbox->addStretch(1);

    vbox->setContentsMargins(30,30,30,10);
    textEdit->setLayout(vbox);

    layout->addWidget(textEdit);


    btnExit->setFocus();

//   _print("APPLIST_get_appid==%d \n",APPLIST_get_appid());

    connect(btndes,SIGNAL(clicked()),this,SLOT(btndes_slot()));
    connect(btn3des,SIGNAL(clicked()),this,SLOT(btn3des_slot()));

}

KeyTest::~KeyTest()
{

}


void KeyTest::btndes_slot()
{
    int rec = 0;
    ENCRYPT_DRV_DATA encrypt_data;
    unsigned char aucBuf[17];

    for(int i=0;i<5;i++)
    {
         for(int j=0;j<5;j++)
         {
             memset(&encrypt_data,0,sizeof(ENCRYPT_DRV_DATA));
             encrypt_data.keytype = ENCRYPT_DECRYPT_BYTRANSKEY;
             encrypt_data.cryptindex = i*100+0;
             encrypt_data.crypttype = ENCRYPT_DES_SINGLE;
             memcpy(encrypt_data.keyvalue,"3132333435363738",16);
             encrypt_data.cryptflag = ENCRYPT_FUNCTION_NORMAL;
             _print("\n0encrypt_data.keytype = %02x,encrypt_data.crypttype=%02x \n",encrypt_data.keytype,encrypt_data.crypttype);
             rec = ENCRYPT_Decrypt_Load_Key(&encrypt_data);
             if(rec)
                 break;
             memset(aucBuf,0,sizeof(aucBuf));
             memcpy(aucBuf,encrypt_data.outdata,encrypt_data.outlen);

             memset(&encrypt_data,0,sizeof(ENCRYPT_DRV_DATA));
             encrypt_data.keytype = ENCRYPT_SAVE_BYTRANSKEY;
             encrypt_data.cryptindex = i*100+0;
             encrypt_data.crypttype = ENCRYPT_DES_SINGLE_DECRYPT;
             encrypt_data.saveindex = i*100+j;
             memcpy(encrypt_data.keyvalue,aucBuf,16);
             encrypt_data.cryptflag = ENCRYPT_FUNCTION_NORMAL;
            _print("\n1encrypt_data.keytype = %02x,encrypt_data.crypttype=%02x \n",encrypt_data.keytype,encrypt_data.crypttype);
             rec = ENCRYPT_Decrypt_Load_Key(&encrypt_data);
             if(rec)
                 break;


             memset(&encrypt_data,0,sizeof(ENCRYPT_DRV_DATA));
             encrypt_data.keytype = ENCRYPT_DECRYPT_BYTRANSKEY;
             encrypt_data.cryptindex = i*100+j;
             encrypt_data.crypttype = ENCRYPT_DES_SINGLE;
             memcpy(encrypt_data.keyvalue,"3132333435363738",16);
             encrypt_data.cryptflag = ENCRYPT_FUNCTION_NORMAL;
              _print("\n2encrypt_data.keytype = %02x,encrypt_data.crypttype=%02x \n",encrypt_data.keytype,encrypt_data.crypttype);
             rec = ENCRYPT_Decrypt_Load_Key(&encrypt_data);
             if(rec)
                 break;
             if(!memcmp(encrypt_data.outdata,"96D0028878D58C89",16))//96D0028878D58C89
             {
                 showInforText(tr("DES ENCRYPT CHECK SUCCESS!"),false);
             }
             else{
                 showInforText(tr("DES ENCRYPT CHECK ERR!"),false);
                break;
             }

             memset(&encrypt_data,0,sizeof(ENCRYPT_DRV_DATA));
             encrypt_data.keytype = ENCRYPT_DECRYPT_BYTRANSKEY;
             encrypt_data.cryptindex = i*100+j;
             encrypt_data.crypttype = ENCRYPT_DES_SINGLE_DECRYPT;
             memcpy(encrypt_data.keyvalue,"3132333435363738",16);
             encrypt_data.cryptflag = ENCRYPT_FUNCTION_NORMAL;
             _print("\n3encrypt_data.keytype = %02x,encrypt_data.crypttype=%02x \n",encrypt_data.keytype,encrypt_data.crypttype);
             rec = ENCRYPT_Decrypt_Load_Key(&encrypt_data);
             if(rec)
                 break;

             if(!memcmp(encrypt_data.outdata,"F5E8E9EB81F28B73",16)){   //  \xF5\xE8\xE9\xEB\x81\xF2\x8B\x73
                 showInforText(tr("DES DECRYPT CHECK SUCCESS!"),false);
             }
             else{
                 showInforText(tr("DES DECRYPT CHECK ERR!"),false);
                 break;
             }
         }

    }
    QTimer::singleShot(2000,informationBox,SLOT(hide()));

}

void KeyTest::btn3des_slot()
{
    int rec = 0;
    ENCRYPT_DRV_DATA encrypt_data;
    unsigned char aucBuf[33];

    for(int i=0;i<5;i++)
    {
         for(int j=0;j<5;j++)
         {
             memset(&encrypt_data,0,sizeof(ENCRYPT_DRV_DATA));
             encrypt_data.keytype = ENCRYPT_DECRYPT_BYTRANSKEY;
             encrypt_data.cryptindex = i*100+0;
             encrypt_data.crypttype = ENCRYPT_3DESDOUBLE;
             memcpy(encrypt_data.keyvalue,"1234567890ABCDEFFEDCBA0987654321",32);//\x12\x34\x56\x78\x90\xAB\xCD\xEF\xFE\xDC\xBA\x09\x87\x65\x43\x21
             encrypt_data.cryptflag = ENCRYPT_FUNCTION_NORMAL;
             _print("\n0encrypt_data.keytype = %02x,encrypt_data.crypttype=%02x \n",encrypt_data.keytype,encrypt_data.crypttype);
             rec = ENCRYPT_Decrypt_Load_Key(&encrypt_data);
             if(rec)
                 break;
             memset(aucBuf,0,sizeof(aucBuf));
             memcpy(aucBuf,encrypt_data.outdata,encrypt_data.outlen);

             memset(&encrypt_data,0,sizeof(ENCRYPT_DRV_DATA));
             encrypt_data.keytype = ENCRYPT_SAVE_BYTRANSKEY;
             encrypt_data.cryptindex = i*100+0;
             encrypt_data.crypttype = ENCRYPT_3DESDOUBLE_DECRYPT;
             encrypt_data.saveindex = i*100+j;
             memcpy(encrypt_data.keyvalue,aucBuf,32);
             encrypt_data.cryptflag = ENCRYPT_FUNCTION_NORMAL;
            _print("\n1encrypt_data.keytype = %02x,encrypt_data.crypttype=%02x \n",encrypt_data.keytype,encrypt_data.crypttype);
             rec = ENCRYPT_Decrypt_Load_Key(&encrypt_data);
             if(rec)
                 break;


             memset(&encrypt_data,0,sizeof(ENCRYPT_DRV_DATA));
             encrypt_data.keytype = ENCRYPT_DECRYPT_BYTRANSKEY;
             encrypt_data.cryptindex = i*100+j;
             encrypt_data.crypttype = ENCRYPT_3DES_SINGLE;
             memcpy(encrypt_data.keyvalue,"3030303030303030",16);
             encrypt_data.cryptflag = ENCRYPT_FUNCTION_NORMAL;
              _print("\n2encrypt_data.keytype = %02x,encrypt_data.crypttype=%02x \n",encrypt_data.keytype,encrypt_data.crypttype);
             rec = ENCRYPT_Decrypt_Load_Key(&encrypt_data);
             if(rec)
                 break;
             if(!memcmp(encrypt_data.outdata,"31C055D2CA4E6A1F",16)){   //\x31\xC0\x55\xD2\xCA\x4E\x6A\x1F
                 showInforText(tr("3DES ENCRYPT CHECK SUCCESS!"),false);
                }
             else{
                 showInforText(tr("3DES ENCRYPT CHECK ERR!"),false);
                  break;
             }
         }


    }

    QTimer::singleShot(2000,informationBox,SLOT(hide()));

}


void KeyTest::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key_1:
        btndes_slot();
        break;
    case Qt::Key_2:
        btn3des_slot();
        break;
    case Qt::Key_Escape:
        close();
        break;
    }
}

#if 0
1.   单DES加解密验证

   for(i=0;i<5;i++)
   {
        for(j=0;j<5;j++)
        {
                用 i 行，0 列的密钥对原始数据“12345678”des加密，得到数据 AAA;  //  ENCRYPT_Decrypt_Load_Key     (ENCRYPT_DECRYPT_BYTRANSKEY  ,ENCRYPT_DES_SINGLE)

                用 i 行，0 列的密钥对数据AAA des解密然后存储在i行，j列上；   //    ENCRYPT_Decrypt_Load_Key     (ENCRYPT_SAVE_BYTRANSKEY ,ENCRYPT_DES_SINGLE_DECRYPT)
                然后开始验证
                用i行，j列的密钥对数据“12345678” des加密，看是否等于 "96D0028878D58C89",如果是就成功 //  ENCRYPT_Decrypt_Load_Key     (ENCRYPT_DECRYPT_BYTRANSKEY  ,ENCRYPT_DES_SINGLE)
                用i行，j列的密钥对数据“12345678” des解密，看是否等于 "F5E8E9EB81F28B73", 如果是就成功 //  ENCRYPT_Decrypt_Load_Key     (ENCRYPT_DECRYPT_BYTRANSKEY  ,ENCRYPT_DES_SINGLE_DECRYPT)
        }
   }

2. 3DES加密验证

   for(i=0;i<5;i++)
   {
        for(j=0;j<5;j++)
        {
                用 i 行，0 列,1列的密钥对原始数据“1234567890ABCDEFFEDCBA0987654321” 进行3des加密，得到数据 AAA;
                          //  ENCRYPT_Decrypt_Load_Key     (ENCRYPT_DECRYPT_BYTRANSKEY  ,ENCRYPT_3DES_SINGLE)
                用 i 行，0 列,1列的密钥对数据AAA 3des解密然后存储在i行，j列，j+1列上；
                          //    ENCRYPT_Decrypt_Load_Key     (ENCRYPT_SAVE_BYTRANSKEY ,ENCRYPT_3DES_SINGLE_DECRYPT)
                然后开始验证
                用i行，j列，j+1列的密钥对数据“00000000”  3des加密，看是否等于 "31C055D2CA4E6A1F",如果是就成功
                         //  ENCRYPT_Decrypt_Load_Key     (ENCRYPT_DECRYPT_BYTRANSKEY  ,ENCRYPT_3DES_SINGLE)
         }
   }
#endif
