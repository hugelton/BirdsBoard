/****************************************************************************
** Meta object code from reading C++ file 'audio_engine.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/audio_engine.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'audio_engine.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN11AudioEngineE_t {};
} // unnamed namespace

template <> constexpr inline auto AudioEngine::qt_create_metaobjectdata<qt_meta_tag_ZN11AudioEngineE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AudioEngine",
        "audioStarted",
        "",
        "audioStopped",
        "audioError",
        "error",
        "bufferUnderrun",
        "handleAudioStateChanged",
        "QAudio::State",
        "state",
        "handleAudioNotify"
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
        // Slot 'handleAudioNotify'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AudioEngine, qt_meta_tag_ZN11AudioEngineE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AudioEngine::staticMetaObject = { {
    QMetaObject::SuperData::link<QIODevice::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11AudioEngineE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11AudioEngineE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11AudioEngineE_t>.metaTypes,
    nullptr
} };

void AudioEngine::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AudioEngine *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->audioStarted(); break;
        case 1: _t->audioStopped(); break;
        case 2: _t->audioError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->bufferUnderrun(); break;
        case 4: _t->handleAudioStateChanged((*reinterpret_cast< std::add_pointer_t<QAudio::State>>(_a[1]))); break;
        case 5: _t->handleAudioNotify(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AudioEngine::*)()>(_a, &AudioEngine::audioStarted, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioEngine::*)()>(_a, &AudioEngine::audioStopped, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioEngine::*)(const QString & )>(_a, &AudioEngine::audioError, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioEngine::*)()>(_a, &AudioEngine::bufferUnderrun, 3))
            return;
    }
}

const QMetaObject *AudioEngine::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AudioEngine::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11AudioEngineE_t>.strings))
        return static_cast<void*>(this);
    return QIODevice::qt_metacast(_clname);
}

int AudioEngine::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QIODevice::qt_metacall(_c, _id, _a);
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
void AudioEngine::audioStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void AudioEngine::audioStopped()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void AudioEngine::audioError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void AudioEngine::bufferUnderrun()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
