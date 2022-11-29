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

#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

#ifdef NO_ARGC_ARGV
int main(void) {
    int argc = 2;
    const char *argv[2] = {
        "IMGFDD", "plusd.img"
    };
#else
int main(int argc, char **argv) {
#endif
    printf("IMGFDD " STRINGIZE_VALUE_OF(VERSION) ":\nCreate PlusD FDD floppy image\n");

    if (argc < 2) {
        printf("Usage: imgfdd file_name_on_sd\n");
        return 0;
    }

    selectDriveA(0);
    z80_delay_ms(500);

    if (fdcForceInterrupt() == 0) {
        int fp = esxdos_f_open(argv[1], ESXDOS_MODE_CREAT_TRUNC | ESXDOS_MODE_CREAT_NOEXIST | ESXDOS_MODE_W);
        if (errno) {
            printf("Error opening file %s, error code %d\n", argv[1], errno);
            z80_delay_ms(2000);
            unselectAllDrives();
            return 0;
        }
        for (uint8_t head = 0; head < 2; ++head) {
            selectDriveA(head);
            z80_delay_ms(500);
            printf(" H%u", head);
            if (fdcForceInterrupt() == 0) {
                uint16_t rc = 0;
                for (uint8_t track = FDD_MIN_TRACK; track <= FDD_MAX_TRACK; ++track) {
                    printf(" T%u", track);
                    rc = 0;
                    for (uint8_t sector = FDD_MIN_SECTOR; sector <= FDD_MAX_SECTOR; ++sector) {
                        for (uint8_t retry = 0; retry < 3; ++retry) {
                            rc = readSector(head, track, sector);
                            if (rc == FDD_MAX_SECT_LEN) {
                                if (esxdos_f_write(fp, gFdcData, FDD_MAX_SECT_LEN) == FDD_MAX_SECT_LEN) {
                                    break;
                                }
                                else {
                                    printf("\nError writing image file!\n");
                                    rc = 0;
                                }
                            }
                        }
                        if (rc != FDD_MAX_SECT_LEN) {
                            printf("\nError reading/writing head=%u track=%u sector=%u\n", head, track, sector);
                            break;
                        }
                    }
                    if (rc != FDD_MAX_SECT_LEN) {
                        break;
                    }
                }
            }
            else {
                printf("\nError reading track0 head%d\n", head);
            }
        }
        printf("\n");
        esxdos_f_close(fp);
    }
    else {
        printf("Drive not ready!\n");
    }

    z80_delay_ms(2000);
    unselectAllDrives();
    return 0;
}
