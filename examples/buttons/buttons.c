
#include "io.h"

int main(void){

    io_init();

    uint32_t btns_30 = 0;
    uint32_t btns_3  = 0;

    while(1){
        if(button_pressed()){
            for(int i=0;i<30;i++){
                btns_30 |= (button(i) << (29 - i));
            }
        }

        if(button_side_pressed()){
            for(int i=0;i<3;i++){
                btns_3 |= (button_side(i)<<i);
            }
        }

        lcd_printf(56,10,1,1,WHITE,BLACK,"%06b\n%06b\n%06b\n%06b\n%06b\n",
                ((btns_30>>24) & 0x3f),
                ((btns_30>>18) & 0x3f),
                ((btns_30>>12) & 0x3f),
                ((btns_30>>6)  & 0x3f),
                ((btns_30>>0)  & 0x3f));

        lcd_printf(56,60,1,1,WHITE,BLACK,"%03b", btns_3);
        lcd_flush_fb();

        btns_30 = 0;
        btns_3 = 0;
    }

    return 0;
}

