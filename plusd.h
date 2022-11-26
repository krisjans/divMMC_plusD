#ifndef __PLUSD_H__
#define __PLUSD_H__

#define FDD_MAX_SECT_LEN    512
extern unsigned char gFdcData[FDD_MAX_SECT_LEN];

void selectDriveA(unsigned char head);
inline void unselectAllDrives(void);
unsigned char fdcForceInterrupt(void);
unsigned short readSector(unsigned char head, unsigned char track, unsigned char sector);

#endif // __PLUSD_H__
