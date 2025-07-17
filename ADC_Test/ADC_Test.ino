// ADC Test Application for BirdsBoard
// Tests and displays all ADC channel values and voltages
// Includes Wren-style FIR filtering for comparison

// ADC pins
#define PITCH_CV   26  // ADC0
#define PITCH_KNOB 27  // ADC1
#define CV1        28  // ADC2
#define CV2        29  // ADC3

// Gate input for reference
#define GATE_IN    2   // GPIO2

// ADC parameters
const float ADC_VREF = 3.3;      // Reference voltage
const int ADC_MAX_VALUE = 4095;  // 12-bit ADC (2^12 - 1)

// ADC range calibration (from CLAUDE.md/Wren)
#define PITCH_CV_MIN    0
#define PITCH_CV_MAX    4095
#define PITCH_KNOB_MIN  10
#define PITCH_KNOB_MAX  4000
#define CV1_MIN         8
#define CV1_MAX         2000
#define CV2_MIN         8
#define CV2_MAX         2000

// OpAmp scaling parameters (from BirdsBoard_Test)
// Formula: ADC_voltage = CV_input × 0.33 + 1.65
const float OPAMP_GAIN = 0.33;
const float OPAMP_OFFSET = 1.65;

// Averaging parameters
const int SAMPLES = 16;  // Number of samples to average
uint32_t adcSum[4] = {0, 0, 0, 0};
uint16_t sampleCount = 0;

// Simple but effective ADC Filter
struct ADCFilter {
  uint16_t filtered;   // Current filtered value (integer for stability)
  uint16_t lastOutput; // Last output value
  uint32_t accumulator; // Running sum for averaging
  uint8_t sampleCount; // Number of samples in accumulator
  uint16_t deadband;   // Deadband threshold
};

ADCFilter adcFilters[4];
const uint8_t AVG_SAMPLES = 16;  // Number of samples to average
const uint16_t DEADBAND_THRESHOLD = 3;  // Ignore changes smaller than this

void setup() {
  Serial.begin(115200);
  
  // Initialize ADC
  analogReadResolution(12);
  
  // Initialize Gate input
  pinMode(GATE_IN, INPUT);
  
  // Initialize ADC filters
  initializeADCFilters();
  
  // Wait for serial connection
  while (!Serial) {
    delay(100);
  }
  
  Serial.println("=== BirdsBoard ADC Test with Wren Filtering ===");
  Serial.println("Monitoring all ADC channels - RAW vs FILTERED comparison");
  Serial.println("Press any key to toggle continuous/single mode");
  Serial.println();
  
  // Print header
  printHeader();
}

void loop() {
  // Read all ADC channels (RAW values)
  uint16_t rawValues[4];
  rawValues[0] = analogRead(PITCH_CV);
  rawValues[1] = analogRead(PITCH_KNOB);
  rawValues[2] = analogRead(CV1);
  rawValues[3] = analogRead(CV2);
  
  // Apply Wren-style filtering
  filterADCValues(rawValues);
  
  // Get filtered values
  uint16_t filteredValues[4];
  for (int i = 0; i < 4; i++) {
    filteredValues[i] = (uint16_t)adcFilters[i].filtered;
  }
  
  // Read gate for reference
  bool gateState = digitalRead(GATE_IN);
  
  // Accumulate samples for averaging
  for (int i = 0; i < 4; i++) {
    adcSum[i] += rawValues[i];
  }
  sampleCount++;
  
  // Display results every 100ms (10Hz update rate)
  static uint32_t lastDisplay = 0;
  if (millis() - lastDisplay >= 100) {
    // Calculate averages
    uint16_t avgValues[4];
    for (int i = 0; i < 4; i++) {
      avgValues[i] = adcSum[i] / sampleCount;
      adcSum[i] = 0;  // Reset sum
    }
    sampleCount = 0;
    
    // Display results with both RAW and FILTERED values
    displayResults(rawValues, filteredValues, avgValues, gateState);
    lastDisplay = millis();
  }
  
  // Check for serial input
  if (Serial.available()) {
    char c = Serial.read();
    // Single reading mode
    Serial.println("\n=== Single Reading ===");
    printHeader();
    uint16_t singleFiltered[4];
    for (int i = 0; i < 4; i++) {
      singleFiltered[i] = (uint16_t)adcFilters[i].filtered;
    }
    displayResults(rawValues, singleFiltered, rawValues, gateState);
    Serial.println("\nPress any key for another reading...");
    
    // Wait for next keypress
    while (!Serial.available()) {
      delay(10);
    }
    Serial.read(); // Clear the input
    Serial.println("\nResuming continuous mode...");
    printHeader();
  }
  
  delay(1); // Small delay for stability
}

