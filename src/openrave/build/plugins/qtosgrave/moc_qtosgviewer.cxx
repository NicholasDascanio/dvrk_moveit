/****************************************************************************
** Meta object code from reading C++ file 'qtosgviewer.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.7)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../plugins/qtosgrave/qtosgviewer.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtosgviewer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_qtosgrave__QtOSGViewer[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      24,   23,   23,   23, 0x0a,
      42,   23,   23,   23, 0x0a,
      62,   23,   23,   23, 0x0a,
      80,   23,   23,   23, 0x0a,
     104,   23,   23,   23, 0x0a,
     122,   23,   23,   23, 0x0a,
     154,   23,   23,   23, 0x0a,
     176,   23,   23,   23, 0x0a,
     198,   23,   23,   23, 0x0a,
     222,   23,   23,   23, 0x0a,
     240,   23,   23,   23, 0x0a,
     258,   23,   23,   23, 0x0a,
     276,   23,   23,   23, 0x0a,
     290,   23,   23,   23, 0x0a,
     312,   23,   23,   23, 0x0a,
     326,  319,   23,   23, 0x0a,
     359,  319,   23,   23, 0x0a,
     401,  392,   23,   23, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_qtosgrave__QtOSGViewer[] = {
    "qtosgrave::QtOSGViewer\0\0LoadEnvironment()\0"
    "ImportEnvironment()\0SaveEnvironment()\0"
    "_UpdateViewerCallback()\0ResetViewToHome()\0"
    "_ProcessPerspectiveViewChange()\0"
    "_ProcessAboutDialog()\0_SetDebugLevelDebug()\0"
    "_SetDebugLevelVerbose()\0_ChangeViewToXY()\0"
    "_ChangeViewToXZ()\0_ChangeViewToYZ()\0"
    "polygonMode()\0_ProcessBoundingBox()\0"
    "axes()\0button\0_ProcessPointerGroupClicked(int)\0"
    "_ProcessDraggerGroupClicked(int)\0"
    "item,num\0_OnObjectTreeClick(QTreeWidgetItem*,int)\0"
};

void qtosgrave::QtOSGViewer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtOSGViewer *_t = static_cast<QtOSGViewer *>(_o);
        switch (_id) {
        case 0: _t->LoadEnvironment(); break;
        case 1: _t->ImportEnvironment(); break;
        case 2: _t->SaveEnvironment(); break;
        case 3: _t->_UpdateViewerCallback(); break;
        case 4: _t->ResetViewToHome(); break;
        case 5: _t->_ProcessPerspectiveViewChange(); break;
        case 6: _t->_ProcessAboutDialog(); break;
        case 7: _t->_SetDebugLevelDebug(); break;
        case 8: _t->_SetDebugLevelVerbose(); break;
        case 9: _t->_ChangeViewToXY(); break;
        case 10: _t->_ChangeViewToXZ(); break;
        case 11: _t->_ChangeViewToYZ(); break;
        case 12: _t->polygonMode(); break;
        case 13: _t->_ProcessBoundingBox(); break;
        case 14: _t->axes(); break;
        case 15: _t->_ProcessPointerGroupClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: _t->_ProcessDraggerGroupClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 17: _t->_OnObjectTreeClick((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData qtosgrave::QtOSGViewer::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject qtosgrave::QtOSGViewer::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_qtosgrave__QtOSGViewer,
      qt_meta_data_qtosgrave__QtOSGViewer, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &qtosgrave::QtOSGViewer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *qtosgrave::QtOSGViewer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *qtosgrave::QtOSGViewer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_qtosgrave__QtOSGViewer))
        return static_cast<void*>(const_cast< QtOSGViewer*>(this));
    if (!strcmp(_clname, "ViewerBase"))
        return static_cast< ViewerBase*>(const_cast< QtOSGViewer*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int qtosgrave::QtOSGViewer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
