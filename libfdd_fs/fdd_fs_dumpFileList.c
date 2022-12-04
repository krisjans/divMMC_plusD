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
