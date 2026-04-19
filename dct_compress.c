#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "types.h"
#include "color_image.h"
#include "dct_compress.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static double dctCoeff(int x, int y, int u, int v, int N)
{
    double cu = (u == 0) ? 1.0 / sqrt(2.0) : 1.0;
    double cv = (v == 0) ? 1.0 / sqrt(2.0) : 1.0;

    return (2.0 / N) * cu * cv
         * cos((2 * x + 1) * u * M_PI / (2.0 * N))
         * cos((2 * y + 1) * v * M_PI / (2.0 * N));
}

static void dct2D(int block[8][8], double dctBlock[8][8])
{
    for (int u = 0; u < 8; u++)
        for (int v = 0; v < 8; v++) {
            double sum = 0.0;
            for (int x = 0; x < 8; x++)
                for (int y = 0; y < 8; y++)
                    sum += block[x][y] * dctCoeff(x, y, u, v, 8);
            dctBlock[u][v] = sum;
        }
}

static void idct2D(double dctBlock[8][8], int block[8][8], int maxVal)
{
    for (int x = 0; x < 8; x++)
        for (int y = 0; y < 8; y++) {
            double sum = 0.0;
            for (int u = 0; u < 8; u++)
                for (int v = 0; v < 8; v++)
                    sum += dctBlock[u][v] * dctCoeff(x, y, u, v, 8);

            int val = (int)round(sum);
            if (val < 0) val = 0;
            if (val > maxVal) val = maxVal;
            block[x][y] = val;
        }
}

static void quantize(double dctBlock[8][8], int quality)
{
    int q = (quality > 0) ? quality : 1;
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            dctBlock[i][j] = round(dctBlock[i][j] / q);
}

static void dequantize(double dctBlock[8][8], int quality)
{
    int q = (quality > 0) ? quality : 1;
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            dctBlock[i][j] *= q;
}

typedef struct {
    long totalCoeffs;
    long nonZeroCoeffs;
} DCTStats;

static DCTStats compressChannel(int **channel, int width, int height,
                                int maxVal, int quality)
{
    DCTStats stats = {0, 0};

    for (int i = 0; i < height; i += 8)
        for (int j = 0; j < width; j += 8) {
            int block[8][8] = {{0}};
            double dctBlock[8][8];

            for (int x = 0; x < 8 && i + x < height; x++)
                for (int y = 0; y < 8 && j + y < width; y++)
                    block[x][y] = channel[i + x][j + y];

            dct2D(block, dctBlock);
            quantize(dctBlock, quality);

            for (int x = 0; x < 8; x++)
                for (int y = 0; y < 8; y++) {
                    stats.totalCoeffs++;
                    if (dctBlock[x][y] != 0.0)
                        stats.nonZeroCoeffs++;
                }

            dequantize(dctBlock, quality);
            idct2D(dctBlock, block, maxVal);

            for (int x = 0; x < 8 && i + x < height; x++)
                for (int y = 0; y < 8 && j + y < width; y++)
                    channel[i + x][j + y] = block[x][y];
        }

    return stats;
}

static void printResults(const char *title,
                         const char *mode,
                         const char *outFile,
                         const char *decFile,
                         long origSize,
                         long compSize,
                         const char *extraLabel,
                         const char *extraValue)
{
    double reduction = (origSize > 0)
        ? (1.0 - (double)compSize / origSize) * 100.0 : 0.0;

    double ratio = (compSize > 0) ? (double)origSize / compSize : 1.0;

    printf("\n");
    printf("+------------------------------------------------------------+\n");
    printf("| %-58s |\n", title);
    printf("+------------------------------------------------------------+\n");
   // printf("| Mode         : %-43s |\n", mode);
    printf("| Output File  : %-43s |\n", outFile);
    printf("+------------------------------------------------------------+\n");
    printf("| Original Size: %-43ld |\n", origSize);
    printf("| Compressed   : %-43ld |\n", compSize);
    printf("+------------------------------------------------------------+\n");
    printf("| Space Saved  : %-6.2f%%                              |\n", reduction);
    printf("| Ratio        : %-10.2f : 1                      |\n", ratio);

    if (extraLabel && extraValue)
        printf("| %-13s : %-41s |\n", extraLabel, extraValue);

    printf("+------------------------------------------------------------+\n");
    printf("| Decompressed : %-43s |\n", decFile);
    printf("+------------------------------------------------------------+\n\n");
}

static void compressColorImage(const char *inputFile)
{
    printf("\nCOLOR IMAGE COMPRESSION (DCT)\n");

    ColorImage *img = loadColorImage(inputFile);
    if (!img) return;

    int quality = 50;

    DCTStats sR = compressChannel(img->R, img->width, img->height, img->maxVal, quality);
    DCTStats sG = compressChannel(img->G, img->width, img->height, img->maxVal, quality);
    DCTStats sB = compressChannel(img->B, img->width, img->height, img->maxVal, quality);
    DCTStats sA = {0, 0};

    if (img->hasAlpha)
        sA = compressChannel(img->A, img->width, img->height, img->maxVal, quality);

    int channels = img->hasAlpha ? 4 : 3;

    long origSize = (long)img->width * img->height * channels;

    long totalNZ = sR.nonZeroCoeffs + sG.nonZeroCoeffs + sB.nonZeroCoeffs + sA.nonZeroCoeffs;

    long compSize = totalNZ * 2;

    char compFile[MAX_FILENAME];
    strcpy(compFile, inputFile);
    char *dot = strrchr(compFile, '.');
    if (dot) *dot = '\0';
    strcat(compFile, "_compressed.bmp");

    saveColorImage(compFile, img);

    char decFile[MAX_FILENAME];
    strcpy(decFile, inputFile);
    dot = strrchr(decFile, '.');
    if (dot) *dot = '\0';
    strcat(decFile, "_decompressed.bmp");

    saveColorImage(decFile, img);

    char extra[64];
    double retained = 100.0 * totalNZ /
        (sR.totalCoeffs + sG.totalCoeffs + sB.totalCoeffs + sA.totalCoeffs);

    snprintf(extra, sizeof(extra), "%.1f%% coeffs kept (Q=%d)", retained, quality);

    printResults("COMPRESSION RESULT",
                 img->hasAlpha ? "RGBA DCT" : "RGB DCT",
                 compFile,
                 decFile,
                 origSize,
                 compSize,
                 "Coefficients",
                 extra);

    freeColorImage(img);
}

static void decompressColorImage(const char *fileName)
{
    printf("\nCOLOR IMAGE DECOMPRESSION (DCT)\n");

    ColorImage *img = loadColorImage(fileName);
    if (!img) return;

    char decFile[MAX_FILENAME];
    strcpy(decFile, fileName);

    char *dot = strrchr(decFile, '.');
    if (dot) *dot = '\0';

    strcat(decFile, "_decompressed.bmp");

    saveColorImage(decFile, img);

    printf("+------------------------------------------------------------+\n");
    printf("| Mode         : DCT Decompression (Lossy)                   |\n");
    printf("| Output File  : %-43s |\n", decFile);
    printf("+------------------------------------------------------------+\n\n");

    freeColorImage(img);
}

void processColorFile(const char *fileName, int operation)
{
    if (operation == 1) {
        compressColorImage(fileName);
        return;
    }

    if (!strstr(fileName, "_compressed")) {
        printf("ERROR: Not a compressed file. Compress first.\n");
        return;
    }

    decompressColorImage(fileName);
}