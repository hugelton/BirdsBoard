/****************************************************************************
** Meta object code from reading C++ file 'simple_audio.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/simple_audio.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'simple_audio.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN17SimpleAudioDeviceE_t {};
} // unnamed namespace

template <> constexpr inline auto SimpleAudioDevice::qt_create_metaobjectdata<qt_meta_tag_ZN17SimpleAudioDeviceE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SimpleAudioDevice"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SimpleAudioDevice, qt_meta_tag_ZN17SimpleAudioDeviceE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SimpleAudioDevice::staticMetaObject = { {
    QMetaObject::SuperData::link<QIODevice::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17SimpleAudioDeviceE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17SimpleAudioDeviceE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN17SimpleAudioDeviceE_t>.metaTypes,
    nullptr
} };

void SimpleAudioDevice::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SimpleAudioDevice *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *SimpleAudioDevice::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SimpleAudioDevice::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17SimpleAudioDeviceE_t>.strings))
        return static_cast<void*>(this);
    return QIODevice::qt_metacast(_clname);
}

int SimpleAudioDevice::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QIODevice::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_tag_ZN17SimpleAudioEngineE_t {};
} // unnamed namespace

template <> constexpr inline auto SimpleAudioEngine::qt_create_metaobjectdata<qt_meta_tag_ZN17SimpleAudioEngineE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SimpleAudioEngine",
        "audioStarted",
        "",
        "audioStopped",
        "audioError",
        "error",
        "handleStateChanged",
        "QAudio::State",
        "state"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'audioStarted'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'audioStopped'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'audioError'
        QtMocHelpers::SignalData<void(const QString &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 },
        }}),
        // Slot 'handleStateChanged'
        QtMocHelpers::SlotData<void(QAudio::State)>(6, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 7, 8 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SimpleAudioEngine, qt_meta_tag_ZN17SimpleAudioEngineE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SimpleAudioEngine::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17SimpleAudioEngineE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17SimpleAudioEngineE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN17SimpleAudioEngineE_t>.metaTypes,
    nullptr
} };

void SimpleAudioEngine::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SimpleAudioEngine *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->audioStarted(); break;
        case 1: _t->audioStopped(); break;
        case 2: _t->audioError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->handleStateChanged((*reinterpret_cast< std::add_pointer_t<QAudio::State>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SimpleAudioEngine::*)()>(_a, &SimpleAudioEngine::audioStarted, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (SimpleAudioEngine::*)()>(_a, &SimpleAudioEngine::audioStopped, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (SimpleAudioEngine::*)(const QString & )>(_a, &SimpleAudioEngine::audioError, 2))
            return;
    }
}

const QMetaObject *SimpleAudioEngine::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SimpleAudioEngine::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17SimpleAudioEngineE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SimpleAudioEngine::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void SimpleAudioEngine::audioStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void SimpleAudioEngine::audioStopped()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void SimpleAudioEngine::audioError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
QT_WARNING_POP
