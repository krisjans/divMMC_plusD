#ifndef __Z80
#define __SPECTRUM
#define __Z80
#endif


#include <arch/zx.h>
#include <arch/zx/esxdos.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <z80.h>
#include "plusd.h"
#include "fdd_fs.h"

#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

#ifdef NO_ARGC_ARGV
int main(void) {
    int argc = 3;
    const char *argv[3] = {
        "CPFDD", "p41", "testfile.bin"
    };
#else
int main(int argc, char **argv) {
#endif
    printf("CPFDD " STRINGIZE_VALUE_OF(VERSION) ":\nCopy files from PlusD FDD\n");

    if (argc < 3) {
        printf("Usage: cpfdd p_file_number_on_fdd file_name_on_sd\n");
    }

    int8_t fileNumber = -1;
    char cmd = argv[1][0];
    if (cmd == 'p' || cmd == 'P') {
        fileNumber = atoi((char *)&argv[1][1]);
    }

    if(fileNumber < 1 || fileNumber > 80) {
        printf("Error: invalid file number:%u\n", fileNumber);
        return 0;
    }

    int fp = esxdos_f_open(argv[2], ESXDOS_MODE_CREAT_TRUNC | ESXDOS_MODE_CREAT_NOEXIST | ESXDOS_MODE_W);
    if (errno) {
        printf("Error opening file %s, error code %d\n", argv[2], errno);
        return 0;
    }

    selectDriveA(0);
    z80_delay_ms(500);

    if (fdcForceInterrupt() == 0) {
        copyFile(fileNumber, fp);
    }
    else {
        printf("Drive not ready!\n");
    }

    esxdos_f_close(fp);

    z80_delay_ms(2000);
    unselectAllDrives();
    return 0;
}
