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
        int len = readSector(track, sector);
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
