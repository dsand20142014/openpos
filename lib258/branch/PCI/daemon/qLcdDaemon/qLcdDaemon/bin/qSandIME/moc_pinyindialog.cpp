/****************************************************************************
** Meta object code from reading C++ file 'pinyindialog.h'
**
** Created: Tue Dec 4 10:36:24 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qSandIME/googlepinyin/pinyindialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'pinyindialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_PinyinDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      27,   14,   13,   13, 0x05,
      52,   13,   13,   13, 0x05,
      71,   13,   13,   13, 0x05,

 // slots: signature, parameters, type, tag, flags
      98,   88,   13,   13, 0x0a,
     130,  120,   13,   13, 0x0a,
     149,   13,   13,   13, 0x0a,
     171,   13,   13,   13, 0x0a,
     191,   13,   13,   13, 0x0a,
     207,   13,   13,   13, 0x0a,
     223,   13,   13,   13, 0x0a,
     239,   13,   13,   13, 0x0a,
     255,   13,   13,   13, 0x0a,
     271,   13,   13,   13, 0x08,
     294,   13,   13,   13, 0x08,
     313,   13,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_PinyinDialog[] = {
    "PinyinDialog\0\0candidateNum\0"
    "alreadyGetCandidate(int)\0pagedown_clicked()\0"
    "pageup_clicked()\0pinyinStr\0"
    "getCandidate(QString)\0pageIndex\0"
    "showCandidate(int)\0on_pagedown_clicked()\0"
    "on_pageup_clicked()\0label1clicked()\0"
    "label2clicked()\0label3clicked()\0"
    "label4clicked()\0label5clicked()\0"
    "sendMyContent(QString)\0confirmMyContent()\0"
    "changeImeType(ImeType)\0"
};

void PinyinDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PinyinDialog *_t = static_cast<PinyinDialog *>(_o);
        switch (_id) {
        case 0: _t->alreadyGetCandidate((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->pagedown_clicked(); break;
        case 2: _t->pageup_clicked(); break;
        case 3: _t->getCandidate((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: _t->showCandidate((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->on_pagedown_clicked(); break;
        case 6: _t->on_pageup_clicked(); break;
        case 7: _t->label1clicked(); break;
        case 8: _t->label2clicked(); break;
        case 9: _t->label3clicked(); break;
        case 10: _t->label4clicked(); break;
        case 11: _t->label5clicked(); break;
        case 12: _t->sendMyContent((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 13: _t->confirmMyContent(); break;
        case 14: _t->changeImeType((*reinterpret_cast< ImeType(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData PinyinDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PinyinDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_PinyinDialog,
      qt_meta_data_PinyinDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PinyinDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PinyinDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PinyinDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PinyinDialog))
        return static_cast<void*>(const_cast< PinyinDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int PinyinDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    return _id;
}

// SIGNAL 0
void PinyinDialog::alreadyGetCandidate(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PinyinDialog::pagedown_clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void PinyinDialog::pageup_clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}
QT_END_MOC_NAMESPACE
