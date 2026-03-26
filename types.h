#ifndef TYPES_H
#define TYPES_H

#define MAX_FILENAME   256
#define MAX_TEXT_SIZE  1000000
#define MAX_TREE_NODES 512
#define PI             3.14159265358979323846

/* ---- File Type Enum ---- */
typedef enum {
    TEXT_FILE,
    BMP_FILE,
    FILE_COLOR,
    UNKNOWN_FILE
} FileType;

/* ---- BMP Header Structures ---- */
#pragma pack(push, 1)

typedef struct {
    unsigned short bfType;       
    unsigned int   bfSize;       
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int   bfOffBits;    
} BMPFileHeader;

typedef struct {
    unsigned int   biSize;
    int            biWidth;
    int            biHeight;     
    unsigned short biPlanes;
    unsigned short biBitCount;   // 8 / 24 / 32          
    unsigned int   biCompression;
    unsigned int   biSizeImage;
    int            biXPelsPerMeter;
    int            biYPelsPerMeter;
    unsigned int   biClrUsed;
    unsigned int   biClrImportant;
} BMPInfoHeader;

#pragma pack(pop)

/* ---- Color image (RGB / RGBA) ---- */
typedef struct {
    int  width, height, maxVal;
    int **R, **G, **B, **A;
    int  hasAlpha;
} ColorImage;

/* ---- File-type detector ---- */
FileType detectFileType(const char *fileName);

#endif /* TYPES_H */
