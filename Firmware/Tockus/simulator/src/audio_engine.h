#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include <QAudioSink>
#include <QAudioFormat>
#include <QIODevice>
#include <QTimer>
#include <QMutex>
#include <QDateTime>
#include <memory>

class TockusDSP;
class PT8211DAC;

/**
 * Audio Engine for Tockus Simulator
 * 
 * Manages Qt audio output and real-time sample generation
 * using the TockusDSP and PT8211DAC components.
 */
class AudioEngine : public QIODevice
{
    Q_OBJECT

public:
    explicit AudioEngine(QObject* parent = nullptr);
    ~AudioEngine();
    
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
    
    // QIODevice interface
    qint64 readData(char* data, qint64 maxlen) override;
    qint64 writeData(const char* data, qint64 len) override;
    bool isSequential() const override { return true; }
    
signals:
    void audioStarted();
    void audioStopped();
    void audioError(const QString& error);
    void bufferUnderrun();
    
private slots:
    void handleAudioStateChanged(QAudio::State state);
    void handleAudioNotify();
    
private:
    void setupAudioFormat();
    void processAudioBuffer(float* buffer, int frameCount);
    
    // Audio components
    std::unique_ptr<QAudioSink> audioOutput;
    QAudioFormat audioFormat;
    
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
    
    // Performance monitoring
    QTimer* notifyTimer;
    qint64 totalSamplesProcessed;
    qint64 lastNotifyTime;
    
    // Audio buffer
    static constexpr int DEFAULT_SAMPLE_RATE = 44100;
    static constexpr int DEFAULT_BUFFER_SIZE = 512;
    static constexpr int DEFAULT_CHANNELS = 2;  // Stereo output
};

#endif // AUDIO_ENGINE_H