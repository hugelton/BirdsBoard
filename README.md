# BirdsBoard

BirdsBoard is an open-source modular synthesizer project based on the RP2350A microcontroller and the PT8211 I2S DAC. This project provides a collection of digitally controlled oscillator (DCO) applications for Eurorack-compatible systems.

-> Based on [("Wren")(https://documents.hugelton.com/Wren)] indivisual module.

## Firmware

This repository contains several firmware applications.

| Firmware   | Status         | Description                                       |
|------------|----------------|---------------------------------------------------|
| **Wren**   | Stable         | Real-time wavetable DCO with USB editing.         |
| **Tockus** | In Development | Versatile drum voice with 8 algorithms.           |
| **Tern**   | Experimental   | LFSR-based noise oscillator.                      |
| **ADC_Test**| Utility        | Utility for debugging and calibrating hardware.   |

- **Waveshare RP2350A Zero + PT8211:** A compact, affordable combination for high-quality audio synthesis.
- **Multiple Applications:** A variety of synthesizer voices with unique features.
- **Eurorack Compatible:** Designed to integrate with standard modular synthesizer systems.
- **Open Source:** Hardware and firmware are released under open-source licenses.

## Applications

### Wren
A powerful wavetable oscillator with 8 user-editable wavetable banks.
- **Real-time USB Editing:** Edit wavetables on the fly using a binary serial protocol.
- **CV Control:** Control wavetable bank selection and modulation amount via CV.
- **Multiple Modulation Types:** Wavefolding, Overflow, Bitcrushing, Phase Distortion, Resonance.
- **Hard Sync:** Reset the oscillator phase with a gate input.
- **NeoPixel Display:** An LED indicates the current wavetable bank and serial connection status.

### Tockus
A flexible drum synthesizer with 8 distinct algorithms.
- **Algorithms:** 808-style Bass Drum, Snare, Hi-hat, Karplus-Strong, and more.
- **CV Control:** Select algorithms and control a parameter for each algorithm via CV.
- **NeoPixel Display:** An LED indicates the currently selected algorithm.

### Tern
A chiptune-style noise generator based on a Linear Feedback Shift Register (LFSR).
- **CV Control:** Control the frequency, register length, and tap position of the LFSR via CV.
- **Gate Reset:** Reset the LFSR to its initial state with a gate input.

## Hardware
The hardware design files (schematics and PCB layouts) are available in the `Hardware` directory.
- **Microcontroller:** Waveshare RP2350A Zero
- **DAC:** PT8211 I2S DAC
- **CV Inputs:** 4x -5V~+5V CV inputs
- **Gate Input:** 1x Gate input

  ### JLCPCB Integration
  Just send me the design file and I'll make it all for you.
  Please prepare your own USB cable and Neopixel.

## Getting Started with arduino-cli

This project uses `arduino-cli` for building and uploading firmware.

1.  **Install `arduino-cli`:**
    Follow the official instructions: [https://arduino.github.io/arduino-cli/latest/installation/](https://arduino.github.io/arduino-cli/latest/installation/)

2.  **Clone the repository:**
    ```bash
    git clone https://github.com/kurogedelic/BirdsBoard.git
    cd BirdsBoard
    ```

3.  **Setup `arduino-cli`:**
    Update the core index and install the necessary core and libraries.
    ```bash
    # Update package index
    arduino-cli core update-index

    # Install the Raspberry Pi Pico/RP2350A core
    arduino-cli core install rp2350A:rp2350A

    # Install libraries
    arduino-cli lib install FastLED EEPROM
    ```
    *Note: The `I2S` and `pico/multicore` libraries are included with the `rp2350A:rp2350A` core.*

4.  **Compile a firmware:**
    To compile a specific firmware (e.g., Wren), run the following command. The FQBN (Fully Qualified Board Name) for the board is `rp2350A:rp2350A:pico`.
    ```bash
    arduino-cli compile --fqbn rp2350A:rp2350A:pico Firmware/Wren/
    ```

5.  **Upload the firmware:**
    First, put your board into bootloader mode and find its port.
    ```bash
    arduino-cli board list
    ```
    Then, upload the firmware. Replace `/dev/ttyACM0` with your board's port.
    ```bash
    arduino-cli upload -p /dev/ttyACM0 --fqbn rp2350A:rp2350A:pico Firmware/Wren/
    ```

## License

This project consists of both hardware and firmware components, each under its own open-source license. See the `Hardware/LICENSE` and `Firmware/LICENSE` files for full license texts.

> 📜 Hardware: CERN-OHL-S v2 / Firmware: LGPL-3.0

### Hardware License
The hardware designs (schematics, PCB layouts) are licensed under the **CERN-OHL-S v2**. A copy of the license is available in the `Hardware/` directory.
The CERN Open Hardware Licence ensures that any modifications to the hardware design are shared under the same terms, preserving openness across derivatives.

### Firmware License
The firmware applications are licensed under the **GNU Lesser General Public License v3.0 (LGPL-3.0)**. A copy of the license is available in the `Firmware/` directory.
The LGPL-3.0 allows use in both open and closed-source systems, as long as modifications to the firmware itself are released under the same license.

## Author

**Leo Kuroshita** ([@kurogedelic](https://x.com/kurogedelic)) for ([Hügelton Instruments](https://github.com/kurogedelic)).
