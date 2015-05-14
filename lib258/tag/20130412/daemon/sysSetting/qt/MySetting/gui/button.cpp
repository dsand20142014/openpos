#include "button.h"
#include <QFile>
#include <QtGui>

/* A Button with a type background:
 * type=0: default (green)
 * type=1: red
 * type=2: yellow
 * type=3:
 */
Button::Button(const QString & text, int type, QWidget *parent) :
    QPushButton(text, parent)
{
    setFocusPolicy(Qt::NoFocus);
    setMinimumSize(102, 36);
    setMaximumSize(215, 40);

    QLabel *redLine = new QLabel();
    redLine->setFixedHeight(3);
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(15, 15, 15, 4);
    layout->addSpacerItem(new QSpacerItem(30, 15));
    layout->addWidget(redLine, Qt::AlignBottom);
    setLayout(layout);

    switch (type) {
    case 0:
        loadStyleSheet("line_green", redLine);
        break;
    case 1:
        loadStyleSheet("line_red", redLine);
        break;
    case 2:
        loadStyleSheet("line_yellow", redLine);
        break;

    }

#if 0
    QString stylesheet;
    if (type == 0) { // default
        stylesheet = "line_green";
    } else if (type == 1) { // red
        stylesheet = "line_red";
    } else if (type == 2) { // yellow
        stylesheet = "line_yellow";
    }
    else { // big button
        //setFixedSize(215, 42);
    }
#endif
}

void Button::loadStyleSheet(const QString &sheetName, QWidget *obj)
{
    QFile file("/daemon/qss/" + sheetName.toLower() + ".qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        if (obj) {
            obj->setStyleSheet(styleSheet);
        } else {
            qApp->setStyleSheet(styleSheet);
        }
        file.close();
    }
    else {
        perror("Can not find the qss file!");
    }
}
