/*
 * Tockus - Versatile Drum Voice
 * Copyright (C) 2025 Leo Kuroshita
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <I2S.h>
#include <EEPROM.h>
#include <FastLED.h>
#include <pico/multicore.h>

// PT8211S I2S pins
#define I2S_BCLK  6   // Bit clock
#define I2S_DOUT  8   // Data output

// ADC pins for control
#define PITCH_CV   26  // ADC0 - Main frequency control
#define PITCH_KNOB 27  // ADC1 - Fine frequency control
#define CV1        28  // ADC2 - Algorithm selection
#define CV2        29  // ADC3 - Algorithm parameter

// Gate input (trigger)
#define GATE_IN    2   // GPIO2 - Gate input for drum triggers

// FastLED for algorithm indication
#define LED_PIN 16
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

// ADC range calibration (from CLAUDE.md)
#define PITCH_CV_MIN    0
#define PITCH_CV_MAX    4095
#define PITCH_KNOB_MIN  10
#define PITCH_KNOB_MAX  4000
#define CV1_MIN         8
#define CV1_MAX         2000
#define CV2_MIN         8
#define CV2_MAX         2000

// Create I2S instance for PT8211S
I2S i2s(OUTPUT, I2S_BCLK, I2S_DOUT);

// Audio parameters
const int sampleRate = 44100;
const int amplitude = 32767;
const float MASTER_GAIN = 2.0;  // 2x gain boost (reduced to prevent clipping)
bool gateState = false;
bool lastGateState = false;
bool triggerActive = false;
uint32_t triggerStartTime = 0;

// Real-time frequency tracking
float currentFrequency = 60.0;     // Current real-time frequency
float baseFrequency = 60.0;        // Base frequency at trigger time

// Drum algorithms
enum DrumAlgorithm {
  ALGO_BASS = 0,      // 808 Bass drum
  ALGO_SNARE = 1,     // 808 Snare drum
  ALGO_HIHAT = 2,     // 808 Hi-hat
  ALGO_KARPLUS = 3,   // Karplus-Strong
  ALGO_MODAL = 4,     // Modal synthesis
  ALGO_ZAP = 5,       // ZAP sound
  ALGO_CLAP = 6,      // 808 Clap
  ALGO_COWBELL = 7,   // Cowbell (two sine oscillators)
  NUM_ALGORITHMS = 8
};

uint8_t currentAlgorithm = ALGO_BASS;
float frequency = 60.0;           // Base frequency
float algorithmParam = 0.5;       // Algorithm-specific parameter
float decayTime = 1.0;           // Decay time multiplier

// Envelope parameters
float envAmplitude = 0.0;
float envFrequency = 0.0;
float envDecayRate = 0.0;

// Phase accumulator
float phase = 0.0;
uint32_t sampleCount = 0;

// Noise generator state
uint32_t noiseState = 1;

// Snare drum parameters
float snareNoiseAmp = 0.0;
float snareToneAmp = 0.0;

// Hi-hat parameters
float hihatEnvelope = 0.0;

// Bandpass filter state variables
struct BandpassFilter {
  float x1, x2;  // Input delay line
  float y1, y2;  // Output delay line
  float a0, a1, a2, b1, b2;  // Filter coefficients
};

BandpassFilter bpf;

// Self-oscillating filter for bass drum (2-pole resonant filter)
struct ResonantFilter {
  float x1, x2;  // Input delay line
  float y1, y2;  // Output delay line
  float cutoff;   // Current cutoff frequency
  float resonance; // Q factor
  float a0, a1, a2, b1, b2;  // Filter coefficients
};

ResonantFilter bassFilter;

// Karplus-Strong delay line
#define KARPLUS_BUFFER_SIZE 200
float karplusBuffer[KARPLUS_BUFFER_SIZE];
int karplusIndex = 0;
float karplusDamping = 0.99;

// Modal synthesis parameters
#define NUM_MODES 4
struct Mode {
  float frequency;
  float amplitude;
  float decay;
  float phase;
};

Mode modes[NUM_MODES];

// Clap parameters
float clapPulseEnv = 0.0;
float clapReverbEnv = 0.0;

// Self-oscillating bass filter parameters
float bassFilterCutoff = 80.0;
float bassFilterEnv = 0.0;
float bassImpulse = 0.0;

// Cowbell oscillator phases
float cowbellPhases[4] = {0.0, 0.0, 0.0, 0.0};
const float cowbellFreqs[4] = {555.0, 835.0, 1370.0, 1940.0};  // Authentic 808 cowbell frequencies

// Anti-aliasing lowpass filter
float lastSample = 0.0;
const float LOWPASS_ALPHA = 0.7;  // Cutoff around 6kHz

// ADC filtering
struct ADCFilter {
  float filtered;
  float alpha;
  uint16_t lastRaw;
  uint16_t deadband;
  uint32_t lastUpdate;
  float baseAlpha;     // Base alpha value for this channel
};

ADCFilter adcFilters[4];
// Different filtering for each channel (K102E-inspired values)
const float FILTER_ALPHA_PITCH = 0.6;   // PITCH_CV - moderate filtering for stability
const float FILTER_ALPHA_KNOB = 0.6;    // PITCH_KNOB - same as pitch CV
const float FILTER_ALPHA_CV1 = 0.3;     // CV1 - slower for algorithm switching
const float FILTER_ALPHA_CV2 = 0.6;     // CV2 - same responsiveness

const uint16_t DEADBAND_THRESHOLD = 1;   // +/-1 noise only
const uint32_t RATE_LIMIT_MS = 1;       // 1ms like K102E

void setup() {
  // No serial debug output for optimized performance
  
  // Initialize ADC
  analogReadResolution(12);
  
  // Initialize Gate input
  pinMode(GATE_IN, INPUT);
  
  // Initialize I2S
  i2s.setBitsPerSample(16);
  i2s.setLSBJFormat();
  
  if (!i2s.begin(sampleRate)) {
    while (1);  // Halt if I2S fails
  }
  
  // Initialize ADC filters
  initializeADCFilters();
  
  // Initialize bandpass filter
  initializeBandpassFilter();
  
  // Initialize resonant filter for bass
  initializeResonantFilter();
  
  // Initialize Karplus-Strong buffer
  initializeKarplusStrong();
  
  // Initialize modal synthesis
  initializeModalSynthesis();
  
  // Initialize FastLED
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
  
  // Start Core1 for LED control
  multicore_launch_core1(core1Task);
}

void loop() {
  // Read parameters every 4 samples for ultra-responsive pitch control
  if (sampleCount % 4 == 0) {
    updateParameters();
    
    // Update real-time frequency for active sounds every sample cycle
    if (triggerActive) {
      updateRealtimeFrequency();
    }
  }
  
  // Generate audio sample based on current algorithm
  int16_t sample = 0;
  
  if (triggerActive) {
    float timeElapsed = (millis() - triggerStartTime) / 1000.0;
    sample = generateDrumSample(timeElapsed);
    
    // Check if envelope has decayed enough to stop
    if (envAmplitude < 0.001) {
      triggerActive = false;
    }
  }
  
  // Output to I2S (stereo)
  i2s.write(sample);
  i2s.write(sample);
  
  sampleCount++;
}

void updateParameters() {
  // Read and filter CV inputs
  uint16_t rawValues[4] = {
    (uint16_t)analogRead(PITCH_CV),
    (uint16_t)analogRead(PITCH_KNOB),
    (uint16_t)analogRead(CV1),
    (uint16_t)analogRead(CV2)
  };
  
  filterADCValues(rawValues);
  
  uint16_t pitchCV = (uint16_t)adcFilters[0].filtered;
  uint16_t pitchKnob = (uint16_t)adcFilters[1].filtered;
  uint16_t cv1 = (uint16_t)adcFilters[2].filtered;
  uint16_t cv2 = (uint16_t)adcFilters[3].filtered;
  
  // Gate handling - trigger on rising edge
  gateState = digitalRead(GATE_IN);
  if (gateState && !lastGateState) {
    triggerDrum();
  }
  lastGateState = gateState;
  
  // Calculate frequency with calibrated ranges (same as Wren)
  float adcVoltage = ((PITCH_CV_MAX - pitchCV) / (float)PITCH_CV_MAX) * 3.3;
  float knobVoltage = map(pitchKnob, PITCH_KNOB_MIN, PITCH_KNOB_MAX, 0, 3300) / 1000.0;
  knobVoltage = constrain(knobVoltage, 0.0, 3.3);
  
  float actualCV = (adcVoltage - 1.65) / 0.33;
  actualCV = constrain(actualCV, 0.0, 5.0);
  
  float cvOctaves = actualCV;
  float knobOctaves = (knobVoltage - 1.65) / 1.65;
  
  // Calculate base frequency (same as Wren)
  float baseFreq = 440.0 * pow(2.0, cvOctaves + knobOctaves - 4.0);
  
  // Apply algorithm-specific frequency scaling and range
  frequency = applyAlgorithmFrequencyScaling(baseFreq, currentAlgorithm);
  
  // CV1: Algorithm selection
  uint8_t newAlgorithm = map(cv1, CV1_MIN, CV1_MAX, 0, NUM_ALGORITHMS - 1);
  newAlgorithm = constrain(newAlgorithm, 0, NUM_ALGORITHMS - 1);
  currentAlgorithm = newAlgorithm;
  
  // CV2: Algorithm parameter
  algorithmParam = map(cv2, CV2_MIN, CV2_MAX, 0, 1000) / 1000.0;
  algorithmParam = constrain(algorithmParam, 0.0, 1.0);
  
  // Debug output removed for performance optimization
}

void triggerDrum() {
  triggerActive = true;
  triggerStartTime = millis();
  phase = 0.0;
  
  // Store base frequency at trigger time
  baseFrequency = frequency;
  currentFrequency = frequency;
  
  // Initialize envelopes based on algorithm
  initializeEnvelopes();
  
  // Reset noise state
  noiseState = 1;
}

void initializeEnvelopes() {
  envAmplitude = 1.0;
  envFrequency = currentFrequency;
  
  // Set decay rates based on algorithm
  switch (currentAlgorithm) {
    case ALGO_BASS:
      // CV2 controls sustain/decay balance
      envDecayRate = 1.5 + algorithmParam * 3.5;  // 1.5-5 Hz decay (longer for bass)
      bassFilterEnv = 1.0;
      break;
    case ALGO_ZAP:
      // CV2 controls decay speed (faster = more aggressive)
      envDecayRate = 8.0 + algorithmParam * 12.0; // 8-20 Hz decay
      break;
    case ALGO_SNARE:
      // CV2 controls decay time (0.5x to 3.0x speed)
      envDecayRate = 8.0 * (0.5 + algorithmParam * 2.5);  // 4-28 Hz decay
      snareNoiseAmp = 1.0;
      snareToneAmp = 1.0;
      break;
    case ALGO_HIHAT:
      // CV2 controls decay time (0.5x to 4.0x speed)
      envDecayRate = 20.0 * (0.5 + algorithmParam * 3.5);  // 10-90 Hz decay
      hihatEnvelope = 1.0;
      break;
    case ALGO_KARPLUS:
      envDecayRate = 3.0 + algorithmParam * 5.0;  // 3-8 Hz decay
      karplusDamping = 0.995 - algorithmParam * 0.2;  // 0.995-0.795 damping
      initializeKarplusStrong();
      break;
    case ALGO_MODAL:
      envDecayRate = 4.0 + algorithmParam * 6.0;  // 4-10 Hz decay
      setupModalModes();
      break;
    case ALGO_CLAP:
      // CV2 controls decay time (0.5x to 3.0x speed)
      envDecayRate = 12.0 * (0.5 + algorithmParam * 2.5);  // 6-42 Hz decay
      clapPulseEnv = 1.0;
      clapReverbEnv = 1.0;
      break;
    case ALGO_COWBELL:
      // CV2 controls metallic resonance (affects filtering)
      envDecayRate = 4.0 + algorithmParam * 6.0;  // 4-10 Hz decay
      // Reset cowbell oscillator phases
      for (int i = 0; i < 4; i++) {
        cowbellPhases[i] = 0.0;
      }
      break;
    default:
      envDecayRate = 5.0 + algorithmParam * 5.0;  // 5-10 Hz decay
      break;
  }
}

int16_t generateDrumSample(float timeElapsed) {
  float sample = 0.0;
  
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
      sample = generateBassDrum(timeElapsed); // Default to bass drum
      break;
  }
  
  // Apply anti-aliasing lowpass filter
  sample = LOWPASS_ALPHA * sample + (1.0 - LOWPASS_ALPHA) * lastSample;
  lastSample = sample;
  
  // Apply master gain with soft clipping
  float boostedSample = sample * MASTER_GAIN;
  
  // Soft saturation instead of hard clipping
  if (boostedSample > 0.8) {
    boostedSample = 0.8 + 0.2 * tanh((boostedSample - 0.8) * 5.0);
  } else if (boostedSample < -0.8) {
    boostedSample = -0.8 + 0.2 * tanh((boostedSample + 0.8) * 5.0);
  }
  
  return (int16_t)(boostedSample * amplitude);
}

void updateEnvelopes(float timeElapsed) {
  // Exponential decay for amplitude
  envAmplitude = exp(-envDecayRate * timeElapsed);
  
  // Pitch envelope handling - BASS and ZAP have pitch envelopes
  if (currentAlgorithm == ALGO_BASS || currentAlgorithm == ALGO_ZAP) {
    float pitchDecay = (currentAlgorithm == ALGO_ZAP) ? 15.0 : 5.0;
    // Use real-time current frequency as base for pitch envelope
    envFrequency = currentFrequency * (1.0 + 2.0 * exp(-pitchDecay * timeElapsed));
  } else {
    // For all other algorithms, use current frequency directly
    envFrequency = currentFrequency;
  }
  
  // Snare-specific envelope updates
  if (currentAlgorithm == ALGO_SNARE) {
    // Noise envelope decays faster than tone
    snareNoiseAmp = exp(-(envDecayRate * 1.5) * timeElapsed);
    snareToneAmp = exp(-envDecayRate * timeElapsed);
  }
  
  // Hi-hat envelope (very fast decay)
  if (currentAlgorithm == ALGO_HIHAT) {
    hihatEnvelope = exp(-envDecayRate * timeElapsed);
  }
  
  // Clap envelope (pulse + reverb) - CV2 controls decay time
  if (currentAlgorithm == ALGO_CLAP) {
    // Multiple short pulses (4 pulses, 30ms apart)
    clapPulseEnv = 0.0;
    for (int i = 0; i < 4; i++) {
      float pulseTime = timeElapsed - i * 0.03;  // 30ms spacing
      if (pulseTime >= 0.0 && pulseTime <= 0.01) {  // 10ms pulse width
        clapPulseEnv += exp(-50.0 * pulseTime);  // Fast decay for each pulse
      }
    }
    
    // CV2 controls reverb decay time (0.5x to 2.0x speed)
    float decayMultiplier = 0.5 + algorithmParam * 1.5;
    clapReverbEnv = exp(-envDecayRate * decayMultiplier * timeElapsed);
  }
}

float generateBassDrum(float timeElapsed) {
  // Authentic 808-style bass drum: self-oscillating resonant filter approach
  // The 808 kick uses a filter set close to self-oscillation, excited by an impulse
  
  // Generate impulse at the start (first few samples)
  if (timeElapsed < 0.002) {  // 2ms impulse
    bassImpulse = 1.0 - (timeElapsed / 0.002);
  } else {
    bassImpulse = 0.0;
  }
  
  // Filter cutoff envelope: starts high, drops to bass frequency
  float cutoffEnv = exp(-8.0 * timeElapsed);  // Fast decay
  bassFilterCutoff = envFrequency + (envFrequency * 3.0 * cutoffEnv);  // 1x to 4x frequency range
  
  // High resonance for self-oscillation (Q factor)
  float resonance = 8.0 + algorithmParam * 12.0;  // Q: 8-20
  
  // Update filter coefficients
  updateResonantFilter(&bassFilter, bassFilterCutoff, resonance);
  
  // Process impulse through resonant filter
  float output = processResonantFilter(&bassFilter, bassImpulse);
  
  // Apply amplitude envelope
  output *= envAmplitude;
  
  return output * 0.8;  // Scale for headroom
}

float generateZapSound(float timeElapsed) {
  // Classic ZAP sound: dramatic pitch sweep from high to low + noise burst
  // More dramatic than original implementation
  
  // Dramatic pitch envelope: starts very high, drops rapidly
  float pitchEnv = exp(-20.0 * timeElapsed);  // Very fast drop
  float startMultiplier = 8.0 + algorithmParam * 12.0;  // 8x to 20x starting frequency
  float zapFreq = currentFrequency * (1.0 + startMultiplier * pitchEnv);
  
  // Main ZAP oscillator (sawtooth for more aggressive sound)
  float phase = fmod(zapFreq * timeElapsed, 1.0);
  float sawtooth = 2.0 * phase - 1.0;  // -1 to +1 sawtooth
  
  float sample = sawtooth * envAmplitude * 0.5;
  
  // Add noise burst at the beginning for extra punch
  if (timeElapsed < 0.05) {  // 50ms noise burst
    float noiseBurst = generateWhiteNoise() * (1.0 - timeElapsed / 0.05) * 0.3;
    sample += noiseBurst;
  }
  
  // Add harmonics based on parameter (more aggressive with higher CV2)
  if (algorithmParam > 0.1) {
    float harmonicLevel = algorithmParam * 0.4;
    float harmonic = sin(2.0 * PI * zapFreq * 2.0 * timeElapsed) * envAmplitude * harmonicLevel;
    sample += harmonic;
  }
  
  return sample * 0.7;  // Scale for headroom
}

float generateSnareDrum(float timeElapsed) {
  // Classic 909/808-style snare: tone with pitch envelope + filtered noise
  
  // Tone component with pitch envelope (starts high, drops quickly)
  float pitchEnv = exp(-25.0 * timeElapsed);  // Very fast pitch drop
  float toneFreq = envFrequency * (1.0 + 2.0 * pitchEnv);  // 1x to 3x frequency
  
  // Main tone oscillator
  float tone = sin(2.0 * PI * toneFreq * timeElapsed) * snareToneAmp;
  
  // Noise component (for snare rattle)
  float noise = generateWhiteNoise() * snareNoiseAmp;
  
  // Bandpass filter the noise (classic snare frequency range)
  setBandpassFilter(800.0 + algorithmParam * 1200.0, 2.0);  // 800-2000Hz
  float filteredNoise = processBandpassFilter(noise);
  
  // Mix tone and noise (adjustable balance)
  float toneMix = 0.6;  // 60% tone
  float noiseMix = 0.4; // 40% noise
  
  float result = (tone * toneMix + filteredNoise * noiseMix) * 0.7;
  
  return result;
}

float generateWhiteNoise() {
  // Improved linear congruential generator for white noise
  noiseState = noiseState * 1103515245 + 12345;
  // Better scaling to prevent harsh edges
  return ((noiseState >> 16) & 0x7FFF) / 32768.0f - 1.0f;
}

float generateHiHat(float timeElapsed) {
  // 808-style hi-hat: multiple square waves + noise through bandpass filter
  float square1 = (sin(2.0 * PI * envFrequency * 2.1 * timeElapsed) > 0) ? 1.0 : -1.0;
  float square2 = (sin(2.0 * PI * envFrequency * 3.3 * timeElapsed) > 0) ? 1.0 : -1.0;
  float square3 = (sin(2.0 * PI * envFrequency * 4.7 * timeElapsed) > 0) ? 1.0 : -1.0;
  float square4 = (sin(2.0 * PI * envFrequency * 6.1 * timeElapsed) > 0) ? 1.0 : -1.0;
  
  float squareSum = (square1 + square2 * 0.8 + square3 * 0.6 + square4 * 0.4) * 0.25;
  
  // Add noise component
  float noise = generateWhiteNoise() * 0.8;
  
  // Mix squares and noise
  float rawSignal = squareSum + noise;
  
  // Fixed bandpass filter (classic hi-hat frequency)
  float centerFreq = 10000.0;  // Fixed 10kHz center
  float Q = 3.0;  // Fixed Q
  
  setBandpassFilter(centerFreq, Q);
  float filtered = processBandpassFilter(rawSignal);
  
  return filtered * hihatEnvelope * 1.5;  // Increased volume (was no multiplier)
}

float generateKarplusStrong(float timeElapsed) {
  // Karplus-Strong algorithm: delay line with feedback and damping
  float output = karplusBuffer[karplusIndex];
  
  // Calculate next buffer position
  int nextIndex = (karplusIndex + 1) % KARPLUS_BUFFER_SIZE;
  
  // Low-pass filter with damping (average of current and next sample)
  float filteredSample = (karplusBuffer[karplusIndex] + karplusBuffer[nextIndex]) * 0.5;
  karplusBuffer[karplusIndex] = filteredSample * karplusDamping;
  
  // Update index
  karplusIndex = nextIndex;
  
  return output * envAmplitude;
}

float generateModalSynthesis(float timeElapsed) {
  // Modal synthesis: sum of multiple decaying sine waves
  float output = 0.0;
  
  for (int i = 0; i < NUM_MODES; i++) {
    float modeOutput = sin(modes[i].phase) * modes[i].amplitude * 
                       exp(-modes[i].decay * timeElapsed);
    output += modeOutput;
    
    // Update phase
    modes[i].phase += 2.0 * PI * modes[i].frequency / sampleRate;
    if (modes[i].phase >= 2.0 * PI) {
      modes[i].phase -= 2.0 * PI;
    }
  }
  
  // Normalize and apply envelope - reduced amplitude to prevent clipping
  output = output * envAmplitude * 0.25;  // Further reduced from 0.5
  return output;  // Remove hard clipping constraint
}

float generateClap(float timeElapsed) {
  // 808-style clap: bandpass filtered noise with multi-pulse envelope
  float noise = generateWhiteNoise() * 1.2;  // Increased noise level (was 0.8)
  
  // Fixed bandpass filter (centered around 1kHz) - no pitch dependency needed
  float centerFreq = 1000.0;
  float Q = 3.0;
  
  setBandpassFilter(centerFreq, Q);
  float filteredNoise = processBandpassFilter(noise);
  
  // Apply pulse envelope + reverb envelope (reverb decay controlled by CV2)
  float pulseComponent = filteredNoise * clapPulseEnv;
  float reverbComponent = filteredNoise * clapReverbEnv * 0.3;
  
  float result = (pulseComponent + reverbComponent) * 1.8;  // Further increased volume
  return result;  // Remove clipping constraint
}

float generateCowbell(float timeElapsed) {
  // Authentic 808 cowbell: 4 pulse oscillators at fixed frequencies
  // Original 808 used: 555Hz, 835Hz, 1.37kHz, 1.94kHz
  
  float output = 0.0;
  
  // Generate 4 pulse waves at authentic 808 frequencies
  for (int i = 0; i < 4; i++) {
    // Update phase for each oscillator
    cowbellPhases[i] += 2.0 * PI * cowbellFreqs[i] / sampleRate;
    if (cowbellPhases[i] >= 2.0 * PI) {
      cowbellPhases[i] -= 2.0 * PI;
    }
    
    // Generate pulse wave (50% duty cycle)
    float pulse = (sin(cowbellPhases[i]) > 0) ? 1.0 : -1.0;
    
    // Weight the oscillators (higher frequencies have less amplitude)
    float weight = 1.0 / (i + 1);  // 1.0, 0.5, 0.33, 0.25
    output += pulse * weight;
  }
  
  // Normalize and apply envelope
  output = output * 0.25 * envAmplitude;  // Scale down since we're adding 4 oscillators
  
  // CV2 controls metallic filtering
  float filterFreq = 2000.0 + algorithmParam * 3000.0;  // 2-5kHz
  setBandpassFilter(filterFreq, 4.0);  // High Q for metallic sound
  output = processBandpassFilter(output);
  
  return output * 0.8;
}

// Core1 Task for LED control
void core1Task() {
  CRGB colors[NUM_ALGORITHMS] = {
    CRGB::Red,       // ALGO_BASS
    CRGB::Green,     // ALGO_SNARE  
    CRGB::Blue,      // ALGO_HIHAT
    CRGB::Yellow,    // ALGO_KARPLUS
    CRGB::Magenta,   // ALGO_MODAL
    CRGB::Cyan,      // ALGO_ZAP
    CRGB::Orange,    // ALGO_CLAP
    CRGB::White      // ALGO_COWBELL
  };
  
  uint32_t lastUpdate = 0;
  
  while (true) {
    uint32_t now = millis();
    
    if (now - lastUpdate >= 100) {
      lastUpdate = now;
      
      FastLED.clear();
      
      // Show current algorithm color
      leds[0] = colors[currentAlgorithm];
      
      // Brighten during trigger
      if (triggerActive) {
        leds[0].fadeToBlackBy(64); // Dim to 75% to show activity
      }
      
      FastLED.show();
    }
    
    delay(5);
  }
}

// ADC filtering functions
void initializeADCFilters() {
  // Set individual alpha values for each channel
  float alphas[4] = {FILTER_ALPHA_PITCH, FILTER_ALPHA_KNOB, FILTER_ALPHA_CV1, FILTER_ALPHA_CV2};
  
  for (int i = 0; i < 4; i++) {
    adcFilters[i].filtered = 0;
    adcFilters[i].alpha = alphas[i];
    adcFilters[i].baseAlpha = alphas[i];
    adcFilters[i].lastRaw = 0;
    adcFilters[i].deadband = DEADBAND_THRESHOLD;
    adcFilters[i].lastUpdate = 0;
  }
}

void filterADCValues(uint16_t rawValues[4]) {
  uint32_t currentTime = millis();
  
  for (int i = 0; i < 4; i++) {
    if (currentTime - adcFilters[i].lastUpdate < RATE_LIMIT_MS) {
      continue;
    }
    
    // K102E-style change detection
    int changeMagnitude = abs((int)rawValues[i] - (int)adcFilters[i].lastRaw);
    
    // Ultra-sensitive deadband for pitch CV (channel 0)
    uint16_t threshold = (i == 0) ? 1 : 2;  // PITCH_CV: ±1, others: ±2
    
    // Only update if change is above threshold
    if (changeMagnitude >= threshold) {
      float baseAlpha = adcFilters[i].baseAlpha;
      
      // Apply stronger filtering for noise-heavy algorithms on CV2
      if (i == 3 && (currentAlgorithm == ALGO_SNARE || 
                     currentAlgorithm == ALGO_HIHAT || 
                     currentAlgorithm == ALGO_CLAP)) {
        baseAlpha = 0.1;  // Stronger filtering for noise-based algorithms
      }
      
      // Simple first-order filter (K102E style)
      if (adcFilters[i].filtered == 0) {
        adcFilters[i].filtered = rawValues[i];
      } else {
        adcFilters[i].filtered = baseAlpha * rawValues[i] + 
                                 (1.0 - baseAlpha) * adcFilters[i].filtered;
      }
      
      adcFilters[i].lastRaw = rawValues[i];
      adcFilters[i].lastUpdate = currentTime;
    }
  }
}

// Bandpass filter functions
void initializeBandpassFilter() {
  bpf.x1 = bpf.x2 = bpf.y1 = bpf.y2 = 0.0;
  setBandpassFilter(8000.0, 2.0);  // Default: 8kHz, Q=2
}

void setBandpassFilter(float centerFreq, float Q) {
  float w = 2.0 * PI * centerFreq / sampleRate;
  float alpha = sin(w) / (2.0 * Q);
  
  float norm = 1.0 / (1.0 + alpha);
  
  bpf.a0 = alpha * norm;
  bpf.a1 = 0.0;
  bpf.a2 = -alpha * norm;
  bpf.b1 = -2.0 * cos(w) * norm;
  bpf.b2 = (1.0 - alpha) * norm;
}

float processBandpassFilter(float input) {
  float output = bpf.a0 * input + bpf.a1 * bpf.x1 + bpf.a2 * bpf.x2
                 - bpf.b1 * bpf.y1 - bpf.b2 * bpf.y2;
  
  // Update delay lines
  bpf.x2 = bpf.x1;
  bpf.x1 = input;
  bpf.y2 = bpf.y1;
  bpf.y1 = output;
  
  return output;
}

// Initialize Karplus-Strong delay line
void initializeKarplusStrong() {
  // Fill buffer with noise burst
  for (int i = 0; i < KARPLUS_BUFFER_SIZE; i++) {
    karplusBuffer[i] = generateWhiteNoise() * 0.5;
  }
  karplusIndex = 0;
}

// Initialize modal synthesis
void initializeModalSynthesis() {
  // Set up basic modal parameters
  setupModalModes();
}

void setupModalModes() {
  // Configure modes based on base frequency and algorithm parameter
  float baseFreq = currentFrequency;
  
  // Mode frequencies (harmonic ratios typical for drums)
  modes[0].frequency = baseFreq * 1.0;          // Fundamental
  modes[1].frequency = baseFreq * 1.6;          // First overtone
  modes[2].frequency = baseFreq * 2.3;          // Second overtone  
  modes[3].frequency = baseFreq * 3.1;          // Third overtone
  
  // Mode amplitudes (decreasing with frequency)
  modes[0].amplitude = 1.0;
  modes[1].amplitude = 0.7;
  modes[2].amplitude = 0.5;
  modes[3].amplitude = 0.3;
  
  // Mode decay rates (higher frequencies decay faster)
  float baseDecay = 2.0 + algorithmParam * 8.0;  // 2-10 Hz base decay
  modes[0].decay = baseDecay;
  modes[1].decay = baseDecay * 1.3;
  modes[2].decay = baseDecay * 1.8;
  modes[3].decay = baseDecay * 2.5;
  
  // Reset phases
  for (int i = 0; i < NUM_MODES; i++) {
    modes[i].phase = 0.0;
  }
}

// Update real-time frequency during playback
void updateRealtimeFrequency() {
  // Always apply algorithm-specific scaling to current frequency
  currentFrequency = applyAlgorithmFrequencyScaling(frequency, currentAlgorithm);
  
  // For most algorithms, update envelope frequency immediately
  // Exception: BASS and ZAP use pitch envelopes that modify this base frequency
  if (currentAlgorithm != ALGO_BASS && currentAlgorithm != ALGO_ZAP) {
    envFrequency = currentFrequency;
  }
  
  // Algorithm-specific real-time updates
  if (currentAlgorithm == ALGO_MODAL) {
    // Update modal frequencies in real-time
    modes[0].frequency = currentFrequency * 1.0;
    modes[1].frequency = currentFrequency * 1.6;
    modes[2].frequency = currentFrequency * 2.3;
    modes[3].frequency = currentFrequency * 3.1;
  }
  
  // For Karplus-Strong, adjust delay line length for new frequency
  if (currentAlgorithm == ALGO_KARPLUS) {
    // Recalculate buffer index for new frequency
    float newDelay = sampleRate / currentFrequency;
    if (newDelay > 0 && newDelay < KARPLUS_BUFFER_SIZE) {
      // Smooth transition to new delay length
      karplusIndex = (int)(newDelay * 0.8);  // 80% of calculated delay
    }
  }
}

// Apply algorithm-specific frequency scaling
float applyAlgorithmFrequencyScaling(float baseFreq, uint8_t algorithm) {
  float scaledFreq = baseFreq;
  
  switch (algorithm) {
    case ALGO_BASS:
      // Bass drum: 20-150Hz range, shifted down 2 octaves
      scaledFreq = baseFreq / 4.0;  // -2 octaves
      scaledFreq = constrain(scaledFreq, 20.0, 150.0);
      break;
      
    case ALGO_SNARE:
      // Snare: 100-400Hz range, shifted down 1 octave
      scaledFreq = baseFreq / 2.0;  // -1 octave
      scaledFreq = constrain(scaledFreq, 100.0, 400.0);
      break;
      
    case ALGO_HIHAT:
      // Hi-hat: 200-2000Hz range, no shift
      scaledFreq = baseFreq;
      scaledFreq = constrain(scaledFreq, 200.0, 2000.0);
      break;
      
    case ALGO_KARPLUS:
      // Karplus-Strong: 80-800Hz range, shifted down 1 octave
      scaledFreq = baseFreq / 2.0;  // -1 octave
      scaledFreq = constrain(scaledFreq, 80.0, 800.0);
      break;
      
    case ALGO_MODAL:
      // Modal synthesis: 240-2400Hz range, raised by 2 octaves
      scaledFreq = baseFreq * 4.0;  // +2 octaves
      scaledFreq = constrain(scaledFreq, 240.0, 2400.0);
      break;
      
    case ALGO_ZAP:
      // ZAP: 50-500Hz range, shifted down 1.5 octaves
      scaledFreq = baseFreq / 2.8;  // -1.5 octaves
      scaledFreq = constrain(scaledFreq, 50.0, 500.0);
      break;
      
    case ALGO_CLAP:
      // Clap: 150-1500Hz range, no shift (noise-based)
      scaledFreq = baseFreq;
      scaledFreq = constrain(scaledFreq, 150.0, 1500.0);
      break;
      
    case ALGO_COWBELL:
      // Cowbell: 2000-8000Hz range, raised by 2 octaves
      scaledFreq = baseFreq * 4.0;  // +2 octaves
      scaledFreq = constrain(scaledFreq, 2000.0, 8000.0);
      break;
      
    default:
      // Default: full range
      scaledFreq = constrain(baseFreq, 20.0, 8000.0);
      break;
  }
  
  return scaledFreq;
}

// Resonant filter functions for self-oscillating bass
void initializeResonantFilter() {
  bassFilter.x1 = bassFilter.x2 = bassFilter.y1 = bassFilter.y2 = 0.0;
  bassFilter.cutoff = 80.0;
  bassFilter.resonance = 10.0;
  updateResonantFilter(&bassFilter, 80.0, 10.0);
}

void updateResonantFilter(ResonantFilter* filter, float cutoff, float resonance) {
  // Prevent filter instability
  cutoff = constrain(cutoff, 20.0, 8000.0);
  resonance = constrain(resonance, 0.5, 20.0);
  
  filter->cutoff = cutoff;
  filter->resonance = resonance;
  
  // Calculate filter coefficients for 2-pole resonant lowpass
  float w = 2.0 * PI * cutoff / sampleRate;
  float cosw = cos(w);
  float sinw = sin(w);
  float alpha = sinw / (2.0 * resonance);
  
  float norm = 1.0 / (1.0 + alpha);
  
  filter->a0 = (1.0 - cosw) * 0.5 * norm;
  filter->a1 = (1.0 - cosw) * norm;
  filter->a2 = (1.0 - cosw) * 0.5 * norm;
  filter->b1 = -2.0 * cosw * norm;
  filter->b2 = (1.0 - alpha) * norm;
}

float processResonantFilter(ResonantFilter* filter, float input) {
  float output = filter->a0 * input + filter->a1 * filter->x1 + filter->a2 * filter->x2
                 - filter->b1 * filter->y1 - filter->b2 * filter->y2;
  
  // Update delay lines
  filter->x2 = filter->x1;
  filter->x1 = input;
  filter->y2 = filter->y1;
  filter->y1 = output;
  
  return output;
}