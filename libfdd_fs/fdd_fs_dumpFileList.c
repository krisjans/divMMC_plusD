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

static void dumpFileName(uint8_t fileNum, FileDescriptorPlusD *fd, int fileFilter) {
    if (fileFilter >=0 ) {
        if (fileFilter != fd->typePlusD) {
            return;
        }
    }
    if (fd->typePlusD != FT_ERASED) {
        uint32_t len = 0;
        switch(fd->typePlusD) {
        case FT_BASIC:
        case FT_NUMBER_ARRAY:
        case FT_STRING_ARRAY:
        case FT_CODE:
        case FT_SCREEN:
            len = fd->header.len;
            break;
        case FT_SNAPSHOT_48K:
            len = 1024UL * 48UL + FILE_HEADER_PLUSD_LEN;
            break;
        case FT_SNAPSHOT_128K:
            len = 1024UL * 128UL + FILE_HEADER_PLUSD_LEN;
            break;
        };
        printf("p%u t%u %.10s", fileNum, fd->typePlusD, fd->name);
        printf(" %6luB", len);
        printf(" %3luKB\n", len / 1024UL);
    }
}

void dumpFileList(int fileFilter) {
    for (int track = 0; track < FDD_FAT_TRACKS; ++track){
        for (int sector = 1; sector <= FDD_MAX_SECTOR; ++sector) {
            uint8_t fileNum = (track * FDD_MAX_SECTOR + sector - 1) * 2 + 1;
            int len = readSector(track, sector);
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
