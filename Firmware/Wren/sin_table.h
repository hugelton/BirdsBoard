// High-speed sine lookup table for Wren modulation effects
// 256 samples covering 0 to 2π with 16-bit precision

#ifndef SIN_TABLE_H
#define SIN_TABLE_H

#include <Arduino.h>

// Sine table constants
#define SIN_TABLE_SIZE 256
#define SIN_TABLE_MASK (SIN_TABLE_SIZE - 1)

// Pre-calculated sine values (16-bit signed, -32767 to +32767)
const int16_t sinTable[SIN_TABLE_SIZE] = {
    0, 804, 1608, 2410, 3212, 4011, 4808, 5602, 6393, 7179, 7962, 8739, 9512, 10278, 11039, 11793,
    12539, 13279, 14010, 14732, 15446, 16151, 16846, 17530, 18204, 18868, 19519, 20159, 20787, 21403, 22005, 22594,
    23170, 23731, 24279, 24811, 25329, 25832, 26319, 26790, 27245, 27683, 28105, 28510, 28898, 29268, 29621, 29956,
    30273, 30571, 30852, 31113, 31356, 31580, 31785, 31971, 32137, 32285, 32412, 32521, 32609, 32678, 32728, 32757,
    32767, 32757, 32728, 32678, 32609, 32521, 32412, 32285, 32137, 31971, 31785, 31580, 31356, 31113, 30852, 30571,
    30273, 29956, 29621, 29268, 28898, 28510, 28105, 27683, 27245, 26790, 26319, 25832, 25329, 24811, 24279, 23731,
    23170, 22594, 22005, 21403, 20787, 20159, 19519, 18868, 18204, 17530, 16846, 16151, 15446, 14732, 14010, 13279,
    12539, 11793, 11039, 10278, 9512, 8739, 7962, 7179, 6393, 5602, 4808, 4011, 3212, 2410, 1608, 804,
    0, -804, -1608, -2410, -3212, -4011, -4808, -5602, -6393, -7179, -7962, -8739, -9512, -10278, -11039, -11793,
    -12539, -13279, -14010, -14732, -15446, -16151, -16846, -17530, -18204, -18868, -19519, -20159, -20787, -21403, -22005, -22594,
    -23170, -23731, -24279, -24811, -25329, -25832, -26319, -26790, -27245, -27683, -28105, -28510, -28898, -29268, -29621, -29956,
    -30273, -30571, -30852, -31113, -31356, -31580, -31785, -31971, -32137, -32285, -32412, -32521, -32609, -32678, -32728, -32757,
    -32767, -32757, -32728, -32678, -32609, -32521, -32412, -32285, -32137, -31971, -31785, -31580, -31356, -31113, -30852, -30571,
    -30273, -29956, -29621, -29268, -28898, -28510, -28105, -27683, -27245, -26790, -26319, -25832, -25329, -24811, -24279, -23731,
    -23170, -22594, -22005, -21403, -20787, -20159, -19519, -18868, -18204, -17530, -16846, -16151, -15446, -14732, -14010, -13279,
    -12539, -11793, -11039, -10278, -9512, -8739, -7962, -7179, -6393, -5602, -4808, -4011, -3212, -2410, -1608, -804
};

// Fast sine function using table lookup with linear interpolation
// Input: phase (0.0 to 1.0)
// Output: sine value (-1.0 to 1.0)
inline float fastSin(float phase) {
    // Normalize phase to 0.0-1.0 range
    while (phase >= 1.0f) phase -= 1.0f;
    while (phase < 0.0f) phase += 1.0f;
    
    // Convert to table index (0 to 255.xxx)
    float tablePos = phase * SIN_TABLE_SIZE;
    int index = (int)tablePos;
    float frac = tablePos - index;
    
    // Linear interpolation between adjacent samples
    int16_t sample1 = sinTable[index & SIN_TABLE_MASK];
    int16_t sample2 = sinTable[(index + 1) & SIN_TABLE_MASK];
    
    float interpolated = sample1 + frac * (sample2 - sample1);
    
    // Convert from int16 to float (-1.0 to 1.0)
    return interpolated / 32767.0f;
}

// Fast cosine function (sine shifted by 90 degrees)
// Input: phase (0.0 to 1.0)  
// Output: cosine value (-1.0 to 1.0)
inline float fastCos(float phase) {
    return fastSin(phase + 0.25f);  // cos(x) = sin(x + π/2)
}

// Fast sine function for 2π phase input
// Input: phase (0.0 to 2π)
// Output: sine value (-1.0 to 1.0)
inline float fastSin2Pi(float phase) {
    return fastSin(phase / (2.0f * PI));
}

#endif // SIN_TABLE_H