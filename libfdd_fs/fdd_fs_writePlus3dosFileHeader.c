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

static void setPlus3dosHeaderCsum(Plus3dosHeader *h) {
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
