#ifndef __Z80
#define __SPECTRUM
#define __Z80
#endif


#include <arch/zx.h>
#include <stdio.h>
#include <stdlib.h>
#include <z80.h>
#include "libfdd_fs/plusd.h"
#include "libfdd_fs/fdd_fs.h"

#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

#ifdef NO_ARGC_ARGV
int main(void) {
    int argc = 1;
    const char *argv[1] = {
        "CATFDD",
    };
#else
int main(int argc, char **argv) {
#endif
    int8_t fileFilter = -1;
    for (int i = 1; i < argc; ++i) {
        char cmd = argv[i][0];
        if (cmd == 't' || cmd == 'T') {
            fileFilter = atoi((char *)&argv[i][1]);
        }
    }

    printf("CATFDD " STRINGIZE_VALUE_OF(VERSION) ":\nLIST files from PlusD FDD\n");
    
    selectDriveA(0);
    z80_delay_ms(500);

    if (fdcForceInterrupt() == 0) {
        dumpFileList(fileFilter);
    }
    else {
        printf("Drive not ready!\n");
    }

    z80_delay_ms(2000);
    unselectAllDrives();
    return 0;
}


