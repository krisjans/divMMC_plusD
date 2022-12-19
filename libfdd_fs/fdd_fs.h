#ifndef __FDD_FS__
#define __FDD_FS__

#define FDD_FAT_TRACKS  4

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


void dumpFileList(int fileFilter);
void loadFile(int fileNumber);
void copyFile(int fileNumber, int fp);
void dumpFileInfo(FileDescriptorPlusD *fd);
int writePlus3dosFileHeader(FileDescriptorPlusD *fd, int fp);
void loadFileData(uint8_t *dst, uint32_t len, uint8_t track, uint8_t sector, uint8_t type, int fp);

#endif
