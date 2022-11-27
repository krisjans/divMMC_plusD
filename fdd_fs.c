#ifndef __Z80
#define __SPECTRUM
#define __Z80
#endif


#include <arch/zx.h>
#include <stdio.h>
#include <z80.h>
#include "plusd.h"
#include "fdd_fs.h"

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
#define FILE_HEADER_PLUSD_LEN 9

typedef union _FileDataHeader {
    struct {
        uint8_t typeTape;
        uint16_t len;
        uint16_t start;
        uint16_t type_spec1;
        uint16_t type_spec2;
    };
    uint8_t u8[FILE_HEADER_PLUSD_LEN];
} FileDataHeader;

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
        FileDataHeader header;
    };
    unsigned char u8[FILE_DESCR_PLUSD_LEN];

} FileDescriptorPlusD;

static void dumpFileName(uint8_t fileNum, FileDescriptorPlusD *fd) {
    if (fd->typePlusD != FT_ERASED) {
        uint16_t len = fd->header.len;
        printf("p%u %10s %5u B /%3u KB\n", fileNum, fd->name, len, len / 1024);
    }
}

void dumpFileList(void) {
    for (int track = 0; track < FDD_FAT_TRACKS; ++track){
        for (int sector = 1; sector <= FDD_MAX_SECTOR; ++sector) {
            uint8_t fileNum = (track * FDD_MAX_SECTOR + sector) * 2 - 1;
            int len = readSector(0, track, sector);
            if (len == FILE_DESCR_PLUSD_LEN || len == FDD_MAX_SECT_LEN) {
                dumpFileName(fileNum, (FileDescriptorPlusD *)gFdcData);
                if (len == FDD_MAX_SECT_LEN) {
                    dumpFileName(fileNum + 1, (FileDescriptorPlusD *)&gFdcData[FILE_DESCR_PLUSD_LEN]);
                }
            }
            else {
                printf("Error reading track %d sector %d!\n", track, sector);
            }
        }
    }
}
