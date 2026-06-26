#include "rng.h"
#include "io.h"
#include <stdint.h>

#define MAX_AMP 2147483647.0f

typedef struct {
    float freq;
    float amp;
    float phase;
} osc_t;

static int32_t square(osc_t *osc)
{
    osc->phase += osc->freq / SAMPLE_RATE;

    if (osc->phase >= 1.0f)
        osc->phase -= 1.0f;

    return (osc->phase < 0.5f)
        ? (int32_t)(osc->amp * MAX_AMP)
        : (int32_t)(-osc->amp * MAX_AMP);
}

void audio_feed(int32_t *stereo_buffer, uint32_t samples)
{
    static osc_t oscL = {440.0f, 0.9f, 0.0f};
    static osc_t oscR = {880.0f, 0.9f, 0.0f};

    for (uint32_t i = 0; i < samples; i += 2) {
        stereo_buffer[i + 0] = square(&oscL);
        stereo_buffer[i + 1] = square(&oscR);
    }

    lcd_draw_waveform(stereo_buffer, 2000);
    lcd_flush_fb();
}

int main(void)
{
    io_init();
    audio_loop_start();

    return 0;
}
