#ifndef __Z80
#define __SPECTRUM
#define __Z80
#endif


#include <arch/zx.h>
#include <stdio.h>
#include <stdlib.h>
#include <z80.h>
#include "plusd.h"
#include "fdd_fs.h"

#ifdef NO_ARGC_ARGV
int main(void) {
    int argc = 2;
    const char *argv[2] = {
        "LOADFDD", "p41"
    };
#else
int main(int argc, char **argv) {
#endif
    int8_t fileNumber = -1;
    for (int i = 1; i < argc; ++i) {
        char cmd = argv[i][0];
        if (cmd == 'p' || cmd == 'P') {
            fileNumber = atoi((char *)&argv[i][1]);
        }
    }

    printf("LOADFDD v0.0.1:\nLIST files from PlusD FDD\n");
    
    if(fileNumber < 1 || fileNumber > 80) {
        printf("Error: invalid file number:%u\n", fileNumber);
        return 0;
    }

    selectDriveA(0);
    z80_delay_ms(500);

    if (fdcForceInterrupt() == 0) {
        loadFile(fileNumber);
    }
    else {
        printf("Drive not ready!\n");
    }

    z80_delay_ms(2000);
    unselectAllDrives();
    return 0;
}


