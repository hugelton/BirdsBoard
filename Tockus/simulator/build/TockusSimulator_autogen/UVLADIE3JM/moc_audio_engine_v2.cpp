/****************************************************************************
** Meta object code from reading C++ file 'audio_engine_v2.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/audio_engine_v2.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'audio_engine_v2.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN13AudioEngineV2E_t {};
} // unnamed namespace

template <> constexpr inline auto AudioEngineV2::qt_create_metaobjectdata<qt_meta_tag_ZN13AudioEngineV2E_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AudioEngineV2",
        "audioStarted",
        "",
        "audioStopped",
        "audioError",
        "error",
        "bufferUnderrun",
        "handleAudioStateChanged",
        "QAudio::State",
        "state",
        "generateAudioData"
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
        // Signal 'bufferUnderrun'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'handleAudioStateChanged'
        QtMocHelpers::SlotData<void(QAudio::State)>(7, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 8, 9 },
        }}),
        // Slot 'generateAudioData'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AudioEngineV2, qt_meta_tag_ZN13AudioEngineV2E_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AudioEngineV2::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13AudioEngineV2E_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13AudioEngineV2E_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13AudioEngineV2E_t>.metaTypes,
    nullptr
} };

void AudioEngineV2::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AudioEngineV2 *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->audioStarted(); break;
        case 1: _t->audioStopped(); break;
        case 2: _t->audioError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->bufferUnderrun(); break;
        case 4: _t->handleAudioStateChanged((*reinterpret_cast< std::add_pointer_t<QAudio::State>>(_a[1]))); break;
        case 5: _t->generateAudioData(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AudioEngineV2::*)()>(_a, &AudioEngineV2::audioStarted, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioEngineV2::*)()>(_a, &AudioEngineV2::audioStopped, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioEngineV2::*)(const QString & )>(_a, &AudioEngineV2::audioError, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioEngineV2::*)()>(_a, &AudioEngineV2::bufferUnderrun, 3))
            return;
    }
}

const QMetaObject *AudioEngineV2::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AudioEngineV2::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13AudioEngineV2E_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AudioEngineV2::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void AudioEngineV2::audioStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void AudioEngineV2::audioStopped()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void AudioEngineV2::audioError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void AudioEngineV2::bufferUnderrun()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
