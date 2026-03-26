/* ============================================================
 * bmp_compress.c  -  Bit-plane slicing for 8-bit grayscale BMP
 *
 *   MSB  (lossy)    - stores only the most-significant bit
 *   Full (lossless) - stores all 8 bit-planes
 * ============================================================ */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "bmp_compress.h"

/* ============================================================
 *  Shared helpers
 * ============================================================ */

static int readGrayscalePixels(FILE *fp, unsigned char *imageData,
                                int width, int height, int isBottomUp,
                                int rowPadding, long pixelOffset)
{
    unsigned char padBuf[4];
    fseek(fp, pixelOffset, SEEK_SET);

    for (int i = 0; i < height; i++) {
        int row = isBottomUp ? (height - 1 - i) : i;
        size_t got = fread(imageData + row * width, 1, width, fp);
        if (got < (size_t)width) {
            printf("WARNING: Expected %d bytes, got %zu.\n", width, got);
            return 0;
        }
        if (rowPadding > 0)
            fread(padBuf, 1, rowPadding, fp);
    }
    return 1;
}

static void writeGrayscalePalette(FILE *fp)
{
    unsigned char palette[1024];
    for (int i = 0; i < 256; i++) {
        palette[i * 4]     = (unsigned char)i;
        palette[i * 4 + 1] = (unsigned char)i;
        palette[i * 4 + 2] = (unsigned char)i;
        palette[i * 4 + 3] = 0;
    }
    fwrite(palette, 1, 1024, fp);
}

static void writeGrayscaleBMP(const char *filename,
                               unsigned char *pixels,
                               int width, int height, int isBottomUp)
{
    int rowPadding = (4 - (width % 4)) % 4;

    BMPFileHeader fh;
    BMPInfoHeader ih;

    fh.bfType      = 0x4D42;
    fh.bfReserved1 = 0;
    fh.bfReserved2 = 0;
    fh.bfOffBits   = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + 1024;
    fh.bfSize      = fh.bfOffBits + (unsigned int)((width + rowPadding) * height);

    ih.biSize          = sizeof(BMPInfoHeader);
    ih.biWidth         = width;
    ih.biHeight        = isBottomUp ? height : -height;
    ih.biPlanes        = 1;
    ih.biBitCount      = 8;
    ih.biCompression   = 0;
    ih.biSizeImage     = (unsigned int)((width + rowPadding) * height);
    ih.biXPelsPerMeter = 2835;
    ih.biYPelsPerMeter = 2835;
    ih.biClrUsed       = 0;
    ih.biClrImportant  = 0;

    FILE *fp = fopen(filename, "wb");
    if (!fp) { printf("Error creating file: %s\n", filename); return; }

    fwrite(&fh, sizeof(BMPFileHeader), 1, fp);
    fwrite(&ih, sizeof(BMPInfoHeader), 1, fp);
    writeGrayscalePalette(fp);

    unsigned char pad[3] = {0, 0, 0};
    for (int i = 0; i < height; i++) {
        int row = isBottomUp ? (height - 1 - i) : i;
        fwrite(pixels + row * width, 1, width, fp);
        if (rowPadding > 0)
            fwrite(pad, 1, rowPadding, fp);
    }
    fclose(fp);
}

static int openGrayscaleBMP(const char *filename,
                              BMPFileHeader *fh, BMPInfoHeader *ih,
                              int *width, int *height, int *isBottomUp,
                              int *rowPadding)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) { printf("Error opening file: %s\n", filename); return 0; }

    fread(fh, sizeof(BMPFileHeader), 1, fp);
    fread(ih, sizeof(BMPInfoHeader), 1, fp);
    fclose(fp);

    if (fh->bfType != 0x4D42) { printf("Not a BMP file.\n"); return 0; }
    if (ih->biBitCount != 8) {
        printf("Only 8-bit grayscale BMP images are supported (%d-bit given)\n",
               ih->biBitCount);
        return 0;
    }

    *width      = ih->biWidth;
    *isBottomUp = (ih->biHeight > 0);
    *height     = *isBottomUp ? ih->biHeight : -ih->biHeight;
    *rowPadding = (4 - (*width % 4)) % 4;
    return 1;
}

