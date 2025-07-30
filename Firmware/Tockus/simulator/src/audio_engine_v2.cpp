#include "audio_engine_v2.h"
#include "tockus_dsp.h"
#include "pt8211_dac.h"
#include <QAudioDevice>
#include <QMediaDevices>
#include <QDateTime>
#include <QDebug>
#include <cstring>

AudioEngineV2::AudioEngineV2(QObject* parent)
    : QObject(parent)
    , currentSampleRate(DEFAULT_SAMPLE_RATE)
    , currentBufferSize(DEFAULT_BUFFER_SIZE)
    , channels(DEFAULT_CHANNELS)
    , audioActive(false)
    , initialized(false)
    , tockusDSP(nullptr)
    , pt8211DAC(nullptr)
    , totalSamplesProcessed(0)
{
    setupAudioFormat();
    
    // Setup audio generation timer
    audioTimer = new QTimer(this);
    connect(audioTimer, &QTimer::timeout, this, &AudioEngineV2::generateAudioData);
}

AudioEngineV2::~AudioEngineV2() {
    stopAudio();
}

void AudioEngineV2::initialize(TockusDSP* dsp, PT8211DAC* dac) {
    tockusDSP = dsp;
    pt8211DAC = dac;
    
    if (tockusDSP && pt8211DAC) {
        tockusDSP->setSampleRate(currentSampleRate);
        pt8211DAC->setSampleRate(currentSampleRate);
        initialized = true;
        qDebug() << "AudioEngineV2 initialized with sample rate:" << currentSampleRate;
    }
}

void AudioEngineV2::setupAudioFormat() {
    audioFormat.setSampleRate(currentSampleRate);
    audioFormat.setChannelCount(channels);
    audioFormat.setSampleFormat(QAudioFormat::Float);
    
    qDebug() << "Audio format setup:" << audioFormat;
}

bool AudioEngineV2::startAudio() {
    if (!initialized || audioActive) {
        qDebug() << "AudioEngineV2::startAudio failed - initialized:" << initialized << "active:" << audioActive;
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
        
        qDebug() << "Using audio device:" << audioDevice.description();
        
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
        
        // Create audio sink
        audioSink = std::make_unique<QAudioSink>(audioDevice, audioFormat, this);
        
        // Connect signals
        connect(audioSink.get(), &QAudioSink::stateChanged,
                this, &AudioEngineV2::handleAudioStateChanged);
        
        // Create audio buffer
        audioBuffer = std::make_unique<QBuffer>(&audioData, this);
        audioBuffer->open(QIODevice::WriteOnly);
        
        // Initialize with some silence
        int bufferSizeBytes = currentBufferSize * channels * sizeof(float) * 4; // 4 buffers worth
        audioData.clear();
        audioData.resize(bufferSizeBytes);
        audioData.fill(0);
        
        audioBuffer->close();
        audioBuffer->open(QIODevice::ReadOnly);
        
        // Start audio sink
        audioSink->start(audioBuffer.get());
        
        // Start audio generation timer
        audioTimer->start(TIMER_INTERVAL_MS);
        
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

void AudioEngineV2::stopAudio() {
    if (!audioActive) {
        return;
    }
    
    QMutexLocker locker(&audioMutex);
    
    // Stop audio generation timer
    audioTimer->stop();
    
    // Stop audio sink
    if (audioSink) {
        audioSink->stop();
        audioSink.reset();
    }
    
    // Close buffer
    if (audioBuffer) {
        audioBuffer->close();
        audioBuffer.reset();
    }
    
    audioData.clear();
    audioActive = false;
    
    emit audioStopped();
    qDebug() << "Audio stopped";
}

void AudioEngineV2::setSampleRate(int sampleRate) {
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

void AudioEngineV2::setBufferSize(int bufferSize) {
    currentBufferSize = bufferSize;
    
    // If audio is active, restart with new buffer size
    if (audioActive) {
        stopAudio();
        startAudio();
    }
}

float AudioEngineV2::getCurrentLatency() const {
    if (audioSink) {
        // Estimate latency based on buffer size and sample rate
        return (float)currentBufferSize / currentSampleRate * 1000.0f; // ms
    }
    return 0.0f;
}

void AudioEngineV2::handleAudioStateChanged(QAudio::State state) {
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
            qDebug() << "Audio state: Idle - may need more data";
            // This is normal when buffer is empty
            break;
    }
}

void AudioEngineV2::generateAudioData() {
    if (!audioActive || !initialized || !tockusDSP || !pt8211DAC) {
        return;
    }
    
    QMutexLocker locker(&audioMutex);
    
    // Check how much space is available in the audio sink
    if (!audioSink) {
        return;
    }
    
    qint64 bytesAvailable = audioSink->bytesFree();
    if (bytesAvailable < (qint64)(currentBufferSize * channels * sizeof(float))) {
        // Not enough space, skip this generation cycle
        return;
    }
    
    // Generate new audio data
    int frameCount = currentBufferSize;
    int bufferSizeBytes = frameCount * channels * sizeof(float);
    
    QByteArray newAudioData;
    newAudioData.resize(bufferSizeBytes);
    float* floatBuffer = reinterpret_cast<float*>(newAudioData.data());
    
    // Process audio buffer
    processAudioBuffer(floatBuffer, frameCount);
    
    // Update the buffer content
    audioBuffer->close();
    audioData.append(newAudioData);
    
    // Keep buffer size reasonable (remove old data)
    int maxBufferSize = currentBufferSize * channels * sizeof(float) * 8; // 8 buffers worth
    if (audioData.size() > maxBufferSize) {
        audioData = audioData.right(maxBufferSize / 2); // Keep half
    }
    
    audioBuffer->setData(audioData);
    audioBuffer->open(QIODevice::ReadOnly);
    
    totalSamplesProcessed += frameCount;
}

void AudioEngineV2::processAudioBuffer(float* buffer, int frameCount) {
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