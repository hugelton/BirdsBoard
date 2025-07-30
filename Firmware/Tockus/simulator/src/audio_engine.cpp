#include "audio_engine.h"
#include "tockus_dsp.h"
#include "pt8211_dac.h"
#include <QAudioDevice>
#include <QMediaDevices>
#include <QDateTime>
#include <QDebug>

AudioEngine::AudioEngine(QObject* parent)
    : QIODevice(parent)
    , currentSampleRate(DEFAULT_SAMPLE_RATE)
    , currentBufferSize(DEFAULT_BUFFER_SIZE)
    , channels(DEFAULT_CHANNELS)
    , audioActive(false)
    , initialized(false)
    , tockusDSP(nullptr)
    , pt8211DAC(nullptr)
    , totalSamplesProcessed(0)
    , lastNotifyTime(0)
{
    setupAudioFormat();
    
    // Setup notify timer for performance monitoring
    notifyTimer = new QTimer(this);
    connect(notifyTimer, &QTimer::timeout, this, &AudioEngine::handleAudioNotify);
}

AudioEngine::~AudioEngine() {
    stopAudio();
}

void AudioEngine::initialize(TockusDSP* dsp, PT8211DAC* dac) {
    tockusDSP = dsp;
    pt8211DAC = dac;
    
    if (tockusDSP && pt8211DAC) {
        tockusDSP->setSampleRate(currentSampleRate);
        pt8211DAC->setSampleRate(currentSampleRate);
        initialized = true;
    }
}

void AudioEngine::setupAudioFormat() {
    audioFormat.setSampleRate(currentSampleRate);
    audioFormat.setChannelCount(channels);
    audioFormat.setSampleFormat(QAudioFormat::Float);
}

bool AudioEngine::startAudio() {
    if (!initialized || audioActive) {
        return false;
    }
    
    QMutexLocker locker(&audioMutex);
    
    try {
        // Get default audio output device
        QAudioDevice audioDevice = QMediaDevices::defaultAudioOutput();
        if (audioDevice.isNull()) {
            emit audioError("No audio output device available");
            return false;
        }
        
        // Check if format is supported
        if (!audioDevice.isFormatSupported(audioFormat)) {
            qDebug() << "Audio format not supported, trying to find nearest format";
            
            // Try to find a supported format
            QAudioFormat nearestFormat = audioDevice.preferredFormat();
            nearestFormat.setSampleFormat(QAudioFormat::Float);
            
            if (audioDevice.isFormatSupported(nearestFormat)) {
                audioFormat = nearestFormat;
                currentSampleRate = audioFormat.sampleRate();
                channels = audioFormat.channelCount();
                
                // Update DSP and DAC with new sample rate
                if (tockusDSP) tockusDSP->setSampleRate(currentSampleRate);
                if (pt8211DAC) pt8211DAC->setSampleRate(currentSampleRate);
                
                qDebug() << "Using audio format:" << audioFormat;
            } else {
                emit audioError("No suitable audio format found");
                return false;
            }
        }
        
        // Create audio output (Qt6 uses QAudioSink)
        audioOutput = std::make_unique<QAudioSink>(audioDevice, audioFormat, this);
        
        // Connect signals (Qt6 uses different signal)
        connect(audioOutput.get(), &QAudioSink::stateChanged,
                this, &AudioEngine::handleAudioStateChanged);
        
        // Start audio output
        this->open(QIODevice::ReadOnly);
        audioOutput->start(this);
        
        // Start notify timer for performance monitoring
        notifyTimer->start(100); // Update every 100ms
        lastNotifyTime = QDateTime::currentMSecsSinceEpoch();
        
        audioActive = true;
        emit audioStarted();
        
        qDebug() << "Audio started successfully";
        qDebug() << "Sample rate:" << currentSampleRate;
        qDebug() << "Channels:" << channels;
        qDebug() << "Buffer size:" << currentBufferSize;
        
        return true;
        
    } catch (const std::exception& e) {
        emit audioError(QString("Audio startup failed: %1").arg(e.what()));
        return false;
    }
}

void AudioEngine::stopAudio() {
    if (!audioActive) {
        return;
    }
    
    QMutexLocker locker(&audioMutex);
    
    // Stop notify timer
    notifyTimer->stop();
    
    // Stop audio output
    if (audioOutput) {
        audioOutput->stop();
        audioOutput.reset();
    }
    
    this->close();
    audioActive = false;
    
    emit audioStopped();
    qDebug() << "Audio stopped";
}

void AudioEngine::setSampleRate(int sampleRate) {
    if (sampleRate != currentSampleRate) {
        bool wasActive = audioActive;
        
        if (wasActive) {
            stopAudio();
        }
        
        currentSampleRate = sampleRate;
        setupAudioFormat();
        
        if (tockusDSP) tockusDSP->setSampleRate(currentSampleRate);
        if (pt8211DAC) pt8211DAC->setSampleRate(currentSampleRate);
        
        if (wasActive) {
            startAudio();
        }
    }
}

void AudioEngine::setBufferSize(int bufferSize) {
    currentBufferSize = bufferSize;
    
    // If audio is active, restart with new buffer size
    if (audioActive) {
        stopAudio();
        startAudio();
    }
}

float AudioEngine::getCurrentLatency() const {
    if (audioOutput) {
        // Estimate latency based on buffer size and sample rate
        return (float)currentBufferSize / currentSampleRate * 1000.0f; // ms
    }
    return 0.0f;
}

qint64 AudioEngine::readData(char* data, qint64 maxlen) {
    if (!initialized || !tockusDSP || !pt8211DAC) {
        // Fill with silence if not initialized
        memset(data, 0, maxlen);
        return maxlen;
    }
    
    QMutexLocker locker(&audioMutex);
    
    // Calculate number of frames to process
    int bytesPerSample = sizeof(float);
    int bytesPerFrame = channels * bytesPerSample;
    int frameCount = maxlen / bytesPerFrame;
    
    // Process audio buffer
    float* floatData = reinterpret_cast<float*>(data);
    processAudioBuffer(floatData, frameCount);
    
    totalSamplesProcessed += frameCount;
    
    return frameCount * bytesPerFrame;
}

qint64 AudioEngine::writeData(const char* data, qint64 len) {
    // Not used for audio output
    Q_UNUSED(data)
    Q_UNUSED(len)
    return 0;
}

void AudioEngine::processAudioBuffer(float* buffer, int frameCount) {
    for (int frame = 0; frame < frameCount; frame++) {
        // Generate one sample from DSP
        float sample = tockusDSP->processNextSample();
        
        // Process through DAC simulation
        float dacOutput = pt8211DAC->processSample(sample);
        
        // Fill all channels with the same mono output
        for (int channel = 0; channel < channels; channel++) {
            buffer[frame * channels + channel] = dacOutput;
        }
    }
}

void AudioEngine::handleAudioStateChanged(QAudio::State state) {
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
                // Unexpected stop
                emit audioError("Audio unexpectedly stopped");
                audioActive = false;
            }
            break;
            
        case QAudio::IdleState:
            qDebug() << "Audio state: Idle";
            // This can happen during normal operation
            break;
    }
}

void AudioEngine::handleAudioNotify() {
    if (!audioOutput || !audioActive) {
        return;
    }
    
    // Qt6 simplified monitoring
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 timeDelta = currentTime - lastNotifyTime;
    
    if (timeDelta > 0) {
        // Simplified monitoring for Qt6
        QAudio::State state = audioOutput->state();
        if (state == QAudio::IdleState) {
            // This might indicate underrun
            emit bufferUnderrun();
        }
    }
    
    lastNotifyTime = currentTime;
}