# Wren - Real-time Wavetable DCO

**Wren** is a real-time wavetable DCO (Digitally Controlled Oscillator) for Arduino-compatible boards featuring USB editing capabilities and NeoPixel visual feedback.

## Features

- **8 Wavetable Banks**: SAW, SINE, SQUARE, TRIANGLE, PULSE50%, PULSE25%, HARMONIC, NOISE
- **Real-time USB Editing**: 50Hz wavetable streaming for live performance
- **CV Control**: Bank selection and wavefolding via control voltage
- **Hard Sync**: Gate input for oscillator synchronization
- **Visual Feedback**: Single LED shows current bank and serial status
- **Dual Core Architecture**: Dedicated core for smooth LED control
- **EEPROM Storage**: Persistent wavetable data

## Hardware Requirements

- Arduino-compatible board with I2S capabilities (RP2350A recommended)
- PT8211S I2S DAC for audio output
- WS2812B addressable LED (single pixel)
- 4 analog CV inputs (0-5V range)
- Gate input for hard sync

## Pin Configuration

```
Audio Output:
- I2S_BCLK: Pin 6 (Bit clock)
- I2S_DOUT: Pin 8 (Data output)

Control Inputs:
- PITCH_CV: Pin 26 (ADC0) - Main pitch control
- PITCH_KNOB: Pin 27 (ADC1) - Pitch knob
- CV1: Pin 28 (ADC2) - Bank selection
- CV2: Pin 29 (ADC3) - Wavefolding amount
- GATE_IN: Pin 2 (GPIO2) - Hard sync gate

Visual Feedback:
- LED_PIN: Pin 16 - WS2812B LED
```

## Audio Specifications

- **Sample Rate**: 44.1kHz, 16-bit stereo
- **Frequency Range**: 20Hz - 8kHz
- **Amplitude**: Full scale (32767) for maximum output
- **Wavetable Size**: 32 samples per waveform
- **Waveform Format**: 16-bit unsigned (0-65535)

## Bank Colors

The LED indicates the current wavetable bank:

| Bank | Waveform | Color |
|------|----------|-------|
| 0 | SAW | Red |
| 1 | SINE | Green |
| 2 | SQUARE | Blue |
| 3 | TRIANGLE | Yellow |
| 4 | PULSE50% | Magenta |
| 5 | PULSE25% | Cyan |
| 6 | HARMONIC | Orange |
| 7 | NOISE | White |

## LED Status

- **Full Brightness**: Normal operation
- **50% Dimmed**: Serial communication active
- **Bank Color**: Current wavetable bank

## USB Serial Protocol v2.0

**Baud Rate**: 115200  
**Format**: Binary commands (no text)

### Commands

| Command | Format | Response | Description |
|---------|--------|----------|-------------|
| PING | `0x01` | `0xA1` | Connection test |
| BANK | `0x02 <bank>` | `0xA2` | Set editing target bank |
| SAVE | `0x03 <bank>` | `0xA3` | Save bank to EEPROM |
| DUMP | `0x04 <bank>` | `0xA4 + [64 bytes]` | Dump bank data |
| REALTIME | `0x05 + [64 bytes]` | (none) | Real-time editing |

### Error Responses

| Code | Description |
|------|-------------|
| `0xE0` | Invalid command |
| `0xE1` | Bank number out of range (0-7) |

### Usage Examples

```
Connection test:
PC → DCO: 0x01
DCO → PC: 0xA1

Set editing target to bank 3:
PC → DCO: 0x02 0x03
DCO → PC: 0xA2

Save bank 5:
PC → DCO: 0x03 0x05
DCO → PC: 0xA3

Real-time edit current bank:
PC → DCO: 0x05 + [64 bytes waveform]
DCO → PC: (no response)
```

### Wavetable Format

- **32 samples** per waveform (64 bytes total)
- **16-bit unsigned** values (0-65535)
- **Little endian** byte order
- **32768 = center**, 0 = minimum, 65535 = maximum

## Control Voltage Mapping

