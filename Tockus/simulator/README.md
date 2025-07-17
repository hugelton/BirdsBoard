# Tockus Simulator

A desktop simulator for the Tockus drum synthesizer, featuring the same DSP algorithms as the Arduino hardware implementation.

## Features

- **8 Drum Algorithms**: BASS, SNARE, HIHAT, KARPLUS, MODAL, ZAP, CLAP, COWBELL
- **Real-time Parameter Control**: Pitch CV, Pitch Knob, CV1, CV2 sliders
- **Gate Input Simulation**: Trigger button for drum sounds
- **PT8211 DAC Simulation**: Accurate modeling of hardware DAC characteristics
- **Visual Feedback**: RGB LED simulation and real-time parameter displays
- **Audio Output**: 44.1kHz stereo audio through Qt Multimedia

## Hardware Simulation

### PT8211 DAC Characteristics
- 16-bit resolution with R-2R ladder quantization
- 0.08% THD (Total Harmonic Distortion)
- 89-93dB SNR (Signal-to-Noise Ratio)
- 2.5V maximum output voltage
- Frequency response modeling

### Control Voltage Mapping
- **PITCH_CV**: 0-4095 range (inverted scaling)
- **PITCH_KNOB**: 10-4000 range (hardware constraint)
- **CV1**: 8-2000 range (algorithm selection)
- **CV2**: 8-2000 range (algorithm parameter)

## Building

### Requirements
- Qt6 (Core, Widgets, Multimedia)
- CMake 3.20+
- C++17 compatible compiler

### macOS
```bash
# Install Qt6 via Homebrew
brew install qt@6

# Build
mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6 ..
make -j$(sysctl -n hw.ncpu)
```

### Linux
```bash
# Install Qt6 (Ubuntu/Debian)
sudo apt install qt6-base-dev qt6-multimedia-dev cmake

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Windows
```bash
# Install Qt6 via vcpkg or Qt installer
# Build with Visual Studio or MinGW
mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64" ..
cmake --build . --config Release
```

## Usage

1. **Launch**: Run the executable to open the simulator
2. **Audio**: Click "Start Audio" to begin real-time audio processing
3. **Controls**: 
   - Adjust sliders to control CV inputs
   - Click "Gate" button to trigger drum sounds
   - Select algorithm from dropdown
4. **Monitoring**: Watch real-time frequency, envelope, and DAC statistics

## Code Structure

- `tockus_dsp.cpp/h`: Core DSP algorithms (ported from Arduino)
- `pt8211_dac.cpp/h`: DAC simulation with hardware characteristics
- `audio_engine.cpp/h`: Qt audio interface and real-time processing
- `mainwindow.cpp/h`: GUI implementation
- `main.cpp`: Application entry point

## Algorithm Details

Each algorithm simulates authentic drum machine sounds:

- **BASS**: Self-oscillating resonant filter (808-style)
- **SNARE**: Tone + filtered noise combination
- **HIHAT**: Multiple square waves + bandpass filtering
- **KARPLUS**: Karplus-Strong physical modeling
- **MODAL**: Multi-mode resonant synthesis
- **ZAP**: Dramatic pitch sweep with noise burst
- **CLAP**: Multi-pulse filtered noise
- **COWBELL**: Four-oscillator metallic synthesis

## Performance

- **Sample Rate**: 44.1kHz (matching hardware)
- **Buffer Size**: 512 samples (configurable)
- **Latency**: ~12ms typical
- **CPU Usage**: <5% on modern systems

## Development

The simulator shares >90% of its DSP code with the Arduino implementation, ensuring accurate behavior matching. Key differences:

- Qt Audio instead of I2S output
- GUI sliders instead of ADC readings
- Desktop timers instead of Arduino timers
- Standard C++ instead of Arduino libraries