#ifndef PT8211_DAC_H
#define PT8211_DAC_H

#include <cstdint>
#include <random>

/**
 * PT8211 DAC Simulator
 * 
 * Simulates the characteristics of the PT8211 16-bit DAC:
 * - 16-bit resolution with R-2R ladder quantization
 * - 0.08% THD at 1kHz
 * - 89-93dB SNR
 * - 2.5V maximum output
 * - Specific frequency response characteristics
 */
class PT8211DAC {
public:
    PT8211DAC();
    ~PT8211DAC();
    
    void setSampleRate(int sampleRate);
    
    // Process a sample through the DAC simulation
    float processSample(float inputSample);
    
    // Configuration
    void setTHD(float thd) { targetTHD = thd; }
    void setSNR(float snr) { targetSNR = snr; }
    void setMaxOutput(float maxV) { maxOutputVoltage = maxV; }
    
    // Statistics
    float getCurrentTHD() const { return currentTHD; }
    float getCurrentSNR() const { return currentSNR; }
    
private:
    // DAC specifications
    static constexpr int BIT_DEPTH = 16;
    static constexpr int MAX_DIGITAL_VALUE = (1 << BIT_DEPTH) - 1;
    static constexpr float DEFAULT_THD = 0.0008f;  // 0.08%
    static constexpr float DEFAULT_SNR = 91.0f;    // 89-93dB (mid-range)
    static constexpr float DEFAULT_MAX_OUTPUT = 2.5f; // 2.5V
    
    // Current settings
    int sampleRate;
    float targetTHD;
    float targetSNR;
    float maxOutputVoltage;
    
    // State variables
    float currentTHD;
    float currentSNR;
    
    // Noise generation
    std::mt19937 noiseGenerator;
    std::uniform_real_distribution<float> noiseDistribution;
    
    // THD simulation (harmonic distortion)
    float harmonicBuffer[5];  // Store last 5 samples for harmonic generation
    int harmonicIndex;
    
    // R-2R ladder quantization simulation
    float quantizationNoise;
    float lastQuantizedSample;
    
    // Internal processing functions
    float applyQuantization(float sample);
    float addHarmonicDistortion(float sample);
    float addThermalNoise(float sample);
    float applyFrequencyResponse(float sample);
    
    // Statistics calculation
    void updateStatistics(float input, float output);
    
    // Running statistics
    float inputRMS;
    float outputRMS;
    float distortionRMS;
    float noiseRMS;
    int statisticsCounter;
    static constexpr int STATS_UPDATE_INTERVAL = 1024;
};

#endif // PT8211_DAC_H