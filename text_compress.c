/* ============================================================
 * text_compress.c  -  Huffman coding for plain-text files
 * ============================================================ */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "huffman.h"
#include "text_compress.h"

/* ============================================================
 *  Box drawing helpers
 *  Inner content width = 70 chars  (box total = 74 with "| " and " |")
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

/* ============================================================
 *  Shared pretty-print summary box
 * ============================================================ */
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
 *  Decode helper
 * ============================================================ */
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
                curIdx = 0; cur[0] = '\0';
                break;
            }
        }
    }
    return outIdx;
}

/* ============================================================
 *  Compress
 * ============================================================ */
static void compressTextHuffman(const char *inputFile)
{
    printf("Detected: Text File\n");
    printf("Algorithm: Huffman Coding\n");
    printf("\n=== TEXT FILE COMPRESSION ===\n");

    FILE *fp = fopen(inputFile, "rb");
    if (!fp) { printf("Error opening file: %s\n", inputFile); return; }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (fileSize == 0) {
        printf("Input file is empty. Nothing to compress.\n");
        fclose(fp); return;
    }

    unsigned char *text = (unsigned char *)malloc(fileSize);
    fread(text, 1, fileSize, fp);
    fclose(fp);

    HuffmanCode  codes[256];
    int          codeCount = 0;
    HuffmanNode *root = buildHuffmanTree(text, (int)fileSize, codes, &codeCount);
    if (!root) { printf("Failed to build Huffman tree\n"); free(text); return; }

    char outFile[MAX_FILENAME];
    strcpy(outFile, inputFile);
    char *dot = strrchr(outFile, '.');
    if (dot) *dot = '\0';
    strcat(outFile, "_compressed.huff");

    char *bitStr = (char *)malloc(fileSize * 256);
    int   bitLen = 0;

    for (int i = 0; i < (int)fileSize; i++)
        for (int j = 0; j < codeCount; j++)
            if (text[i] == codes[j].character) {
                int clen = (int)strlen(codes[j].code);
                memcpy(bitStr + bitLen, codes[j].code, clen);
                bitLen += clen;
                break;
            }

    fp = fopen(outFile, "wb");
    fwrite(&codeCount, sizeof(int), 1, fp);
    for (int i = 0; i < codeCount; i++) {
        int clen = (int)strlen(codes[i].code);
        fwrite(&codes[i].character, sizeof(unsigned char), 1, fp);
        fwrite(&clen,               sizeof(int),           1, fp);
        fwrite(codes[i].code,       sizeof(char),   clen,    fp);
    }
    fwrite(&fileSize, sizeof(long), 1, fp);
    fwrite(&bitLen,   sizeof(int),  1, fp);
    fwrite(bitStr,    sizeof(char), bitLen, fp);
    fclose(fp);

    long compBytes = (bitLen / 8) + (bitLen % 8 != 0 ? 1 : 0);

    /* Inline decompression */
    fp = fopen(outFile, "rb");
    int rCount; HuffmanCode rCodes[256];
    fread(&rCount, sizeof(int), 1, fp);
    for (int i = 0; i < rCount; i++) {
        int clen;
        fread(&rCodes[i].character, sizeof(unsigned char), 1, fp);
        fread(&clen,                sizeof(int),           1, fp);
        fread(rCodes[i].code,       sizeof(char),   clen,    fp);
        rCodes[i].code[clen] = '\0';
    }
    long origSize; int rBitLen;
    fread(&origSize, sizeof(long), 1, fp);
    fread(&rBitLen,  sizeof(int),  1, fp);
    char *rBits = (char *)malloc(rBitLen + 1);
    fread(rBits, sizeof(char), rBitLen, fp);
    rBits[rBitLen] = '\0';
    fclose(fp);

    unsigned char *decoded = (unsigned char *)malloc(origSize + 1);
    int decLen = decodeBitString(rBits, rBitLen, rCodes, rCount, decoded, origSize);

    char decFile[MAX_FILENAME];
    strcpy(decFile, inputFile);
    dot = strrchr(decFile, '.');
    if (dot) *dot = '\0';
    strcat(decFile, "_decompressed.txt");

    fp = fopen(decFile, "wb");
    fwrite(decoded, 1, decLen, fp);
    fclose(fp);

    char extraVal[128];
    snprintf(extraVal, sizeof(extraVal), "%d unique symbols encoded", codeCount);

    printResultsBox("  COMPRESSION SUMMARY  (HUFFMAN CODING)",
                    "Lossless - variable-length prefix codes",
                    outFile, decFile,
                    fileSize, compBytes,
                    "Symbols", extraVal);

    free(text); free(bitStr); free(rBits); free(decoded);
    freeHuffmanTree(root);
}

/* ============================================================
 *  Decompress (standalone, from .huff)
 * ============================================================ */
static void decompressTextHuffman(const char *inputFile)
{
    printf("\n=== TEXT FILE DECOMPRESSION ===\n");

    FILE *fp = fopen(inputFile, "rb");
    if (!fp) { printf("Error: Cannot open compressed file!\n"); return; }

    int codeCount; HuffmanCode codes[256];
    fread(&codeCount, sizeof(int), 1, fp);
    for (int i = 0; i < codeCount; i++) {
        int clen;
        fread(&codes[i].character, sizeof(unsigned char), 1, fp);
        fread(&clen,               sizeof(int),           1, fp);
        fread(codes[i].code,       sizeof(char),   clen,    fp);
        codes[i].code[clen] = '\0';
    }

    long origSize; int bitLen;
    fread(&origSize, sizeof(long), 1, fp);
    fread(&bitLen,   sizeof(int),  1, fp);

    char *bits = (char *)malloc(bitLen + 1);
    fread(bits, sizeof(char), bitLen, fp);
    bits[bitLen] = '\0';
    fclose(fp);

    unsigned char *decoded = (unsigned char *)malloc(origSize + 1);
    int decLen = decodeBitString(bits, bitLen, codes, codeCount, decoded, origSize);

    char decFile[MAX_FILENAME];
    strcpy(decFile, inputFile);
    char *dot = strrchr(decFile, '.');
    if (dot) *dot = '\0';
    strcat(decFile, "_decompressed.txt");

    fp = fopen(decFile, "wb");
    fwrite(decoded, 1, decLen, fp);
    fclose(fp);

    char buf[256];
    printf("\n");
    box_divider();
    box_row("  DECOMPRESSION COMPLETE  (HUFFMAN CODING)");
    box_divider();
    box_row("  Algorithm    : Huffman Coding (Lossless)");
    snprintf(buf, sizeof(buf), "  Output File  : %s", decFile);          box_row(buf);
    snprintf(buf, sizeof(buf), "  Bytes Restored: %d  (expected %ld)", decLen, origSize); box_row(buf);
    box_divider();
    printf("\n");

    free(bits); free(decoded);
}

/* ============================================================
 *  Public entry point
 * ============================================================ */
void processTextFile(const char *fileName, int operation)
{
    if (operation == 1) { compressTextHuffman(fileName); return; }

    const char *ext = strrchr(fileName, '.');
    if (!ext || strcmp(ext, ".huff") != 0) {
        printf("\nERROR: This file is not compressed!\n");
        printf("Please compress the file first (choose operation 1).\n\n");
        return;
    }
    decompressTextHuffman(fileName);
}