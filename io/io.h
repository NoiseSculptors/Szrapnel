
#ifndef IO_H
#define IO_H

#include "rng.h"
#include "io_config.h"
#include "systick.h"
#include <stdint.h>
#include <stddef.h>

#define BLACK 0
#define WHITE 0xffff

void io_buttons_init(void);
void io_encoders_init(void);
void io_init(void);
void io_lcd_init(void);
void io_serial_init(void);

struct audio_cfg {
    uint32_t sample_rate;
    uint8_t  channels;
    uint16_t samples;
};

float get_sample_rate(void);
struct audio_cfg* get_audio_cfg(void);
void audio_loop_start(void);
void audio_config(uint32_t s_freq, uint8_t ch, uint16_t buf_ms);

__attribute__((weak))
void audio_feed(int32_t *dst_interleaved_lr, uint32_t frames);

__attribute__((weak)) void user_systick(void);

typedef struct { uint16_t value;
                 int16_t delta;
                 uint8_t pushed; } io_enc_t;

size_t io_enc_count(void);

io_enc_t io_enc_read(size_t idx);

uint32_t button_pressed(void);
uint32_t button_side_pressed(void);
uint8_t button(int i);
uint8_t button_side(int i);

void lcd_draw_waveform(const int32_t* audio_buf, int32_t audio_samples);
void lcd_dma_send(uint8_t *arr, uint16_t n);
void lcd_led_brightness(uint8_t pct);
void lcd_flush_fb(void);
void lcd_flush_fb_async(void);
uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b);
int lcd_printf(int x, int y, int xmul, int ymul,
               uint16_t text_color,
               uint16_t bg_color, const char *fmt, ...);
void lcd_clear(uint16_t color);
uint16_t *lcd_get_fb(void);
void lcd_pixel(int x, int y, uint16_t c);
void lcd_hline(int x0, int x1, int y, uint16_t c);
void lcd_vline(int x, int y0, int y1, uint16_t c);
void lcd_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t c);
void lcd_box_filled(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t c);
void lcd_box(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t c);
void lcd_circle(uint8_t x0, uint8_t y0, uint8_t r, uint16_t c);
void lcd_circle_filled(uint8_t x0, uint8_t y0, uint8_t r, uint16_t c);

void fb_to_leds(uint8_t *fb);
void led_on(uint8_t led);
void led_off(void);
void leds_VU(int32_t L, int32_t R);

void hexdump(uint32_t addr, size_t len);

#endif /* IO_H */