/* ---- Pretty-print a compression summary box ---- */
/* ============================================================
 *  Box drawing helpers  (BOX_INNER = 70, total width = 74)
 * ============================================================ */
#define BOX_INNER 70

static void box_divider(void)
{
    printf("+");
    for (int i = 0; i < BOX_INNER + 2; i++) printf("-");
    printf("+\n");
}

static void box_row(const char *text)
{
    int len = (int)strlen(text);
    if (len <= BOX_INNER)
        printf("| %s%*s |\n", text, BOX_INNER - len, "");
    else
        printf("| %.*s... |\n", BOX_INNER - 3, text);
}

static void printResultsBox(const char *title, const char *mode,
                             const char *outFile, const char *decFile,
                             long origSize, long compSize)
{
    double reduction = (origSize > 0 && compSize < origSize)
                       ? (1.0 - (double)compSize / origSize) * 100.0 : 0.0;
    double ratio     = (compSize > 0) ? (double)origSize / compSize : 1.0;

    int filled = (int)(reduction / 10.0);
    if (filled > 10) filled = 10;
    if (filled <  0) filled = 0;
    char bar[11];
    for (int i = 0; i < 10; i++) bar[i] = (i < filled) ? '#' : '-';
    bar[10] = '\0';

    const char *stars, *verdict;
    if      (reduction >= 85) { stars = "* * * * *"; verdict = "EXCELLENT"; }
    else if (reduction >= 70) { stars = "* * * * ."; verdict = "GREAT    "; }
    else if (reduction >= 50) { stars = "* * * . ."; verdict = "GOOD     "; }
    else if (reduction >= 30) { stars = "* * . . ."; verdict = "FAIR     "; }
    else                      { stars = "* . . . ."; verdict = "LOW      "; }

    char buf[256];

    printf("\n");
    box_divider();
    box_row(title);
    box_divider();
    snprintf(buf, sizeof(buf), "  Mode         : %s", mode);             box_row(buf);
    snprintf(buf, sizeof(buf), "  Saved to     : %s", outFile);          box_row(buf);
    box_divider();
    snprintf(buf, sizeof(buf), "  Original     : %ld bytes", origSize);  box_row(buf);
    snprintf(buf, sizeof(buf), "  Compressed   : %ld bytes", compSize);  box_row(buf);
    box_divider();
    snprintf(buf, sizeof(buf), "  Space Saved  : %.2f%%   [%s]", reduction, bar); box_row(buf);
    snprintf(buf, sizeof(buf), "  Ratio        : %.2f : 1", ratio);               box_row(buf);
    snprintf(buf, sizeof(buf), "  Rating       : %s   %s", verdict, stars);       box_row(buf);
    box_divider();
    snprintf(buf, sizeof(buf), "  Decompressed : %s", decFile);          box_row(buf);
    box_divider();
    printf("\n");
}



/* ============================================================
 *  MSB (lossy) - compress
 * ============================================================ */

