/****************************************************************************
** Meta object code from reading C++ file 'audio_test.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/audio_test.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'audio_test.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN15AudioTestDeviceE_t {};
} // unnamed namespace

template <> constexpr inline auto AudioTestDevice::qt_create_metaobjectdata<qt_meta_tag_ZN15AudioTestDeviceE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AudioTestDevice"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AudioTestDevice, qt_meta_tag_ZN15AudioTestDeviceE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AudioTestDevice::staticMetaObject = { {
    QMetaObject::SuperData::link<QIODevice::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15AudioTestDeviceE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15AudioTestDeviceE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15AudioTestDeviceE_t>.metaTypes,
    nullptr
} };

void AudioTestDevice::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AudioTestDevice *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *AudioTestDevice::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AudioTestDevice::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15AudioTestDeviceE_t>.strings))
        return static_cast<void*>(this);
    return QIODevice::qt_metacast(_clname);
}

int AudioTestDevice::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QIODevice::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_tag_ZN15AudioTestEngineE_t {};
} // unnamed namespace

template <> constexpr inline auto AudioTestEngine::qt_create_metaobjectdata<qt_meta_tag_ZN15AudioTestEngineE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AudioTestEngine",
        "testStarted",
        "",
        "testStopped",
        "testError",
        "error",
        "handleStateChanged",
        "QAudio::State",
        "state"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'testStarted'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'testStopped'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'testError'
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
    return QtMocHelpers::metaObjectData<AudioTestEngine, qt_meta_tag_ZN15AudioTestEngineE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AudioTestEngine::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15AudioTestEngineE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15AudioTestEngineE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15AudioTestEngineE_t>.metaTypes,
    nullptr
} };

void AudioTestEngine::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AudioTestEngine *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->testStarted(); break;
        case 1: _t->testStopped(); break;
        case 2: _t->testError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->handleStateChanged((*reinterpret_cast< std::add_pointer_t<QAudio::State>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AudioTestEngine::*)()>(_a, &AudioTestEngine::testStarted, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioTestEngine::*)()>(_a, &AudioTestEngine::testStopped, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioTestEngine::*)(const QString & )>(_a, &AudioTestEngine::testError, 2))
            return;
    }
}

const QMetaObject *AudioTestEngine::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AudioTestEngine::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15AudioTestEngineE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AudioTestEngine::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void AudioTestEngine::testStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void AudioTestEngine::testStopped()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void AudioTestEngine::testError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
QT_WARNING_POP
