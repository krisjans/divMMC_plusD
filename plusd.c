#ifdef __Z80
    #define IO_PORT(name, addr) __sfr __at addr name
#else
    #define IO_PORT(name, addr) volatile unsigned char name
#endif
#ifndef __Z80
#define __SPECTRUM
#define __Z80
#endif

#include <arch/zx.h>
#include <stdio.h>
#include <z80.h>
#include "plusd.h"


#ifdef UNMODIFIED_PLUSD
IO_PORT(IO_FDC_CMD_STS, 0xE3); // 227
IO_PORT(IO_FDC_TRACK,   0xEB); // 235
IO_PORT(IO_PLUSD_CTRL,  0xEF); // 239
IO_PORT(IO_FDC_SECT,    0xF3); // 243
IO_PORT(IO_FDC_DATA,    0xFB); // 251
#else
IO_PORT(IO_FDC_CMD_STS, 0xA7); // originally at 0xE3 227
IO_PORT(IO_FDC_TRACK,   0xAF);  // originally at 0xEB 235
IO_PORT(IO_PLUSD_CTRL,  0xEF);  // originally at 0xEF 239
IO_PORT(IO_FDC_SECT,    0xB7);    // originally at 0xF3 243
IO_PORT(IO_FDC_DATA,    0xBF);    // originally at 0xFB 251
#endif

#define PLUSD_DRIVE_A   (1 << 0)
#define PLUSD_DRIVE_B   (1 << 1)
#define PLUSD_HEAD      (1 << 7)

// wd1772 type 1 commands
#define FDC_CMD_RESTORE         0x00  // seek track 0
#define FDC_CMD_SEEK            0x10  // Seek to track specified in data register
#define FDC_CMD_STEP_NUPD       0x20  // Step in previous direction without track register update
#define FDC_CMD_STEP_UPD        0x30  // Step in previous direction and update track register
#define FDC_CMD_STEP_IN_NUPD    0x40  // Step towards disk center without track register update
#define FDC_CMD_STEP_IN_UPD     0x50  // Step towards disk center and update track register
#define FDC_CMD_STEP_OUT_NUPD   0x60  // Step away from disk center without track register update
#define FDC_CMD_STEP_OUT_UPD    0x70  // Step away from disk center and update track register
#define FDC_CMD_FORCE_INTERPT   0xD0  // Stop current command

#define FDC_ARG_STEP_RATE_6MS   0x00
#define FDC_ARG_STEP_RATE_12MS  0x01
#define FDC_ARG_STEP_RATE_2MS   0x02
#define FDC_ARG_STEP_RATE_3MS   0x03
#define FDC_ARG_VERIFY_TRACK    0x04
#define FDC_ARG_SPIN_UP         0x08

#define FDC_STS_BUSY        (1 << 0)
#define FDC_STS_INDEX_PULSE (1 << 1)
#define FDC_STS_TRACK0      (1 << 2)

// wd1772 Type 2 commands
#define FDC_CMD_READ_SECTOR     0x80
#define FDC_CMD_READ_SECTORS    0x90
#define FDC_CMD_WRITE_SECTOR    0xA0
#define FDC_CMD_WRITE_SECTORS   0xB0

#define FDC_STS_DRQ (1 << 1) // read or write to DATA register requested
#define FDC_STS_OWR (1 << 2) // DATA register owerflow

void selectDriveA(unsigned char head) {
    IO_PLUSD_CTRL = (PLUSD_DRIVE_A | (head ? PLUSD_HEAD : 0x00));
}

inline void unselectAllDrives(void) {
    IO_PLUSD_CTRL = 0x00;
}

static inline unsigned char readFdcStatus(void) {
    return IO_FDC_CMD_STS;
}

static inline void waitFdcReady(void) {
    while (readFdcStatus() & FDC_STS_BUSY) {
    }
}

static inline void sendFdcCmd(const unsigned char cmd) {
    IO_FDC_CMD_STS = cmd;
}

static unsigned char seek(unsigned char head, unsigned char track, unsigned char sector) {
    selectDriveA(head);
    z80_delay_ms(20);
    waitFdcReady();
    IO_FDC_SECT = sector;
    while (1) {
        waitFdcReady();
        unsigned char t = IO_FDC_TRACK;
        if (t == track) {
            break;
        }
        if (t > track) {
            IO_FDC_CMD_STS = FDC_CMD_STEP_OUT_UPD | FDC_ARG_SPIN_UP;
        }
        else {
            IO_FDC_CMD_STS = FDC_CMD_STEP_IN_UPD | FDC_ARG_SPIN_UP;
        }
    }
    return 0;
}

typedef union _FdcStatus {
    struct {
        unsigned char busy: 1;
        unsigned char drq: 1;
        unsigned char errLost: 1; // lost data?
        unsigned char errCrc: 1; // Sector corrupted
        unsigned char errNotFound: 1; //Non-existent track/sector or no more data to read
        unsigned char errType: 1; // Read: record type; Write: write fault
        unsigned char writeProtect: 1; // Disk is write protected
        unsigned char motorOn: 1; // Motor is on or drive not ready
    } bits;
    unsigned char u8;
} FdcStatus;


unsigned char gFdcData[FDD_MAX_SECT_LEN];

unsigned short readSector(unsigned char head, unsigned char track, unsigned char sector) {
    unsigned short l = 0;
    unsigned char oldStatus = 0;
    seek(head, track, sector);
    waitFdcReady();

    IO_FDC_CMD_STS = FDC_CMD_READ_SECTOR;
    while (((IO_FDC_CMD_STS & FDC_STS_BUSY) == 0)) {

    }
     register unsigned char *pp = gFdcData;
    __asm__("di");
    while (1) {
        if (IO_FDC_CMD_STS & FDC_STS_DRQ) *pp++ = IO_FDC_DATA;
        if (IO_FDC_CMD_STS & FDC_STS_DRQ) *pp++ = IO_FDC_DATA;
        if (IO_FDC_CMD_STS & FDC_STS_DRQ) *pp++ = IO_FDC_DATA;
        if (IO_FDC_CMD_STS & FDC_STS_DRQ) *pp++ = IO_FDC_DATA;
        if (!(IO_FDC_CMD_STS & FDC_STS_BUSY)) break;
    }
    __asm__("ei");

    return pp - gFdcData;
}

unsigned char fdcForceInterrupt(void) {
    unsigned char oldIndex = 1;
    IO_FDC_CMD_STS = FDC_CMD_FORCE_INTERPT;
    z80_delay_ms(1);
    for (unsigned short i = 0; i < 60000; ++i) {
        if (IO_FDC_CMD_STS & FDC_STS_INDEX_PULSE) {
            if (oldIndex == 0) {
                oldIndex = 2;
                break;
            }
        }
        else {
            oldIndex = 0;
        }
    }
    if (oldIndex != 2) {
        return 2;
    }
    for (int i = 0; i < 100; ++i) {
        if (IO_FDC_CMD_STS & FDC_STS_TRACK0) {
            return 0;
        }
        IO_FDC_CMD_STS = FDC_CMD_STEP_OUT_UPD | FDC_ARG_SPIN_UP;
        z80_delay_ms(20);
    }
    return 9;
}
