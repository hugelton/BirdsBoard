#include "tockus_dsp.h"
#include <chrono>
#include <cstring>

// Cowbell frequencies (authentic 808 values)
const float TockusDSP::cowbellFreqs[4] = {555.0f, 835.0f, 1370.0f, 1940.0f};

TockusDSP::TockusDSP() 
    : sampleRate(44100)
    , MASTER_GAIN(2.0f)
    , gateState(false)
    , lastGateState(false)
    , triggerActive(false)
    , triggerStartTime(0)
    , currentAlgorithm(ALGO_BASS)
    , frequency(60.0f)
    , currentFrequency(60.0f)
    , baseFrequency(60.0f)
    , algorithmParam(0.5f)
    , envAmplitude(0.0f)
    , envFrequency(0.0f)
    , envDecayRate(0.0f)
    , phase(0.0f)
    , sampleCount(0)
    , noiseState(1)
    , snareNoiseAmp(0.0f)
    , snareToneAmp(0.0f)
    , hihatEnvelope(0.0f)
    , clapPulseEnv(0.0f)
    , clapReverbEnv(0.0f)
    , bassFilterEnv(0.0f)
    , bassImpulse(0.0f)
    , karplusIndex(0)
    , karplusDamping(0.99f)
    , lastSample(0.0f)
    , LOWPASS_ALPHA(0.7f)
{
    // Initialize cowbell phases
    for (int i = 0; i < 4; i++) {
        cowbellPhases[i] = 0.0f;
    }
    
    // Initialize filters
    initializeBandpassFilter();
    initializeResonantFilter();
    initializeKarplusStrong();
    initializeModalSynthesis();
}

TockusDSP::~TockusDSP() {
}

void TockusDSP::setSampleRate(int sr) {
    sampleRate = sr;
    
    // Reinitialize filters with new sample rate
    initializeBandpassFilter();
    initializeResonantFilter();
}

void TockusDSP::setParameters(float pitch, float cv1, float cv2, bool gate) {
    // Convert GUI values to ADC ranges
    uint16_t pitchCV = (uint16_t)(pitch * (PITCH_CV_MAX - PITCH_CV_MIN) + PITCH_CV_MIN);
    uint16_t cv1Val = (uint16_t)(cv1 * (CV1_MAX - CV1_MIN) + CV1_MIN);
    uint16_t cv2Val = (uint16_t)(cv2 * (CV2_MAX - CV2_MIN) + CV2_MIN);
    
    // Gate handling - trigger on rising edge
    if (gate && !lastGateState) {
        triggerDrum();
    }
    lastGateState = gate;
    gateState = gate;
    
    // Calculate frequency with calibrated ranges (same as Arduino)
    float adcVoltage = ((PITCH_CV_MAX - pitchCV) / (float)PITCH_CV_MAX) * 3.3f;
    float knobVoltage = 1.65f; // Default knob position
    
    float actualCV = (adcVoltage - 1.65f) / 0.33f;
    actualCV = std::max(0.0f, std::min(actualCV, 5.0f));
    
    float cvOctaves = actualCV;
    float knobOctaves = (knobVoltage - 1.65f) / 1.65f;
    
    // Calculate base frequency (same as Arduino)
    float baseFreq = 440.0f * std::pow(2.0f, cvOctaves + knobOctaves - 4.0f);
    
    // Apply algorithm-specific frequency scaling
    frequency = applyAlgorithmFrequencyScaling(baseFreq, currentAlgorithm);
    
    // CV1: Algorithm selection
    uint8_t newAlgorithm = (uint8_t)((cv1Val - CV1_MIN) * (NUM_ALGORITHMS - 1) / (CV1_MAX - CV1_MIN));
    newAlgorithm = std::max((uint8_t)0, std::min(newAlgorithm, (uint8_t)(NUM_ALGORITHMS - 1)));
    currentAlgorithm = newAlgorithm;
    
    // CV2: Algorithm parameter
    algorithmParam = (float)(cv2Val - CV2_MIN) / (CV2_MAX - CV2_MIN);
    algorithmParam = std::max(0.0f, std::min(algorithmParam, 1.0f));
}

void TockusDSP::triggerDrum() {
    triggerActive = true;
    triggerStartTime = getTimeMs();
    phase = 0.0f;
    
    // Store base frequency at trigger time
    baseFrequency = frequency;
    currentFrequency = frequency;
    
    // Initialize envelopes based on algorithm
    initializeEnvelopes();
    
    // Reset noise state
    noiseState = 1;
}

