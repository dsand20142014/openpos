#include "com2test.h"

#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

COM2Test::COM2Test(QWidget *parent) : BaseForm(tr("SERIAL COM TEST"), parent)
{
    textEdit = new QTextEdit("DATA HAS BEEN SENT");

    textEdit->setReadOnly(true);

    layout->addWidget(textEdit);

    FILE* fd;
    fd = fopen("/dev/ttymxc4","w");
    fprintf(fd,"\nARE YOU OK?\nSERIAL COM TEST OK\n");
    fclose(fd);
}

COM2Test::~COM2Test()
{

}


void COM2Test::get_Result(void)
{
#if 0
    int i,error;
    unsigned short Status;
    char status[5];
    struct __item *item;

    const struct __item test_items[]={
        {"115200 bps",0x14},
        {"57600  bps",0x13},
        {"38400  bps",0x12},
        {"19200  bps",0x11},
        {"9600   bps",0xC},
        {"4800   bps",0xB},
        {"2400   bps",0xA},
        {"1200   bps",0x9},
        {"300    bps",0x7},
        {"300    bps",0x0},
    };

    for (item = test_items; item->speed; item++){


        textEdit->append("COM SPEED:");
        textEdit->append(QString(item->string));

        Os__end_com3();

        if ( Os__init_com3(0x0303, item->speed<<8, item->speed) == OK ){

            textEdit->append("PRESS ANY KEY, POS'S  ");


            for (i='A'; i < 'Z'+1; i++)
                Os__txcar3(i);


            textEdit->append("PRESS ANY KEY, POS'S  ");
            textEdit->append("COM WILL RECEIVE    ");


            textEdit->append("RECE 10 BYTES......");

            error = 0;
            for (i=0; i < 10; i++){

                Status = Os__rxcar3(2000);

                sprintf(status,"%04x",Status);
                textEdit->append(status);

                if ( (Status >>8) == 1 ){ //timeout
                    error = 1;
                    break;
                }
                if ( (Status >>8) == 2 || (Status >>8) == 3 ){ //other error
                    error = 2;
                    break;
                }


                if ( (Status & 0x00ff) != i+0x30 ){
                    error = 3;
                    break;
                }
            }
            if (!error)
                textEdit->append("TEST OK      ");
            else if (error == 1)
                textEdit->append("TIME OUT !   ");
            else if (error == 2)
                textEdit->append("OTHER ERROR !");
            else if (error == 3)
                textEdit->append("DATA ERROR   ");
        }

        else
            textEdit->append("ERROR INIT. COM3");


        Os__end_com3();
        Os__xget_key();

    }

    return ;

#endif
}

