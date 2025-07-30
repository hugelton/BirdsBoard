#include "pt8211_dac.h"
#include <cmath>
#include <algorithm>

PT8211DAC::PT8211DAC() 
    : sampleRate(44100)
    , targetTHD(DEFAULT_THD)
    , targetSNR(DEFAULT_SNR)
    , maxOutputVoltage(DEFAULT_MAX_OUTPUT)
    , currentTHD(0.0f)
    , currentSNR(0.0f)
    , noiseGenerator(std::random_device{}())
    , noiseDistribution(-1.0f, 1.0f)
    , harmonicIndex(0)
    , quantizationNoise(0.0f)
    , lastQuantizedSample(0.0f)
    , inputRMS(0.0f)
    , outputRMS(0.0f)
    , distortionRMS(0.0f)
    , noiseRMS(0.0f)
    , statisticsCounter(0)
{
    // Initialize harmonic buffer
    for (int i = 0; i < 5; i++) {
        harmonicBuffer[i] = 0.0f;
    }
}

PT8211DAC::~PT8211DAC() {
}

void PT8211DAC::setSampleRate(int sr) {
    sampleRate = sr;
}

float PT8211DAC::processSample(float inputSample) {
    // Store original input for statistics
    float originalInput = inputSample;
    
    // Step 1: Apply frequency response (minimal for PT8211 in audio range)
    float processedSample = applyFrequencyResponse(inputSample);
    
    // Step 2: Apply quantization (16-bit R-2R ladder)
    processedSample = applyQuantization(processedSample);
    
    // Step 3: Add harmonic distortion (THD simulation)
    processedSample = addHarmonicDistortion(processedSample);
    
    // Step 4: Add thermal noise (SNR simulation)
    processedSample = addThermalNoise(processedSample);
    
    // Step 5: Apply output voltage scaling (2.5V max)
    processedSample = processedSample * (maxOutputVoltage / 2.5f);
    
    // Update statistics
    updateStatistics(originalInput, processedSample);
    
    return processedSample;
}

float PT8211DAC::applyQuantization(float sample) {
    // Clamp to valid range
    sample = std::max(-1.0f, std::min(sample, 1.0f));
    
    // Convert to 16-bit integer domain
    int16_t quantized = (int16_t)(sample * MAX_DIGITAL_VALUE);
    
    // Add R-2R ladder quantization noise
    // R-2R ladders have characteristic step-like quantization with slight non-linearity
    float quantizationError = (float)quantized / MAX_DIGITAL_VALUE - sample;
    
    // R-2R ladder introduces correlated quantization noise
    quantizationNoise = quantizationNoise * 0.95f + quantizationError * 0.05f;
    
    // Convert back to float with quantization applied
    float quantizedSample = (float)quantized / MAX_DIGITAL_VALUE;
    
    // Add small amount of quantization noise correlation
    quantizedSample += quantizationNoise * 0.1f;
    
    lastQuantizedSample = quantizedSample;
    return quantizedSample;
}

float PT8211DAC::addHarmonicDistortion(float sample) {
    // Store sample in harmonic buffer
    harmonicBuffer[harmonicIndex] = sample;
    harmonicIndex = (harmonicIndex + 1) % 5;
    
    // Calculate harmonic content for THD
    float fundamental = harmonicBuffer[0];  // Current sample as fundamental
    float distortion = 0.0f;
    
    if (std::abs(fundamental) > 0.01f) {  // Only add distortion for significant signals
        // Second harmonic (most prominent in PT8211)
        float secondHarmonic = fundamental * fundamental * 0.5f;
        distortion += secondHarmonic * targetTHD * 2.0f;
        
        // Third harmonic (less prominent)
        float thirdHarmonic = fundamental * fundamental * fundamental;
        distortion += thirdHarmonic * targetTHD * 0.5f;
        
        // Higher order harmonics (very small)
        float higherHarmonics = std::sin(fundamental * 4.0f * M_PI) * targetTHD * 0.1f;
        distortion += higherHarmonics;
    }
    
    return sample + distortion;
}

float PT8211DAC::addThermalNoise(float sample) {
    // Calculate noise level based on target SNR
    float signalLevel = std::abs(sample);
    float noiseLevel = signalLevel / std::pow(10.0f, targetSNR / 20.0f);
    
    // Generate thermal noise (Gaussian-like distribution)
    float thermalNoise = noiseDistribution(noiseGenerator) * noiseLevel;
    
    // Add some 1/f noise characteristic of analog circuits
    static float oneFNoise = 0.0f;
    oneFNoise = oneFNoise * 0.999f + thermalNoise * 0.001f;
    
    // Combine thermal and 1/f noise
    float totalNoise = thermalNoise * 0.8f + oneFNoise * 0.2f;
    
    return sample + totalNoise;
}

float PT8211DAC::applyFrequencyResponse(float sample) {
    // PT8211 has relatively flat frequency response in audio range
    // Apply minimal high-frequency rolloff characteristic of R-2R DACs
    
    // Simple first-order lowpass at ~20kHz (well above audio range)
    static float lastOutput = 0.0f;
    float cutoffFreq = 20000.0f;  // 20kHz
    float alpha = 1.0f / (1.0f + 2.0f * M_PI * cutoffFreq / sampleRate);
    
    float output = alpha * sample + (1.0f - alpha) * lastOutput;
    lastOutput = output;
    
    return output;
}

void PT8211DAC::updateStatistics(float input, float output) {
    // Update running RMS values
    inputRMS = inputRMS * 0.999f + (input * input) * 0.001f;
    outputRMS = outputRMS * 0.999f + (output * output) * 0.001f;
    
    // Calculate distortion (difference between input and output)
    float distortion = output - input;
    distortionRMS = distortionRMS * 0.999f + (distortion * distortion) * 0.001f;
    
    // Update statistics every so often
    statisticsCounter++;
    if (statisticsCounter >= STATS_UPDATE_INTERVAL) {
        statisticsCounter = 0;
        
        // Calculate current THD
        if (inputRMS > 0.0001f) {
            currentTHD = std::sqrt(distortionRMS / inputRMS);
        } else {
            currentTHD = 0.0f;
        }
        
        // Calculate current SNR
        if (outputRMS > 0.0001f) {
            float noiseLevel = std::sqrt(distortionRMS);
            float signalLevel = std::sqrt(outputRMS);
            currentSNR = 20.0f * std::log10(signalLevel / noiseLevel);
        } else {
            currentSNR = 0.0f;
        }
        
        // Clamp to reasonable values
        currentTHD = std::max(0.0f, std::min(currentTHD, 1.0f));
        currentSNR = std::max(0.0f, std::min(currentSNR, 120.0f));
    }
}