### CV1 - Bank Selection
- **Range**: 0-5V
- **Function**: Selects wavetable bank (0-7)
- **Hysteresis**: 100ms delay prevents rapid switching

### CV2 - Wavefolding
- **Range**: 0-5V
- **Function**: Wavefolding amount (0-100%)
- **Algorithm**: Triangle wave folding with harmonic distortion

### Pitch Control
- **PITCH_CV**: Main pitch control (1V/octave)
- **PITCH_KNOB**: Manual pitch adjustment
- **Base Frequency**: 440Hz (A4)
- **Calibration**: Follows standard 1V/octave scaling

## Hard Sync

The gate input provides hard sync functionality:
- **Rising edge**: Resets oscillator phase to 0
- **Threshold**: Standard gate levels (>2.5V = HIGH)
- **Response**: Immediate phase reset for tight sync

## ADC Filtering

Advanced filtering ensures stable CV control:
- **Low-pass filtering**: Removes high-frequency noise
- **Hysteresis**: Prevents parameter jitter
- **Rate limiting**: Smooth parameter changes
- **Deadband**: Ignores small fluctuations

## Usage

1. **Power On**: Default wavetables are loaded automatically
2. **Bank Selection**: Use CV1 to select playback bank (LED shows color)
3. **Pitch Control**: Use PITCH_CV and PITCH_KNOB for frequency
4. **Wavefolding**: Use CV2 to add harmonic distortion
5. **Hard Sync**: Connect gate signal to GATE_IN for sync
6. **USB Editing**: Connect to computer for real-time wavetable editing

### Editing Workflow

1. **Set Editing Target**: Send `BANK` command to select editing bank
2. **Real-time Edit**: Send `REALTIME` commands with waveform data
3. **Listen to Changes**: Use CV1 to switch to the editing bank to hear changes
4. **Save Changes**: Send `SAVE` command to store in EEPROM

### Bank Control

- **Editing Bank**: Controlled by `BANK` command from PC
- **Playback Bank**: Controlled by CV1 knob
- **Independent**: Can edit one bank while playing another

## Installation

1. Install required libraries:
   - I2S library for Arduino
   - FastLED library
   - pico/multicore (included in RP2040 core)

2. Set target board to RP2350A (Raspberry Pi Pico 2)

3. Upload the firmware

4. Connect hardware according to pin configuration

## Troubleshooting

### EEPROM Issues
If presets don't load correctly:
1. Uncomment `clearEEPROM();` in setup()
2. Upload once
3. Comment it out again and re-upload

### LED Not Working
- Verify Pin 16 connection
- Check FastLED library installation
- Ensure proper power supply for WS2812B

### Audio Issues
- Check I2S connections (pins 6 and 8)
- Verify PT8211S DAC wiring
- Test with headphones or line input

### CV Response
- Use ADC_Test application to verify CV input ranges
- Check hardware scaling circuits
- Verify 0-5V input range

## Technical Details

### Dual Core Architecture
- **Core 0**: Audio processing, binary protocol handling, CV reading
- **Core 1**: LED control (non-blocking, 100ms updates)

### Memory Usage
- **RAM**: Wavetables stored in RAM for fast access
- **EEPROM**: Persistent storage for custom wavetables
- **PROGMEM**: Default wavetables stored in flash

### Performance
- **Audio Latency**: Sub-millisecond (44.1kHz processing)
- **CV Response**: 64-sample intervals (~1.5ms)
- **USB Protocol**: Binary commands for reliable communication
- **LED Updates**: 100ms smooth animation

### Bank Management
- **Editing Bank** (`currentBank`): Target for real-time editing and save operations
- **Playback Bank** (`playbackBank`): Currently playing audio bank (CV1 controlled)
- **Real-time Updates**: Only applied when editing bank matches playback bank

## License

Part of the BirdsBoards modular synthesizer project.

## Version History

- **v1.0**: Initial release with real-time wavetable editing
- **v1.1**: Added FastLED support and improved LED feedback
- **v1.2**: Enhanced ADC filtering and CV response
- **v2.0**: Binary protocol implementation, separated editing/playback banks