static void compressBMPBitPlaneMSB(const char *inputFile)
{
    printf("\n=== BMP IMAGE COMPRESSION (MSB) ===\n");
    printf("Detected: BMP ImageFile\n");
    printf("Bit Plane Slicing (MSB) is used for compression.\n");

    BMPFileHeader fh; BMPInfoHeader ih;
    int width, height, isBottomUp, rowPadding;
    if (!openGrayscaleBMP(inputFile, &fh, &ih, &width, &height, &isBottomUp, &rowPadding))
        return;

    int imageSize = width * height;
    printf("Image info:\n");
    printf("  Dimensions : %d x %d pixels\n", width, height);
    printf("  Bit depth  : 8-bit grayscale\n");
    printf("  Total px   : %d\n\n", imageSize);

    FILE *fp = fopen(inputFile, "rb");
    fread(&fh, sizeof(BMPFileHeader), 1, fp);
    fread(&ih, sizeof(BMPInfoHeader), 1, fp);
    unsigned char palette[1024];
    fread(palette, 1, 1024, fp);

    unsigned char *imageData = (unsigned char *)malloc(imageSize);
    readGrayscalePixels(fp, imageData, width, height, isBottomUp,
                        rowPadding, fh.bfOffBits);
    fclose(fp);

    printf("Step 1: Extracting MSB from each pixel...\n");
    unsigned char *msbPlane = (unsigned char *)malloc(imageSize);
    for (int i = 0; i < imageSize; i++)
        msbPlane[i] = (imageData[i] >> 7) & 1;

    printf("Step 2: Packing MSB bits into bytes...\n");
    int packedSize = (imageSize + 7) / 8;
    unsigned char *packed = (unsigned char *)calloc(packedSize, 1);
    for (int i = 0; i < imageSize; i++)
        if (msbPlane[i])
            packed[i / 8] |= (1 << (7 - (i % 8)));

    char outFile[MAX_FILENAME];
    strcpy(outFile, inputFile);
    char *dot = strrchr(outFile, '.');
    if (dot) *dot = '\0';
    strcat(outFile, "_compressed.bps");

    fp = fopen(outFile, "wb");
    fwrite(&width,  sizeof(int), 1, fp);
    fwrite(&height, sizeof(int), 1, fp);
    fwrite(packed,  1, packedSize, fp);
    fclose(fp);

    printf("Step 3: Decompressing...\n");

    unsigned char *unpackedMSB   = (unsigned char *)malloc(imageSize);
    unsigned char *reconstructed = (unsigned char *)calloc(imageSize, 1);
    for (int i = 0; i < imageSize; i++) {
        unpackedMSB[i]   = (packed[i / 8] >> (7 - (i % 8))) & 1;
        reconstructed[i] = unpackedMSB[i] ? 128 : 0;
    }

    char decFile[MAX_FILENAME];
    strcpy(decFile, inputFile);
    dot = strrchr(decFile, '.');
    if (dot) *dot = '\0';
    strcat(decFile, "_decompressed.bmp");

    writeGrayscaleBMP(decFile, reconstructed, width, height, isBottomUp);

    long origSize = fh.bfSize;
    long compSize = (long)(sizeof(int) * 2) + packedSize;
    printResultsBox("COMPRESSION SUMMARY  (MSB - LOSSY)",
                    "Lossy - only MSB stored, quality reduced",
                    outFile, decFile, origSize, compSize);

    free(imageData); free(msbPlane); free(packed);
    free(unpackedMSB); free(reconstructed);
}


/* ============================================================
 *  Full (lossless) - compress
 * ============================================================ */

