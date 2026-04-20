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
    double cu;
    double cv;

    if (u == 0) {
        cu = 1.0 / sqrt(2.0);
    } 
    else
    {
        cu = 1.0;
    }

    if (v == 0)
     {
        cv = 1.0 / sqrt(2.0);
    } 
    else 
    {
        cv = 1.0;
    }

    double term1 = (2 * x + 1) * u * M_PI;
    double term2 = (2.0 * N);
    double cos1 = cos(term1 / term2);

    double term3 = (2 * y + 1) * v * M_PI;
    double cos2 = cos(term3 / term2);

    double result = (2.0 / N) * cu * cv * cos1 * cos2;

    return result;
}

static void dct2D(int block[8][8], double dctBlock[8][8])
{
    int u, v, x, y;

    for (u = 0; u < 8; u++) {
        for (v = 0; v < 8; v++) {
            double sum = 0.0;
            for (x = 0; x < 8; x++) {
                for (y = 0; y < 8; y++) {
                    double pixel = block[x][y];
                    double coeff = dctCoeff(x, y, u, v, 8);
                    sum = sum + (pixel * coeff);
                }
            }
            dctBlock[u][v] = sum;
        }
    }
}

static void idct2D(double dctBlock[8][8], int block[8][8], int maxVal)
{
    int x, y, u, v;
    for (x = 0; x < 8; x++) {
        for (y = 0; y < 8; y++) {
            double sum = 0.0;
            for (u = 0; u < 8; u++) {
                for (v = 0; v < 8; v++) {
                    double freqValue = dctBlock[u][v];
                    double coeff = dctCoeff(x, y, u, v, 8);
                    double product = freqValue * coeff;

                    sum = sum + product;
                }
            }
            double roundedValue = round(sum);
            int val = (int)roundedValue;

            if (val < 0) 
            {
                val = 0;
            }

            if (val > maxVal) 
            {
                val = maxVal;
            }

            block[x][y] = val;
        }
    }
}

static void quantize(double dctBlock[8][8], int quality)
{
    int q;
    if (quality > 0) 
    {
        q = quality;
    } 
    else
     {
        q = 1;
     }
    int i, j;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            double value = dctBlock[i][j];
            double divided = value / q;
            double rounded = round(divided);
            dctBlock[i][j] = rounded;
        }
    }
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
    DCTStats stats;
    stats.totalCoeffs = 0;
    stats.nonZeroCoeffs = 0;
    int i, j;
    for (i = 0; i < height; i = i + 8) {
        for (j = 0; j < width; j = j + 8) {
            int x, y;
            int block[8][8];
            double dctBlock[8][8];

            // Initialize block with zeros
            for (x = 0; x < 8; x++) {
                for (y = 0; y < 8; y++) {
                    block[x][y] = 0;
                }
            }

            // Copy 8x8 pixels from image into block
            for (x = 0; x < 8; x++) {
                for (y = 0; y < 8; y++) {
                    if ((i + x < height) && (j + y < width)) {
                        block[x][y] = channel[i + x][j + y];
                    }
                }
            }

            // Step 1: Apply DCT
            dct2D(block, dctBlock);

            // Step 2: Quantize (compression step)
            quantize(dctBlock, quality);

            // Step 3: Count coefficients
            for (x = 0; x < 8; x++) {

                for (y = 0; y < 8; y++) {

                    stats.totalCoeffs = stats.totalCoeffs + 1;

                    if (dctBlock[x][y] != 0.0) {
                        stats.nonZeroCoeffs = stats.nonZeroCoeffs + 1;
                    }
                }
            }

            // Step 4: Dequantize
            dequantize(dctBlock, quality);

            // Step 5: Apply inverse DCT
            idct2D(dctBlock, block, maxVal);

            // Step 6: Write back to image
            for (x = 0; x < 8; x++) {

                for (y = 0; y < 8; y++) {

                    if ((i + x < height) && (j + y < width)) {
                        channel[i + x][j + y] = block[x][y];
                    }
                }
            }
        }
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
    printf("| Output File  : %-43s |\n", outFile);
    printf("+------------------------------------------------------------+\n");
    printf("| Original Size: %-43ld |\n", origSize);
    printf("| Compressed   : %-43ld |\n", compSize);
    printf("+------------------------------------------------------------+\n");
    printf("| Space Saved  : %-6.2f%%                                     |\n", reduction);
    printf("| Ratio        : %-10.2f : 1                              |\n", ratio);

    if (extraLabel && extraValue)
        printf("| %-13s : %-41s  |\n", extraLabel, extraValue);

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
    double retained;
    double totalCoeffs;

    totalCoeffs = sR.totalCoeffs + sG.totalCoeffs + sB.totalCoeffs + sA.totalCoeffs;      retained = 100.0 * totalNZ;
    retained = retained / totalCoeffs;

    char percentStr[100];
    sprintf(percentStr, "%.1f%% coeffs kept (Q=%d)", retained, quality);
    strcpy(extra, percentStr);
    
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

    ColorImage *img;
    img = loadColorImage(fileName);
      if (img == NULL)
      {
        return;
      }

    char decFile[MAX_FILENAME];
    strcpy(decFile, fileName);
    char *dot;
    dot = strrchr(decFile, '.');

    if (dot!= NULL) 
       *dot = '\0';

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
    char *check;
    check = strstr(fileName, "_compressed");

    if (check == NULL)
    {
        printf("ERROR: Not a compressed file. Compress first.\n");
        return;
    }
    decompressColorImage(fileName);
}