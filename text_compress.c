#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "huffman.h"
#include "text_compress.h"

//decode
static int decodeBitString(const char *bits, int bitLen,
                            HuffmanCode *codes, int codeCount,
                            unsigned char *out, long originalSize)
{
    int  outIdx = 0;
    char cur[256] = "";
    int  curIdx   = 0;

    for (int i = 0; i < bitLen && outIdx < originalSize; i++) {
        cur[curIdx++] = bits[i];
        cur[curIdx]   = '\0';

        for (int j = 0; j < codeCount; j++) {
            if (strcmp(cur, codes[j].code) == 0) {
                out[outIdx++] = codes[j].character;
                curIdx = 0;
                cur[0] = '\0';
                break;
            }
        }
    }
    return outIdx;
}

static void printResults(const char *title, const char *mode,
                         const char *outFile, const char *decFile,
                         long origSize, long compSize,
                         const char *extraLabel, const char *extraValue)
{
    double reduction = (origSize > 0 && compSize < origSize)
                       ? (1.0 - (double)compSize / origSize) * 100.0 : 0.0;

    double ratio = (compSize > 0) ? (double)origSize / compSize : 1.0;

    int filled = (int)(reduction / 10.0);
    if (filled > 10) filled = 10;
    if (filled < 0) filled = 0;

    char bar[11];
    for (int i = 0; i < 10; i++)
        bar[i] = (i < filled) ? '#' : '-';
    bar[10] = '\0';

    const char *stars, *verdict;
    if      (reduction >= 85) { stars = "* * * * *"; verdict = "EXCELLENT"; }
    else if (reduction >= 70) { stars = "* * * * ."; verdict = "GREAT"; }
    else if (reduction >= 50) { stars = "* * * . ."; verdict = "GOOD"; }
    else if (reduction >= 30) { stars = "* * . . ."; verdict = "FAIR"; }
    else                      { stars = "* . . . ."; verdict = "LOW"; }

//     printf("\n");
//     printf("----------------------------------------------------------\n");
//     printf("%s\n", title);
//     printf("----------------------------------------------------------\n");

//     printf("Mode         : %s\n", mode);
//     printf("Saved to     : %s\n\n", outFile);

//     printf("Original     : %ld bytes\n", origSize);
//     printf("Compressed   : %ld bytes\n\n", compSize);

//     printf("Space Saved  : %.2f%%   [%s]\n", reduction, bar);
//     printf("Ratio        : %.2f : 1\n", ratio);
//     printf("Rating       : %s   %s\n\n", verdict, stars);

//     if (extraLabel && extraValue)
//         printf("%s : %s\n\n", extraLabel, extraValue);

//     printf("Decompressed : %s\n", decFile);

//     printf("----------------------------------------------------------\n\n");
// }
 printf("\n");
    printf("+------------------------------------------------------------+\n");
    printf("| %-58s |\n", title);
    printf("+------------------------------------------------------------+\n");
    printf("| Output File  : %-43s |\n", outFile);
    printf("+------------------------------------------------------------+\n");
    printf("| Original Size: %-43ld |\n", origSize);
    printf("| Compressed   : %-43ld |\n", compSize);
    printf("+------------------------------------------------------------+\n");
    printf("| Space Saved  : %-6.2f%%  [%-10s]                       |\n", reduction, bar);
    printf("| Ratio        : %-10.2f : 1                              |\n", ratio);
    printf("| Rating       : %-10s %-5s                        |\n", verdict, stars);

    if (extraLabel && extraValue)
        printf("| %-13s : %-41s  |\n", extraLabel, extraValue);

    printf("+------------------------------------------------------------+\n");
    printf("| Decompressed : %-43s |\n", decFile);
    printf("+------------------------------------------------------------+\n\n");
}