static void compressBMPBitPlaneFull(const char *inputFile)
{
    printf("\n=== BMP IMAGE COMPRESSION (FULL) ===\n");
    printf("Detected: BMP ImageFile\n");
    printf("Bit Plane Slicing (Full Lossless) is used for compression.\n");

    BMPFileHeader fh; BMPInfoHeader ih;
    int width, height, isBottomUp, rowPadding;
    if (!openGrayscaleBMP(inputFile, &fh, &ih, &width, &height, &isBottomUp, &rowPadding))
        return;

    int imageSize = width * height;

    FILE *fp = fopen(inputFile, "rb");
    fread(&fh, sizeof(BMPFileHeader), 1, fp);
    fread(&ih, sizeof(BMPInfoHeader), 1, fp);
    unsigned char palette[1024];
    fread(palette, 1, 1024, fp);

    unsigned char *imageData = (unsigned char *)malloc(imageSize);
    for (int i = 0; i < height; i++) {
        int row = isBottomUp ? (height - 1 - i) : i;
        fread(imageData + row * width, 1, width, fp);
        fseek(fp, rowPadding, SEEK_CUR);
    }
    fclose(fp);

    printf("Step 1: Extracting all 8 bit-planes...\n");
    unsigned char *bitPlanes[8];
    for (int p = 0; p < 8; p++) {
        bitPlanes[p] = (unsigned char *)malloc(imageSize);
        for (int i = 0; i < imageSize; i++)
            bitPlanes[p][i] = (imageData[i] >> p) & 1;
    }

    printf("Step 2: Packing bits into bytes for each plane...\n");
    int packedPerPlane = (imageSize + 7) / 8;
    int totalPacked    = packedPerPlane * 8;
    unsigned char *allPacked = (unsigned char *)calloc(totalPacked, 1);

    for (int p = 0; p < 8; p++) {
        int base = p * packedPerPlane;
        for (int i = 0; i < imageSize; i++)
            if (bitPlanes[p][i])
                allPacked[base + i / 8] |= (1 << (7 - (i % 8)));
    }

    char outFile[MAX_FILENAME];
    strcpy(outFile, inputFile);
    char *dot = strrchr(outFile, '.');
    if (dot) *dot = '\0';
    strcat(outFile, "_full_compressed.bps");

    fp = fopen(outFile, "wb");
    fwrite(&width,    sizeof(int), 1, fp);
    fwrite(&height,   sizeof(int), 1, fp);
    fwrite(allPacked, 1, totalPacked, fp);
    fclose(fp);

    printf("Step 3: Decompressing...\n");
    unsigned char *reconstructed = (unsigned char *)calloc(imageSize, 1);
    for (int p = 0; p < 8; p++) {
        int base = p * packedPerPlane;
        for (int i = 0; i < imageSize; i++) {
            int val = (allPacked[base + i / 8] >> (7 - (i % 8))) & 1;
            if (val) reconstructed[i] |= (1 << p);
        }
    }

    char decFile[MAX_FILENAME];
    strcpy(decFile, inputFile);
    dot = strrchr(decFile, '.');
    if (dot) *dot = '\0';
    strcat(decFile, "_full_decompressed.bmp");

    writeGrayscaleBMP(decFile, reconstructed, width, height, isBottomUp);

    long origSize = fh.bfSize;
    long compSize = (long)(sizeof(int) * 2) + totalPacked;
    printResultsBox("COMPRESSION SUMMARY  (FULL - LOSSLESS)",
                    "Lossless - all 8 planes stored, no data lost",
                    outFile, decFile, origSize, compSize);

    free(imageData);
    for (int p = 0; p < 8; p++) free(bitPlanes[p]);
    free(allPacked); free(reconstructed);
}


/* ============================================================
 *  Standalone decompression (from .bps file)
 * ============================================================ */

