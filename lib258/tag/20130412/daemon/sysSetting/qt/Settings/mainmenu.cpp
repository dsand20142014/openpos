#include <QDebug>

#include "mainmenu.h"
#include "ui_mainmenu.h"

MainMenu::MainMenu(QWidget *parent) :
        //QDialog(parent)
    QDialog(parent),
    ui(new Ui::MainMenu)
{
    ui->setupUi(this);
    ui->pbEnter->hide();
}

MainMenu::~MainMenu()
{
    delete ui;
}

void MainMenu::on_pbGsm_clicked()
{
    Gsm *gsm = new Gsm(this);
    gsm->showFullScreen();
    //gsm->show();
}

void MainMenu::on_pbDateTime_clicked()
{
    Date *date = new Date(this);
    date->showFullScreen();
    //date->show();
}

void MainMenu::on_pbModle_clicked()
{
    Module *module = new Module(this);
    module->showFullScreen();
    //module->show();
}

void MainMenu::on_pbScreen_clicked()
{
    Screen *screen = new Screen(this);
    screen->showFullScreen();
    //screen->show();
}

void MainMenu::on_pbWifi_clicked()
{
    Wifi *wifi = new Wifi(this);
    wifi->showFullScreen();
    //wifi->show();
}

void MainMenu::on_pbCancel_clicked()
{
    //close();
}

void MainMenu::on_pbEthernet_clicked()
{
    Ethernet *ethernet = new Ethernet(this);
    ethernet->showFullScreen();
    //ethernet->show();
}

void MainMenu::on_pbGps_clicked()
{
    Gps *gps = new Gps(this);
    gps->showFullScreen();
    //gps->show();
}

void MainMenu::on_pbModen_clicked()
{

}