float TockusDSP::processNextSample() {
    // Read parameters every 4 samples for responsive pitch control
    if (sampleCount % 4 == 0) {
        // Update real-time frequency for active sounds
        if (triggerActive) {
            updateRealtimeFrequency();
        }
    }
    
    // Generate audio sample based on current algorithm
    float sample = 0.0f;
    
    if (triggerActive) {
        float timeElapsed = (getTimeMs() - triggerStartTime) / 1000.0f;
        sample = generateDrumSample(timeElapsed);
        
        // Check if envelope has decayed enough to stop
        if (envAmplitude < 0.001f) {
            triggerActive = false;
        }
    }
    
    sampleCount++;
    return sample;
}

void TockusDSP::initializeEnvelopes() {
    envAmplitude = 1.0f;
    envFrequency = currentFrequency;
    
    // Set decay rates based on algorithm (same as Arduino)
    switch (currentAlgorithm) {
        case ALGO_BASS:
            envDecayRate = 1.5f + algorithmParam * 3.5f;  // 1.5-5 Hz decay
            bassFilterEnv = 1.0f;
            break;
        case ALGO_ZAP:
            envDecayRate = 8.0f + algorithmParam * 12.0f; // 8-20 Hz decay
            break;
        case ALGO_SNARE:
            envDecayRate = 8.0f * (0.5f + algorithmParam * 2.5f);  // 4-28 Hz decay
            snareNoiseAmp = 1.0f;
            snareToneAmp = 1.0f;
            break;
        case ALGO_HIHAT:
            envDecayRate = 20.0f * (0.5f + algorithmParam * 3.5f);  // 10-90 Hz decay
            hihatEnvelope = 1.0f;
            break;
        case ALGO_KARPLUS:
            envDecayRate = 3.0f + algorithmParam * 5.0f;  // 3-8 Hz decay
            karplusDamping = 0.995f - algorithmParam * 0.2f;  // 0.995-0.795 damping
            initializeKarplusStrong();
            break;
        case ALGO_MODAL:
            envDecayRate = 4.0f + algorithmParam * 6.0f;  // 4-10 Hz decay
            setupModalModes();
            break;
        case ALGO_CLAP:
            envDecayRate = 12.0f * (0.5f + algorithmParam * 2.5f);  // 6-42 Hz decay
            clapPulseEnv = 1.0f;
            clapReverbEnv = 1.0f;
            break;
        case ALGO_COWBELL:
            envDecayRate = 4.0f + algorithmParam * 6.0f;  // 4-10 Hz decay
            // Reset cowbell oscillator phases
            for (int i = 0; i < 4; i++) {
                cowbellPhases[i] = 0.0f;
            }
            break;
        default:
            envDecayRate = 5.0f + algorithmParam * 5.0f;  // 5-10 Hz decay
            break;
    }
}

float TockusDSP::generateDrumSample(float timeElapsed) {
    float sample = 0.0f;
    
    // Update envelopes
    updateEnvelopes(timeElapsed);
    
    switch (currentAlgorithm) {
        case ALGO_BASS:
            sample = generateBassDrum(timeElapsed);
            break;
        case ALGO_ZAP:
            sample = generateZapSound(timeElapsed);
            break;
        case ALGO_SNARE:
            sample = generateSnareDrum(timeElapsed);
            break;
        case ALGO_HIHAT:
            sample = generateHiHat(timeElapsed);
            break;
        case ALGO_KARPLUS:
            sample = generateKarplusStrong(timeElapsed);
            break;
        case ALGO_MODAL:
            sample = generateModalSynthesis(timeElapsed);
            break;
        case ALGO_CLAP:
            sample = generateClap(timeElapsed);
            break;
        case ALGO_COWBELL:
            sample = generateCowbell(timeElapsed);
            break;
        default:
            sample = generateBassDrum(timeElapsed);
            break;
    }
    
    // Apply anti-aliasing lowpass filter
    sample = LOWPASS_ALPHA * sample + (1.0f - LOWPASS_ALPHA) * lastSample;
    lastSample = sample;
    
    // Apply algorithm-specific gain adjustments to prevent clipping
    float algorithmGain = 1.0f;
    switch (currentAlgorithm) {
        case ALGO_BASS:
            algorithmGain = 1.0f;
            break;
        case ALGO_SNARE:
            algorithmGain = 0.4f;  // Reduce SNARE gain significantly
            break;
        case ALGO_HIHAT:
            algorithmGain = 0.8f;
            break;
        case ALGO_KARPLUS:
            algorithmGain = 0.5f;  // Reduce KARPLUS gain
            break;
        case ALGO_MODAL:
            algorithmGain = 0.3f;  // Reduce MODAL gain significantly
            break;
        case ALGO_ZAP:
            algorithmGain = 0.3f;  // Reduce ZAP gain significantly
            break;
        case ALGO_CLAP:
            algorithmGain = 0.7f;
            break;
        case ALGO_COWBELL:
            algorithmGain = 0.8f;
            break;
        default:
            algorithmGain = 1.0f;
            break;
    }
    
    // Apply master gain with algorithm-specific adjustment
    float boostedSample = sample * MASTER_GAIN * algorithmGain;
    
    // Soft saturation instead of hard clipping
    if (boostedSample > 0.8f) {
        boostedSample = 0.8f + 0.2f * std::tanh((boostedSample - 0.8f) * 5.0f);
    } else if (boostedSample < -0.8f) {
        boostedSample = -0.8f + 0.2f * std::tanh((boostedSample + 0.8f) * 5.0f);
    }
    
    return boostedSample;
}

