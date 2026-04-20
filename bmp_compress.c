#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "types.h"
#include "bmp_compress.h"

//Reads pixel data from a BMP file row by row, handles padding, and stores image pixels in correct order (top-down or bottom-up).                               
static int readGrayscalePixels(FILE *fp, unsigned char *imageData,    
                                int width, int height, int isBottomUp,
                                int rowPadding, long pixelOffset)
{
    unsigned char padBuf[4];
    fseek(fp, pixelOffset, SEEK_SET);

    for (int i = 0; i < height; i++) {
        int row;

        if (isBottomUp)
         {
           row = height - 1 - i;
         } 
        else 
        {
            row = i;
        }

        size_t got = fread(imageData + row * width, 1, width, fp);

        if (got < (size_t)width) {
            printf("Error reading pixel data\n");
            return 0;
        }
        if (rowPadding > 0)
            fread(padBuf, 1, rowPadding, fp);
    }
    return 1;
}

//Creates and writes a 256-color grayscale palette (0–255 intensity values) into the BMP file.
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

//Writes a full 8-bit grayscale BMP image to a file, including header, palette, pixel data, and row padding.
static void writeGrayscaleBMP(const char *filename,    
                               unsigned char *pixels,
                               int width, int height, int isBottomUp)
{
    //int rowPadding = (4 - (width % 4)) % 4;
    int rowPadding;
    int remainder = width % 4;
    rowPadding = 4 - remainder;

    if (rowPadding == 4) 
    {
         rowPadding = 0;
    }

    BMPFileHeader fh;
    BMPInfoHeader ih;

    fh.bfType          = 0x4D42;
    fh.bfReserved1     = 0;
    fh.bfReserved2     = 0;
    fh.bfOffBits       = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + 1024;
    fh.bfSize          = fh.bfOffBits + (unsigned int)((width + rowPadding) * height);

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
    if (!fp)
     { 
          printf("Error creating file\n");
          return;
         }

    fwrite(&fh, sizeof(BMPFileHeader), 1, fp);
    fwrite(&ih, sizeof(BMPInfoHeader), 1, fp);

    writeGrayscalePalette(fp);
    unsigned char pad[3] = {0, 0, 0};

    for (int i = 0; i < height; i++) 
    {
        int row;

        if (isBottomUp)
         {
           row = height - 1 - i;
         }
        else 
         {
          row = i;
         }
        fwrite(pixels + row * width, 1, width, fp);
        if (rowPadding > 0)
            fwrite(pad, 1, rowPadding, fp);
    }
    fclose(fp);
}

//Opens a BMP file, reads headers, validates format (8-bit grayscale BMP), and extracts image metadata like width, height, and padding.
static int openGrayscaleBMP(const char *filename,       
                              BMPFileHeader *fh, BMPInfoHeader *ih,
                              int *width, int *height, int *isBottomUp,
                              int *rowPadding)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) 
    { 
        printf("Error opening file\n"); 
        return 0; 
    }

    fread(fh, sizeof(BMPFileHeader), 1, fp);
    fread(ih, sizeof(BMPInfoHeader), 1, fp);
    fclose(fp);

    if (fh->bfType != 0x4D42) 
    {
        printf("Not a BMP file.\n");
        return 0;
    }

    if (ih->biBitCount != 8) 
    {
        printf("Only 8-bit grayscale BMP images are supported\n");
        return 0;
    }

    *width      = ih->biWidth;
    *isBottomUp = (ih->biHeight > 0);
    *height     = *isBottomUp ? ih->biHeight : -ih->biHeight;
    *rowPadding = (4 - (*width % 4)) % 4;
    return 1;
}

static void printResultsBox(const char *title, const char *mode,
                           const char *outFile, const char *decFile,
                           long origSize, long compSize)

