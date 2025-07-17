# Tockus - Algorithmic Drum Synthesizer Suite 🥁

Tockus is a comprehensive algorithmic drum synthesizer for the RP2350A microcontroller, featuring 8 different synthesis algorithms ranging from classic 808-style analog drums to advanced physical modeling techniques.

## Features

### 🎵 8 Synthesis Algorithms
1. **Bass Drum** - 808-style bass drum with pitch and amplitude envelopes
2. **Snare Drum** - 808-style snare with multiple sine tones and noise
3. **Hi-Hat** - Multiple square waves and noise through bandpass filter
4. **Karplus-Strong** - Physical modeling using delay line feedback
5. **Modal Synthesis** - Multiple decaying oscillators for realistic drum timbres
6. **ZAP Sound** - Rapid pitch-drop effects with sine/square wave mixing
7. **808 Clap** - Bandpass filtered noise with multi-pulse envelope
8. **Membrane** - Simplified drum head physical model (reserved for future implementation)

### 🎛️ Real-time Control
- **Algorithm Selection**: CV1 input (0-7)
- **Pitch Control**: 1V/octave standard via Pitch CV + Pitch Knob
- **Algorithm Parameters**: CV2 input (algorithm-specific)
- **Gate Trigger**: Rising edge trigger input
- **Real-time Pitch Control**: Pitch can be modified during playback

### 💡 Visual Feedback
- **Single RGB LED**: Shows current algorithm with color coding
- **Activity Indication**: LED dims during trigger events

## Hardware Requirements

### Microcontroller
- **RP2350A** (Raspberry Pi Pico 2) or compatible Arduino board
- **I2S capability** for audio output

### Audio Output
- **PT8211S I2S DAC** (16-bit, 44.1kHz)
- **Stereo output** capability

### Control Inputs
- **4 x Analog CV inputs** (0-5V range)
- **1 x Digital gate input** (trigger)
- **1 x RGB LED** (WS2812B/NeoPixel)

## Pin Configuration

```
I2S Audio:
- I2S_BCLK: Pin 6   (Bit clock)
- I2S_DOUT: Pin 8   (Data output)

CV Inputs:
- PITCH_CV:   Pin 26 (ADC0) - Main pitch control
- PITCH_KNOB: Pin 27 (ADC1) - Fine pitch tuning
- CV1:        Pin 28 (ADC2) - Algorithm selection
- CV2:        Pin 29 (ADC3) - Algorithm parameter

Digital:
- GATE_IN:    Pin 2  (GPIO2) - Trigger input
- LED:        Pin 16 (GPIO16) - Status LED
```

## Algorithm Details

### 🥁 Bass Drum (Algorithm 0)
- **Frequency Range**: 20-150Hz
- **Synthesis**: Sine wave with pitch and amplitude envelopes
- **CV2 Parameter**: Harmonic content (0-100%)
- **Color**: Red

### 🥁 Snare Drum (Algorithm 1)
- **Frequency Range**: 100-400Hz
- **Synthesis**: 3 sine waves + white noise
- **CV2 Parameter**: Tone/noise balance (0=all noise, 100%=all tone)
- **Color**: Green

### 🔸 Hi-Hat (Algorithm 2)
- **Frequency Range**: 200-2000Hz
- **Synthesis**: 4 square waves + noise through bandpass filter
- **CV2 Parameter**: Filter frequency and Q factor
- **Color**: Blue

### 🎸 Karplus-Strong (Algorithm 3)
- **Frequency Range**: 80-800Hz
- **Synthesis**: Noise through delay line with feedback
- **CV2 Parameter**: Damping amount (0=long decay, 100%=short decay)
- **Color**: Yellow

### 🔔 Modal Synthesis (Algorithm 4)
- **Frequency Range**: 60-600Hz
- **Synthesis**: 4 harmonic modes with individual decay rates
- **CV2 Parameter**: Overall decay time
- **Color**: Magenta

### ⚡ ZAP Sound (Algorithm 5)
- **Frequency Range**: 50-500Hz
- **Synthesis**: Sine wave with rapid pitch drop + square wave
- **CV2 Parameter**: Square wave mix amount
- **Color**: Cyan

### 👏 808 Clap (Algorithm 6)
- **Frequency Range**: 150-1500Hz
- **Synthesis**: Bandpass filtered noise with 4-pulse envelope + reverb
- **CV2 Parameter**: Filter center frequency
- **Color**: Orange

### 🪘 Membrane (Algorithm 7)
- **Frequency Range**: 40-400Hz
- **Synthesis**: Reserved for future implementation
- **CV2 Parameter**: TBD
- **Color**: White

## Control Specifications

### Pitch Control (1V/Octave)
- **Standard**: Follows 1V/octave tuning
- **Range**: 20Hz - 8kHz base range, algorithm-specific scaling applied
- **Real-time**: Pitch can be modified during playback
- **Calibration**: Auto-calibrated for hardware voltage scaling

### CV Input Ranges (Hardware Calibrated)
```
PITCH_CV:   0-4095 (full range, inverted)
PITCH_KNOB: 10-4000 (hardware limited)
CV1:        8-2000 (OpAmp scaling issue)
CV2:        8-2000 (OpAmp scaling issue)
```

