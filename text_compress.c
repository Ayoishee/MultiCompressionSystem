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
       double reduction;
       double ratio;

       if (origSize > 0 && compSize < origSize)
         {
            reduction = (1.0 - (double)compSize / origSize) * 100.0;
          }
      else
         {
            reduction = 0.0;
       } 

      if (compSize > 0)
        {
            ratio = (double)origSize / compSize;
        }
      else
       {
            ratio = 1.0;
       }

       int filled;
        filled = (int)(reduction / 10.0);

        if (filled > 10)
         {
           filled = 10;
          }

        if (filled < 0)
         {
           filled = 0;
         }

       char bar[11];
       int i;
/* build progress bar */
      for (i = 0; i < 10; i++)
        {
          if (i < filled)
            {
              bar[i] = '#';
            }
          else
           {
             bar[i] = '-';
           }
        }

bar[10] = '\0';

    const char *stars, *verdict;
    if (reduction >= 85) { 
        stars = "* * * * *"; verdict = "EXCELLENT";
     }
    else if(reduction >= 70) { 
        stars = "* * * * ."; verdict = "GREAT"; 
    }
    else if (reduction >= 50) { 
        stars = "* * * . ."; verdict = "GOOD";
     }
    else if (reduction >= 30) {
         stars = "* * . . ."; verdict = "FAIR";
         }
    else { 
        stars = "* . . . ."; verdict = "LOW";
     }

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

  /*--- Auto-decompress: decode the bit-string we just created --- */
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

void processTextFile(const char *fileName, int operation)
{
    (void)operation; /* always compress + auto-decompress */
    compressTextHuffman(fileName);
}