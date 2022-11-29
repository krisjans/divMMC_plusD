#ifndef __Z80
#define __SPECTRUM
#define __Z80
#endif


#include <arch/zx.h>
#include <stdio.h>
#include <z80.h>
#include <string.h>
#include "debug.h"
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

static void loadFileData(uint8_t *dst, uint16_t len, uint8_t track, uint8_t sector) {
    uint8_t first = 1;
    while (len) {
        DBG("T=%2d S=%2d\n", track, sector);
        if (readSector(0, track, sector) != FDD_MAX_SECT_LEN) {
            printf("\n\nRead error track==%d sector %d\n\n", track, sector);
            break;
        }
        uint16_t chunk = FDD_MAX_SECT_LEN > len ? len : FDD_MAX_SECT_LEN - 2;
        uint8_t *src = gFdcData;
        if (first) {
            first = 0;
            src = gFdcData + sizeof(FileDataHeader);
            chunk -= sizeof(FileDataHeader);
            FileDataHeader *h = (FileDataHeader *)gFdcData;
            DBG("T==%2u len==%5u adr==%5u\n", h->typeTape, h->len, h->start);
        }
        memcpy(dst, src, chunk);
        dst += chunk;
        len -= chunk;
        track = gFdcData[FDD_MAX_SECT_LEN - 2];
        sector = gFdcData[FDD_MAX_SECT_LEN - 1];
        if (track + sector == 0) {
            break;
        }
    }
    zx_border(INK_YELLOW);
    z80_delay_ms(5000);
}

static void dumpFileInfo(FileDescriptorPlusD *fd) {
    if (fd->typePlusD == FT_ERASED) {
        printf("No file, Erased sector\n");
        return;
    }
    printf("sectors: %u\n", fd->sectCntHi << 8 | fd->sectCntLo);
    printf("track  : %u\n", fd->track);
    printf("sector : %u\n", fd->sector);
    printf("start  : %u\n*\n", fd->header.start);
    printf("ts1    : %u\n*\n", fd->header.type_spec1);
    printf("ts2    : %u\n*\n", fd->header.type_spec2);
}


static void dumpFileName(uint8_t fileNum, FileDescriptorPlusD *fd, int fileFilter) {
    if (fileFilter >=0 ) {
        if (fileFilter != fd->typePlusD) {
            return;
        }
    }
    if (fd->typePlusD != FT_ERASED) {
        uint16_t len = fd->header.len;
        printf("p%u %10s %5u B /%3u KB\n", fileNum, fd->name, len, len / 1024);
    }
}

void dumpFileList(int fileFilter) {
    for (int track = 0; track < FDD_FAT_TRACKS; ++track){
        for (int sector = 1; sector <= FDD_MAX_SECTOR; ++sector) {
            uint8_t fileNum = (track * FDD_MAX_SECTOR + sector - 1) * 2 + 1;
            int len = readSector(0, track, sector);
            if (len == FILE_DESCR_PLUSD_LEN || len == FDD_MAX_SECT_LEN) {
                dumpFileName(fileNum, (FileDescriptorPlusD *)gFdcData, fileFilter);
                if (len == FDD_MAX_SECT_LEN) {
                    dumpFileName(fileNum + 1, (FileDescriptorPlusD *)&gFdcData[FILE_DESCR_PLUSD_LEN], fileFilter);
                }
            }
            else {
                printf("Error reading track %d sector %d!\n", track, sector);
            }
        }
    }
}

void loadFile(int fileNumber) {
    uint8_t track, sector, offset;
    if (fileNumber > 0 && fileNumber <= 80) {
        printf("fn=%u ", fileNumber);
        --fileNumber;
        offset = fileNumber % 2;
        fileNumber /= 2;
        track  = fileNumber / FDD_MAX_SECTOR;
        sector  = fileNumber % FDD_MAX_SECTOR + 1;
        printf("T=%d S=%d offset=%d\n", track, sector, offset);
        int len = readSector(0, track, sector);
        FileDescriptorPlusD *fd = (FileDescriptorPlusD *)gFdcData;
        if (len == FDD_MAX_SECT_LEN) {
            dumpFileInfo(&fd[offset]);
            loadFileData((uint8_t *)fd[offset].header.start, fd[offset].header.len, fd[offset].track, fd[offset].sector);
        }
        else {
            printf("Error reading track %d sector %d!\n", track, sector);
        }
    }
    else {
        printf("Error: invalid file number: %u\n", fileNumber);
    }
}
