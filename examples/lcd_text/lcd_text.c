
#include "init.h"
#include "io.h"
#include "delay.h"
#include <stdint.h>

static void init_hw(void)
{
	init_clock();
    io_init();
    lcd_led_brightness(100);
}

int main(void){

	init_hw();
    uint16_t bg = rgb565(0xff,0xff,0x00);
    uint16_t fg = 0;

    lcd_clear(bg);

    lcd_printf(0,0,2,2,fg,bg,"BIG FONT");
    lcd_printf(0,16,1,1,fg,bg,"normal font");
    lcd_printf(0,24,2,1,fg,bg,"stretch X");
    lcd_printf(0,32,1,2,fg,bg,"stretch Y...\nnewline");

    lcd_flush_fb();
    return 0;
}

