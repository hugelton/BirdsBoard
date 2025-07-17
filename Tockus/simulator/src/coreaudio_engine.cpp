#include "coreaudio_engine.h"
#include "tockus_dsp.h"
#include "pt8211_dac.h"
#include <QDebug>
#include <cmath>
#include <cstring>

CoreAudioEngine::CoreAudioEngine(QObject* parent)
    : QObject(parent)
    , audioUnit(nullptr)
    , audioActive(false)
    , initialized(false)
    , testToneActive(false)
    , tockusDSP(nullptr)
    , pt8211DAC(nullptr)
    , testTonePhase(0.0f)
{
    qDebug() << "CoreAudioEngine created";
}

CoreAudioEngine::~CoreAudioEngine() {
    stopAudio();
    stopTestTone();
    cleanupAudioUnit();
}

void CoreAudioEngine::initialize(TockusDSP* dsp, PT8211DAC* dac) {
    tockusDSP = dsp;
    pt8211DAC = dac;
    
    if (tockusDSP && pt8211DAC) {
        tockusDSP->setSampleRate(SAMPLE_RATE);
        pt8211DAC->setSampleRate(SAMPLE_RATE);
        initialized = true;
        qDebug() << "CoreAudioEngine initialized";
    }
}

bool CoreAudioEngine::setupAudioUnit() {
    OSStatus status;
    
    // Get default output device
    AudioDeviceID deviceID;
    UInt32 size = sizeof(AudioDeviceID);
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwarePropertyDefaultOutputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };
    
    status = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                        &propertyAddress,
                                        0, NULL,
                                        &size, &deviceID);
    if (status != noErr) {
        qDebug() << "Failed to get default output device:" << status;
        return false;
    }
    
    // Create audio component description
    AudioComponentDescription desc;
    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_DefaultOutput;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    desc.componentFlags = 0;
    desc.componentFlagsMask = 0;
    
    // Find audio component
    AudioComponent component = AudioComponentFindNext(NULL, &desc);
    if (component == NULL) {
        qDebug() << "Failed to find audio component";
        return false;
    }
    
    // Create audio unit
    status = AudioComponentInstanceNew(component, &audioUnit);
    if (status != noErr) {
        qDebug() << "Failed to create audio unit:" << status;
        return false;
    }
    
    // Set audio format
    AudioStreamBasicDescription format;
    format.mSampleRate = SAMPLE_RATE;
    format.mFormatID = kAudioFormatLinearPCM;
    format.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
    format.mChannelsPerFrame = CHANNELS;
    format.mFramesPerPacket = 1;
    format.mBitsPerChannel = 32;
    format.mBytesPerFrame = format.mChannelsPerFrame * sizeof(Float32);
    format.mBytesPerPacket = format.mBytesPerFrame * format.mFramesPerPacket;
    
    status = AudioUnitSetProperty(audioUnit,
                                  kAudioUnitProperty_StreamFormat,
                                  kAudioUnitScope_Input,
                                  0,
                                  &format,
                                  sizeof(format));
    if (status != noErr) {
        qDebug() << "Failed to set audio format:" << status;
        cleanupAudioUnit();
        return false;
    }
    
    // Set render callback
    AURenderCallbackStruct callbackStruct;
    callbackStruct.inputProc = audioCallback;
    callbackStruct.inputProcRefCon = this;
    
    status = AudioUnitSetProperty(audioUnit,
                                  kAudioUnitProperty_SetRenderCallback,
                                  kAudioUnitScope_Input,
                                  0,
                                  &callbackStruct,
                                  sizeof(callbackStruct));
    if (status != noErr) {
        qDebug() << "Failed to set render callback:" << status;
        cleanupAudioUnit();
        return false;
    }
    
    // Initialize audio unit
    status = AudioUnitInitialize(audioUnit);
    if (status != noErr) {
        qDebug() << "Failed to initialize audio unit:" << status;
        cleanupAudioUnit();
        return false;
    }
    
    qDebug() << "Audio unit setup successful";
    return true;
}

void CoreAudioEngine::cleanupAudioUnit() {
    if (audioUnit) {
        AudioUnitUninitialize(audioUnit);
        AudioComponentInstanceDispose(audioUnit);
        audioUnit = nullptr;
    }
}

bool CoreAudioEngine::startAudio() {
    if (audioActive) {
        qDebug() << "Audio already active";
        return true;
    }
    
    if (!initialized) {
        qDebug() << "CoreAudioEngine not initialized";
        return false;
    }
    
    qDebug() << "Starting CoreAudio...";
    
    if (!setupAudioUnit()) {
        emit audioError("Failed to setup audio unit");
        return false;
    }
    
    OSStatus status = AudioOutputUnitStart(audioUnit);
    if (status != noErr) {
        qDebug() << "Failed to start audio unit:" << status;
        cleanupAudioUnit();
        emit audioError("Failed to start audio output");
        return false;
    }
    
    audioActive = true;
    emit audioStarted();
    qDebug() << "CoreAudio started successfully";
    return true;
}

void CoreAudioEngine::stopAudio() {
    if (!audioActive) {
        return;
    }
    
    qDebug() << "Stopping CoreAudio...";
    
    if (audioUnit) {
        AudioOutputUnitStop(audioUnit);
    }
    
    cleanupAudioUnit();
    audioActive = false;
    emit audioStopped();
    qDebug() << "CoreAudio stopped";
}

