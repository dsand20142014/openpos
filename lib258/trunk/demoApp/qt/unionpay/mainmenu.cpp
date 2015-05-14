#include "mainmenu.h"
#include "ui_mainmenu.h"
#include "mainkey4.h"
#include "mainkey5.h"
#include "mainkey6.h"
#include "mainkey7.h"
#include "mainkey8.h"
#include "mainkey9.h"
#include "pro_debit.h"
#include "pro_cancel.h"
#include "pro_refund.h"

#include "frame.h"

#include "include.h"
#include "global.h"
#include "xdata.h"


MainMenu::MainMenu(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainMenu)
{
    ui->setupUi(this);
    //ui->showFullScreen();
    setWindowFlags(Qt::FramelessWindowHint);
    ui->pushButton->hide();
    UTIL_ClearGlobalData();
    UTIL_GetTerminalInfo();

	printf("\nMainMenu TermId:[%s]",DataSave0.ConstantParamData.aucTerminalID);
	
}

MainMenu::~MainMenu()
{
    delete ui;
}


void MainMenu::on_pushButton_close_clicked()
{	
    emit(returnFromMainMenu());
    destroy();
}


void MainMenu::on_pushButton_1_clicked()
{
    Pro_Debit *desk=new Pro_Debit();
    desk->show();
    desk->showFullScreen();
//	PBOC_CPU_TEST();
	
}

void MainMenu::on_pushButton_2_clicked()
{
    Pro_Cancel *desk=new Pro_Cancel();
    desk->show();
    desk->showFullScreen();

}

void MainMenu::on_pushButton_3_clicked()
{
    Pro_Refund *desk=new Pro_Refund();
    desk->show();
    desk->showFullScreen();
}

void MainMenu::on_pushButton_4_clicked()
{
    mainkey4 *desk=new mainkey4();
    desk->show();
    desk->showFullScreen();
}

void MainMenu::on_pushButton_5_clicked()
{

    mainkey5 *desk=new mainkey5();
    desk->show();
    desk->showFullScreen();
}

void MainMenu::on_pushButton_6_clicked()
{
    mainkey6 *desk=new mainkey6();
    desk->show();
    desk->showFullScreen();
}

void MainMenu::on_pushButton_7_clicked()
{
    //    mainkey7 *desk = new mainkey7(this);
    //    desk->show();
    //    desk->showFullScreen();
    mainkey7 desk;
    desk.exec();
    desk.showFullScreen();
}

void MainMenu::on_pushButton_8_clicked()
{
    mainkey8 *desk=new mainkey8();
    desk->show();
    desk->showFullScreen();
}

void MainMenu::on_pushButton_9_clicked()
{
    mainkey9 *desk=new mainkey9();
    desk->show();
    desk->showFullScreen();
}

void MainMenu::on_pushButton_clicked()
{
    PRINT_xprint((unsigned char *)"-------Begin---------------");
    PRINT_xprint((unsigned char *)"Qt APP Print test.");
    PRINT_xprint((unsigned char *)"上海山的测试");
    PRINT_xprint((unsigned char *)"ABCDEGAAFSDAF");
    PRINT_xprint((unsigned char *)"12345464548789797");
    PRINT_xprint((unsigned char *)"!@#$%%^^&*()_+");
    PRINT_xprint((unsigned char *)"-------====---------------");

    PRINT_xlinefeed(LINENUM*3);

    PRINT_xprint((unsigned char *)"-------End---------------");

    PRINT_xlinefeed(LINENUM*3);

    ProUiFace.UiToPro.ucTransType = TRANS_PRINT_TTS;
//    MyThread *a=new MyThread();
//    a->start();

}

}
