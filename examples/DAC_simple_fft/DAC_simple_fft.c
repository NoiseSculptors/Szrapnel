
#include "arm_math.h"
#include "gpio.h"
#include "io.h"
#include "rng.h"
#include <stdint.h>

#define SAMPLE_RATE       384000
#define BUFFER_MS         25
#define FREQ_HZ_L         440.0f
#define FREQ_HZ_R_MIN     20.0f
#define FREQ_HZ_R_MAX     20000.0f
#define FREQ_STEP_R       10.0f

/*
Generates stereo audio into an int32_t buffer for I2S/DAC output.

Left channel: 440 Hz sine wave.
Right channel: sine sweep from 20 Hz to 20 kHz.

Press buttons:
Button 25 replaces left channel with random noise.
Button 26 replaces right channel with random noise.
*/

static arm_rfft_fast_instance_f32 fft_inst;
static float32_t current_freq_R = FREQ_HZ_R_MIN;
static float32_t phaseL = 0.0f;
static float32_t phaseR = 0.0f;
static float32_t phaseStepL = 2.0f * PI * FREQ_HZ_L / (float)SAMPLE_RATE;
static uint32_t fft_size = 2048;
static uint8_t fft_initialized = 0;
extern uint16_t fb[WIDTH*HEIGHT];

static inline float32_t q31_to_f32(int32_t x)
{
    return (float32_t)x * (1.0f / 2147483648.0f);
}

static void draw_fft_channel(const int32_t *audio_buf,
                             uint32_t frames,
                             uint32_t fft_size,
                             uint32_t channel,
                             int y_start,
                             int y_height,
                             uint16_t fg)
{
    const float sample_rate = get_sample_rate();
    float fft_in[fft_size];
    float fft_out[fft_size];
    float mag[fft_size / 2 + 1];

    // Fill input for this channel
    for (uint32_t i = 0; i < fft_size; i++) {
        if (i < frames) {
            fft_in[i] = q31_to_f32(audio_buf[2 * i + channel]);
        } else {
            fft_in[i] = 0.0f;
        }
    }

    arm_rfft_fast_f32(&fft_inst, fft_in, fft_out, 0);

    // Power spectrum
    mag[0] = fft_out[0] * fft_out[0];
    mag[fft_size / 2] = fft_out[1] * fft_out[1];

    float32_t max_mag = 1e-12f;
    for (uint32_t k = 1; k < fft_size/ 2; k++) {
        float32_t re = fft_out[2 * k];
        float32_t im = fft_out[2 * k + 1];
        mag[k] = re * re + im * im;
        if (mag[k] > max_mag) max_mag = mag[k];
    }

    // Draw bars
    const float32_t f_min = 20.0f;
    float32_t f_max = 20000.0f;
    float32_t nyquist = 0.5f * sample_rate;
    if (f_max > nyquist) f_max = nyquist;

    for (int x = 0; x < WIDTH; x++) {
        float32_t t = (WIDTH <= 1) ? 0.0f : (float32_t)x / (float32_t)(WIDTH - 1);
        float32_t f = f_min + t * (f_max - f_min);

        uint32_t bin = (uint32_t)(f * fft_size / sample_rate);
        if (bin > fft_size / 2) bin = fft_size / 2;

        int h = (int)((mag[bin] / max_mag) * (y_height - 1));

        if (h < 0) h = 0;
        if (h > y_height - 1) h = y_height - 1;

        for (int y = 0; y < h; y++) {
            fb[(y_start + (y_height - 1 - y)) * WIDTH + x] = fg;
        }
    }
}

static void lcd_draw_fft(const int32_t *audio_buf, uint32_t num_samples, uint32_t fft_size)
{
    if (!fft_initialized) {
        if (arm_rfft_fast_init_f32(&fft_inst, fft_size) != ARM_MATH_SUCCESS) {
            return;
        }
        fft_initialized = 1;
    }

    uint32_t frames = num_samples / 2;
    if (frames > fft_size) frames = fft_size;

    uint16_t fg = rgb565(255, 255, 0);
    const uint16_t bg = rgb565(0, 64, 0);

    lcd_clear(bg);

    draw_fft_channel(audio_buf, frames, fft_size, 0, 0, HEIGHT / 2, fg); // left
    fg = rgb565(0,255,255);
    draw_fft_channel(audio_buf, frames, fft_size, 1, HEIGHT / 2, HEIGHT / 2, fg); // right
}

void audio_feed(int32_t *audio_buffer, uint32_t samples_in_buffer)
{
    float32_t phaseStepR = 2.0f * PI * current_freq_R / (float)SAMPLE_RATE;

    for (uint32_t i = 0; i < samples_in_buffer; i += 2) {
        phaseL += phaseStepL;
        phaseR += phaseStepR;

        if (phaseL > 2.0f * PI) phaseL -= 2.0f * PI;
        if (phaseR > 2.0f * PI) phaseR -= 2.0f * PI;

        int32_t L = (int32_t)(arm_sin_f32(phaseL) * (2147483647.0f/2.0f));
        int32_t R = (int32_t)(arm_sin_f32(phaseR) * (2147483647.0f/2.0f));

        if (*GPIOC_IDR & (1 << 7)) L = rng_rnd();
        if (*GPIOA_IDR & (1 << 9)) R = rng_rnd();

        audio_buffer[i]   = L;
        audio_buffer[i+1] = R;
    }

    current_freq_R += FREQ_STEP_R;
    if (current_freq_R > FREQ_HZ_R_MAX) {
        current_freq_R = FREQ_HZ_R_MIN;
    }

    lcd_draw_fft(audio_buffer, samples_in_buffer, fft_size);
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
    audio_config(SAMPLE_RATE, STEREO, BUFFER_MS);
    audio_loop_start();
    return 0;
}

