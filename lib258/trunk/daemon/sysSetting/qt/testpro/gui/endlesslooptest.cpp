#include "endlesslooptest.h"
#include "ui_endlesslooptest.h"

#include "new_drv.h"
#include "sand_print.h"
#include "test/test.h"

EndlessLoopTest::EndlessLoopTest(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EndlessLoopTest)
{
    ui->setupUi(this);
    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QColor(122,254,198)));//(QPixmap("./image/back2.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    setAttribute(Qt::WA_DeleteOnClose);


    ui->pushButton->setFocus();

//    connect(&timer,SIGNAL(timeout()),this,SLOT(slot_timer()));
//    timer.start(100);
    getResult();
}

EndlessLoopTest::~EndlessLoopTest()
{

    delete ui;
}


//void EndlessLoopTest::slot_timer()
//{
//    timer.stop();

//    unsigned char ucPrint[41];
//    unsigned char ucResult = 0;

//    memset(ucPrint,0,sizeof(ucPrint));
//    memcpy(&ucPrint[3],"1234567890abcdefghijklmnopqrstuvwxyz\n",38);

//    switch(printer_test_step) {
//        case 1:
//    #ifndef QC430_v01
//            ucResult = Os__sign_xprint((unsigned char*)"/mnt/pos/app/lcd/printtest.bmp") ;
//    #endif
//            break;
//        case 2:
//            memcpy(ucPrint,"00 ",3);
//            ucResult = Os__GB2312_xprint(ucPrint,0x00);
//            break;
//        case 3:
//            memcpy(ucPrint,"01 ",3);
//            ucResult = Os__GB2312_xprint(ucPrint,0x01);
//            break;
//        case 4:
//            memcpy(ucPrint,"1c ",3);
//            ucResult = Os__GB2312_xprint(ucPrint,0x1c);
//            break;
//        case 5:
//            memcpy(ucPrint,"1d ",3);
//            ucResult = Os__GB2312_xprint(ucPrint,0x1d);
//            break;
//        case 6:
//            memcpy(ucPrint,"1e ",3);
//            ucResult = Os__GB2312_xprint(ucPrint,0x1e);
//            break;
//        default:
//            printer_test_step = 0;
//            while(!Os__printer_idle()); /* liugm ,2011/4/12 */
//            sleep(2);
//            break;
//    }


//    if (ucResult < 0)
//    {
//        printf("[%s:%d] ucResult = %d \n",__FUNCTION__,__LINE__,ucResult);
//        return ;
//    }
//    else if (ucResult == 0)// successfully
//    {
//        flag = true;
//        printer_test_step++;
//        timer.start();
//        return;
//    }
//    else if (ucResult > 0)// have status
//    {
//        if (ucResult == PRN_ERROR_NO_PAPER ) {
//            ui->label->setText("ERROR: NO PAPER");
//        } else if (ucResult == PRN_ERROR_HOT ) {
//            ui->label->setText("ERROR: OVER HOT");
//        } else if (ucResult == PRN_ERROR_HOT_PAPER) {
//            ui->label->setText("ERROR: NO PAPER");
//            ui->label_2->setText("ERROR: OVER HOT");
//        }else  if (ucResult == PRN_ERROR_FIFO ) {
//            usleep(10000);
//            timer.start();
//            return;// not display this error message
//        }else  {
//            ui->label->setText("ERROR: UNKNOW");
//        }

//        flag = true;
//        return;
//    }
//}


//void EndlessLoopTest::keyPressEvent(QKeyEvent *event)
//{
//    switch(event->key())
//    {
//        case Qt::Key_Escape:
//        if(timer.isActive())
//            timer.stop();
//         close();
//        break;


//    }

//}

void EndlessLoopTest::getResult()
{
    unsigned char ucPrint[41];
    unsigned char ucResult = 0;
    unsigned char printer_test_step = 1;
    volatile unsigned int timeout ;
    NEW_DRV_TYPE  new_drv;

    ui->label->setText("PRINT...");

    Os_Wait_Event(KEY_DRV, &new_drv, 5000);

    do {
        memset(ucPrint,0,sizeof(ucPrint));
        memcpy(&ucPrint[3],"1234567890abcdefghijklmnopqrstuvwxyz\n",38);

        switch(printer_test_step) {
        case 1:
    #ifndef QC430_v01
            ucResult = Os__sign_xprint((unsigned char*)"/mnt/pos/app/lcd/printtest.bmp") ;
    #endif
            break;
        case 2:
            memcpy(ucPrint,"00 ",3);
            ucResult = Os__GB2312_xprint(ucPrint,0x00);
            break;
        case 3:
            memcpy(ucPrint,"01 ",3);
            ucResult = Os__GB2312_xprint(ucPrint,0x01);
            break;
        case 4:
            memcpy(ucPrint,"1c ",3);
            ucResult = Os__GB2312_xprint(ucPrint,0x1c);
            break;
        case 5:
            memcpy(ucPrint,"1d ",3);
            ucResult = Os__GB2312_xprint(ucPrint,0x1d);
            break;
        case 6:
            memcpy(ucPrint,"1e ",3);
            ucResult = Os__GB2312_xprint(ucPrint,0x1e);
            break;
        default:
            printer_test_step = 0;
            while(!Os__printer_idle()); /* liugm ,2011/4/12 */
            sleep(2);
            break;
        }//end switch

        if (ucResult < 0) {
            printf("[%s:%d] ucResult = %d \n",__FUNCTION__,__LINE__,ucResult);
            return ;
        }

        if (ucResult == 0) { // successfully
            if (new_drv.drv_data.xxdata[1]==KEY_CLEAR) {
                Os__clear_printbuf();
                return ;
            }
            printer_test_step++;
            continue;
        }

        if (ucResult > 0) { // have status
            // OSTEST_printer_mmi_error(ucResult);

            if (ucResult == PRN_ERROR_FIFO) { // fifo overflow
                usleep(10000);   // wait 10ms
                continue;
            }

            Os_Wait_Event(KEY_DRV, &new_drv, 20000);

            if (!timeout){
                ui->label->setText("PRINT...");
                continue;
            }

            if (new_drv.drv_data.xxdata[1]==KEY_CLEAR) {
                Os__clear_printbuf();
                return ; // will cancel
            } else{
                ui->label->setText("PRINT...");
                Os_Wait_Event(KEY_DRV , &new_drv, 5000);
                continue;
            }

        }// end error
    } while (1);
}

void EndlessLoopTest::on_pushButton_clicked()
{
    close();
}
