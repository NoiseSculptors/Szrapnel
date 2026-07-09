
#include "delay.h"
#include "io.h"
#include <stdint.h>

int main(void){

    io_init();

    uint16_t bg = rgb565(0xff,0xff,0x00);
    uint16_t fg = 0;
    uint8_t ctr = 0;

    lcd_clear(bg);

    while(1){
        /* collect everything to display */
        lcd_printf(0,  /* x */
                   0,  /* y */
                   2,  /* multiplier for x size 2: double x size */
                   2,  /* multiplier for y size 2: double y size */
                   fg, /* foreground in rgb565, uint16_t */
                   bg, /* background in rgb565, uint16_t */
                   "BIG FONT");
        lcd_printf(0,16,1,1,fg,bg,"normal font");
        lcd_printf(0,24,2,1,fg,bg,"stretch X");
        lcd_printf(0,32,1,2,fg,bg,"stretch Y...\nnewline, ctr:%02d", ctr++%11);

        /* display it */
        lcd_flush_fb();
        delay_ms(400);
    }

    return 0;
}

