
#include "delay.h"
#include "dma.h"
#include "dmamux.h"
#include "gpio.h"
#include "i2c.h"
#include "nvic.h"
#include "printf.h"
#include "rcc.h"
#include "rng.h"
#include "rng.h"
#include "spi.h"
#include "serial.h"
#include "syscall.h"
#include "systick.h"
#include "timer.h"
#include "io.h"
#include <string.h>

uint32_t leds;

/*
 charlieplexed led
 line | GPIO 
 :---:|------
   0  |  D0 
   1  |  D1
   2  |  D2
   3  |  D3
   4  |  D4
   5  |  D5
 */


/* PORTD is also used for some buttons (configured as 0-input) and D8,D9,D10
   configured as outputs, for the LCD display. */

uint32_t lookup_moder[] __attribute__((section(".dtcm_data"))) = {
    0x150ff5, 0x150ff5, 0x150fdd, 0x150f7d, 0x150dfd, 0x1507fd, 0x150fdd,
    0x150fd7, 0x150fd7, 0x150f77, 0x150df7, 0x1507f7, 0x150fbd, 0x150f77,
    0x150f5f, 0x150f5f, 0x150ddf, 0x1507df, 0x150dfd, 0x150df7, 0x150ddf,
    0x150d7f, 0x150d7f, 0x15077f, 0x1507fd, 0x1507f7, 0x1507df, 0x15077f,
    0x1505ff, 0x1505ff,
};

uint32_t lookup_bsrr[] __attribute__((section(".dtcm_data"))) = {
    0x02003d, 0x01003e, 0x01003e, 0x01003e, 0x01003e, 0x01003e, 0x04003b,
    0x04003b, 0x02003d, 0x02003d, 0x02003d, 0x02003d, 0x080037, 0x080037,
    0x080037, 0x04003b, 0x04003b, 0x04003b, 0x10002f, 0x10002f, 0x10002f,
    0x10002f, 0x080037, 0x080037, 0x20001f, 0x20001f, 0x20001f, 0x20001f,
    0x20001f, 0x10002f,
};

/* only through SysTick_Handler */
void fb_to_leds(uint8_t *led_fb){
    leds = (led_fb[0]|
            led_fb[1]<<6*1|
            led_fb[2]<<6*2|
            led_fb[3]<<6*3|
            led_fb[4]<<6*4|
            led_fb[5]<<6*5);
}

/* turns on a single led without SysTick_Handler */
void led_on(uint8_t led)
{
    *GPIOD_MODER = lookup_moder[led];
    *GPIOD_BSRR = lookup_bsrr[led];
}

/* turns off all leds without SysTick_Handler */
void led_off(void)
{
    *GPIOD_BSRR = 0x3F0000;
}

inline void leds_VU(int32_t L, int32_t R)
{
    int32_t mono = (L+R) >> 1;
    uint32_t abs_mono = (mono < 0) ? -mono : mono;
    uint8_t leds_on  = (uint8_t)((abs_mono * 60ULL) >> 31);
    leds = (1 << leds_on)-1;
}