static void compressTextHuffman(const char *inputFile)
{
    printf("\n=== TEXT FILE COMPRESSION ===\n");
    printf("Algorithm : Huffman Coding\n");

    FILE *fp = fopen(inputFile, "rb");
    if (!fp) {
        printf("Error opening file: %s\n", inputFile);
        return;
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (fileSize == 0) {
        printf("Input file is empty.\n");
        fclose(fp);
        return;
    }

    unsigned char *text = (unsigned char *)malloc(fileSize);
    fread(text, 1, fileSize, fp);
    fclose(fp);

    HuffmanCode  codes[256];
    int codeCount = 0;

    HuffmanNode *root = buildHuffmanTree(text, (int)fileSize, codes, &codeCount);
    if (!root) {
        printf("Failed to build Huffman tree\n");
        free(text);
        return;
    }
    /* --- Encode: build bit-string for the whole file --- */
    char *bitStr = (char *)malloc(fileSize * 256);
    int   bitLen = 0;
 
    for (int i = 0; i < (int)fileSize; i++) {
        for (int j = 0; j < codeCount; j++) {
            if (text[i] == codes[j].character) {
                int clen = (int)strlen(codes[j].code);
                memcpy(bitStr + bitLen, codes[j].code, clen);
                bitLen += clen;
                break;
            }
        }
    }

    char outFile[MAX_FILENAME];
    strcpy(outFile, inputFile);
    char *dot = strrchr(outFile, '.');
    if (dot) *dot = '\0';
    strcat(outFile, "_compressed.huff");

    // char *bitStr = (char *)malloc(fileSize * 256);
    // int bitLen = 0;

    // for (int i = 0; i < (int)fileSize; i++) {
    //     for (int j = 0; j < codeCount; j++) {
    //         if (text[i] == codes[j].character) {
    //             int clen = (int)strlen(codes[j].code);
    //             memcpy(bitStr + bitLen, codes[j].code, clen);
    //             bitLen += clen;
    //             break;
    //         }
    //     }
    // }

    fp = fopen(outFile, "wb");

    fwrite(&codeCount, sizeof(int), 1, fp);
    for (int i = 0; i < codeCount; i++) {
        int clen = (int)strlen(codes[i].code);
        fwrite(&codes[i].character, sizeof(unsigned char), 1, fp);
        fwrite(&clen, sizeof(int), 1, fp);
        fwrite(codes[i].code, sizeof(char), clen, fp);
    }

    fwrite(&fileSize, sizeof(long), 1, fp);
    fwrite(&bitLen, sizeof(int), 1, fp);
    fwrite(bitStr, sizeof(char), bitLen, fp);
    fclose(fp);

    long compBytes = (bitLen / 8) + (bitLen % 8 != 0 ? 1 : 0);

    /* inline decompress */
//     fp = fopen(outFile, "rb");

//     int rCount;
//     HuffmanCode rCodes[256];

//     fread(&rCount, sizeof(int), 1, fp);

//     for (int i = 0; i < rCount; i++) {
//         int clen;
//         fread(&rCodes[i].character, sizeof(unsigned char), 1, fp);
//         fread(&clen, sizeof(int), 1, fp);
//         fread(rCodes[i].code, sizeof(char), clen, fp);
//         rCodes[i].code[clen] = '\0';
//     }

//     long origSize;
//     int rBitLen;

//     fread(&origSize, sizeof(long), 1, fp);
//     fread(&rBitLen, sizeof(int), 1, fp);

//     char *rBits = (char *)malloc(rBitLen + 1);
//     fread(rBits, sizeof(char), rBitLen, fp);
//     rBits[rBitLen] = '\0';

//     fclose(fp);

//     unsigned char *decoded = (unsigned char *)malloc(origSize + 1);
//     int decLen = decodeBitString(rBits, rBitLen, rCodes, rCount, decoded, origSize);

//     char decFile[MAX_FILENAME];
//     strcpy(decFile, inputFile);
//     dot = strrchr(decFile, '.');
//     if (dot) *dot = '\0';
//     strcat(decFile, "_decompressed.txt");

//     fp = fopen(decFile, "wb");
//     fwrite(decoded, 1, decLen, fp);
//     fclose(fp);

//     char extra[128];
//     snprintf(extra, sizeof(extra), "%d unique symbols", codeCount);

//     printResults("COMPRESSION SUMMARY (HUFFMAN CODING)",
//                  "Lossless Huffman Encoding",
//                  outFile, decFile,
//                  fileSize, compBytes,
//                  "Symbols", extra);

//     free(text);
//     free(bitStr);
//     free(rBits);
//     free(decoded);
//     freeHuffmanTree(root);
// }
/* --- Auto-decompress: decode the bit-string we just created --- */
    unsigned char *decoded = (unsigned char *)malloc(fileSize + 1);
    int decLen = decodeBitString(bitStr, bitLen, codes, codeCount,
                                  decoded, fileSize);
 
    char decFile[MAX_FILENAME];
    strcpy(decFile, inputFile);
    dot = strrchr(decFile, '.');
    if (dot) *dot = '\0';
    strcat(decFile, "_decompressed.txt");
 
    fp = fopen(decFile, "wb");
    fwrite(decoded, 1, decLen, fp);
    fclose(fp);
 
    /* --- Print summary --- */
    char extra[128];
    snprintf(extra, sizeof(extra), "%d unique symbols", codeCount);
    printResults("COMPRESSION SUMMARY (HUFFMAN CODING)",
                 "Lossless Huffman Encoding",
                 outFile, decFile,
                 fileSize, compBytes,
                 "Symbols", extra);
 
    free(text);
    free(bitStr);
    free(decoded);
    freeHuffmanTree(root);
}


// static void decompressTextHuffman(const char *inputFile)
// {
//     printf("\n=== TEXT FILE DECOMPRESSION ===\n");

//     FILE *fp = fopen(inputFile, "rb");
//     if (!fp) {
//         printf("Error opening compressed file\n");
//         return;
//     }

//     int codeCount;
//     HuffmanCode codes[256];

//     fread(&codeCount, sizeof(int), 1, fp);

//     for (int i = 0; i < codeCount; i++) {
//         int clen;
//         fread(&codes[i].character, sizeof(unsigned char), 1, fp);
//         fread(&clen, sizeof(int), 1, fp);
//         fread(codes[i].code, sizeof(char), clen, fp);
//         codes[i].code[clen] = '\0';
//     }

//     long origSize;
//     int bitLen;

//     fread(&origSize, sizeof(long), 1, fp);
//     fread(&bitLen, sizeof(int), 1, fp);

//     char *bits = (char *)malloc(bitLen + 1);
//     fread(bits, sizeof(char), bitLen, fp);
//     bits[bitLen] = '\0';

//     fclose(fp);

//     unsigned char *decoded = (unsigned char *)malloc(origSize + 1);
//     int decLen = decodeBitString(bits, bitLen, codes, codeCount, decoded, origSize);

//     char decFile[MAX_FILENAME];
//     strcpy(decFile, inputFile);
//     char *dot = strrchr(decFile, '.');
//     if (dot) *dot = '\0';
//     strcat(decFile, "_decompressed.txt");

//     fp = fopen(decFile, "wb");
//     fwrite(decoded, 1, decLen, fp);
//     fclose(fp);

//     printf("\n----------------------------------------------------------\n");
//     printf("DECOMPRESSION COMPLETE (HUFFMAN CODING)\n");
//     printf("----------------------------------------------------------\n");
//     printf("Algorithm    : Huffman Coding (Lossless)\n");
//     printf("Output File  : %s\n", decFile);
//     printf("Bytes Restored: %d (expected %ld)\n", decLen, origSize);
//     printf("----------------------------------------------------------\n\n");

//     free(bits);
//     free(decoded);
// }

//public entry
// void processTextFile(const char *fileName, int operation)
// {
//     if (operation == 1) {
//         compressTextHuffman(fileName);
//         return;
//     }

//     const char *ext = strrchr(fileName, '.');
//     if (!ext || strcmp(ext, ".huff") != 0) {
//         printf("\nERROR: Not a compressed file!\n");
//         return;
//     }

//     decompressTextHuffman(fileName);
// }
void processTextFile(const char *fileName, int operation)
{
    (void)operation; /* always compress + auto-decompress */
    compressTextHuffman(fileName);
}