bool CoreAudioEngine::startTestTone() {
    if (testToneActive) {
        stopTestTone();
    }
    
    qDebug() << "Starting test tone...";
    
    if (!setupAudioUnit()) {
        emit audioError("Failed to setup audio unit for test");
        return false;
    }
    
    // Set test tone callback
    AURenderCallbackStruct callbackStruct;
    callbackStruct.inputProc = testToneCallback;
    callbackStruct.inputProcRefCon = this;
    
    OSStatus status = AudioUnitSetProperty(audioUnit,
                                           kAudioUnitProperty_SetRenderCallback,
                                           kAudioUnitScope_Input,
                                           0,
                                           &callbackStruct,
                                           sizeof(callbackStruct));
    if (status != noErr) {
        qDebug() << "Failed to set test callback:" << status;
        cleanupAudioUnit();
        return false;
    }
    
    status = AudioOutputUnitStart(audioUnit);
    if (status != noErr) {
        qDebug() << "Failed to start test audio:" << status;
        cleanupAudioUnit();
        return false;
    }
    
    testToneActive = true;
    testTonePhase = 0.0f;
    qDebug() << "Test tone started - You should hear 440Hz";
    return true;
}

void CoreAudioEngine::stopTestTone() {
    if (!testToneActive) {
        return;
    }
    
    qDebug() << "Stopping test tone...";
    
    if (audioUnit) {
        AudioOutputUnitStop(audioUnit);
    }
    
    cleanupAudioUnit();
    testToneActive = false;
    qDebug() << "Test tone stopped";
}

OSStatus CoreAudioEngine::audioCallback(void* inRefCon,
                                         AudioUnitRenderActionFlags* ioActionFlags,
                                         const AudioTimeStamp* inTimeStamp,
                                         UInt32 inBusNumber,
                                         UInt32 inNumberFrames,
                                         AudioBufferList* ioData) {
    CoreAudioEngine* engine = static_cast<CoreAudioEngine*>(inRefCon);
    
    Float32* leftChannel = static_cast<Float32*>(ioData->mBuffers[0].mData);
    Float32* rightChannel = nullptr;
    
    if (ioData->mNumberBuffers > 1) {
        rightChannel = static_cast<Float32*>(ioData->mBuffers[1].mData);
    }
    
    // Clear the buffer first
    for (UInt32 buffer = 0; buffer < ioData->mNumberBuffers; buffer++) {
        memset(ioData->mBuffers[buffer].mData, 0, ioData->mBuffers[buffer].mDataByteSize);
    }
    
    for (UInt32 frame = 0; frame < inNumberFrames; frame++) {
        float sample = 0.0f;
        
        if (engine->tockusDSP && engine->pt8211DAC) {
            // Generate sample from DSP
            sample = engine->tockusDSP->processNextSample();
            sample = engine->pt8211DAC->processSample(sample);
        }
        
        // Apply reduced gain to prevent clipping
        sample *= 0.1f;  // Further reduced to 10%
        
        // Ensure sample is in valid range
        sample = fmaxf(-1.0f, fminf(1.0f, sample));
        
        // Fill channels correctly
        if (ioData->mNumberBuffers == 2) {
            // Separate left/right buffers
            leftChannel[frame] = sample;
            if (rightChannel) {
                rightChannel[frame] = sample;
            }
        } else if (ioData->mBuffers[0].mNumberChannels == 2) {
            // Interleaved stereo
            Float32* interleavedData = static_cast<Float32*>(ioData->mBuffers[0].mData);
            interleavedData[frame * 2] = sample;     // Left
            interleavedData[frame * 2 + 1] = sample; // Right
        } else {
            // Mono
            leftChannel[frame] = sample;
        }
    }
    
    return noErr;
}

OSStatus CoreAudioEngine::testToneCallback(void* inRefCon,
                                          AudioUnitRenderActionFlags* ioActionFlags,
                                          const AudioTimeStamp* inTimeStamp,
                                          UInt32 inBusNumber,
                                          UInt32 inNumberFrames,
                                          AudioBufferList* ioData) {
    CoreAudioEngine* engine = static_cast<CoreAudioEngine*>(inRefCon);
    
    const float frequency = 440.0f; // A4 note - exactly 440Hz
    const float amplitude = 0.1f;   // Reduced to 10% to prevent clipping
    const float phaseIncrement = 2.0f * M_PI * frequency / (float)SAMPLE_RATE;
    
    Float32* leftChannel = static_cast<Float32*>(ioData->mBuffers[0].mData);
    Float32* rightChannel = nullptr;
    
    if (ioData->mNumberBuffers > 1) {
        rightChannel = static_cast<Float32*>(ioData->mBuffers[1].mData);
    }
    
    // Clear the buffer first
    for (UInt32 buffer = 0; buffer < ioData->mNumberBuffers; buffer++) {
        memset(ioData->mBuffers[buffer].mData, 0, ioData->mBuffers[buffer].mDataByteSize);
    }
    
    for (UInt32 frame = 0; frame < inNumberFrames; frame++) {
        // Generate clean sine wave
        float sample = sinf(engine->testTonePhase) * amplitude;
        engine->testTonePhase += phaseIncrement;
        
        // Keep phase in valid range
        while (engine->testTonePhase >= 2.0f * M_PI) {
            engine->testTonePhase -= 2.0f * M_PI;
        }
        
        // Ensure sample is in valid range
        sample = fmaxf(-1.0f, fminf(1.0f, sample));
        
        // Fill channels correctly
        if (ioData->mNumberBuffers == 2) {
            // Separate left/right buffers
            leftChannel[frame] = sample;
            if (rightChannel) {
                rightChannel[frame] = sample;
            }
        } else if (ioData->mBuffers[0].mNumberChannels == 2) {
            // Interleaved stereo
            Float32* interleavedData = static_cast<Float32*>(ioData->mBuffers[0].mData);
            interleavedData[frame * 2] = sample;     // Left
            interleavedData[frame * 2 + 1] = sample; // Right
        } else {
            // Mono
            leftChannel[frame] = sample;
        }
    }
    
    return noErr;
}