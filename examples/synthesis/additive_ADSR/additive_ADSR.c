
#include "ns_audio.h"
#include "delay.h"
#include "io.h"
#include "rng.h"
#include <stdint.h>

/*
   Additive synthesis demo:
   - 2100(!) sine oscillators are summed together
   - frequencies are chosen to form a simple harmonic pattern
   - an ADSR envelope shapes each note
   - stereo width is created with a small channel delay
 */

#define SAMPLE_RATE     48000
#define BUFFER_MS       30
#define N_OSC           (2112 + (16 * 0))
#define DELAY_SAMPLES   ((SAMPLE_RATE/1000) * 50) // ms
#define ENV_ATTACK_MS   1000.0f
#define ENV_DECAY_MS    1000.0f
#define ENV_RELEASE_MS  3000.0f
#define TWO_PI          6.283185307179586f
#define LUT_SHIFT       19 // 2^13=8192 (our LUT size), 32-13=19

#if (N_OSC % 16) != 0
#error "N_OSC must be divisible by 16"
#endif

extern float sin_lut[SINE_LUT_SIZE] __attribute__((section(".dtcm_bss")));
int32_t delay_buf[DELAY_SAMPLES] __attribute__((section(".dtcm_bss")));
static float env = 0.0f;
static int env_state = 0;
static uint32_t delay_pos = 0;
static uint32_t env_pos = 0;
static uint32_t incs[N_OSC] __attribute__((section(".dtcm_bss")));
static uint32_t midi_inc[128] __attribute__((section(".dtcm_bss")));
static uint32_t phase[N_OSC] __attribute__((section(".dtcm_bss")));

static void midi_inc_init(void)
{
    for (int n = 0; n < 128; n++)
    {
        float f = midi_to_hz(n);
        midi_inc[n] = (uint32_t)(f * (4294967296.0f / (float)SAMPLE_RATE));
    }
}

__attribute__((section(".itcm"),used))
static inline void make_harmony(void)
{
    static const int8_t offsets[] = { 0, 0, 0, 2, 3, 5, 7, 10, 12, 12, 15, 17, 19, 22, 24 };
    const int root = 30 + (rng_rnd() % 10);
    const int n = (int)(sizeof(offsets) / sizeof(offsets[0]));

    for (int i = 0; i < N_OSC; i++)
    {
        uint32_t r = rng_rnd();
        int note = root + offsets[r % n] + (((r >> 8) & 1) ? 12 : 0);
        if (note > 127) note = 127;
        incs[i] = midi_inc[note];

        phase[i] = 0; // reset phase so every harmony starts like first run
    }
}

