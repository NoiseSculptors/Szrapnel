
#ifndef __NS_AUDIO__ 
#define __NS_AUDIO__ 

#include <stdint.h>

#define SINE_LUT_SIZE   8192 // must be power of two
#define TWO_PI   6.283185307179586f

float approx_rand_float(void);
float approx_sin(const float phase);
float midi_to_hz(int n);
float map_u32_to_01(uint32_t value, uint32_t high, uint32_t low);
uint32_t approx_rand_uint32(void);
void sin_lut_init(void);

#endif

