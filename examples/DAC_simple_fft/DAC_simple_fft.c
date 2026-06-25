
#include "arm_math.h"
#include "gpio.h"
#include "io.h"
#include "rng.h"
#include <stdint.h>

#define FREQ_HZ_L         440.0f
#define FREQ_HZ_R_MIN     20.0f
#define FREQ_HZ_R_MAX     20000.0f
#define FREQ_STEP_R       50.0f

/*
Generates stereo audio into an int32_t buffer for I2S/DAC output.

Left channel: 440 Hz sine wave.
Right channel: sine sweep from 20 Hz to 20 kHz.

Key 25 replaces left channel with random noise.
Key 26 replaces right channel with random noise.
*/

static float32_t phaseL = 0.0f;
static float32_t phaseR = 0.0f;
static float32_t phaseStepL = 2.0f * PI * FREQ_HZ_L / SAMPLE_RATE;
static float32_t current_freq_R = FREQ_HZ_R_MIN;

void audio_feed(int32_t *audio_buffer, uint32_t samples_in_buffer)
{
    float32_t phaseStepR = 2.0f * PI * current_freq_R / SAMPLE_RATE;

    for (uint32_t i = 0; i < samples_in_buffer; i += 2) {
        phaseL += phaseStepL;
        phaseR += phaseStepR;

        if (phaseL > 2.0f * PI) phaseL -= 2.0f * PI;
        if (phaseR > 2.0f * PI) phaseR -= 2.0f * PI;

        int32_t L = (int32_t)(arm_sin_f32(phaseL) * 2147483647.0f);
        int32_t R = (int32_t)(arm_sin_f32(phaseR) * 2147483647.0f);

        if (*GPIOC_IDR & (1 << 7)) L = rng_rnd();
        if (*GPIOA_IDR & (1 << 9)) R = rng_rnd();

        audio_buffer[i]   = L;
        audio_buffer[i+1] = R;
    }

    current_freq_R += FREQ_STEP_R;
    if (current_freq_R > FREQ_HZ_R_MAX) {
        current_freq_R = FREQ_HZ_R_MIN;
    }

    lcd_draw_fft(audio_buffer, samples_in_buffer);
    if (*GPIOC_IDR & (1 << 7))
       lcd_printf(85,5,0,0,0xffff,0,"L:Noise");
    else
       lcd_printf(85,5,0,0,0xffff,0,"L:%.0fHz",(double)FREQ_HZ_L);
    if (*GPIOA_IDR & (1 << 9))
        lcd_printf(85,13,0,0,0xffff,0,"R:Noise");
    else
        lcd_printf(85,13,0,0,0xffff,0,"R:%.0fHz",(double)current_freq_R);
    lcd_flush_fb();
}

int main(void)
{
    io_init();
    audio_start();
    return 0;
}

