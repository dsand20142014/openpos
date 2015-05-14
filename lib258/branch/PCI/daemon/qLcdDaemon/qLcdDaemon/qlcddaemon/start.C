#include "start.h"
#include "ui_start.h"
extern "C"
{
#include "sand_key.h"
}
#define KEY_UP			0x50    /*上页*/
#define KEY_DOWN		0x51    /*下页*/
#define KEY_SYS_SET		0x52    /*设置*/
#define KEY_SYS_TEST	0x53    /*测试*/

Start::Start(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Start)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
}

Start::~Start()
{
    delete ui;
}

void Start::on_pushButton_clicked()
{
    this->deleteLater();
//     system("poweroff");
//     setDisabled(true);
}

void Start::on_pushButton_2_clicked()
{
    if(Os__set_key_ensure(KEY_SYS_SET) == 0)
    {
        {
            setDisabled(true);
//            printf("setDisabled  %d\n", __LINE__);
        }
        this->close();
    }
}

void Start::on_pushButton_1_clicked()
{
    if(Os__set_key_ensure(KEY_SYS_TEST) == 0)
    {
        {
            setDisabled(true);
 //           printf("setDisabled  %d\n", __LINE__);
        }
        this->close();
    }
}
