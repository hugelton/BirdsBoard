/*
 * Tern - LFSR-based Noise Oscillator
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

// PT8211S I2S pins
#define I2S_BCLK  6   // Bit clock
#define I2S_DOUT  8   // Data output

// ADC pins for control
#define PITCH_CV   26  // ADC0
#define PITCH_KNOB 27  // ADC1
#define CV1        28  // ADC2 - Tap position
#define CV2        29  // ADC3 - Register length (bit depth)

// Gate input
#define GATE_IN    2   // GPIO2 - Gate input for LFSR reset

// Create I2S instance for PT8211S
I2S i2s(OUTPUT, I2S_BCLK, I2S_DOUT);

const int sampleRate = 44100;
float frequency = 440.0;
const int amplitude = 12000;

// LFSR parameters
uint32_t lfsrState = 0xACE1u; // Initial seed (non-zero)
uint8_t registerLength = 16;  // Number of bits to use (8-32)
uint8_t tapPosition = 0;      // Tap position for feedback
uint32_t registerMask = 0xFFFF; // Mask for register length

// Timing and gate
float phaseAccumulator = 0.0;
bool lastGateState = false;
uint32_t sampleCounter = 0;

void setup() {
  Serial.begin(115200);
  
  // Initialize ADC
  analogReadResolution(12);
  
  // Initialize Gate input
  pinMode(GATE_IN, INPUT);
  
  i2s.setBitsPerSample(16);
  i2s.setLSBJFormat();
  
  if (!i2s.begin(sampleRate)) {
    Serial.println("Failed to initialize I2S!");
    while (1);
  }
  
  updateRegisterMask();
  Serial.println("Tern LFSR Oscillator - CV Controlled");
}

void loop() {
  // Read CV inputs every 64 samples
  if (sampleCounter % 64 == 0) {
    updateParameters();
  }
  
  // Generate LFSR sample at the specified frequency
  phaseAccumulator += frequency / sampleRate;
  
  if (phaseAccumulator >= 1.0) {
    phaseAccumulator -= 1.0;
    
    // Clock the LFSR
    clockLFSR();
  }
  
  // Convert LFSR output to audio sample
  // Use the LSB of the LFSR as the output bit
  float sampleFloat = (lfsrState & 1) ? 1.0 : -1.0;
  int16_t sample = (int16_t)(sampleFloat * amplitude);
  
  // Write same sample to both channels
  i2s.write(sample);
  i2s.write(sample);
  
  sampleCounter++;
}

void clockLFSR() {
  // Get the feedback bit from the tap position
  uint8_t actualTapPos = tapPosition % registerLength;
  uint8_t feedbackBit = (lfsrState >> actualTapPos) & 1;
  
  // XOR with the MSB (for 1-tap LFSR)
  uint8_t msbPos = registerLength - 1;
  uint8_t msbBit = (lfsrState >> msbPos) & 1;
  uint8_t newBit = feedbackBit ^ msbBit;
  
  // Shift left and insert new bit
  lfsrState = ((lfsrState << 1) | newBit) & registerMask;
  
  // Prevent all-zero state
  if (lfsrState == 0) {
    lfsrState = 1;
  }
}

void updateParameters() {
  // Read CV inputs and gate
  uint16_t pitchCV = analogRead(PITCH_CV);
  uint16_t pitchKnob = analogRead(PITCH_KNOB);
  uint16_t cv1 = analogRead(CV1);
  uint16_t cv2 = analogRead(CV2);
  bool gateState = digitalRead(GATE_IN);
  
  // Handle gate input - reset LFSR on rising edge
  if (gateState && !lastGateState) {
    lfsrState = 0xACE1u; // Reset to initial seed
  }
  lastGateState = gateState;
  
  // Calculate frequency (same as BirdsBoard_Test)
  float adcVoltage = ((4095 - pitchCV) / 4095.0) * 3.3;
  float knobVoltage = (pitchKnob / 4095.0) * 3.3;
  
  float actualCV = (adcVoltage - 1.65) / 0.33;
  actualCV = constrain(actualCV, 0.0, 5.0);
  
  float cvOctaves = actualCV;
  float knobOctaves = (knobVoltage - 1.65) / 1.65;
  
  frequency = 440.0 * pow(2.0, cvOctaves + knobOctaves - 4.0);
  frequency = constrain(frequency, 1.0, 8000.0);
  
  // CV1: Tap position (0 to registerLength-1)
  tapPosition = map(cv1, 0, 4095, 0, registerLength - 1);
  
  // CV2: Register length (1-32 bits)
  uint8_t newRegisterLength = map(cv2, 0, 4095, 1, 32);
  if (newRegisterLength != registerLength) {
    registerLength = newRegisterLength;
    updateRegisterMask();
  }
  
  // Debug output
  static uint32_t lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    Serial.print("Freq: ");
    Serial.print(frequency, 1);
    Serial.print("Hz | RegLen: ");
    Serial.print(registerLength);
    Serial.print(" | Tap: ");
    Serial.print(tapPosition);
    Serial.print(" | LFSR: 0x");
    Serial.print(lfsrState, HEX);
    Serial.print(" | Gate: ");
    Serial.print(gateState ? "HIGH" : "LOW");
    Serial.println();
    lastPrint = millis();
  }
}

void updateRegisterMask() {
  registerMask = (1UL << registerLength) - 1;
  // Clamp LFSR state to new register length
  lfsrState &= registerMask;
  if (lfsrState == 0) {
    lfsrState = 1;
  }
}