void TockusDSP::updateEnvelopes(float timeElapsed) {
    // Exponential decay for amplitude
    envAmplitude = std::exp(-envDecayRate * timeElapsed);
    
    // Pitch envelope handling - BASS and ZAP have pitch envelopes
    if (currentAlgorithm == ALGO_BASS || currentAlgorithm == ALGO_ZAP) {
        float pitchDecay = (currentAlgorithm == ALGO_ZAP) ? 15.0f : 5.0f;
        envFrequency = currentFrequency * (1.0f + 2.0f * std::exp(-pitchDecay * timeElapsed));
    } else {
        envFrequency = currentFrequency;
    }
    
    // Snare-specific envelope updates
    if (currentAlgorithm == ALGO_SNARE) {
        snareNoiseAmp = std::exp(-(envDecayRate * 1.5f) * timeElapsed);
        snareToneAmp = std::exp(-envDecayRate * timeElapsed);
    }
    
    // Hi-hat envelope (very fast decay)
    if (currentAlgorithm == ALGO_HIHAT) {
        hihatEnvelope = std::exp(-envDecayRate * timeElapsed);
    }
    
    // Clap envelope (pulse + reverb)
    if (currentAlgorithm == ALGO_CLAP) {
        clapPulseEnv = 0.0f;
        for (int i = 0; i < 4; i++) {
            float pulseTime = timeElapsed - i * 0.03f;  // 30ms spacing
            if (pulseTime >= 0.0f && pulseTime <= 0.01f) {  // 10ms pulse width
                clapPulseEnv += std::exp(-50.0f * pulseTime);
            }
        }
        
        float decayMultiplier = 0.5f + algorithmParam * 1.5f;
        clapReverbEnv = std::exp(-envDecayRate * decayMultiplier * timeElapsed);
    }
}

void TockusDSP::updateRealtimeFrequency() {
    currentFrequency = applyAlgorithmFrequencyScaling(frequency, currentAlgorithm);
    
    // For most algorithms, update envelope frequency immediately
    if (currentAlgorithm != ALGO_BASS && currentAlgorithm != ALGO_ZAP) {
        envFrequency = currentFrequency;
    }
    
    // Algorithm-specific real-time updates
    if (currentAlgorithm == ALGO_MODAL) {
        // Update modal frequencies in real-time
        modes[0].frequency = currentFrequency * 1.0f;
        modes[1].frequency = currentFrequency * 1.6f;
        modes[2].frequency = currentFrequency * 2.3f;
        modes[3].frequency = currentFrequency * 3.1f;
    }
    
    // For Karplus-Strong, adjust delay line length for new frequency
    if (currentAlgorithm == ALGO_KARPLUS) {
        float newDelay = sampleRate / currentFrequency;
        if (newDelay > 0 && newDelay < KARPLUS_BUFFER_SIZE) {
            karplusIndex = (int)(newDelay * 0.8f);  // 80% of calculated delay
        }
    }
}

