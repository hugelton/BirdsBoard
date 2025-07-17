# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

BirdsBoards is an Arduino-based modular synthesizer project featuring multiple DCO (Digitally Controlled Oscillator) applications for the RP2350A microcontroller with PT8211S I2S DAC.

### Applications
1. **BirdsBoard_Test** - Basic CV-controlled oscillator with waveform morphing
2. **Tern** - LFSR (Linear Feedback Shift Register) noise oscillator  
3. **Wren** - Real-time wavetable DCO with USB editing ⭐ **MILESTONE**
4. **ADC_Test** - Hardware debugging utility

## Hardware Architecture

- **Microcontroller**: Arduino-compatible board with I2S capabilities
- **Audio Output**: PT8211S I2S DAC (16-bit, 44.1kHz)
  - **Hardware Output Range**: 20Vpp (full range, too high for eurorack)
  - **Target Output Range**: 10Vpp (eurorack standard, requires software limiting)
- **CV Inputs**: 4 analog control voltage inputs (0-5V range)
  - PITCH_CV (ADC0): Main pitch control via CV
  - PITCH_KNOB (ADC1): Pitch knob for manual tuning
  - CV1 (ADC2): Waveform selection (SAW to SIN mix)
  - CV2 (ADC3): Wavefold amount (currently unused)
- **Gate Input**: Digital gate input for envelope control

## Pin Configuration

- I2S_BCLK: Pin 6 (Bit clock)
- I2S_DOUT: Pin 8 (Data output)
- PITCH_CV: Pin 26 (ADC0)
- PITCH_KNOB: Pin 27 (ADC1)
- CV1: Pin 28 (ADC2)
- CV2: Pin 29 (ADC3)
- GATE_IN: Pin 2 (GPIO2)

## Audio Processing Architecture

The synthesizer generates audio at 44.1kHz sample rate with the following signal path:
1. **Oscillator**: Generates both sawtooth and sine waves
2. **Waveform Mixing**: Morphs between SAW and SIN based on CV1
3. **Gate Control**: Mutes output when gate is LOW
4. **I2S Output**: Sends 16-bit stereo samples to PT8211S

## CV Scaling and Calibration

The project uses a voltage divider circuit to scale 0-5V CV inputs to the microcontroller's 0-3.3V ADC range:
- Formula: `ADC_voltage = CV_input × 0.33 + 1.65`
- Reverse calculation: `CV_input = (ADC_voltage - 1.65) / 0.33`
- Pitch CV follows 1V/octave standard
- Current implementation has OpAmp scaling issues on CV1/CV2, so raw ADC values are used

### ADC Range Calibration
- **PITCH_CV (ADC0)**: Full range 0-4095 (inverted scaling)
- **PITCH_KNOB (ADC1)**: Limited range 10-4000 (hardware constraint)
- **CV1 (ADC2)**: Limited range 8-2000 (OpAmp scaling issue)
- **CV2 (ADC3)**: Limited range 8-2000 (OpAmp scaling issue)

### ADC Filtering Strategy
Digital filtering is implemented for stable CV control:
- **Low-pass filtering**: Remove high-frequency noise from CV inputs
- **Sample averaging**: Average multiple ADC readings for stability
- **Hysteresis**: Prevent rapid switching near threshold values
- **Rate limiting**: Limit maximum change rate per update cycle
- **Deadband**: Ignore small changes to reduce parameter jitter

## Wren DCO - Real-time Wavetable Synthesizer ⭐

### Core Features
- **8 Wavetable Banks**: SAW, SINE, SQUARE, TRIANGLE, PULSE50%, PULSE25%, HARMONIC, NOISE
- **Real-time USB Editing**: 50Hz wavetable streaming capability
- **CV Control**: Bank selection (CV1) and Wavefolding (CV2)
- **Hard Sync**: Gate input for oscillator synchronization
- **NeoPixel Display**: 8 LEDs showing current bank and serial status

### Hardware Connections
- **NeoPixel**: Pin 16 (8 LEDs for bank indication)
- **Hard Sync**: Pin 2 (Gate input, rising edge resets phase)
- **CV1**: Bank selection (0-7) with hysteresis
- **CV2**: Wavefolding amount (0-100%)
- **Pitch CV/Knob**: 1V/octave frequency control

### USB Serial Protocol (115200 bps)
```
PING\n → PONG\n                    // Connection test
[64 bytes binary] → (no response)   // Real-time waveform (50Hz)
SAVE\n → OK\n                      // Save current bank to EEPROM
DUMP<bank>\n → [64 bytes binary]   // Dump specified bank
```

### Wavetable Format
- **32 samples** per waveform (64 bytes)
- **16-bit unsigned** values (0-65535)
- **Little endian** byte order
- **32768 = center**, 0 = minimum, 65535 = maximum

### NeoPixel Colors
- Bank 0 (SAW): Red
- Bank 1 (SINE): Green  
- Bank 2 (SQUARE): Blue
- Bank 3 (TRIANGLE): Yellow
- Bank 4 (PULSE50): Magenta
- Bank 5 (PULSE25): Cyan
- Bank 6 (HARMONIC): Orange
- Bank 7 (NOISE): White
- **Serial Active**: All LEDs blink (3-second timeout)

### Dual Core Architecture
- **Core 0**: Audio processing, serial communication, CV reading
- **Core 1**: NeoPixel control (non-blocking, 100ms updates)

### Audio Processing
- **Sample Rate**: 44.1kHz, 16-bit stereo
- **Amplitude**: Full scale (32767) for maximum output
- **Wavefolding**: CV2-controlled harmonic distortion
- **Hard Sync**: Phase reset on gate rising edge
- **ADC Filtering**: Multi-stage filtering for stable CV control

## Development Commands

### Building and Uploading
- Use Arduino IDE or PlatformIO to compile and upload
- Target board: RP2350A (Raspberry Pi Pico 2)
- Required libraries: 
  - I2S library for Arduino
  - Adafruit_NeoPixel
  - pico/multicore (included in RP2040 core)

### Serial Monitoring
- Baud rate: 115200
- **Wren**: No debug output (optimized for real-time performance)
- **Other apps**: Various debug outputs for development

## Code Structure

- **Main Loop**: Generates audio samples and updates I2S output
- **updateFrequency()**: Reads CV inputs and calculates frequency/parameters (called every 64 samples)
- **Waveform Generation**: Inline sawtooth and sine wave generation with morphing
- **CV Processing**: Handles voltage scaling and 1V/octave pitch calculation

## Key Implementation Details

- 12-bit ADC resolution for precise CV reading
- Phase accumulator for stable oscillator frequency
- Full amplitude (32767) for maximum output level
- Multi-stage ADC filtering for stable parameter control
- Frequency clamped to 20-8000Hz range
- EEPROM storage for persistent wavetable data
- Dual-core architecture for real-time performance

## Troubleshooting

### EEPROM Issues
If presets don't load correctly, uncomment `clearEEPROM();` in setup(), upload once, then comment it out again.

### ADC Calibration
- Use ADC_Test application to verify CV input ranges
- Check hardware OpAmp circuits if CV1/CV2 values seem limited
- PITCH_CV should show full 0-4095 range
- PITCH_KNOB should show 10-4000 range

### NeoPixel Issues
- Verify Pin 16 connection and power supply
- Check that Adafruit_NeoPixel library is installed
- LEDs should show current bank color when serial is inactive