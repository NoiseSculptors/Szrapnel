
#include "rng.h"
#include "io.h"
#include <stdint.h>

#define SAMPLE_RATE    384000
#define BUFFER_MS      25
#define MAX_AMP 2147483647.0f

typedef struct {
    float freq;
    float amp;
    float phase;
} osc_t;

static int32_t triangle(osc_t *osc)
{
    osc->phase += osc->freq / (float)SAMPLE_RATE;

    if (osc->phase >= 1.0f)
        osc->phase -= 1.0f;

    float tri = (osc->phase<0.5f)?(osc->phase*4.0f-1.0f):(3.0f-osc->phase*4.0f);

    return (int32_t)(tri * osc->amp * MAX_AMP);
}

void audio_feed(int32_t *stereo_buffer, uint32_t samples)
{
    static osc_t oscL = {440.0f, 0.9f, 0.0f};
    static osc_t oscR = {880.3f, 0.9f, 0.0f};

    for (uint32_t i = 0; i < samples; i += 2) {
        stereo_buffer[i + 0] = triangle(&oscL);
        stereo_buffer[i + 1] = triangle(&oscR);
    }

    lcd_draw_waveform(stereo_buffer, SAMPLE_RATE/(BUFFER_MS*8));
    lcd_flush_fb();
}

int main(void)
{
    io_init();
    audio_config(SAMPLE_RATE, STEREO, BUFFER_MS);
    audio_loop_start();

    return 0;
}

