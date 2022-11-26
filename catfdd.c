#ifndef __Z80
#define __SPECTRUM
#define __Z80
#endif


#include <arch/zx.h>
#include <stdio.h>
#include <z80.h>
#include "plusd.h"

#define FDD_MAX_SECTOR  10
#define FDD_FAT_TRACKS  3

enum FileTypesPlusD {
    FT_ERASED = 0,
    FT_BASIC = 1,
    FT_NUMBER_ARRAY = 2,
    FT_STRING_ARRAY = 3,
    FT_CODE = 4,
    FT_SNAPSHOT_48K = 5,
    FT_SCREEN = 7,
    FT_SNAPSHOT_128K = 9,
};

#define FILE_DESCR_PLUSD_LEN 256

typedef union _FileDescriptor {
    struct {
        unsigned char typePlusD;
        char name[10];
        unsigned char sectCntHi;
        unsigned char sectCntLo;
        unsigned char track;
        unsigned char sector;
        unsigned char bitmap[195];
        unsigned char reserved;
        unsigned char typeTape;
        unsigned char lenLo;
        unsigned char lenHi;
    };
    unsigned char u8[FILE_DESCR_PLUSD_LEN];

} FileDescriptorPlusD;

static void dumpFileName(FileDescriptorPlusD *fd) {
    if (fd->typePlusD != FT_ERASED) {
        unsigned short len = ((unsigned short)fd->lenHi << 8) | fd->lenLo;
        printf("%10s %6d B / %3d KB\n", fd->name, len, len / 1024);
    }
}

static void dumpFileList(void) {
    for (int track = 0; track < FDD_FAT_TRACKS; ++track){
        for (int sector = 1; sector <= FDD_MAX_SECTOR; ++sector) {
            int len = readSector(0, track, sector);
            if (len == FILE_DESCR_PLUSD_LEN || len == FDD_MAX_SECT_LEN) {
                dumpFileName((FileDescriptorPlusD *)gFdcData);
                if (len == FDD_MAX_SECT_LEN) {
                    dumpFileName((FileDescriptorPlusD *)&gFdcData[FILE_DESCR_PLUSD_LEN]);
                }
            }
            else {
                printf("Error reading track %d sector %d!\n", track, sector);
            }
        }
    }
}

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