{

    double reduction = 0.0;
    if (origSize > 0 && compSize < origSize) 
    {
       reduction = (1.0 - (double)compSize / origSize) * 100.0;
    }

     double ratio = 1.0;
     if (compSize > 0)
      {
        ratio = (double)origSize / compSize;
      }

    int filled = (int)(reduction / 10.0);   //rating'****'
    if (filled > 10) 
        filled = 10;
    if (filled <  0) 
        filled = 0;
    char bar[11];

   for (int i = 0; i < 10; i++)
    {
       if (i < filled) {
           bar[i] = '#';
        }    
         else
          {
            bar[i] = '-';
         }
    }

    bar[10] = '\0';

    const char *stars, *verdict;
    if(reduction >= 85) 
     { 
        stars = "*****";
        verdict = "EXCELLENT";
     }
    else if(reduction >= 70) 
      {
         stars = "**** ";
         verdict = "GREAT"; 
        }
    else if(reduction >= 50)
        {
         stars = "***  ";
         verdict = "GOOD";
         }
    else if(reduction >= 30)
     { 
        stars = "**   ";
       verdict = "FAIR"; 
    }
    else                     
     { 
        stars = "*    ";
        verdict = "LOW";
     }

    printf("\n");
    printf("+---------------------------------------------------------------+\n");
    printf("| %-58s    |\n", title);
    printf("+---------------------------------------------------------------+\n");
    printf("| Output File  : %-43s    |\n", outFile);
    printf("+---------------------------------------------------------------+\n");
    printf("| Original Size: %-43ld    |\n", origSize);
    printf("| Compressed   : %-43ld    |\n", compSize);
    printf("+---------------------------------------------------------------+\n");
    printf("| Space Saved  : %-6.2f%%  [%-10s]                          |\n", reduction, bar);
    printf("| Ratio        : %-10.2f : 1                                 |\n", ratio);
    printf("| Rating       : %-10s %-5s                               |\n", verdict, stars);
    printf("+---------------------------------------------------------------+\n");
    printf("| Decompressed : %-43s    |\n", decFile);
    printf("+---------------------------------------------------------------+\n\n");
}

static void compressBMPBitPlaneMSB(const char *inputFile)    // MSB (lossy) compression
{
    printf("\n=== BMP IMAGE COMPRESSION (MSB) ===\n");
    printf("Detected: BMP ImageFile\n");
    printf("Bit Plane Slicing (MSB) is used for compression.\n");

    BMPFileHeader fh;
    BMPInfoHeader ih;

    int width, height, isBottomUp, rowPadding;

    if (!openGrayscaleBMP(inputFile, &fh, &ih, &width, &height, &isBottomUp, &rowPadding))
        return;

    int imageSize = width * height;
    printf("Image infos:\n");
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

    // Step 1: keep only the most-significant bit of each pixel 
    printf("Step 1: Extracting MSB from each pixel...\n");
    unsigned char *msbPlane = (unsigned char *)malloc(imageSize);

    for (int i = 0; i < imageSize; i++)
     {
        if (imageData[i] >= 128)
           msbPlane[i] = 1;
        else
            msbPlane[i] = 0;
      }
    
   // printf("Step 2: Packing MSB bits into bytes...\n");
    int packedSize = (imageSize + 7) / 8;
    unsigned char *packed = (unsigned char *)calloc(packedSize, 1);
    for (int i = 0; i < imageSize; i++) {

    if (msbPlane[i] == 0) {

        int byteIndex = i / 8;              
        int bitPosition = 7 - (i % 8);      

        int value = 1;

        for (int j = 0; j < bitPosition; j++)  // calculate 2^(bitPosition)
        {
            value *= 2;
        }
       
        packed[byteIndex] += value;
      }
    }

    /* Write compressed file: width + height + packed bits */
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

    /* Step 3: decompress*/
    //printf("Step 3: Decompressing...\n");
    unsigned char *unpackedMSB   = (unsigned char *)malloc(imageSize);
    unsigned char *reconstructed = (unsigned char *)calloc(imageSize, 1);

    for (int i = 0; i < imageSize; i++) 
    {
       int byteIndex = i / 8;           
       int bitPosition = 7 - (i % 8);  
       int value = packed[byteIndex];

       for (int j = 0; j < bitPosition; j++) 
       {
          value /= 2;
       }

       int bit = value % 2;
       unpackedMSB[i] = bit;

        if (bit == 1)
           reconstructed[i] = 128;
        else
            reconstructed[i] = 0;
    }

    char decFile[MAX_FILENAME];
    strcpy(decFile, inputFile);
    dot = strrchr(decFile, '.');
    if (dot) 
        *dot = '\0';
    strcat(decFile, "_decompressed.bmp");

    writeGrayscaleBMP(decFile, reconstructed, width, height, isBottomUp);

    long origSize = fh.bfSize;
    long compSize = (long)(sizeof(int) * 2) + packedSize;
    printResultsBox("COMPRESSION SUMMARY  (MSB - LOSSY)",
                    "Lossy - only MSB stored, quality reduced",
                    outFile, decFile, origSize, compSize);

    free(imageData); 
    free(msbPlane);
    free(packed);
    free(unpackedMSB); 
    free(reconstructed);
}

