
#include "printf.h"
#include "rng.h"
#include "io.h"

#include <stdint.h>
#include <limits.h>
#include <math.h>

#define DEMO                      1u

#define INITIAL_SAMPLE_RATE       384000u
#define BUFFER_MS                 25u

#define FREQUENCY_MIN_HZ          1u
#define FREQUENCY_MAX_HZ          96000u
#define FREQUENCY_INITIAL_HZ      1000u

#define TWO_PI_F                  6.28318530717958647692f
#define PHASE_TO_FLOAT            (1.0f / 4294967296.0f)
#define UINT32_TO_FLOAT           (1.0f / 4294967295.0f)
#define INT32_FULL_SCALE_F        2147483648.0f

#define WAVEFORM_COUNT            5u
#define SAMPLE_RATE_COUNT         6u

#define RPT_DELAY1                100u
#define RPT_DELAYN                50u
#define NO_BUTTONS                30u

enum
{
    WAVE_SQUARE = 0u,
    WAVE_SAW,
    WAVE_TRIANGLE,
    WAVE_SINE,
    WAVE_NOISE
};

enum
{
    SR_16000 = 0u,
    SR_32000,
    SR_48000,
    SR_96000,
    SR_192000,
    SR_384000
};

static const uint32_t sample_rates[SAMPLE_RATE_COUNT] =
{
    16000u,
    32000u,
    48000u,
    96000u,
    192000u,
    384000u
};

static const char *const waveform_names[WAVEFORM_COUNT] =
{
    "SQR",
    "SAW",
    "TRI",
    "SIN",
    "NSE"
};

static uint32_t selected_waveform = WAVE_SINE;
static uint32_t selected_sample_rate = SAMPLE_RATE_COUNT - 1u;
static uint32_t sample_rate = INITIAL_SAMPLE_RATE;
static uint32_t frequency_hz = FREQUENCY_INITIAL_HZ;
static uint32_t phase, phase_increment;
static uint16_t active_color, inactive_color, fg, bg;

void settings_init(void)
{
    sample_rate = INITIAL_SAMPLE_RATE;
    selected_sample_rate = SAMPLE_RATE_COUNT - 1u;
    frequency_hz = FREQUENCY_INITIAL_HZ;
    selected_waveform = WAVE_SINE;
    active_color = 0xffff;
    inactive_color = 0;
    fg = rgb565(255u, 255u, 255u);
    bg = rgb565(63u, 127u, 0u);
    phase = 0u;
}

static inline void update_phase_increment(void)
{
    phase_increment = (uint32_t)
    (
        ((uint64_t)frequency_hz << 32) / (uint64_t)sample_rate
    );
}

static inline int32_t float_to_pcm32(const float value)
{
    int64_t pcm;

    if (value >= 1.0f)
        return INT32_MAX;

    if (value <= -1.0f)
        return INT32_MIN;

    pcm = (int64_t)(value * INT32_FULL_SCALE_F);

    if (pcm > INT32_MAX)
        return INT32_MAX;

    if (pcm < INT32_MIN)
        return INT32_MIN;

    return (int32_t)pcm;
}

static inline float next_audio_value(void)
{
    const uint32_t current_phase = phase;
    const float normalized_phase =
        (float)current_phase * PHASE_TO_FLOAT;

    float value;

    switch (selected_waveform)
    {
        case WAVE_SQUARE:
            value = (current_phase < 0x80000000u) ? 1.0f : -1.0f;
            break;

        case WAVE_SAW:
            value = (normalized_phase * 2.0f) - 1.0f;
            break;

        case WAVE_TRIANGLE:
            value = 1.0f - (4.0f * fabsf(normalized_phase - 0.5f));
            break;

        case WAVE_SINE:
            value = sinf(normalized_phase * TWO_PI_F);
            break;

        default:
        {
            const float random_value =
                (float)rng_rnd() * UINT32_TO_FLOAT;

            value = (random_value * 2.0f) - 1.0f;
            break;
        }
    }

    phase += phase_increment;

    return value;
}

void audio_feed(int32_t *restrict stereo_buffer, uint32_t samples)
{
    for (uint32_t i = 0u; i < samples; i += 2u)
    {
        const int32_t value = float_to_pcm32(next_audio_value());

        stereo_buffer[i] = value;
        stereo_buffer[i + 1u] = value;
    }
}

static inline void draw_waveforms(void)
{
    for (uint32_t i = 0u; i < WAVEFORM_COUNT; ++i) {
        lcd_printf( (int)(i * 32u)+4, 4, 1, 2,
            (i == selected_waveform) ?
                active_color : inactive_color, bg, "%s", waveform_names[i]);
    }
}

static inline void draw_sample_rates(void)
{
    for (uint32_t i = 0u; i < SAMPLE_RATE_COUNT; ++i) {
        lcd_printf( i==5?(int)(i * 24u)+12u:(int)(i * 24u)+4, 28, 1, 2,
            (i == selected_sample_rate) ?  active_color : inactive_color,
            bg, "%d", sample_rates[i] / 1000u);
    }
}

static inline void draw_frequency(void)
{
    if (selected_waveform == WAVE_NOISE)
        lcd_printf( 10, 56, 2, 2, fg, bg, "    Noise");
    else
        lcd_printf( 10, 56, 2, 2, fg, bg, "%6d Hz", frequency_hz);
}

static inline void draw_user_interface(void)
{
    draw_waveforms();
    draw_sample_rates();
    draw_frequency();
    lcd_flush_fb();
}

