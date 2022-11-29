#ifndef __Z80
#define __SPECTRUM
#define __Z80
#endif


#include <arch/zx.h>
#include <arch/zx/esxdos.h>
#include <stdio.h>
#include <z80.h>
#include <string.h>
#include "debug.h"
#include "plusd.h"
#include "fdd_fs.h"

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

void dumpFileTapInfo(FileDataHeader *fh) {
    printf("typetap: %u\n", fh->typeTape);
    printf("start  : %u\n", fh->start);
    printf("len    : %u\n", fh->len);
    printf("ts1    : %u\n", fh->type_spec1);
    printf("ts2    : %u\n", fh->type_spec2);
}

static void loadFileData(uint8_t *dst, uint16_t len, uint8_t track, uint8_t sector, uint8_t type, int fp) {
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
            if ((type >= FT_BASIC && type <=FT_CODE) || (type == FT_SCREEN)) {
                src = gFdcData + sizeof(FileDataHeader);
                chunk -= sizeof(FileDataHeader);
                FileDataHeader *h = (FileDataHeader *)gFdcData;
                DBG("T==%2u len==%5u adr==%5u\n", h->typeTape, h->len, h->start);
                #ifdef DEBUG
                    dumpFileTapInfo(h);
                #endif
            }
        }
        if (dst) {
            memcpy(dst, src, chunk);
            dst += chunk;
        }
        if (fp >= 0) {
            if (esxdos_f_write(fp, src, chunk) != chunk) {
                printf("Error writing esxDOS file!\n");
                break;
            }
        }
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
    dumpFileTapInfo(&fd->header);
}


static void dumpFileName(uint8_t fileNum, FileDescriptorPlusD *fd, int fileFilter) {
    if (fileFilter >=0 ) {
        if (fileFilter != fd->typePlusD) {
            return;
        }
    }
    if (fd->typePlusD != FT_ERASED) {
        uint16_t len = fd->header.len;
        printf("p%u t%u %10s %5u B %3uKB\n", fileNum, fd->typePlusD, fd->name, len, len / 1024);
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
        printf("p%u ", fileNumber);
        --fileNumber;
        offset = fileNumber % 2;
        fileNumber /= 2;
        track  = fileNumber / FDD_MAX_SECTOR;
        sector  = fileNumber % FDD_MAX_SECTOR + 1;
        printf("T=%d S=%d offset=%3d\n", track, sector, offset ? 256 : 0);
        int len = readSector(0, track, sector);
        FileDescriptorPlusD *fd = (FileDescriptorPlusD *)gFdcData;
        if (len == FDD_MAX_SECT_LEN) {
            printf("\"%10s\" type=%d typeTap=%d\n", fd[offset].name, fd[offset].typePlusD, fd[offset].header.typeTape);
            dumpFileInfo(&fd[offset]);
            loadFileData((uint8_t *)fd[offset].header.start, fd[offset].header.len, fd[offset].track, fd[offset].sector, fd[offset].typePlusD, -1);
        }
        else {
            printf("Error reading track %d sector %d!\n", track, sector);
        }
    }
    else {
        printf("Error: invalid file number: %u\n", fileNumber);
    }
}

#define PLUS3DOS_HEADER_LEN 128
#define PLUS3DOS_HEADER_CSUM (PLUS3DOS_HEADER_LEN -1)

typedef union _Plus3dosHeader {
    struct {
        char sigStr[8];
        uint8_t sigNum;
        uint8_t issueNum;
        uint8_t verNum;
        uint16_t totalLenLo;
        uint16_t totalLenHi;
        uint8_t fileType;
        uint16_t fileLen;
        uint16_t param1;
        uint16_t param2;
    };
    uint8_t u8[PLUS3DOS_HEADER_LEN];
} Plus3dosHeader;

void setPlus3dosHeaderCsum(Plus3dosHeader *h) {
    uint8_t csum = 0;
    for (uint8_t i = 0; i < PLUS3DOS_HEADER_CSUM; ++i) {
        csum += h->u8[i];
    }
    h->u8[PLUS3DOS_HEADER_CSUM] = csum;
}

int writePlus3dosFileHeader(FileDescriptorPlusD *fd, int fp) {
    Plus3dosHeader h;
    memset(&h, 0, sizeof(h));
    memcpy(h.u8, "PLUS3DOS", sizeof(h.sigStr));
    h.sigNum = 0x1a;
    h.issueNum = 0x01;
    h.verNum = 0x00;
    h.fileType = fd->header.typeTape;
    h.fileLen = fd->header.len;
    switch(fd->typePlusD) {
    case FT_BASIC:
        h.param1 = fd->header.type_spec2;
        h.param2 = fd->header.type_spec1;
        break;
    case FT_NUMBER_ARRAY:
    case FT_STRING_ARRAY:
        h.param1 = fd->header.type_spec1;
        break;
    case FT_CODE:
    case FT_SCREEN:
        h.param1 = fd->header.start;
        h.param2 = fd->header.type_spec2;
        break;
    }
    if (fd->typePlusD == FT_BASIC) {
    }
    else if (fd->typePlusD == 4) {
        h.param1 = fd->header.type_spec2;
        h.param2 = fd->header.type_spec1;
    }
    h.totalLenLo = fd->header.len + PLUS3DOS_HEADER_LEN;
    h.totalLenHi = ((uint32_t)fd->header.len + PLUS3DOS_HEADER_LEN) >> 16;
    setPlus3dosHeaderCsum(&h);
    return esxdos_f_write(fp, &h, PLUS3DOS_HEADER_LEN);
}

void copyFile(int fileNumber, int fp) {
    uint8_t track, sector, offset;
    if (fileNumber > 0 && fileNumber <= 80) {
        printf("p%u ", fileNumber);
        --fileNumber;
        offset = fileNumber % 2;
        fileNumber /= 2;
        track  = fileNumber / FDD_MAX_SECTOR;
        sector  = fileNumber % FDD_MAX_SECTOR + 1;
        printf("T=%d S=%d offset=%3d\n", track, sector, offset ? 256 : 0);
        int len = readSector(0, track, sector);
        FileDescriptorPlusD *fd = (FileDescriptorPlusD *)gFdcData;
        if (len == FDD_MAX_SECT_LEN) {
            printf("\"%10s\" type=%d typeTap=%d\n", fd[offset].name, fd[offset].typePlusD, fd[offset].header.typeTape);
            dumpFileInfo(&fd[offset]);
            writePlus3dosFileHeader(&fd[offset], fp);
            loadFileData(NULL, fd[offset].header.len, fd[offset].track, fd[offset].sector, fd[offset].typePlusD, fp);
        }
        else {
            printf("Error reading track %d sector %d!\n", track, sector);
        }
    }
    else {
        printf("Error: invalid file number: %u\n", fileNumber);
    }
}
