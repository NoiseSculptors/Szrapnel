
#ifndef USER_IO_H
#define USER_IO_H

#include <stdint.h>
#include <stddef.h>

#define WIDTH   160 
#define HEIGHT  80

void io_init(void);
void io_buttons_init(void);
void io_encoders_init(void);
void io_serial_init(void);

typedef struct { uint16_t value;
                 int16_t delta;
                 uint8_t pushed; } io_enc_sample_t;

size_t io_enc_count(void);

io_enc_sample_t io_enc_read(size_t idx);

void lcd_dma_send(uint8_t *arr, uint16_t n);
void lcd_init(void);
void lcd_led_brightness(uint8_t pct);
void lcd_flush_fb(void);
uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b);
//int lcd_printf(int x, int y, uint16_t text_color, uint16_t bg_color, const char *fmt, ...);
int lcd_printf(int x, int y, int xmul, int ymul,
               uint16_t text_color,
               uint16_t bg_color, const char *fmt, ...);
void lcd_pixel(uint8_t x, uint8_t y, uint16_t color);
void lcd_clear(uint16_t color);
void lcd_clear_flush(uint16_t color);

void fb_to_leds(uint8_t *fb);
void led_on(uint8_t led);
void led_off(void);

void hexdump(uint32_t addr, size_t len);

#endif /* USER_IO_H */

