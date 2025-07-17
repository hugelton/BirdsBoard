#ifndef AUDIO_TEST_H
#define AUDIO_TEST_H

#include <QObject>
#include <QTimer>
#include <QAudioSink>
#include <QAudioFormat>
#include <QIODevice>
#include <QDebug>

/**
 * Audio testing and debugging utility
 */
class AudioTestDevice : public QIODevice
{
    Q_OBJECT

public:
    explicit AudioTestDevice(QObject* parent = nullptr);
    
    qint64 readData(char* data, qint64 maxlen) override;
    qint64 writeData(const char* data, qint64 len) override { Q_UNUSED(data) Q_UNUSED(len) return 0; }
    bool isSequential() const override { return true; }

private:
    float phase;
    float frequency;
    int sampleRate;
    int channels;
};

class AudioTestEngine : public QObject
{
    Q_OBJECT

public:
    explicit AudioTestEngine(QObject* parent = nullptr);
    ~AudioTestEngine();
    
    void listAudioDevices();
    bool testAudioOutput();
    void stopTest();

signals:
    void testStarted();
    void testStopped();
    void testError(const QString& error);

private slots:
    void handleStateChanged(QAudio::State state);

private:
    std::unique_ptr<QAudioSink> audioSink;
    std::unique_ptr<AudioTestDevice> testDevice;
    QAudioFormat audioFormat;
    bool isRunning;
};

#endif // AUDIO_TEST_H