float TockusDSP::applyAlgorithmFrequencyScaling(float baseFreq, uint8_t algorithm) {
    float scaledFreq = baseFreq;
    
    switch (algorithm) {
        case ALGO_BASS:
            scaledFreq = baseFreq / 4.0f;  // -2 octaves
            scaledFreq = std::max(20.0f, std::min(scaledFreq, 150.0f));
            break;
        case ALGO_SNARE:
            scaledFreq = baseFreq / 2.0f;  // -1 octave
            scaledFreq = std::max(100.0f, std::min(scaledFreq, 400.0f));
            break;
        case ALGO_HIHAT:
            scaledFreq = baseFreq;
            scaledFreq = std::max(200.0f, std::min(scaledFreq, 2000.0f));
            break;
        case ALGO_KARPLUS:
            scaledFreq = baseFreq / 2.0f;  // -1 octave
            scaledFreq = std::max(80.0f, std::min(scaledFreq, 800.0f));
            break;
        case ALGO_MODAL:
            scaledFreq = baseFreq * 4.0f;  // +2 octaves
            scaledFreq = std::max(240.0f, std::min(scaledFreq, 2400.0f));
            break;
        case ALGO_ZAP:
            scaledFreq = baseFreq / 2.8f;  // -1.5 octaves
            scaledFreq = std::max(50.0f, std::min(scaledFreq, 500.0f));
            break;
        case ALGO_CLAP:
            scaledFreq = baseFreq;
            scaledFreq = std::max(150.0f, std::min(scaledFreq, 1500.0f));
            break;
        case ALGO_COWBELL:
            scaledFreq = baseFreq * 4.0f;  // +2 octaves
            scaledFreq = std::max(2000.0f, std::min(scaledFreq, 8000.0f));
            break;
        default:
            scaledFreq = std::max(20.0f, std::min(baseFreq, 8000.0f));
            break;
    }
    
    return scaledFreq;
}

