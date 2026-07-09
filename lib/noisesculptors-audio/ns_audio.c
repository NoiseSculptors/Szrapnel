
#include "ns_audio.h"
#include <stdint.h>
#include <math.h>

#define SINE_LUT_MASK   (SINE_LUT_SIZE - 1u)

const float midi_lut[128] = {
    8.175798f,   8.661958f,  9.177023f,  9.722718f, 10.300862f, 10.913382f,
    11.562325f, 12.249859f, 12.978270f, 13.750000f, 14.567620f, 15.433851f,
    16.351597f, 17.323915f, 18.354046f, 19.445436f, 20.601725f, 21.826763f,
    23.124651f, 24.499718f, 25.956539f, 27.500000f, 29.135233f, 30.867710f,
    32.703194f, 34.647827f, 36.708099f, 38.890873f, 41.203442f, 43.653530f,
    46.249302f, 48.999424f, 51.913090f, 55.000000f, 58.270466f, 61.735420f,
    65.406387f, 69.295654f, 73.416199f, 77.781746f, 82.406883f, 87.307060f,
    92.498604f, 97.998848f, 103.826180f, 110.000000f, 116.540947f,
    123.470825f, 130.812775f, 138.591324f, 146.832382f, 155.563492f,
    164.813782f, 174.614120f, 184.997208f, 195.997726f, 207.652344f,
    220.000000f, 233.081863f, 246.941650f, 261.625549f, 277.182648f,
    293.664764f, 311.126984f, 329.627563f, 349.228241f, 369.994415f,
    391.995422f, 415.304688f, 440.000000f, 466.163788f, 493.883301f,
    523.251099f, 554.365295f, 587.329529f, 622.253967f, 659.255127f,
    698.456482f, 739.988831f, 783.990845f, 830.609375f, 880.000000f,
    932.327576f, 987.766602f, 1046.502197f, 1108.730591f, 1174.659058f,
    1244.507935f, 1318.510254f, 1396.912964f, 1479.977661f, 1567.981812f,
    1661.218750f, 1760.000000f, 1864.654907f, 1975.533447f, 2093.004395f,
    2217.460938f, 2349.318359f, 2489.015869f, 2637.020264f, 2793.825928f,
    2959.955322f, 3135.963135f, 3322.437744f, 3520.000000f, 3729.309814f,
    3951.066895f, 4186.008789f, 4434.921875f, 4698.636719f, 4978.031738f,
    5274.040527f, 5587.651855f, 5919.910645f, 6271.926270f, 6644.875488f,
    7040.000000f, 7458.621582f, 7902.131836f, 8372.017578f, 8869.844727f,
    9397.271484f, 9956.063477f, 10548.083008f, 11175.302734f,
    11839.821289f, 12543.855469f
};

float sin_lut[SINE_LUT_SIZE] __attribute__((section(".dtcm_bss")));
static const float k_rad_to_idx = (float)SINE_LUT_SIZE / TWO_PI;

void sin_lut_init(void)
{
    const float step = TWO_PI / (float)SINE_LUT_SIZE;
    const float offset = 0.5f * step; // midpoint
    for (uint32_t i = 0; i < SINE_LUT_SIZE; ++i) {
        sin_lut[i] = sinf(offset + step * (float)i);
    }
}

__attribute__((section(".itcm"),used))
inline float approx_sin(const float phase)
{
    uint32_t idx = (uint32_t)(phase * k_rad_to_idx);
    idx &= SINE_LUT_MASK; // cheap modulo (size must be power of two)
    return sin_lut[idx];
}

__attribute__((section(".itcm"),used))
inline float midi_to_hz(int n)
{
    if (n < 0)   n = 0;
    if (n > 127) n = 127;
    return midi_lut[n];
}

__attribute__((section(".itcm"),used))
inline float approx_rand_float(void)
{
    static uint32_t noise_seed = 0xa34fb438;
    noise_seed ^= noise_seed << 13;
    noise_seed ^= noise_seed >> 17;
    noise_seed ^= noise_seed << 5;
    uint32_t res = (noise_seed & 0x7FFFFF) | 0x40000000;
    float f_res = *((float*)&res);

    return f_res - 3.0f;
}

__attribute__((section(".itcm"),used))
inline uint32_t approx_rand_uint32(void)
{
    static uint32_t xorshift_state = 0xa34fb438;
    xorshift_state ^= xorshift_state << 13;
    xorshift_state ^= xorshift_state >> 17;
    xorshift_state ^= xorshift_state << 5;
    return xorshift_state;
}

inline float map_u32_to_01(uint32_t value, uint32_t high, uint32_t low)
{
    if (value>=high) return 0.0f;
    if (value<=low)  return 1.0f;

    return (float)(high-value)/(float)(high-low);
}

