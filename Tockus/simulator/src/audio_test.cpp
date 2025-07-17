#include "audio_test.h"
#include <QAudioDevice>
#include <QMediaDevices>
#include <QDebug>
#include <cmath>

AudioTestDevice::AudioTestDevice(QObject* parent)
    : QIODevice(parent)
    , phase(0.0f)
    , frequency(440.0f)
    , sampleRate(44100)
    , channels(2)
{
}

qint64 AudioTestDevice::readData(char* data, qint64 maxlen) {
    const float amplitude = 0.3f; // 30% volume
    const float phaseIncrement = 2.0f * M_PI * frequency / sampleRate;
    
    int frameCount = maxlen / (channels * sizeof(float));
    float* floatData = reinterpret_cast<float*>(data);
    
    for (int frame = 0; frame < frameCount; frame++) {
        float sample = std::sin(phase) * amplitude;
        phase += phaseIncrement;
        if (phase >= 2.0f * M_PI) {
            phase -= 2.0f * M_PI;
        }
        
        // Fill all channels
        for (int channel = 0; channel < channels; channel++) {
            floatData[frame * channels + channel] = sample;
        }
    }
    
    return frameCount * channels * sizeof(float);
}

AudioTestEngine::AudioTestEngine(QObject* parent)
    : QObject(parent)
    , isRunning(false)
{
    listAudioDevices();
}

AudioTestEngine::~AudioTestEngine() {
    stopTest();
}

void AudioTestEngine::listAudioDevices() {
    qDebug() << "=== Audio Device Information ===";
    
    // List all available audio output devices
    auto audioDevices = QMediaDevices::audioOutputs();
    qDebug() << "Found" << audioDevices.size() << "audio output devices:";
    
    for (int i = 0; i < audioDevices.size(); i++) {
        const QAudioDevice& device = audioDevices[i];
        qDebug() << "Device" << i << ":";
        qDebug() << "  Description:" << device.description();
        qDebug() << "  ID:" << device.id();
        qDebug() << "  Is Default:" << (device == QMediaDevices::defaultAudioOutput());
        qDebug() << "  Is Null:" << device.isNull();
        
        // Test supported formats
        QAudioFormat preferredFormat = device.preferredFormat();
        qDebug() << "  Preferred Format:" << preferredFormat;
        
        // Test common formats
        QAudioFormat testFormat;
        testFormat.setSampleRate(44100);
        testFormat.setChannelCount(2);
        testFormat.setSampleFormat(QAudioFormat::Float);
        qDebug() << "  Supports 44.1kHz Float:" << device.isFormatSupported(testFormat);
        
        testFormat.setSampleFormat(QAudioFormat::Int16);
        qDebug() << "  Supports 44.1kHz Int16:" << device.isFormatSupported(testFormat);
        
        testFormat.setSampleRate(48000);
        testFormat.setSampleFormat(QAudioFormat::Float);
        qDebug() << "  Supports 48kHz Float:" << device.isFormatSupported(testFormat);
    }
    
    qDebug() << "=== End Audio Device Information ===";
}

bool AudioTestEngine::testAudioOutput() {
    if (isRunning) {
        qDebug() << "Audio test already running";
        return false;
    }
    
    qDebug() << "Starting audio test...";
    
    try {
        // Get default audio output device
        QAudioDevice audioDevice = QMediaDevices::defaultAudioOutput();
        if (audioDevice.isNull()) {
            emit testError("No default audio output device found");
            return false;
        }
        
        qDebug() << "Using device:" << audioDevice.description();
        
        // Try multiple formats until one works
        QList<QAudioFormat> formatsToTry;
        
        // Format 1: 44.1kHz Float Stereo
        QAudioFormat format1;
        format1.setSampleRate(44100);
        format1.setChannelCount(2);
        format1.setSampleFormat(QAudioFormat::Float);
        formatsToTry.append(format1);
        
        // Format 2: 44.1kHz Int16 Stereo
        QAudioFormat format2;
        format2.setSampleRate(44100);
        format2.setChannelCount(2);
        format2.setSampleFormat(QAudioFormat::Int16);
        formatsToTry.append(format2);
        
        // Format 3: Device preferred format
        formatsToTry.append(audioDevice.preferredFormat());
        
        // Format 4: 48kHz Float Stereo
        QAudioFormat format4;
        format4.setSampleRate(48000);
        format4.setChannelCount(2);
        format4.setSampleFormat(QAudioFormat::Float);
        formatsToTry.append(format4);
        
        QAudioFormat workingFormat;
        bool formatFound = false;
        
        for (const QAudioFormat& format : formatsToTry) {
            qDebug() << "Testing format:" << format;
            if (audioDevice.isFormatSupported(format)) {
                workingFormat = format;
                formatFound = true;
                qDebug() << "Format supported!";
                break;
            } else {
                qDebug() << "Format not supported";
            }
        }
        
        if (!formatFound) {
            emit testError("No supported audio format found");
            return false;
        }
        
        audioFormat = workingFormat;
        qDebug() << "Using audio format:" << audioFormat;
        
        // Create audio sink
        audioSink = std::make_unique<QAudioSink>(audioDevice, audioFormat, this);
        qDebug() << "Audio sink created";
        
        // Create test device
        testDevice = std::make_unique<AudioTestDevice>(this);
        testDevice->open(QIODevice::ReadOnly);
        qDebug() << "Test device created and opened";
        
        // Connect signals
        connect(audioSink.get(), &QAudioSink::stateChanged,
                this, &AudioTestEngine::handleStateChanged);
        
        // Start audio
        qDebug() << "Starting audio sink...";
        audioSink->start(testDevice.get());
        
        isRunning = true;
        emit testStarted();
        
        qDebug() << "Audio test started successfully";
        qDebug() << "You should hear a 440Hz sine wave";
        
        return true;
        
    } catch (const std::exception& e) {
        QString errorMsg = QString("Audio test failed: %1").arg(e.what());
        qDebug() << errorMsg;
        emit testError(errorMsg);
        return false;
    }
}

void AudioTestEngine::stopTest() {
    if (!isRunning) {
        return;
    }
    
    qDebug() << "Stopping audio test...";
    
    if (audioSink) {
        audioSink->stop();
        audioSink.reset();
    }
    
    if (testDevice) {
        testDevice->close();
        testDevice.reset();
    }
    
    isRunning = false;
    emit testStopped();
    
    qDebug() << "Audio test stopped";
}

void AudioTestEngine::handleStateChanged(QAudio::State state) {
    switch (state) {
        case QAudio::ActiveState:
            qDebug() << "Audio Test State: Active - AUDIO SHOULD BE PLAYING NOW";
            break;
            
        case QAudio::SuspendedState:
            qDebug() << "Audio Test State: Suspended";
            break;
            
        case QAudio::StoppedState:
            qDebug() << "Audio Test State: Stopped";
            if (isRunning) {
                emit testError("Audio unexpectedly stopped");
                isRunning = false;
            }
            break;
            
        case QAudio::IdleState:
            qDebug() << "Audio Test State: Idle";
            break;
    }
}