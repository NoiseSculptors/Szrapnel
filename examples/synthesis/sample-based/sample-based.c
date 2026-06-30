
#include "drums.h"
#include "delay.h"
#include "init.h"
#include "printf.h"
#include "io.h"

/*
   Warning:
   This patch can be loud — please start with your volume low.
  
   Controls:
     - Left encoder: adjusts sample shift
         • changes loudness
         • also affects bit depth / crunch
  
     - Right encoder: adjusts playback speed
         • 1 = normal speed
         • 2 = 2x speed
         • higher values = faster playback
  
   Notes:
     - The top-left display shows the current values.
     - For a normal starting point, try:
         • speed = 01
         • shift = 16
  
     test different values, such as 09 24, or tune shift = 29 then
     adjust speed from 1 to higher values
  
     sampling rate should be adjusted to 48kHz, but you can play with other
      values also, but then you have to lower delay samples, for 384kHz * 2
  
     - Output is limited to 70%, but it can still be loud.
 */

#define SAMPLE_RATE     384000
#define BUFFER_MS       25
#define DELAY_MS        50
#define DELAY_SAMPLES   ((SAMPLE_RATE/1000) * DELAY_MS)
#define Q31(x) ((int32_t)((x) * 2147483647.0f))
#define LIMIT Q31(0.7f)

/* L/R delay for width */
static int32_t delay_buf[DELAY_SAMPLES];
static uint32_t delay_pos = 0;

const uint32_t total_samples = sizeof(drums) / sizeof(drums[0]);
io_enc_t enc0, enc1;

void audio_feed(int32_t *audio_buffer, uint32_t samples_in_buffer)
{
    static uint32_t current_sample = 0;
    static int32_t wait = 3, wait_set = 1, shift = 16;

    enc0 = io_enc_read(0);
    enc1 = io_enc_read(1);

    wait_set += enc0.delta;
    if (wait_set < 1) wait_set = 1;

    shift += enc1.delta;
    if (shift < 1) shift = 1;
    if (shift > 31) shift = 31;

    for (uint32_t i = 0; i < samples_in_buffer; i += 2)
    {
        if (current_sample >= total_samples)
            current_sample = 0;

        q31_t s = drums[current_sample];
        q31_t dry = s * (1 << shift);

        if (dry > LIMIT) dry = LIMIT;
        if (dry < -LIMIT) dry = -LIMIT;

        int32_t delayed = delay_buf[delay_pos];
        delay_buf[delay_pos++] = dry;
        if (delay_pos >= DELAY_SAMPLES) delay_pos = 0;

        audio_buffer[i]   = dry;
        audio_buffer[i+1] = delayed;

        if (--wait <= 0)
        {
            current_sample++;
            wait = wait_set;
        }
    }

    lcd_draw_waveform(audio_buffer, SAMPLE_RATE/(BUFFER_MS*4));
    lcd_printf(0,0,1,1,0xffff,0,"%02d\n%02d", wait_set, shift);
    lcd_flush_fb();
}

int main(void)
{
    io_init();
    audio_config(SAMPLE_RATE, STEREO, BUFFER_MS);
    audio_loop_start();
    return 0;
}