static inline void select_previous_waveform(void)
{
    if (selected_waveform == 0u)
        selected_waveform = WAVEFORM_COUNT - 1u;
    else
        --selected_waveform;
}

static inline void select_next_waveform(void)
{
    ++selected_waveform;

    if (selected_waveform >= WAVEFORM_COUNT)
        selected_waveform = 0u;
}

static inline void set_frequency(uint32_t freq)
{
    frequency_hz = freq;
    update_phase_increment();
}

static inline void change_frequency(int32_t amount)
{
    int32_t new_frequency = (int32_t)frequency_hz + amount;

    if (new_frequency < (int32_t)FREQUENCY_MIN_HZ)
        new_frequency = (int32_t)FREQUENCY_MIN_HZ;

    if (new_frequency > (int32_t)FREQUENCY_MAX_HZ)
        new_frequency = (int32_t)FREQUENCY_MAX_HZ;

    set_frequency((uint32_t)new_frequency);
}

static inline void set_sample_rate(uint32_t sr)
{
    sample_rate = sr;
    audio_config(sr, STEREO, BUFFER_MS);
    update_phase_increment();
}

static inline void change_sample_rate(int32_t amount)
{

    int32_t new_index = (int32_t)selected_sample_rate + amount;

    if (new_index < 0)
        new_index = 0;

    if (new_index >= (int32_t)SAMPLE_RATE_COUNT)
        new_index = (int32_t)SAMPLE_RATE_COUNT - 1;

    selected_sample_rate = (uint32_t)new_index;
    sample_rate = sample_rates[selected_sample_rate];

    set_sample_rate(sample_rate);
}

static uint32_t btn_rpt(uint32_t btn)
{
    static uint32_t rpt_slots[NO_BUTTONS];

    uint32_t btn_value = button(btn);

    if(btn_value == 0){
        rpt_slots[btn] = 0;
        return 0;
    } else if(btn_value && rpt_slots[btn] == 0){
        rpt_slots[btn]++;
        return 1;
    } else if(btn_value && rpt_slots[btn] < RPT_DELAY1){
        rpt_slots[btn]++;
        return 0;
    } else if(btn_value && rpt_slots[btn] == RPT_DELAY1){
        rpt_slots[btn]++;
        return 1;
    } else if(btn_value && rpt_slots[btn] < RPT_DELAYN + RPT_DELAY1){
        rpt_slots[btn]++;
        return 0;
    } else if(btn_value && rpt_slots[btn] == RPT_DELAYN + RPT_DELAY1){
        rpt_slots[btn] = RPT_DELAY1 + 1;
        return 1; 
    }

    return 0;
}

static inline void service_buttons(void)
{
    if (btn_rpt(24u)) {select_previous_waveform();}
    if (btn_rpt(18u)) {select_next_waveform();}

    if (btn_rpt(19u)) {change_sample_rate(1);}
    if (btn_rpt(25u)) {change_sample_rate(-1);}

    if (selected_waveform != WAVE_NOISE){
        if (btn_rpt(20u)) {change_frequency(1000);}
        if (btn_rpt(26u)) {change_frequency(-1000);}

        if (btn_rpt(21u)) {change_frequency(100);}
        if (btn_rpt(27u)) {change_frequency(-100);}

        if (btn_rpt(22u)) {change_frequency(10);}
        if (btn_rpt(28u)) {change_frequency(-10);}

        if (btn_rpt(23u)) {change_frequency(1);}
        if (btn_rpt(29u)) {change_frequency(-1);}
    }
}

void service_demo(void)
{
#define TIMER_TRIG 1000U
    const uint32_t demo_freqs[] = { /* 200u, 500u, 800u, 1000u, 2000u, 4000u, 8000u,*/ 16000u, 19200u, 24000u };
    static uint8_t demo_init, demo_freqs_idx, demo_freqs_count;
    static uint32_t timer; 

    if(demo_init == 0){

        demo_freqs_idx = 0;
        demo_freqs_count = sizeof(demo_freqs)/sizeof(demo_freqs[0]);
        selected_sample_rate = SR_16000;
        selected_waveform = WAVE_SQUARE;

        set_frequency(demo_freqs[demo_freqs_idx]);
        set_sample_rate(sample_rates[selected_sample_rate]);

        demo_init = 1;

        return;
    }

    if(timer++ >= TIMER_TRIG){

        if(selected_sample_rate < SAMPLE_RATE_COUNT-1u){
            selected_sample_rate++;
        } else {
            selected_sample_rate = 0u;
            if(selected_waveform < WAVEFORM_COUNT-2u) // skip WAVE_NOISE
                selected_waveform++;
            else {
                selected_waveform = 0u;
                if(demo_freqs_idx < demo_freqs_count-1){
                    demo_freqs_idx++;
                } else{
                    demo_freqs_idx = 0;
                }
                set_frequency(demo_freqs[demo_freqs_idx]);
            }
        }

        set_sample_rate(sample_rates[selected_sample_rate]);

        timer = 0u;
    }
}


int main(void)
{
    io_init();
    settings_init();
    update_phase_increment();
    audio_config(sample_rate, STEREO, BUFFER_MS);
    lcd_clear(bg);
    draw_user_interface();

    for (;;)
    {
        audio_service();
#if DEMO
        service_demo();
#else
        service_buttons();
#endif
        draw_user_interface();
    }

    return 0;
}

