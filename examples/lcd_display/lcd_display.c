
#include "init.h"
#include "io.h"
#include "lcd_display.h"
#include <stdint.h>

#define GREEN 0x6535

static uint16_t *fb;

static void draw_logos(void)
{
	 int n;

	 for(int i=0;i<200;i++){
         for(int j=0;j<160*80;j++)
             fb[j]=rng_rnd();

         n = 160*17;
         for(unsigned int j=0; j<sizeof(img);j++){
             for(int k=0;k<8;k++){
                 if(img[j]&(1<<k))
                         fb[n] = rng_rnd();
                 else {
                     if(n%160<32)
                         fb[n] = 0b111111110000001; /* the red part */
                     else
                         fb[n] = 0xffff;
                 }
                 n++;
             }
         }
         lcd_flush_fb();
     }

     for(int i=0;i<550;i++){
         for(int j=0;j<160*80;j++)
             fb[j]=GREEN;

         n = 160*26;
         for(unsigned int j=0; j<sizeof(img2);j++){
             for(int k=0;k<8;k++){
                 if(img2[j]&(1<<k))
                     fb[n] = 0xffff;
                 else
                     fb[n] = GREEN;
                 n++;
             }
         }
         lcd_flush_fb();
     }
}

int main(void){

    io_init();
    lcd_flush_fb();

    fb = lcd_get_fb();

    while(1){
       draw_logos();
    }

    return 0;
}

