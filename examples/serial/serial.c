
#include "init.h"

/* in this example, tiny printf implementation saves ~6 KB of flash (~33%)
   versus stdio printf. */
#include "printf.h"
// #include <stdio.h> 

#include "io.h"

int main(void) {

    init_clock();
    io_serial_init();

    printf("\033[2J\033[H"); /* clear screen */

    hexdump(0x8000000,0x100); /* 0x8000000, beginning of internal flash */

    return 0;
}

