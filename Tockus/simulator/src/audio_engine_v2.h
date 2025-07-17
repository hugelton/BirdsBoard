#ifndef AUDIO_ENGINE_V2_H
#define AUDIO_ENGINE_V2_H

#include <QObject>
#include <QAudioSink>
#include <QAudioFormat>
#include <QBuffer>
#include <QTimer>
#include <QMutex>
#include <QDateTime>
#include <memory>

class TockusDSP;
class PT8211DAC;

/**
 * Simplified Audio Engine for Qt6
 * Uses push mode instead of pull mode for better compatibility
 */
class AudioEngineV2 : public QObject
{
    Q_OBJECT

public:
    explicit AudioEngineV2(QObject* parent = nullptr);
    ~AudioEngineV2();
    
    // Initialize with DSP and DAC components
    void initialize(TockusDSP* dsp, PT8211DAC* dac);
    
    // Audio control
    bool startAudio();
    void stopAudio();
    bool isAudioActive() const { return audioActive; }
    
    // Audio settings
    void setSampleRate(int sampleRate);
    void setBufferSize(int bufferSize);
    
    // Statistics
    int getCurrentSampleRate() const { return currentSampleRate; }
    int getCurrentBufferSize() const { return currentBufferSize; }
    float getCurrentLatency() const;
    
signals:
    void audioStarted();
    void audioStopped();
    void audioError(const QString& error);
    void bufferUnderrun();
    
private slots:
    void handleAudioStateChanged(QAudio::State state);
    void generateAudioData();
    
private:
    void setupAudioFormat();
    void processAudioBuffer(float* buffer, int frameCount);
    
    // Audio components
    std::unique_ptr<QAudioSink> audioSink;
    QAudioFormat audioFormat;
    std::unique_ptr<QBuffer> audioBuffer;
    QByteArray audioData;
    
    // DSP components
    TockusDSP* tockusDSP;
    PT8211DAC* pt8211DAC;
    
    // Audio settings
    int currentSampleRate;
    int currentBufferSize;
    int channels;
    
    // State
    bool audioActive;
    bool initialized;
    
    // Thread safety
    QMutex audioMutex;
    
    // Audio generation timer
    QTimer* audioTimer;
    
    // Performance monitoring
    qint64 totalSamplesProcessed;
    
    // Audio buffer constants
    static constexpr int DEFAULT_SAMPLE_RATE = 44100;
    static constexpr int DEFAULT_BUFFER_SIZE = 512;
    static constexpr int DEFAULT_CHANNELS = 2;  // Stereo output
    static constexpr int TIMER_INTERVAL_MS = 10; // Generate audio every 10ms
};

#endif // AUDIO_ENGINE_V2_H