/* ============================================================
 * dct_compress.c  -  DCT compression for color images
 *
 *  Pipeline per 8x8 block:
 *    Forward DCT -> quantize -> (count non-zeros)
 *    -> dequantize -> inverse DCT -> write back
 * ============================================================ */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "types.h"
#include "color_image.h"
#include "dct_compress.h"

/* ============================================================
 *  DCT math
 * ============================================================ */

static double dctCoeff(int x, int y, int u, int v, int N)
{
    double cu = (u == 0) ? 1.0 / sqrt(2.0) : 1.0;
    double cv = (v == 0) ? 1.0 / sqrt(2.0) : 1.0;
    return (2.0 / N) * cu * cv
         * cos((2 * x + 1) * u * PI / (2.0 * N))
         * cos((2 * y + 1) * v * PI / (2.0 * N));
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
            if (val < 0)      val = 0;
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

/* ============================================================
 *  Per-channel compression
 * ============================================================ */

typedef struct { long totalCoeffs; long nonZeroCoeffs; } DCTStats;

static DCTStats compressChannel(int **channel, int width, int height,
                                 int maxVal, int quality)
{
    DCTStats stats = {0, 0};

    for (int i = 0; i < height; i += 8) {
        for (int j = 0; j < width; j += 8) {
            int    block[8][8]    = {{0}};
            double dctBlock[8][8];

            for (int x = 0; x < 8 && i + x < height; x++)
                for (int y = 0; y < 8 && j + y < width; y++)
                    block[x][y] = channel[i + x][j + y];

            dct2D(block, dctBlock);
            quantize(dctBlock, quality);

            for (int x = 0; x < 8; x++)
                for (int y = 0; y < 8; y++) {
                    stats.totalCoeffs++;
                    if (dctBlock[x][y] != 0.0) stats.nonZeroCoeffs++;
                }

            dequantize(dctBlock, quality);
            idct2D(dctBlock, block, maxVal);

            for (int x = 0; x < 8 && i + x < height; x++)
                for (int y = 0; y < 8 && j + y < width; y++)
                    channel[i + x][j + y] = block[x][y];
        }
    }
    return stats;
}

/* ============================================================
 *  Shared pretty-printer (same style as bmp_compress.c)
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
                             long origSize, long compSize,
                             const char *extraLabel, const char *extraValue)
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
    if (extraLabel && extraValue) {
        snprintf(buf, sizeof(buf), "  %-13s: %s", extraLabel, extraValue);
        box_row(buf);
    }
    box_divider();
    snprintf(buf, sizeof(buf), "  Decompressed : %s", decFile);          box_row(buf);
    box_divider();
    printf("\n");
}

/* ============================================================
 *  Public entry points
 * ============================================================ */

static void compressColorImage(const char *inputFile)
{
    printf("\n=== COLOR IMAGE COMPRESSION ===\n");
    printf("Algorithm: DCT (Discrete Cosine Transform)\n\n");

    ColorImage *img = loadColorImage(inputFile);
    if (!img) return;

    const int quality = 50;

    printf("Applying DCT compression to color channels...\n");
    DCTStats sR = compressChannel(img->R, img->width, img->height, img->maxVal, quality);
    DCTStats sG = compressChannel(img->G, img->width, img->height, img->maxVal, quality);
    DCTStats sB = compressChannel(img->B, img->width, img->height, img->maxVal, quality);
    DCTStats sA = {0, 0};
    if (img->hasAlpha) {
        printf("Compressing alpha channel...\n");
        sA = compressChannel(img->A, img->width, img->height, img->maxVal, quality);
    }

    int  channels    = img->hasAlpha ? 4 : 3;
    long origSize    = (long)img->width * img->height * channels;
    long totalNZ     = sR.nonZeroCoeffs + sG.nonZeroCoeffs + sB.nonZeroCoeffs + sA.nonZeroCoeffs;
    long totalCoeffs = sR.totalCoeffs   + sG.totalCoeffs   + sB.totalCoeffs   + sA.totalCoeffs;
    long compSize    = totalNZ * 2;

    /* Build filenames */
    const char *inputExt = strrchr(inputFile, '.');
    char compFile[MAX_FILENAME];
    strcpy(compFile, inputFile);
    char *dot = strrchr(compFile, '.');
    if (dot) *dot = '\0';

    if (inputExt && (strcmp(inputExt, ".bmp") == 0 || strcmp(inputExt, ".BMP") == 0))
        strcat(compFile, "_compressed.bmp");
    else if (img->hasAlpha)
        strcat(compFile, "_compressed.pam");
    else
        strcat(compFile, "_compressed.ppm");

    saveColorImage(compFile, img);

    char decFile[MAX_FILENAME];
    strcpy(decFile, inputFile);
    dot = strrchr(decFile, '.');
    if (dot) *dot = '\0';

    if (inputExt && (strcmp(inputExt, ".bmp") == 0 || strcmp(inputExt, ".BMP") == 0))
        strcat(decFile, "_decompressed.bmp");
    else if (img->hasAlpha)
        strcat(decFile, "_decompressed.pam");
    else
        strcat(decFile, "_decompressed.ppm");

    saveColorImage(decFile, img);

    /* Extra info: coefficient retention */
    char extraVal[64];
    double retained = totalCoeffs > 0 ? 100.0 * totalNZ / totalCoeffs : 0.0;
    snprintf(extraVal, sizeof(extraVal), "%.1f%% coeffs kept  (Q=%d)", retained, quality);

    printResultsBox("COMPRESSION SUMMARY  (DCT - LOSSY)",
                    img->hasAlpha ? "Lossy - 32-bit RGBA, 8x8 block DCT"
                                  : "Lossy - 24-bit RGB, 8x8 block DCT",
                    compFile, decFile,
                    origSize, compSize,
                    "Coefficients", extraVal);

    freeColorImage(img);
}

static void decompressColorImage(const char *fileName)
{
    printf("\n=== COLOR IMAGE DECOMPRESSION ===\n");
    printf("Note: DCT is lossy - quality was already reduced during compression.\n\n");

    ColorImage *img = loadColorImage(fileName);
    if (!img) return;

    char decFile[MAX_FILENAME];
    strcpy(decFile, fileName);
    char *dot = strrchr(decFile, '.');
    if (dot) *dot = '\0';

    char *compStr = strstr(decFile, "_compressed");
    if (compStr) *compStr = '\0';
    strcat(decFile, "_decompressed");

    const char *ext = strrchr(fileName, '.');
    strcat(decFile, ext ? ext : ".bmp");

    saveColorImage(decFile, img);

    { char buf[256];
      printf("\n");
      box_divider();
      box_row("  DECOMPRESSION COMPLETE  (DCT)");
      box_divider();
      box_row("  Algorithm    : DCT (Lossy)");
      snprintf(buf, sizeof(buf), "  Format       : %s", img->hasAlpha ? "32-bit RGBA" : "24-bit RGB"); box_row(buf);
      snprintf(buf, sizeof(buf), "  Output       : %s", decFile); box_row(buf);
      box_row("  Note         : Quality was reduced during compression");
      box_divider();
      printf("\n"); }

    freeColorImage(img);
}

void processColorFile(const char *fileName, int operation)
{
    if (operation == 1) {
        compressColorImage(fileName);
        return;
    }

    if (!strstr(fileName, "_compressed")) {
        printf("\nERROR: This file is not compressed!\n");
        printf("Please compress the file first (choose operation 1).\n\n");
        return;
    }

    decompressColorImage(fileName);
}