#ifndef TOCKUS_DSP_H
#define TOCKUS_DSP_H

#include <cmath>
#include <cstdint>
#include <algorithm>

// Constants from Arduino code
#define PI 3.14159265359
#define KARPLUS_BUFFER_SIZE 200
#define NUM_MODES 4
#define NUM_ALGORITHMS 8

// ADC range calibration (from CLAUDE.md)
#define PITCH_CV_MIN    0
#define PITCH_CV_MAX    4095
#define PITCH_KNOB_MIN  10
#define PITCH_KNOB_MAX  4000
#define CV1_MIN         8
#define CV1_MAX         2000
#define CV2_MIN         8
#define CV2_MAX         2000

// Drum algorithms (same as Arduino)
enum DrumAlgorithm {
    ALGO_BASS = 0,      // 808 Bass drum
    ALGO_SNARE = 1,     // 808 Snare drum
    ALGO_HIHAT = 2,     // 808 Hi-hat
    ALGO_KARPLUS = 3,   // Karplus-Strong
    ALGO_MODAL = 4,     // Modal synthesis
    ALGO_ZAP = 5,       // ZAP sound
    ALGO_CLAP = 6,      // 808 Clap
    ALGO_COWBELL = 7,   // Cowbell (two sine oscillators)
};

// Bandpass filter state (from Arduino)
struct BandpassFilter {
    float x1, x2;  // Input delay line
    float y1, y2;  // Output delay line
    float a0, a1, a2, b1, b2;  // Filter coefficients
};

// Resonant filter state (from Arduino)
struct ResonantFilter {
    float x1, x2;  // Input delay line
    float y1, y2;  // Output delay line
    float cutoff;   // Current cutoff frequency
    float resonance; // Q factor
    float a0, a1, a2, b1, b2;  // Filter coefficients
};

// Modal synthesis mode (from Arduino)
struct Mode {
    float frequency;
    float amplitude;
    float decay;
    float phase;
};

class TockusDSP {
public:
    TockusDSP();
    ~TockusDSP();
    
    // Main processing functions
    void setSampleRate(int sampleRate);
    void setParameters(float pitch, float cv1, float cv2, bool gate);
    void triggerDrum();
    float processNextSample();
    
    // Getters for UI
    uint8_t getCurrentAlgorithm() const { return currentAlgorithm; }
    float getCurrentFrequency() const { return currentFrequency; }
    bool isTriggerActive() const { return triggerActive; }
    float getEnvelopeAmplitude() const { return envAmplitude; }
    
private:
    // Audio parameters
    int sampleRate;
    const float MASTER_GAIN = 2.0f;
    bool gateState;
    bool lastGateState;
    bool triggerActive;
    uint64_t triggerStartTime;
    
    // Current parameters
    uint8_t currentAlgorithm;
    float frequency;
    float currentFrequency;
    float baseFrequency;
    float algorithmParam;
    
    // Envelope parameters
    float envAmplitude;
    float envFrequency;
    float envDecayRate;
    
    // Phase accumulator
    float phase;
    uint64_t sampleCount;
    
    // Noise generator state
    uint32_t noiseState;
    
    // Algorithm-specific parameters
    float snareNoiseAmp;
    float snareToneAmp;
    float hihatEnvelope;
    float clapPulseEnv;
    float clapReverbEnv;
    float bassFilterEnv;
    float bassImpulse;
    float cowbellPhases[4];
    
    // Filter instances
    BandpassFilter bpf;
    ResonantFilter bassFilter;
    
    // Karplus-Strong parameters
    float karplusBuffer[KARPLUS_BUFFER_SIZE];
    int karplusIndex;
    float karplusDamping;
    
    // Modal synthesis parameters
    Mode modes[NUM_MODES];
    
    // Anti-aliasing filter
    float lastSample;
    const float LOWPASS_ALPHA = 0.7f;
    
    // Core DSP functions (ported from Arduino)
    void initializeEnvelopes();
    void updateEnvelopes(float timeElapsed);
    void updateRealtimeFrequency();
    float applyAlgorithmFrequencyScaling(float baseFreq, uint8_t algorithm);
    
    // Drum generators (ported from Arduino)
    float generateDrumSample(float timeElapsed);
    float generateBassDrum(float timeElapsed);
    float generateZapSound(float timeElapsed);
    float generateSnareDrum(float timeElapsed);
    float generateHiHat(float timeElapsed);
    float generateKarplusStrong(float timeElapsed);
    float generateModalSynthesis(float timeElapsed);
    float generateClap(float timeElapsed);
    float generateCowbell(float timeElapsed);
    
    // Utility functions
    float generateWhiteNoise();
    uint64_t getTimeMs();
    
    // Filter functions (ported from Arduino)
    void initializeBandpassFilter();
    void setBandpassFilter(float centerFreq, float Q);
    float processBandpassFilter(float input);
    
    void initializeResonantFilter();
    void updateResonantFilter(ResonantFilter* filter, float cutoff, float resonance);
    float processResonantFilter(ResonantFilter* filter, float input);
    
    void initializeKarplusStrong();
    void initializeModalSynthesis();
    void setupModalModes();
    
    // Authentic cowbell frequencies from Arduino
    static const float cowbellFreqs[4];
};

#endif // TOCKUS_DSP_H