### Envelope Characteristics
- **Exponential decay** for natural drum sounds
- **Algorithm-specific decay rates**:
  - Bass: 2-5 Hz (slow)
  - Snare: 8-15 Hz (medium)
  - Hi-hat: 15-40 Hz (fast)
  - Karplus: 3-8 Hz (medium-slow)
  - Modal: 4-10 Hz (medium)
  - ZAP: 10-30 Hz (fast)
  - Clap: 12-20 Hz (medium-fast)

## Performance Specifications

### Audio Quality
- **Sample Rate**: 44.1 kHz
- **Bit Depth**: 16-bit stereo
- **Latency**: <1ms trigger response
- **Dynamic Range**: Full 16-bit range

### System Performance
- **CPU Usage**: ~50% (including real-time pitch control)
- **RAM Usage**: ~8KB (including Karplus-Strong buffer)
- **Update Rate**: 16 samples (0.36ms) for CV parameters
- **Dual Core**: Audio on Core 0, LED control on Core 1

## Installation

### Arduino IDE Setup
1. Install Arduino IDE 2.0+
2. Add RP2040 board package
3. Install required libraries:
   - `I2S library for Arduino`
   - `FastLED`
   - `EEPROM` (included)

### Library Dependencies
```cpp
#include <I2S.h>
#include <EEPROM.h>
#include <FastLED.h>
#include <pico/multicore.h>
```

### Compilation Settings
- **Board**: Raspberry Pi Pico 2 (RP2350)
- **USB Stack**: Pico SDK
- **CPU Speed**: 133MHz (default)
- **Flash Size**: 4MB

## Usage

### Basic Operation
1. **Power on** - LED shows current algorithm (default: Red = Bass Drum)
2. **Select algorithm** - Turn CV1 knob to choose synthesis method
3. **Set pitch** - Use Pitch CV and/or Pitch Knob for frequency
4. **Adjust parameter** - CV2 controls algorithm-specific parameter
5. **Trigger** - Send gate pulse to trigger drum sound

### Live Performance
- **Algorithm switching**: Instant switching between synthesis methods
- **Real-time pitch control**: Bend pitch during sound playback
- **Parameter morphing**: Smooth real-time parameter changes
- **Gate timing**: Supports rapid triggering for rolls and patterns

### Debugging
- **Serial Monitor**: 115200 baud for CV value monitoring
- **Debug Output**: Shows CV values, algorithm, frequency, and gate status
- **LED Status**: Visual confirmation of current algorithm

## Advanced Features

### Real-time Pitch Control
- Pitch can be modified during drum sound playback
- Smooth parameter interpolation prevents audio artifacts
- Algorithm-specific frequency scaling maintains musical intervals

### Dual-Core Architecture
- **Core 0**: Audio processing, CV reading, serial communication
- **Core 1**: LED control, non-blocking status updates
- **100ms LED updates** for smooth visual feedback

### ADC Filtering
- **Multi-stage filtering**: Low-pass, deadband, rate limiting
- **Stable CV control**: Eliminates parameter jitter
- **Configurable sensitivity**: Adjustable filter parameters

## Troubleshooting

### No Audio Output
- Check I2S connections (pins 6, 8)
- Verify PT8211S power supply
- Confirm sample rate compatibility

### CV Inputs Not Responding
- Check voltage range (0-5V expected)
- Monitor serial debug output
- Verify ADC pin connections

### LED Not Working
- Check pin 16 connection
- Verify FastLED library installation
- Check power supply to LED

### Algorithm Not Switching
- Monitor CV1 value in serial output
- Check CV1 voltage range
- Verify map() function scaling

## Technical Specifications

### Memory Usage
```
Program Memory: ~45KB
RAM Usage: ~8KB
EEPROM: Not used (reserved for future presets)
Stack: ~2KB per core
```

### Signal Processing
- **Bandpass Filter**: 2nd order IIR, configurable Q
- **Envelope Generation**: Exponential decay curves
- **Noise Generation**: Linear congruential generator
- **Phase Accumulation**: 32-bit precision
- **Anti-aliasing**: Software band-limiting

### Timing Characteristics
- **Trigger Latency**: <22.7μs (1 sample)
- **CV Update Rate**: 2.7kHz (16 samples)
- **Parameter Smoothing**: 10ms time constant
- **Gate Debouncing**: Hardware dependent

## Future Enhancements

### Planned Features
- **EEPROM presets**: Save/recall algorithm settings
- **MIDI input**: MIDI note triggering
- **Advanced membrane model**: 2D physical modeling
- **Pattern sequencer**: Built-in rhythm patterns
- **Velocity sensitivity**: Dynamic response control

### Expansion Possibilities
- **Multiple outputs**: Individual drum voices
- **CV outputs**: Envelope and trigger outputs
- **External sync**: Clock input/output
- **USB audio**: Direct computer connection

## Credits

**Tockus** is part of the BirdsBoards modular synthesizer project.

- **Hardware**: RP2350A microcontroller, PT8211S DAC
- **Software**: Arduino framework, custom DSP algorithms
- **Inspiration**: Classic analog drum machines and modern physical modeling

## License

This project is part of the BirdsBoards ecosystem. See project documentation for licensing details.

---

*Tockus - Where algorithmic precision meets percussive expression* 🎵