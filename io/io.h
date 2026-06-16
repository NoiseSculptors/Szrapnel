
#ifndef IO_H
#define IO_H

#include "arm_math.h"
#include "io_config.h"
#include <stdint.h>
#include <stddef.h>

void io_buttons_init(void);
void io_dac_init(void);
void io_encoders_init(void);
void io_init(void);
void io_lcd_init(void);
void io_serial_init(void);

void dac_dma_loop(void);
__attribute__((weak)) void audio_fill_buffer(int32_t *dst_interleaved_lr,
                                             uint32_t frames);

typedef struct { uint16_t value;
                 int16_t delta;
                 uint8_t pushed; } io_enc_t;

size_t io_enc_count(void);

io_enc_t io_enc_read(size_t idx);

void drawSoundBuffer(const int32_t *buffer,
                     uint32_t samples,
                     uint16_t fg,
                     uint16_t bg);
void lcd_draw_waveform(const int32_t* audio_buf, int32_t audio_samples);
void lcd_draw_fft(const int32_t *audio_buf, uint32_t num_samples);
void lcd_dma_send(uint8_t *arr, uint16_t n);
void lcd_led_brightness(uint8_t pct);
void lcd_flush_fb(void);
uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b);
int lcd_printf(int x, int y, int xmul, int ymul,
               uint16_t text_color,
               uint16_t bg_color, const char *fmt, ...);
void lcd_pixel(uint8_t x, uint8_t y, uint16_t color);
void lcd_clear(uint16_t color);
void lcd_clear_flush(uint16_t color);
uint16_t *lcd_get_fb(void);

void fb_to_leds(uint8_t *fb);
void led_on(uint8_t led);
void led_off(void);

void hexdump(uint32_t addr, size_t len);

#endif /* IO_H */

