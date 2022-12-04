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

static void dumpFileTapInfo(FileDataHeader *fh) {
    printf("typetap: %u\n", fh->typeTape);
    printf("start  : %u\n", fh->start);
    printf("len    : %u\n", fh->len);
    printf("ts1    : %u\n", fh->type_spec1);
    printf("ts2    : %u\n", fh->type_spec2);
}

void dumpFileInfo(FileDescriptorPlusD *fd) {
    if (fd->typePlusD == FT_ERASED) {
        printf("No file, Erased sector\n");
        return;
    }
    printf("sectors: %u\n", fd->sectCntHi << 8 | fd->sectCntLo);
    printf("track  : %u\n", fd->track);
    printf("sector : %u\n", fd->sector);
    dumpFileTapInfo(&fd->header);
}