uint64_t TockusDSP::getTimeMs() {
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

// Generate white noise (same as Arduino)
float TockusDSP::generateWhiteNoise() {
    noiseState = noiseState * 1103515245 + 12345;
    return ((noiseState >> 16) & 0x7FFF) / 32768.0f - 1.0f;
}

// Bass drum generator (ported from Arduino)
float TockusDSP::generateBassDrum(float timeElapsed) {
    // Generate impulse at the start
    if (timeElapsed < 0.002f) {  // 2ms impulse
        bassImpulse = 1.0f - (timeElapsed / 0.002f);
    } else {
        bassImpulse = 0.0f;
    }
    
    // Filter cutoff envelope
    float cutoffEnv = std::exp(-8.0f * timeElapsed);
    float bassFilterCutoff = envFrequency + (envFrequency * 3.0f * cutoffEnv);
    
    // High resonance for self-oscillation
    float resonance = 8.0f + algorithmParam * 12.0f;  // Q: 8-20
    
    // Update filter coefficients
    updateResonantFilter(&bassFilter, bassFilterCutoff, resonance);
    
    // Process impulse through resonant filter
    float output = processResonantFilter(&bassFilter, bassImpulse);
    
    // Apply amplitude envelope
    output *= envAmplitude;
    
    return output * 0.8f;  // Scale for headroom
}

// ZAP sound generator (ported from Arduino)
float TockusDSP::generateZapSound(float timeElapsed) {
    // Dramatic pitch envelope
    float pitchEnv = std::exp(-20.0f * timeElapsed);
    float startMultiplier = 8.0f + algorithmParam * 12.0f;
    float zapFreq = currentFrequency * (1.0f + startMultiplier * pitchEnv);
    
    // Main ZAP oscillator (sawtooth)
    float phaseInc = zapFreq / sampleRate;
    float sawtoothPhase = std::fmod(zapFreq * timeElapsed, 1.0f);
    float sawtooth = 2.0f * sawtoothPhase - 1.0f;
    
    float sample = sawtooth * envAmplitude * 0.5f;
    
    // Add noise burst at the beginning
    if (timeElapsed < 0.05f) {
        float noiseBurst = generateWhiteNoise() * (1.0f - timeElapsed / 0.05f) * 0.3f;
        sample += noiseBurst;
    }
    
    // Add harmonics based on parameter
    if (algorithmParam > 0.1f) {
        float harmonicLevel = algorithmParam * 0.4f;
        float harmonic = std::sin(2.0f * PI * zapFreq * 2.0f * timeElapsed) * envAmplitude * harmonicLevel;
        sample += harmonic;
    }
    
    return sample * 0.7f;
}

// Continue with remaining algorithm implementations...
float TockusDSP::generateSnareDrum(float timeElapsed) {
    // Tone component with pitch envelope
    float pitchEnv = std::exp(-25.0f * timeElapsed);
    float toneFreq = envFrequency * (1.0f + 2.0f * pitchEnv);
    
    // Main tone oscillator
    float tone = std::sin(2.0f * PI * toneFreq * timeElapsed) * snareToneAmp;
    
    // Noise component
    float noise = generateWhiteNoise() * snareNoiseAmp;
    
    // Bandpass filter the noise
    setBandpassFilter(800.0f + algorithmParam * 1200.0f, 2.0f);
    float filteredNoise = processBandpassFilter(noise);
    
    // Mix tone and noise
    float toneMix = 0.6f;
    float noiseMix = 0.4f;
    
    float result = (tone * toneMix + filteredNoise * noiseMix) * 0.7f;
    return result;
}

float TockusDSP::generateHiHat(float timeElapsed) {
    // Multiple square waves
    float square1 = (std::sin(2.0f * PI * envFrequency * 2.1f * timeElapsed) > 0) ? 1.0f : -1.0f;
    float square2 = (std::sin(2.0f * PI * envFrequency * 3.3f * timeElapsed) > 0) ? 1.0f : -1.0f;
    float square3 = (std::sin(2.0f * PI * envFrequency * 4.7f * timeElapsed) > 0) ? 1.0f : -1.0f;
    float square4 = (std::sin(2.0f * PI * envFrequency * 6.1f * timeElapsed) > 0) ? 1.0f : -1.0f;
    
    float squareSum = (square1 + square2 * 0.8f + square3 * 0.6f + square4 * 0.4f) * 0.25f;
    
    // Add noise component
    float noise = generateWhiteNoise() * 0.8f;
    
    // Mix squares and noise
    float rawSignal = squareSum + noise;
    
    // Bandpass filter
    setBandpassFilter(10000.0f, 3.0f);
    float filtered = processBandpassFilter(rawSignal);
    
    return filtered * hihatEnvelope * 1.5f;
}

float TockusDSP::generateKarplusStrong(float timeElapsed) {
    float output = karplusBuffer[karplusIndex];
    
    // Calculate next buffer position
    int nextIndex = (karplusIndex + 1) % KARPLUS_BUFFER_SIZE;
    
    // Low-pass filter with damping
    float filteredSample = (karplusBuffer[karplusIndex] + karplusBuffer[nextIndex]) * 0.5f;
    karplusBuffer[karplusIndex] = filteredSample * karplusDamping;
    
    // Update index
    karplusIndex = nextIndex;
    
    return output * envAmplitude;
}

float TockusDSP::generateModalSynthesis(float timeElapsed) {
    float output = 0.0f;
    
    for (int i = 0; i < NUM_MODES; i++) {
        float modeOutput = std::sin(modes[i].phase) * modes[i].amplitude * 
                          std::exp(-modes[i].decay * timeElapsed);
        output += modeOutput;
        
        // Update phase
        modes[i].phase += 2.0f * PI * modes[i].frequency / sampleRate;
        if (modes[i].phase >= 2.0f * PI) {
            modes[i].phase -= 2.0f * PI;
        }
    }
    
    return output * envAmplitude * 0.25f;
}

float TockusDSP::generateClap(float timeElapsed) {
    float noise = generateWhiteNoise() * 1.2f;
    
    // Bandpass filter
    setBandpassFilter(1000.0f, 3.0f);
    float filteredNoise = processBandpassFilter(noise);
    
    // Apply pulse envelope + reverb envelope
    float pulseComponent = filteredNoise * clapPulseEnv;
    float reverbComponent = filteredNoise * clapReverbEnv * 0.3f;
    
    float result = (pulseComponent + reverbComponent) * 1.8f;
    return result;
}

float TockusDSP::generateCowbell(float timeElapsed) {
    float output = 0.0f;
    
    // Generate 4 pulse waves at authentic 808 frequencies
    for (int i = 0; i < 4; i++) {
        // Update phase for each oscillator
        cowbellPhases[i] += 2.0f * PI * cowbellFreqs[i] / sampleRate;
        if (cowbellPhases[i] >= 2.0f * PI) {
            cowbellPhases[i] -= 2.0f * PI;
        }
        
        // Generate pulse wave
        float pulse = (std::sin(cowbellPhases[i]) > 0) ? 1.0f : -1.0f;
        
        // Weight the oscillators
        float weight = 1.0f / (i + 1);
        output += pulse * weight;
    }
    
    // Normalize and apply envelope
    output = output * 0.25f * envAmplitude;
    
    // CV2 controls metallic filtering
    float filterFreq = 2000.0f + algorithmParam * 3000.0f;
    setBandpassFilter(filterFreq, 4.0f);
    output = processBandpassFilter(output);
    
    return output * 0.8f;
}

// Filter implementations (ported from Arduino)
void TockusDSP::initializeBandpassFilter() {
    bpf.x1 = bpf.x2 = bpf.y1 = bpf.y2 = 0.0f;
    setBandpassFilter(8000.0f, 2.0f);
}

void TockusDSP::setBandpassFilter(float centerFreq, float Q) {
    float w = 2.0f * PI * centerFreq / sampleRate;
    float alpha = std::sin(w) / (2.0f * Q);
    
    float norm = 1.0f / (1.0f + alpha);
    
    bpf.a0 = alpha * norm;
    bpf.a1 = 0.0f;
    bpf.a2 = -alpha * norm;
    bpf.b1 = -2.0f * std::cos(w) * norm;
    bpf.b2 = (1.0f - alpha) * norm;
}

float TockusDSP::processBandpassFilter(float input) {
    float output = bpf.a0 * input + bpf.a1 * bpf.x1 + bpf.a2 * bpf.x2
                   - bpf.b1 * bpf.y1 - bpf.b2 * bpf.y2;
    
    // Update delay lines
    bpf.x2 = bpf.x1;
    bpf.x1 = input;
    bpf.y2 = bpf.y1;
    bpf.y1 = output;
    
    return output;
}

void TockusDSP::initializeResonantFilter() {
    bassFilter.x1 = bassFilter.x2 = bassFilter.y1 = bassFilter.y2 = 0.0f;
    bassFilter.cutoff = 80.0f;
    bassFilter.resonance = 10.0f;
    updateResonantFilter(&bassFilter, 80.0f, 10.0f);
}

void TockusDSP::updateResonantFilter(ResonantFilter* filter, float cutoff, float resonance) {
    cutoff = std::max(20.0f, std::min(cutoff, 8000.0f));
    resonance = std::max(0.5f, std::min(resonance, 20.0f));
    
    filter->cutoff = cutoff;
    filter->resonance = resonance;
    
    // Calculate filter coefficients
    float w = 2.0f * PI * cutoff / sampleRate;
    float cosw = std::cos(w);
    float sinw = std::sin(w);
    float alpha = sinw / (2.0f * resonance);
    
    float norm = 1.0f / (1.0f + alpha);
    
    filter->a0 = (1.0f - cosw) * 0.5f * norm;
    filter->a1 = (1.0f - cosw) * norm;
    filter->a2 = (1.0f - cosw) * 0.5f * norm;
    filter->b1 = -2.0f * cosw * norm;
    filter->b2 = (1.0f - alpha) * norm;
}

float TockusDSP::processResonantFilter(ResonantFilter* filter, float input) {
    float output = filter->a0 * input + filter->a1 * filter->x1 + filter->a2 * filter->x2
                   - filter->b1 * filter->y1 - filter->b2 * filter->y2;
    
    // Update delay lines
    filter->x2 = filter->x1;
    filter->x1 = input;
    filter->y2 = filter->y1;
    filter->y1 = output;
    
    return output;
}

void TockusDSP::initializeKarplusStrong() {
    // Fill buffer with noise burst
    for (int i = 0; i < KARPLUS_BUFFER_SIZE; i++) {
        karplusBuffer[i] = generateWhiteNoise() * 0.5f;
    }
    karplusIndex = 0;
}

void TockusDSP::initializeModalSynthesis() {
    setupModalModes();
}

void TockusDSP::setupModalModes() {
    float baseFreq = currentFrequency;
    
    // Mode frequencies (harmonic ratios)
    modes[0].frequency = baseFreq * 1.0f;
    modes[1].frequency = baseFreq * 1.6f;
    modes[2].frequency = baseFreq * 2.3f;
    modes[3].frequency = baseFreq * 3.1f;
    
    // Mode amplitudes
    modes[0].amplitude = 1.0f;
    modes[1].amplitude = 0.7f;
    modes[2].amplitude = 0.5f;
    modes[3].amplitude = 0.3f;
    
    // Mode decay rates
    float baseDecay = 2.0f + algorithmParam * 8.0f;
    modes[0].decay = baseDecay;
    modes[1].decay = baseDecay * 1.3f;
    modes[2].decay = baseDecay * 1.8f;
    modes[3].decay = baseDecay * 2.5f;
    
    // Reset phases
    for (int i = 0; i < NUM_MODES; i++) {
        modes[i].phase = 0.0f;
    }
}