
#include "delay.h"
#include "init.h"
#include "rcc.h"
#include "io.h"

static void init_hw(void)
{
	init_clock();
    io_lcd_init();
    /* adjusting backlight */
    lcd_led_brightness(100);
}

int main(void){

	init_hw();

    for(int y=0;y<80;y++) {
    int zone=y/20;

    for(int x=0;x<160;x++) {
            uint8_t val=(x*255)/159;
            uint8_t r=0,g=0,b=0;
            switch(zone) {
                case 0: r=val; break;
                case 1: g=val; break;
                case 2: b=val; break;
                case 3: r=g=b=val; break;
            }

            lcd_pixel(x,y,rgb565(r,g,b));
        }
    }

    lcd_flush_fb();

    while(1){
        for(int i=0;i<=100;i++){
            lcd_led_brightness(i);
            delay_ms(25);
        }
        for(int i=100;i>0;i--){
            lcd_led_brightness(i);
            delay_ms(25);
        }
    }

    return 0;
}