void printHeader() {
  Serial.println("Channel    | Pin | RAW ADC | Filtered | Averaged | ADC Volt | CV Input | Description");
  Serial.println("-----------|-----|---------|----------|----------|----------|----------|------------------");
}

void displayResults(uint16_t rawValues[], uint16_t filteredValues[], uint16_t avgValues[], bool gateState) {
  const char* channelNames[] = {"PITCH_CV ", "PITCH_KNOB", "CV1       ", "CV2       "};
  const char* descriptions[] = {"Pitch CV Input", "Pitch Knob", "Waveform/Tap Sel", "Wavefold/Reg Len"};
  const int pins[] = {26, 27, 28, 29};
  
  // Display ADC channels
  for (int i = 0; i < 4; i++) {
    // Use filtered values for voltage calculation
    float adcVoltage = (float)filteredValues[i] / ADC_MAX_VALUE * ADC_VREF;
    
    // Calculate estimated CV input (reverse OpAmp scaling)
    float cvInput = (adcVoltage - OPAMP_OFFSET) / OPAMP_GAIN;
    
    // Handle inverted scaling for PITCH_CV (from BirdsBoard_Test)
    if (i == 0) { // PITCH_CV
      adcVoltage = ((ADC_MAX_VALUE - filteredValues[i]) / (float)ADC_MAX_VALUE) * ADC_VREF;
      cvInput = (adcVoltage - OPAMP_OFFSET) / OPAMP_GAIN;
    }
    
    // Constrain CV input to reasonable range
    cvInput = constrain(cvInput, 0.0, 5.0);
    
    // Print formatted results
    Serial.print(channelNames[i]);
    Serial.print(" | ");
    Serial.print(pins[i]);
    Serial.print("  | ");
    
    // Raw ADC value (4 digits)
    Serial.print(rawValues[i]);
    if (rawValues[i] < 1000) Serial.print(" ");
    if (rawValues[i] < 100) Serial.print(" ");
    if (rawValues[i] < 10) Serial.print(" ");
    Serial.print(" | ");
    
    // Filtered ADC value (4 digits)
    Serial.print(filteredValues[i]);
    if (filteredValues[i] < 1000) Serial.print(" ");
    if (filteredValues[i] < 100) Serial.print(" ");
    if (filteredValues[i] < 10) Serial.print(" ");
    Serial.print(" | ");
    
    // Averaged ADC value (4 digits)
    Serial.print(avgValues[i]);
    if (avgValues[i] < 1000) Serial.print(" ");
    if (avgValues[i] < 100) Serial.print(" ");
    if (avgValues[i] < 10) Serial.print(" ");
    Serial.print(" | ");
    
    // ADC voltage (3 decimal places)
    Serial.print(adcVoltage, 3);
    Serial.print("V | ");
    
    // CV input estimation (2 decimal places)
    Serial.print(cvInput, 2);
    Serial.print("V   | ");
    
    // Description
    Serial.println(descriptions[i]);
  }
  
  // Display gate state
  Serial.print("GATE      | ");
  Serial.print("2   | ");
  Serial.print(gateState ? "HIGH" : "LOW ");
  Serial.print("    | ");
  Serial.print("    ---   | ");
  Serial.print("    ---   | ");
  Serial.print(gateState ? "3.3V" : "0.0V");
  Serial.print("   | ");
  Serial.print(gateState ? "5.0V" : "0.0V");
  Serial.print("   | Gate Input");
  Serial.println();
  
  // Separator line
  Serial.println("-----------|-----|---------|----------|----------|----------|----------|------------------");
  
  // Additional calculations using filtered values
  displayCalculations(filteredValues);
  
  // Show filter status
  displayFilterStatus();
}