__attribute__((section(".itcm"),used))
static inline float adsr_next(void)
{
    const float atk_n = ((ENV_ATTACK_MS  * 0.001f) * (float)SAMPLE_RATE);
    const float dec_n = ((ENV_DECAY_MS   * 0.001f) * (float)SAMPLE_RATE);
    const float rel_n = ((ENV_RELEASE_MS * 0.001f) * (float)SAMPLE_RATE);
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

__attribute__((section(".itcm"),used))
void audio_feed(int32_t *audio_buffer, uint32_t samples_in_buffer)
{
    const float out_gain = 1.0f / (float)N_OSC;
    const uint32_t lfo_inc =
      (uint32_t)(4.0f * (4294967296.0f / (float)SAMPLE_RATE)); /* LFO rate Hz */

    static uint32_t lfo_phase = 0;

    uint32_t dp = delay_pos;

    for (uint32_t i = 0; i < samples_in_buffer; i += 2)
    {
        const float e = adsr_next();
        const float * restrict lut = sin_lut;
        uint32_t * restrict ic = incs;
        uint32_t * restrict ph = phase;

        float a0=0.0f;
        float a1=0.0f;
        float a2=0.0f;
        float a3=0.0f;
        float a4=0.0f;
        float a5=0.0f;
        float a6=0.0f;
        float a7=0.0f;
        float a8=0.0f;
        float a9=0.0f;
        float a10=0.0f;
        float a11=0.0f;
        float a12=0.0f;
        float a13=0.0f;
        float a14=0.0f;
        float a15=0.0f;

        int o = 0;

        for (; o <= (N_OSC-16); o += 16)
        {
            uint32_t p0 = ph[o + 0] + ic[o + 0];
            uint32_t p1 = ph[o + 1] + ic[o + 1];
            uint32_t p2 = ph[o + 2] + ic[o + 2];
            uint32_t p3 = ph[o + 3] + ic[o + 3];
            uint32_t p4 = ph[o + 4] + ic[o + 4];
            uint32_t p5 = ph[o + 5] + ic[o + 5];
            uint32_t p6 = ph[o + 6] + ic[o + 6];
            uint32_t p7 = ph[o + 7] + ic[o + 7];
            uint32_t p8 = ph[o + 8] + ic[o + 8];
            uint32_t p9 = ph[o + 9] + ic[o + 9];
            uint32_t p10 = ph[o + 10] + ic[o + 10];
            uint32_t p11 = ph[o + 11] + ic[o + 11];
            uint32_t p12 = ph[o + 12] + ic[o + 12];
            uint32_t p13 = ph[o + 13] + ic[o + 13];
            uint32_t p14 = ph[o + 14] + ic[o + 14];
            uint32_t p15 = ph[o + 15] + ic[o + 15];

            ph[o + 0] = p0;
            ph[o + 1] = p1;
            ph[o + 2] = p2;
            ph[o + 3] = p3;
            ph[o + 4] = p4;
            ph[o + 5] = p5;
            ph[o + 6] = p6;
            ph[o + 7] = p7;
            ph[o + 8] = p8;
            ph[o + 9] = p9;
            ph[o + 10] = p10;
            ph[o + 11] = p11;
            ph[o + 12] = p12;
            ph[o + 13] = p13;
            ph[o + 14] = p14;
            ph[o + 15] = p15;

            a0 += lut[p0 >> LUT_SHIFT];
            a1 += lut[p1 >> LUT_SHIFT];
            a2 += lut[p2 >> LUT_SHIFT];
            a3 += lut[p3 >> LUT_SHIFT];
            a4 += lut[p4 >> LUT_SHIFT];
            a5 += lut[p5 >> LUT_SHIFT];
            a6 += lut[p6 >> LUT_SHIFT];
            a7 += lut[p7 >> LUT_SHIFT];
            a8 += lut[p8 >> LUT_SHIFT];
            a9 += lut[p9 >> LUT_SHIFT];
            a10 += lut[p10 >> LUT_SHIFT];
            a11 += lut[p11 >> LUT_SHIFT];
            a12 += lut[p12 >> LUT_SHIFT];
            a13 += lut[p13 >> LUT_SHIFT];
            a14 += lut[p14 >> LUT_SHIFT];
            a15 += lut[p15 >> LUT_SHIFT];
        }

        float s = (a0 + a1) + (a2 + a3) + (a4 + a5) + (a6 + a7) +
                  (a8 + a9) + (a10 + a11) + (a12 + a13) + (a14 + a15);

        lfo_phase += lfo_inc;
        const float lfo = 0.65f + 0.35f * lut[lfo_phase >> LUT_SHIFT]; /* 0.3 .. 1.0 */

        s *= (e * out_gain * lfo);

        if (s > 0.999f) s = 0.999f;
        if (s < -0.999f) s = -0.999f;

        const int32_t dry = (int32_t)(s * 2147483647.0f);

        const int32_t delayed = delay_buf[dp];
        delay_buf[dp] = dry;
        dp++;
        if (dp >= DELAY_SAMPLES) dp = 0;

        audio_buffer[i + 0] = dry;
        audio_buffer[i + 1] = delayed;
    }

    delay_pos = dp;

    leds_VU(audio_buffer[0], audio_buffer[1]);
    lcd_draw_waveform(audio_buffer, samples_in_buffer / 2);
    lcd_flush_fb_async();
}

int main(void)
{
    io_init();
    sin_lut_init();
    midi_inc_init();
    init_systick_1ms();
    audio_config(SAMPLE_RATE, STEREO, BUFFER_MS);
    make_harmony();
    audio_loop_start();
    return 0;
}

