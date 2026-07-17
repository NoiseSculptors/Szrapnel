
#include "ns_audio.h" /* map_u32_to_01 */
#include "drivers.h"  /* io_hc_sr04_... */
#include "io.h"
#include <stdint.h>

#define FG   rgb565(255,255,0)
#define BG   rgb565(0,0,127)

static uint16_t smooth_u16(uint16_t in)
{
    static uint32_t y = 0;
    if (!y) y = in;
    y += (in - (uint16_t)y) >> 7; /* 3,4,5 3-faster response,  5-smoother */
    return (uint16_t)y;
}

int main(void){

    io_init();
    io_hc_sr04_init();

    lcd_clear(BG);

    while(1) {
        uint32_t sensor_raw = io_hc_sr04_get_raw();
        uint32_t sensor_filtered = smooth_u16(sensor_raw);

        /* map 1200..140 -> 0.0f..1.0f */
        float sensor_mapped = map_u32_to_01(sensor_filtered, 1200, 140);

        float distance_cm = io_hc_sr04_get_cm();
        uint8_t bar_x = (uint8_t)(sensor_mapped*160.0f);

        lcd_box_filled(0,0,159,10,BG);
        lcd_box_filled(0,0,bar_x,10,FG);

        lcd_printf(30,15,1,2,FG,BG,"filter: %04d",   sensor_filtered);
        lcd_printf(30,31,1,2,FG,BG,"mapped: %04.3f", sensor_mapped);
        lcd_printf(30,47,1,2,FG,BG,"    %04.1fcm ",  distance_cm);
        lcd_flush_fb();
    }

    return 0;
}

