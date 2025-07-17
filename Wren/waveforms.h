#ifndef WAVEFORMS_H
#define WAVEFORMS_H

#include <Arduino.h>

// Default waveform presets for 32-sample wavetables
// Each waveform uses 16-bit unsigned values (0-65535)
// 32768 = center, 0 = minimum, 65535 = maximum

const uint16_t PROGMEM preset_saw[32] = {
  0, 2113, 4226, 6339, 8452, 10565, 12678, 14791,
  16904, 19017, 21130, 23243, 25356, 27469, 29582, 31695,
  33808, 35921, 38034, 40147, 42260, 44373, 46486, 48599,
  50712, 52825, 54938, 57051, 59164, 61277, 63390, 65535
};

const uint16_t PROGMEM preset_sine[32] = {
  32768, 39186, 45341, 51018, 56000, 60078, 63041, 64717,
  65535, 64717, 63041, 60078, 56000, 51018, 45341, 39186,
  32768, 26350, 20195, 14518, 9536, 5458, 2495, 819,
  0, 819, 2495, 5458, 9536, 14518, 20195, 26350
};

const uint16_t PROGMEM preset_square[32] = {
  65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535,
  65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0
};

const uint16_t PROGMEM preset_triangle[32] = {
  32768, 36864, 40960, 45056, 49152, 53248, 57344, 61440,
  65535, 61440, 57344, 53248, 49152, 45056, 40960, 36864,
  32768, 28672, 24576, 20480, 16384, 12288, 8192, 4096,
  0, 4096, 8192, 12288, 16384, 20480, 24576, 28672
};

const uint16_t PROGMEM preset_pulse50[32] = {
  65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535,
  65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0
};

const uint16_t PROGMEM preset_pulse25[32] = {
  65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0
};

const uint16_t PROGMEM preset_harmonic_sine[32] = {
  32768, 42598, 51575, 58626, 62929, 63967, 61575, 55928,
  47494, 36042, 22652, 8702, 0, 6554, 12288, 16384,
  18204, 18204, 16384, 12288, 6554, 0, 8702, 22652,
  36042, 47494, 55928, 61575, 63967, 62929, 58626, 51575
};

const uint16_t PROGMEM preset_noise[32] = {
  45231, 12087, 58392, 3021, 61847, 19283, 42917, 7629,
  55643, 28391, 4782, 51029, 15847, 38291, 62013, 9384,
  47592, 21847, 58392, 6284, 43857, 17293, 52847, 8391,
  37492, 59183, 14729, 48392, 2847, 56293, 19847, 41583
};

// Array of pointers to all presets for easy access
const uint16_t* const preset_waveforms[8] PROGMEM = {
  preset_saw,           // Bank 0
  preset_sine,          // Bank 1  
  preset_square,        // Bank 2
  preset_triangle,      // Bank 3
  preset_pulse50,       // Bank 4
  preset_pulse25,       // Bank 5
  preset_harmonic_sine, // Bank 6
  preset_noise          // Bank 7
};

// Waveform names for debugging/display
const char* const waveform_names[8] PROGMEM = {
  "SAW",
  "SINE", 
  "SQUARE",
  "TRIANGLE",
  "PULSE50",
  "PULSE25", 
  "HARMONIC",
  "NOISE"
};

#endif // WAVEFORMS_H