#ifndef COREAUDIO_ENGINE_H
#define COREAUDIO_ENGINE_H

#include <QObject>
#include <QTimer>
#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudio.h>

class TockusDSP;
class PT8211DAC;

/**
 * Direct CoreAudio implementation for macOS
 * Bypasses Qt6 audio issues
 */
class CoreAudioEngine : public QObject
{
    Q_OBJECT

public:
    explicit CoreAudioEngine(QObject* parent = nullptr);
    ~CoreAudioEngine();
    
    void initialize(TockusDSP* dsp, PT8211DAC* dac);
    bool startAudio();
    void stopAudio();
    bool isAudioActive() const { return audioActive; }
    
    // Test function
    bool startTestTone();
    void stopTestTone();

signals:
    void audioStarted();
    void audioStopped();
    void audioError(const QString& error);

private:
    static OSStatus audioCallback(void* inRefCon,
                                  AudioUnitRenderActionFlags* ioActionFlags,
                                  const AudioTimeStamp* inTimeStamp,
                                  UInt32 inBusNumber,
                                  UInt32 inNumberFrames,
                                  AudioBufferList* ioData);
    
    static OSStatus testToneCallback(void* inRefCon,
                                     AudioUnitRenderActionFlags* ioActionFlags,
                                     const AudioTimeStamp* inTimeStamp,
                                     UInt32 inBusNumber,
                                     UInt32 inNumberFrames,
                                     AudioBufferList* ioData);
    
    bool setupAudioUnit();
    void cleanupAudioUnit();
    
    // Audio components
    AudioUnit audioUnit;
    bool audioActive;
    bool initialized;
    bool testToneActive;
    
    // DSP components
    TockusDSP* tockusDSP;
    PT8211DAC* pt8211DAC;
    
    // Test tone
    float testTonePhase;
    
    // Audio settings
    static constexpr int SAMPLE_RATE = 44100;
    static constexpr int CHANNELS = 2;
    static constexpr int BUFFER_SIZE = 512;
};

#endif // COREAUDIO_ENGINE_H