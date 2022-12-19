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

void loadFileData(uint8_t *dst, uint16_t len, uint8_t track, uint8_t sector, uint8_t type, int fp) {
    uint8_t first = 1;
    while (len) {
        DBG("T=%2d S=%2d\n", track, sector);
        if (readSector(track, sector) != FDD_MAX_SECT_LEN) {
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
