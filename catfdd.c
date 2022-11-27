#ifndef __Z80
#define __SPECTRUM
#define __Z80
#endif


#include <arch/zx.h>
#include <stdio.h>
#include <z80.h>
#include "plusd.h"
#include "fdd_fs.h"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    printf("CATFDD v0.0.1:\nLIST files from PlusD FDD\n");
    
    selectDriveA(0);
    z80_delay_ms(500);

    if (fdcForceInterrupt() == 0) {
        dumpFileList();
    }
    else {
        printf("Drive not ready!\n");
    }

    z80_delay_ms(2000);
    unselectAllDrives();
    return 0;
}


