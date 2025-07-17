#include "simple_audio.h"
#include "tockus_dsp.h"
#include "pt8211_dac.h"
#include <QAudioDevice>
#include <QMediaDevices>
#include <QDebug>
#include <cstring>
#include <cmath>

SimpleAudioDevice::SimpleAudioDevice(TockusDSP* dsp, PT8211DAC* dac, QObject* parent)
    : QIODevice(parent)
    , tockusDSP(dsp)
    , pt8211DAC(dac)
    , channels(2)
{
}

SimpleAudioDevice::~SimpleAudioDevice() {
}

void SimpleAudioDevice::setParameters(TockusDSP* dsp, PT8211DAC* dac) {
    tockusDSP = dsp;
    pt8211DAC = dac;
}

qint64 SimpleAudioDevice::readData(char* data, qint64 maxlen) {
    // Calculate number of frames
    int bytesPerSample = sizeof(float);
    int bytesPerFrame = channels * bytesPerSample;
    int frameCount = maxlen / bytesPerFrame;
    
    float* floatData = reinterpret_cast<float*>(data);
    
    // Generate audio samples
    static float testPhase = 0.0f;
    const float testFreq = 440.0f; // A4 note for testing
    const float sampleRate = 44100.0f;
    const float phaseIncrement = 2.0f * M_PI * testFreq / sampleRate;
    
    for (int frame = 0; frame < frameCount; frame++) {
        float sample = 0.0f;
        
        if (tockusDSP && pt8211DAC) {
            // Generate one sample from DSP
            sample = tockusDSP->processNextSample();
            
            // Process through DAC simulation
            sample = pt8211DAC->processSample(sample);
        } else {
            // Fallback: generate test tone when DSP is not available
            sample = sin(testPhase) * 0.1f; // 10% volume test tone
            testPhase += phaseIncrement;
            if (testPhase >= 2.0f * M_PI) {
                testPhase -= 2.0f * M_PI;
            }
        }
        
        // Apply gain for audibility
        sample *= 0.5f;
        
        // Fill all channels with the same mono output
        for (int channel = 0; channel < channels; channel++) {
            floatData[frame * channels + channel] = sample;
        }
    }
    
    return frameCount * bytesPerFrame;
}

SimpleAudioEngine::SimpleAudioEngine(QObject* parent)
    : QObject(parent)
    , tockusDSP(nullptr)
    , pt8211DAC(nullptr)
    , audioActive(false)
    , initialized(false)
{
    // Setup audio format
    audioFormat.setSampleRate(SAMPLE_RATE);
    audioFormat.setChannelCount(CHANNELS);
    audioFormat.setSampleFormat(QAudioFormat::Float);
    
    qDebug() << "SimpleAudioEngine created with format:" << audioFormat;
}

SimpleAudioEngine::~SimpleAudioEngine() {
    stopAudio();
}

void SimpleAudioEngine::initialize(TockusDSP* dsp, PT8211DAC* dac) {
    tockusDSP = dsp;
    pt8211DAC = dac;
    
    if (tockusDSP && pt8211DAC) {
        tockusDSP->setSampleRate(SAMPLE_RATE);
        pt8211DAC->setSampleRate(SAMPLE_RATE);
        initialized = true;
        qDebug() << "SimpleAudioEngine initialized";
    }
}

bool SimpleAudioEngine::startAudio() {
    if (!initialized || audioActive) {
        qDebug() << "SimpleAudioEngine::startAudio failed - initialized:" << initialized << "active:" << audioActive;
        return false;
    }
    
    try {
        // Get default audio output device
        QAudioDevice audioDevice = QMediaDevices::defaultAudioOutput();
        if (audioDevice.isNull()) {
            emit audioError("No audio output device available");
            return false;
        }
        
        qDebug() << "Using audio device:" << audioDevice.description();
        
        // Check if format is supported
        if (!audioDevice.isFormatSupported(audioFormat)) {
            qDebug() << "Requested format not supported, using preferred format";
            audioFormat = audioDevice.preferredFormat();
            audioFormat.setSampleFormat(QAudioFormat::Float);
            
            if (!audioDevice.isFormatSupported(audioFormat)) {
                emit audioError("No suitable audio format found");
                return false;
            }
        }
        
        qDebug() << "Final audio format:" << audioFormat;
        
        // Create audio sink
        audioSink = std::make_unique<QAudioSink>(audioDevice, audioFormat, this);
        
        // Create audio device
        audioIODevice = std::make_unique<SimpleAudioDevice>(tockusDSP, pt8211DAC, this);
        audioIODevice->open(QIODevice::ReadOnly);
        
        // Connect signals
        connect(audioSink.get(), &QAudioSink::stateChanged,
                this, &SimpleAudioEngine::handleStateChanged);
        
        // Start audio
        audioSink->start(audioIODevice.get());
        
        audioActive = true;
        emit audioStarted();
        
        qDebug() << "Audio started successfully";
        qDebug() << "Sample rate:" << audioFormat.sampleRate();
        qDebug() << "Channels:" << audioFormat.channelCount();
        
        return true;
        
    } catch (const std::exception& e) {
        emit audioError(QString("Audio startup failed: %1").arg(e.what()));
        return false;
    }
}

void SimpleAudioEngine::stopAudio() {
    if (!audioActive) {
        return;
    }
    
    if (audioSink) {
        audioSink->stop();
        audioSink.reset();
    }
    
    if (audioIODevice) {
        audioIODevice->close();
        audioIODevice.reset();
    }
    
    audioActive = false;
    emit audioStopped();
    qDebug() << "Audio stopped";
}

void SimpleAudioEngine::handleStateChanged(QAudio::State state) {
    switch (state) {
        case QAudio::ActiveState:
            qDebug() << "Audio state: Active";
            break;
            
        case QAudio::SuspendedState:
            qDebug() << "Audio state: Suspended";
            break;
            
        case QAudio::StoppedState:
            qDebug() << "Audio state: Stopped";
            if (audioActive) {
                emit audioError("Audio unexpectedly stopped");
                audioActive = false;
            }
            break;
            
        case QAudio::IdleState:
            qDebug() << "Audio state: Idle";
            break;
    }
}