static void compressBMPBitPlaneFull(const char *inputFile)  //full bit compression
{
    printf("\n=== BMP IMAGE COMPRESSION (FULL) ===\n");
    printf("Detected: BMP ImageFile\n");
    printf("Bit Plane Slicing (Full Lossless) is used for compression.\n");

    BMPFileHeader fh;
    BMPInfoHeader ih;

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
    for (int i = 0; i < height; i++) 
    {
       int row;

       if (isBottomUp) 
       {
          row = height - 1 - i;
       }
        else
         {
           row = i;
          }
        fread(imageData + row * width, 1, width, fp);
        fseek(fp, rowPadding, SEEK_CUR);
    }
    fclose(fp);

    /* Step 1: extract all 8 bit-planes */
    //printf("Step 1: Extracting all 8 bit-planes...\n");
   
     unsigned char *bitPlanes[8];

     for (int p = 0; p < 8; p++)
      {
         bitPlanes[p] = (unsigned char *)malloc(imageSize);

         for (int i = 0; i < imageSize; i++) 
         {
           int divisor = (int)pow(2, p);
           bitPlanes[p][i] = (imageData[i] / divisor) % 2;
    }
}

    /* Step 2: pack each plane's bits into bytes, store back-to-back */
    //printf("Step 2: Packing bits into bytes for each plane...\n");
    int packedPerPlane = (imageSize + 7) / 8;
    int totalPacked    = packedPerPlane * 8;
    unsigned char *allPacked = (unsigned char *)calloc(totalPacked, 1);

    for (int p = 0; p < 8; p++)
     {
       int base = p * packedPerPlane;

       for (int i = 0; i < imageSize; i++) 
       {
          if (bitPlanes[p][i]) 
          {
            int byteIndex = base + (i / 8);
            int bitPosition = 7 - (i % 8);
            int value = (int)pow(2, bitPosition);
            allPacked[byteIndex] += value;
        }
    }
}

    /* Writing the compressed file */
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

    /* Step 3: reconstruct each pixel by OR-ing bits back */
    //printf("Step 3: Decompressing...\n");
    unsigned char *reconstructed = (unsigned char *)calloc(imageSize, 1);
   

    for (int p = 0; p < 8; p++) 
    {
       int base = p * packedPerPlane;

       for (int i = 0; i < imageSize; i++)
        {
            int byteIndex = base + (i / 8);
            int bitPosition = 7 - (i % 8);
            int value = allPacked[byteIndex];

            int divisor = (int)pow(2, bitPosition);
            value = value / divisor;
            int bit = value % 2;

            if (bit == 1) 
            {
               int addValue = (int)pow(2, p);
                reconstructed[i] += addValue;
            }
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
    for (int p = 0; p < 8; p++) 
    free(bitPlanes[p]);
    free(allPacked); 
    free(reconstructed);
}

// Public entry point
void processBMPFile(const char *fileName, int operation)
{
    /* Compression */
    int choice;
    printf("\n  +-------------------------------------------+\n");
    printf("  |       BMP COMPRESSION OPTIONS             |\n");
    printf("  +-------------------------------------------+\n");
    printf("  | 1. MSB Bit Plane  (Lossy  - max savings)  |\n");
    printf("  | 2. Full Bit Plane (Lossless - no loss)    |\n");
    printf("  | 3. Both methods   (compare side by side)  |\n");
    printf("  | 4. Quit                                   |\n");
    printf("  +-------------------------------------------+\n");
    printf("  Enter your choice (1-4): ");

    int result = scanf("%d", &choice);

    if (result != 1) {
       printf("Invalid input.\n");

     int ch;
     while (1) {
         ch = getchar();
         if (ch == '\n' || ch == EOF) 
         {
            break;
         }
       }
     return;
   }

    int ch;
    while (1) {
    ch = getchar();
    if (ch == '\n' || ch == EOF)
     {
        break;
     }
   }

    switch (choice) 
    {
        case 1:
          compressBMPBitPlaneMSB(fileName);
          break;
        case 2: 
          compressBMPBitPlaneFull(fileName); 
          break;
        case 3:
            compressBMPBitPlaneMSB(fileName);
            printf("\n  ========================================================\n\n");
            compressBMPBitPlaneFull(fileName);
            break;
        case 4:
            printf("Quitting...\n");
            return;
        default:
            printf("Invalid choice.\n"); 
            break;
    }
}