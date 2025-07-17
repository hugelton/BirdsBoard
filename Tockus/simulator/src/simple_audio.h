#ifndef SIMPLE_AUDIO_H
#define SIMPLE_AUDIO_H

#include <QIODevice>
#include <QAudioSink>
#include <QAudioFormat>
#include <QTimer>
#include <QMutex>
#include <memory>

class TockusDSP;
class PT8211DAC;

/**
 * Ultra-simple audio device for Qt6
 * Implements QIODevice pull mode properly
 */
class SimpleAudioDevice : public QIODevice
{
    Q_OBJECT

public:
    explicit SimpleAudioDevice(TockusDSP* dsp, PT8211DAC* dac, QObject* parent = nullptr);
    ~SimpleAudioDevice();
    
    // QIODevice interface
    qint64 readData(char* data, qint64 maxlen) override;
    qint64 writeData(const char* data, qint64 len) override { Q_UNUSED(data) Q_UNUSED(len) return 0; }
    bool isSequential() const override { return true; }
    
    void setParameters(TockusDSP* dsp, PT8211DAC* dac);

private:
    TockusDSP* tockusDSP;
    PT8211DAC* pt8211DAC;
    int channels;
};

/**
 * Minimal audio engine using direct QIODevice
 */
class SimpleAudioEngine : public QObject
{
    Q_OBJECT

public:
    explicit SimpleAudioEngine(QObject* parent = nullptr);
    ~SimpleAudioEngine();
    
    void initialize(TockusDSP* dsp, PT8211DAC* dac);
    bool startAudio();
    void stopAudio();
    bool isAudioActive() const { return audioActive; }
    
signals:
    void audioStarted();
    void audioStopped();
    void audioError(const QString& error);

private slots:
    void handleStateChanged(QAudio::State state);

private:
    std::unique_ptr<QAudioSink> audioSink;
    std::unique_ptr<SimpleAudioDevice> audioIODevice;
    QAudioFormat audioFormat;
    
    TockusDSP* tockusDSP;
    PT8211DAC* pt8211DAC;
    
    bool audioActive;
    bool initialized;
    
    static constexpr int SAMPLE_RATE = 44100;
    static constexpr int CHANNELS = 2;
};

#endif // SIMPLE_AUDIO_H