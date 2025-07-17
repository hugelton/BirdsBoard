#include <I2S.h>

// PT8211S I2S pins
#define I2S_BCLK  6   // Bit clock
#define I2S_DOUT  8   // Data output

// ADC pins for pitch control
#define PITCH_CV   26  // ADC0
#define PITCH_KNOB 27  // ADC1
#define CV1        28  // ADC2 - Waveform select (SAW-SIN)
#define CV2        29  // ADC3 - Wavefold amount

// Gate input
#define GATE_IN    2   // GPIO2 - Gate input

// Create I2S instance for PT8211S
I2S i2s(OUTPUT, I2S_BCLK, I2S_DOUT);

const int sampleRate = 44100;
float frequency = 440.0; // Base frequency
const int amplitude = 8000; // Reduced amplitude
float phase = 0.0;
float waveformMix = 0.0; // 0=SAW, 1=SIN
float wavefoldAmount = 0.0; // 0=no fold, 1=max fold
bool gateState = false;

void setup() {
  Serial.begin(115200);
  
  // Initialize ADC
  analogReadResolution(12); // 12-bit ADC
  
  // Initialize Gate input
  pinMode(GATE_IN, INPUT);
  
  i2s.setBitsPerSample(16);
  i2s.setLSBJFormat(); // PT8211S format
  
  if (!i2s.begin(sampleRate)) {
    Serial.println("Failed to initialize I2S!");
    while (1);
  }
  
  Serial.println("PT8211S Test - CV Controlled Sine Wave");
}

void loop() {
  // Read CV inputs every 64 samples to reduce CPU load
  static int sampleCount = 0;
  if (sampleCount % 64 == 0) {
    updateFrequency();
  }
  
  // Generate waveform sample (SAW to SIN mix)
  float sineSample = sin(phase * 2.0 * PI);
  float sawSample = (phase < 0.5) ? (phase * 4.0 - 1.0) : (3.0 - phase * 4.0);
  
  // Mix between SAW and SIN based on CV1
  float sampleFloat = sawSample * (1.0 - waveformMix) + sineSample * waveformMix;
  
  // Apply gate (mute when gate is LOW)
  if (!gateState) {
    sampleFloat = 0.0;
  }
  
  int16_t sample = (int16_t)(sampleFloat * amplitude);
  
  // Write same sample to both channels
  i2s.write(sample);
  i2s.write(sample);
  
  // Update phase
  phase += frequency / sampleRate;
  if (phase >= 1.0) phase -= 1.0;
  
  sampleCount++;
}

void updateFrequency() {
  // Read CV inputs and gate
  uint16_t pitchCV = analogRead(PITCH_CV);
  uint16_t pitchKnob = analogRead(PITCH_KNOB);
  uint16_t cv1 = analogRead(CV1);
  uint16_t cv2 = analogRead(CV2);
  gateState = digitalRead(GATE_IN);
  
  // Calculate frequency with proper CV scaling
  // ADC range: 0-4095 (12-bit), voltage range: 0-3.3V (inverted)
  float adcVoltage = ((4095 - pitchCV) / 4095.0) * 3.3; // Inverted ADC voltage
  float knobVoltage = (pitchKnob / 4095.0) * 3.3;
  
  // Convert ADC voltage back to actual CV input (0-5V scale)
  // ADC voltage = CV input × 0.33 + 1.65, so CV input = (ADC voltage - 1.65) / 0.33
  float actualCV = (adcVoltage - 1.65) / 0.33;
  actualCV = constrain(actualCV, 0.0, 5.0); // Clamp to 0-5V range
  
  // CV: 0-5V input = 5 octaves (1V/octave)
  // Knob: ±1 octave range around center
  float cvOctaves = actualCV; // 1V/octave standard
  float knobOctaves = (knobVoltage - 1.65) / 1.65; // ±1 octave
  
  // Base frequency 440Hz - 4 octaves down
  frequency = 440.0 * pow(2.0, cvOctaves + knobOctaves - 4.0);
  
  // Clamp frequency to reasonable range
  frequency = constrain(frequency, 20.0, 8000.0);
  
  // CV1: Waveform mix (0=SAW, 1=SIN)
  // Temporary fix: use raw ADC values due to OpAmp issues
  waveformMix = constrain(cv1 / 4095.0, 0.0, 1.0);
  
  // Original OpAmp scaling (commented out due to hardware issue):
  // float cv1Voltage = (cv1 / 4095.0) * 3.3;
  // float actualCV1 = (cv1Voltage - 1.65) / 0.33;
  // actualCV1 = constrain(actualCV1, 0.0, 5.0);
  // waveformMix = actualCV1 / 5.0;
  
  // Print frequency and ADC raw values every second
  static uint32_t lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    Serial.print("Freq: ");
    Serial.print(frequency, 1);
    Serial.print("Hz | ADC: CV=");
    Serial.print(pitchCV);
    Serial.print(" Knob=");
    Serial.print(pitchKnob);
    Serial.print(" CV1=");
    Serial.print(cv1);
    Serial.print(" CV2=");
    Serial.print(cv2);
    Serial.print(" | Gate=");
    Serial.print(gateState ? "HIGH" : "LOW");
    Serial.print(" | Wave=");
    Serial.print(waveformMix, 2);
    Serial.println();
    lastPrint = millis();
  }
}