static void decompressBPS(const char *fileName)
{
    printf("\n=== BMP DECOMPRESSION ===\n");

    FILE *fp = fopen(fileName, "rb");
    if (!fp) { printf("Error: Cannot open compressed file!\n"); return; }

    int width, height;
    fread(&width,  sizeof(int), 1, fp);
    fread(&height, sizeof(int), 1, fp);
    fclose(fp);

    fp = fopen(fileName, "rb");
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fclose(fp);

    int imageSize  = width * height;
    int msbSize    = (int)(sizeof(int) * 2) + (imageSize + 7) / 8;
    int fullSize   = (int)(sizeof(int) * 2) + ((imageSize + 7) / 8) * 8;
    int isMSB      = (labs(fileSize - msbSize) < labs(fileSize - fullSize));

    char decFile[MAX_FILENAME];
    strcpy(decFile, fileName);
    char *dot = strrchr(decFile, '.');
    if (dot) *dot = '\0';
    strcat(decFile, "_decompressed.bmp");

    if (isMSB) {
        printf("Detected: MSB Bit Plane compressed file\n\n");

        int packedSize = (imageSize + 7) / 8;
        unsigned char *packed        = (unsigned char *)malloc(packedSize);
        unsigned char *reconstructed = (unsigned char *)calloc(imageSize, 1);

        fp = fopen(fileName, "rb");
        fread(&width,  sizeof(int), 1, fp);
        fread(&height, sizeof(int), 1, fp);
        fread(packed, 1, packedSize, fp);
        fclose(fp);

        for (int i = 0; i < imageSize; i++)
            reconstructed[i] = ((packed[i / 8] >> (7 - (i % 8))) & 1) ? 128 : 0;

        writeGrayscaleBMP(decFile, reconstructed, width, height, 1);

        { char buf[256];
          printf("\n");
          box_divider();
          box_row("  DECOMPRESSION COMPLETE  (BIT PLANE - MSB)");
          box_divider();
          box_row("  Mode   : MSB Bit Plane (Lossy)");
          snprintf(buf, sizeof(buf), "  Output : %s", decFile); box_row(buf);
          box_row("  Note   : Image quality reduced (MSB only)");
          box_divider();
          printf("\n"); }

        free(packed); free(reconstructed);
    } else {
        printf("Detected: Full Bit Plane compressed file\n\n");

        int packedPerPlane = (imageSize + 7) / 8;
        int totalPacked    = packedPerPlane * 8;
        unsigned char *packed        = (unsigned char *)malloc(totalPacked);
        unsigned char *reconstructed = (unsigned char *)calloc(imageSize, 1);

        fp = fopen(fileName, "rb");
        fread(&width,  sizeof(int), 1, fp);
        fread(&height, sizeof(int), 1, fp);
        fread(packed, 1, totalPacked, fp);
        fclose(fp);

        for (int p = 0; p < 8; p++) {
            int base = p * packedPerPlane;
            for (int i = 0; i < imageSize; i++) {
                int val = (packed[base + i / 8] >> (7 - (i % 8))) & 1;
                if (val) reconstructed[i] |= (1 << p);
            }
        }

        writeGrayscaleBMP(decFile, reconstructed, width, height, 1);

        { char buf[256];
          printf("\n");
          box_divider();
          box_row("  DECOMPRESSION COMPLETE  (BIT PLANE - FULL LOSSLESS)");
          box_divider();
          box_row("  Mode   : Full Bit Plane (Lossless)");
          snprintf(buf, sizeof(buf), "  Output : %s", decFile); box_row(buf);
          box_row("  Note   : Perfect reconstruction - no data lost");
          box_divider();
          printf("\n"); }

        free(packed); free(reconstructed);
    }
}


/* ============================================================
 *  Public entry point
 * ============================================================ */

void processBMPFile(const char *fileName, int operation)
{
    if (operation == 2) {
        const char *ext = strrchr(fileName, '.');
        if (!ext || strcmp(ext, ".bps") != 0) {
            printf("\nERROR: This file is not compressed!\n");
            printf("Please compress the file first (choose option 1).\n\n");
            return;
        }
        decompressBPS(fileName);
        return;
    }

    /* Compression: let user pick variant */
    int choice;
    printf("\n  +-------------------------------------------+\n");
    printf("  |       BMP COMPRESSION OPTIONS             |\n");
    printf("  +-------------------------------------------+\n");
    printf("  | 1. MSB Bit Plane  (Lossy  - max savings)  |\n");
    printf("  | 2. Full Bit Plane (Lossless - no loss)    |\n");
    printf("  | 3. Both methods   (compare side by side)  |\n");
    printf("  +-------------------------------------------+\n");
    printf("  Enter your choice (1-3): ");

    if (scanf("%d", &choice) != 1) {
        printf("Invalid input.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n'); /* flush trailing newline so next fgets works */

    switch (choice) {
        case 1: compressBMPBitPlaneMSB(fileName);  break;
        case 2: compressBMPBitPlaneFull(fileName); break;
        case 3:
            compressBMPBitPlaneMSB(fileName);
            printf("\n  ========================================================\n\n");
            compressBMPBitPlaneFull(fileName);
            break;
        default: printf("Invalid choice.\n"); break;
    }
}