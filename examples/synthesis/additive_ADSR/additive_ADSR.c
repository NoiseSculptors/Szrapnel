
#include "delay.h"
#include "io.h"
#include "rng.h"
#include <stdint.h>

/*
   Additive synthesis demo:
   - 190(!) sine oscillators are summed together
   - frequencies are chosen to form a simple harmonic pattern
   - an ADSR envelope shapes each note
   - stereo width is created with a small channel delay
 */

#define SAMPLE_RATE     48000
#define BUFFER_MS       25
#define N_OSC           190
#define DELAY_SAMPLES   ((SAMPLE_RATE/1000) * 25) // ms
#define ENV_ATTACK_MS   3000.0f
#define ENV_DECAY_MS    1500.0f
#define ENV_RELEASE_MS  5000.0f
#define SINE_LUT_SIZE   8192 // must be power of two
#define SINE_LUT_MASK   (SINE_LUT_SIZE - 1u)
#define TWO_PI          6.283185307179586f

static float phase[N_OSC], freq[N_OSC];
static float env = 0.0f;
static int env_state = 0;
static uint32_t env_pos = 0;
static const float k_rad_to_idx = (float)SINE_LUT_SIZE / TWO_PI;

int32_t delay_buf[DELAY_SAMPLES] __attribute__((section(".dtcm_bss")));
float sin_lut[SINE_LUT_SIZE] __attribute__((section(".dtcm_bss")));
static uint32_t delay_pos = 0;

static float midi_note_to_hz_lut[128] __attribute__((section(".dtcm_bss")));

static void sin_lut_init(void)
{
    const float step = TWO_PI / (float)SINE_LUT_SIZE;
    const float offset = 0.5f * step; // midpoint
    for (uint32_t i = 0; i < SINE_LUT_SIZE; ++i) {
        sin_lut[i] = sinf(offset + step * (float)i);
    }
}

static void midi_lut_init(void)
{
    for (int n = 0; n < 128; n++) {
        midi_note_to_hz_lut[n] = 440.0f * powf(2.0f, (n - 69) / 12.0f);
    }
}

// No-interp lookup (nearest/lower bin). phase ∈ [0, 2π]
static inline float fast_sin(const float phase)
{
    // Assumes 'phase' already wrapped to [0, 2π); avoid fmodf here.
    uint32_t idx = (uint32_t)(phase * k_rad_to_idx);
    idx &= SINE_LUT_MASK; // cheap modulo (size must be power of two)
    return sin_lut[idx];
}

static inline float midi_to_hz(int n)
{
    if (n < 0)   n = 0;
    if (n > 127) n = 127;
    return midi_note_to_hz_lut[n];
}

static inline void make_harmony(void)
{
    int root = 33 + (rng_rnd() % 24);
    float f0 = midi_to_hz(root);

    for (int i = 0; i < N_OSC; i++)
    {
        float f = f0;

        if (i & 1) f *= 1.5f;
        if (i & 2) f *= 1.25f;

        if (rng_rnd() & 1) f *= 0.5f;
        if (rng_rnd() & 2) f *= 2.0f;

        f *= 1.0f + ((int)(rng_rnd() & 255) - 128) * 0.0002f;
        freq[i] = f;
    }
}

static inline float adsr_next(void)
{
    const float atk_n = (ENV_ATTACK_MS  * 0.001f) * (float)SAMPLE_RATE;
    const float dec_n = (ENV_DECAY_MS   * 0.001f) * (float)SAMPLE_RATE;
    const float rel_n = (ENV_RELEASE_MS * 0.001f) * (float)SAMPLE_RATE;
    const float inv_atk_n = 1/atk_n;
    const float inv_dec_n = 1/dec_n;
    const float inv_rel_n = 1/rel_n;

    switch (env_state)
    {
        case 0:
            env = (atk_n > 0.0f) ? ((float)env_pos * inv_atk_n) : 1.0f;
            if (++env_pos >= (uint32_t)atk_n) { env_state = 1; env_pos = 0; }
            break;

        case 1:
            env = 1.0f - (1.0f - 0.7f) * ((dec_n > 0.0f) ? ((float)env_pos * inv_dec_n) : 1.0f);
            if (++env_pos >= (uint32_t)dec_n) { env_state = 2; env_pos = 0; }
            break;

        case 2:
            env = 0.7f * (1.0f - ((rel_n > 0.0f) ? ((float)env_pos * inv_rel_n) : 1.0f));
            if (++env_pos >= (uint32_t)rel_n)
            {
                env = 0.0f;
                env_state = 0;
                env_pos = 0;
                make_harmony();
            }
            break;
    }

    if (env < 0.0f) env = 0.0f;
    if (env > 1.0f) env = 1.0f;
    return env;
}
 __attribute__((section(".itcm")))
void audio_feed(int32_t *audio_buffer, uint32_t samples_in_buffer)
{
    const float amp = 3.0f / (float)N_OSC; /* adjust amplitude here */

    for (uint32_t i = 0; i < samples_in_buffer; i += 2)
    {
        float s = 0.0f;
        float e = adsr_next();

        for (int o = 0; o < N_OSC; o++)
        {
            s += fast_sin(phase[o]);
            phase[o] += 2.0f * PI * freq[o] / (float)SAMPLE_RATE;
            if (phase[o] >= 2.0f * PI) phase[o] -= 2.0f * PI;
        }

        s *= amp * e;

        if (s > 1.0f) s = 1.0f;
        if (s < -1.0f) s = -1.0f;

        int32_t dry = (int32_t)(s * 2147483647.0f);

        int32_t delayed = delay_buf[delay_pos];
        delay_buf[delay_pos] = dry;
        delay_pos++;
        if (delay_pos >= DELAY_SAMPLES) delay_pos = 0;

        audio_buffer[i + 0] = dry;     // L
        audio_buffer[i + 1] = delayed; // R
    }

    leds_VU(audio_buffer[0], audio_buffer[1]);
    lcd_draw_waveform(audio_buffer, samples_in_buffer / 2);
    lcd_flush_fb_async();
}

int main(void)
{
    midi_lut_init();
    sin_lut_init();
    io_init();
    audio_config(SAMPLE_RATE, STEREO, BUFFER_MS);
    init_systick_1ms();
    make_harmony();
    audio_loop_start();
    return 0;
}

