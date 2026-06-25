
#include "io.h"
#include "gpio.h"
#include <stdint.h>

/*
   This is a simple example of fm synthesis. 
   left encoder + button 25 = carrier frequency
   left encoder = modulation frequency
   right encoder + button 26 = modulation depth multiplier
   right encoder = modulation depth 
 */

#define TWO_PI 6.283185307179586f
#define BTN25 (*GPIOC_IDR & (1<<7))
#define BTN26 (*GPIOA_IDR & (1<<9))

static float mod_phase = 0.0f, car_phase = 0.0f;

static float car_freq  = 220.0f;
static float mod_ratio = 3.5f;
static float mod_index = 5.0f;
static float mod_multiplier = 0.01f;
static float volume    = 0.9f;

static inline float wrap_two_pi(float x){
    x -= TWO_PI * (int)(x / TWO_PI);
    if (x < 0.0f) x += TWO_PI;
    return x;
}

static void read_input(void){
    io_enc_t e0 = io_enc_read(0);
    io_enc_t e1 = io_enc_read(1);
    if(BTN25)
        car_freq += (float)e0.delta * 8.0f;
    else
        mod_index += (float)e0.delta * 8.0f;
    if(BTN26)
        mod_multiplier += (float)e1.delta * 0.0005f;
    else
        mod_ratio += (float)e1.delta * mod_multiplier;
    if(car_freq < 0.0f) car_freq = 0.0f;
    if(mod_ratio < 0.0f) mod_ratio = 0.0f;
    if(mod_index < 0.0f) mod_index = 0.0f;
    if(mod_multiplier < 0.0005f) mod_multiplier = 0.0005f;
}

static float limit(float sample)
{
    if (sample > 0.9f)  sample = 0.9f;
    if (sample < -0.9f) sample = -0.9f;
    return sample;
}

static void voice_fill(int32_t *buf, uint32_t samples){
    const float inv_sr = 1.0f / SAMPLE_RATE;

    const float mod_freq       = car_freq * mod_ratio;
    const float mod_step       = mod_freq * TWO_PI * inv_sr;
    const float car_step_base  = car_freq * TWO_PI * inv_sr;

    for (uint32_t i = 0; i < samples; i += 2){
        float mod_sig = arm_sin_f32(mod_phase);
        mod_phase = wrap_two_pi(mod_phase + mod_step);

        float modulated_step = car_step_base + (mod_sig * mod_index * TWO_PI * inv_sr);
        float car_sig = arm_sin_f32(car_phase);
        car_phase = wrap_two_pi(car_phase + modulated_step);

        float s = car_sig * volume;

        int32_t q31 = (int32_t)(limit(s) * Q31_MAX);
        buf[i] = q31;
        buf[i + 1] = q31;
    }
}

void audio_feed(int32_t *stereo_buffer, uint32_t samples){
    read_input();
    voice_fill(stereo_buffer, samples);

    lcd_draw_waveform(stereo_buffer, 2400);
    lcd_printf(0,0,1,1,0xffff,0,"%.2f\n%.2f\n%.3f x %.3f",
            (double)car_freq,
            (double)mod_index,
            (double)mod_ratio,
            (double)mod_multiplier);
    lcd_flush_fb();
}

int main(void){
    io_init();
    audio_loop_start();
    return 0;
}
