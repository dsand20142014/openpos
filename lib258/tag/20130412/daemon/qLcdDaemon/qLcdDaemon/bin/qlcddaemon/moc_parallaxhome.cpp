/****************************************************************************
** Meta object code from reading C++ file 'parallaxhome.h'
**
** Created: Tue Dec 4 10:36:43 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qlcddaemon/gui/parallaxhome.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'parallaxhome.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ParallaxHome[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      17,   14,   13,   13, 0x0a,
      37,   32,   13,   13, 0x0a,
      59,   53,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ParallaxHome[] = {
    "ParallaxHome\0\0dx\0slideBy(qreal)\0page\0"
    "choosePage(int)\0frame\0shiftPage(int)\0"
};

void ParallaxHome::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ParallaxHome *_t = static_cast<ParallaxHome *>(_o);
        switch (_id) {
        case 0: _t->slideBy((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 1: _t->choosePage((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->shiftPage((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ParallaxHome::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ParallaxHome::staticMetaObject = {
    { &QGraphicsView::staticMetaObject, qt_meta_stringdata_ParallaxHome,
      qt_meta_data_ParallaxHome, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ParallaxHome::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ParallaxHome::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ParallaxHome::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ParallaxHome))
        return static_cast<void*>(const_cast< ParallaxHome*>(this));
    return QGraphicsView::qt_metacast(_clname);
}

int ParallaxHome::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