void displayCalculations(uint16_t adcValues[]) {
  // Calculate frequency from pitch CV (like BirdsBoard_Test)
  float adcVoltage = ((ADC_MAX_VALUE - adcValues[0]) / (float)ADC_MAX_VALUE) * ADC_VREF;
  float knobVoltage = (adcValues[1] / (float)ADC_MAX_VALUE) * ADC_VREF;
  
  float actualCV = (adcVoltage - OPAMP_OFFSET) / OPAMP_GAIN;
  actualCV = constrain(actualCV, 0.0, 5.0);
  
  float cvOctaves = actualCV;
  float knobOctaves = (knobVoltage - OPAMP_OFFSET) / OPAMP_OFFSET;
  
  float frequency = 440.0 * pow(2.0, cvOctaves + knobOctaves - 4.0);
  frequency = constrain(frequency, 20.0, 8000.0);
  
  Serial.print("Calculated Frequency: ");
  Serial.print(frequency, 1);
  Serial.print("Hz");
  
  // Display CV1 and CV2 interpretations
  Serial.print(" | CV1: ");
  Serial.print(map(adcValues[2], 0, ADC_MAX_VALUE, 0, 100));
  Serial.print("%");
  
  Serial.print(" | CV2: ");
  Serial.print(map(adcValues[3], 0, ADC_MAX_VALUE, 0, 100));
  Serial.println("%");
  
  Serial.println();
}

// Wren-style ADC filter functions
void initializeADCFilters() {
  // Simple deadband per channel
  uint16_t deadbands[4] = {
    2,  // PITCH_CV: ±2 for tuning
    3,  // PITCH_KNOB: ±3 for knob
    4,  // CV1: ±4 for algorithm switching
    4   // CV2: ±4 for parameters
  };
  
  for (int i = 0; i < 4; i++) {
    adcFilters[i].filtered = 0;
    adcFilters[i].lastOutput = 0;
    adcFilters[i].accumulator = 0;
    adcFilters[i].sampleCount = 0;
    adcFilters[i].deadband = deadbands[i];
  }
}

void filterADCValues(uint16_t rawValues[4]) {
  for (int i = 0; i < 4; i++) {
    // Simple running average
    adcFilters[i].accumulator += rawValues[i];
    adcFilters[i].sampleCount++;
    
    // Calculate average when we have enough samples
    if (adcFilters[i].sampleCount >= AVG_SAMPLES) {
      uint16_t avgValue = adcFilters[i].accumulator / AVG_SAMPLES;
      
      // Apply deadband - only update if change is significant
      if (adcFilters[i].filtered == 0) {
        // Initialize
        adcFilters[i].filtered = avgValue;
        adcFilters[i].lastOutput = avgValue;
      } else {
        int change = abs((int)avgValue - (int)adcFilters[i].lastOutput);
        if (change > adcFilters[i].deadband) {
          adcFilters[i].filtered = avgValue;
          adcFilters[i].lastOutput = avgValue;
        }
        // If within deadband, keep the last output value
      }
      
      // Reset accumulator
      adcFilters[i].accumulator = 0;
      adcFilters[i].sampleCount = 0;
    }
  }
}

void displayFilterStatus() {
  Serial.println("Simple Filter Status:");
  const char* channelNames[] = {"PITCH_CV ", "PITCH_KNOB", "CV1       ", "CV2       "};
  
  for (int i = 0; i < 4; i++) {
    Serial.print(channelNames[i]);
    Serial.print(" - Filtered: ");
    Serial.print(adcFilters[i].filtered);
    Serial.print(", LastOut: ");
    Serial.print(adcFilters[i].lastOutput);
    Serial.print(", Deadband: ±");
    Serial.print(adcFilters[i].deadband);
    Serial.print(", Samples: ");
    Serial.print(adcFilters[i].sampleCount);
    Serial.print("/");
    Serial.println(AVG_SAMPLES);
  }
  Serial.println();
}