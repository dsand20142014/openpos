/****************************************************************************
** Meta object code from reading C++ file 'lcddaemon.h'
**
** Created: Tue Dec 4 10:36:41 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qlcddaemon/lcddaemon.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'lcddaemon.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_LcdDaemon[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      18,   11,   10,   10, 0x0a,
      39,   10,   10,   10, 0x08,
      52,   10,   10,   10, 0x08,
      69,   10,   10,   10, 0x08,
      93,   81,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_LcdDaemon[] = {
    "LcdDaemon\0\0msgVar\0lcdMsgProc(QVariant)\0"
    "timeupdate()\0sand_execl_app()\0start_win()\0"
    "gif_content\0gifloadopt(gif_loading_struct*)\0"
};

void LcdDaemon::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        LcdDaemon *_t = static_cast<LcdDaemon *>(_o);
        switch (_id) {
        case 0: _t->lcdMsgProc((*reinterpret_cast< QVariant(*)>(_a[1]))); break;
        case 1: _t->timeupdate(); break;
        case 2: _t->sand_execl_app(); break;
        case 3: _t->start_win(); break;
        case 4: _t->gifloadopt((*reinterpret_cast< gif_loading_struct*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData LcdDaemon::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject LcdDaemon::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_LcdDaemon,
      qt_meta_data_LcdDaemon, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &LcdDaemon::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *LcdDaemon::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *LcdDaemon::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_LcdDaemon))
        return static_cast<void*>(const_cast< LcdDaemon*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int LcdDaemon::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
