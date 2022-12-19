#ifndef __PLUSD_H__
#define __PLUSD_H__

#define FDD_MAX_SECT_LEN    512
#define FDD_MIN_SECTOR   1
#define FDD_MAX_SECTOR  10
#define FDD_MIN_TRACK    0
#define FDD_MAX_TRACK   79

extern unsigned char gFdcData[FDD_MAX_SECT_LEN];

void selectDriveA(unsigned char head);
inline void unselectAllDrives(void);
unsigned char fdcForceInterrupt(void);
unsigned short readSector(unsigned char track, unsigned char sector);

#endif // __PLUSD_H__
