
#include "ns_audio.h" /* map_u32_to_01 */
#include "drivers.h"  /* io_hc_sr04_... */
#include "io.h"
#include <stdint.h>

int main(void){

    io_init();
    io_hc_sr04_init();

    while(1) {
        uint32_t sensor_raw = io_hc_sr04_get_raw();

        /* map 1200..140 -> 0.0f..1.0f */
        float sensor_mapped = map_u32_to_01(sensor_raw, 1200, 140);

        float distance_cm = io_hc_sr04_get_cm();

        lcd_printf(30,15,1,2,WHITE,BLACK,"   raw: %04d",    sensor_raw);
        lcd_printf(30,31,1,2,WHITE,BLACK,"mapped: %04.2f",  sensor_mapped);
        lcd_printf(30,47,1,2,WHITE,BLACK,"    %04.1fcm ", distance_cm);
        lcd_flush_fb();
    }

    return 0;
}

