
#include "printf.h"   // smaller
//#include <stdio.h>

#include "io.h"

int main(void) {

    io_init();

    printf("\n\n\n");

    /* 0x8000000, beginning of internal flash */
    hexdump(0x8000000,0x100);

    return 0;
}

