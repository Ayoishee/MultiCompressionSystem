#include <stdio.h>
#include <string.h>
#include "types.h"
#include "text_compress.h"
#include "bmp_compress.h"
#include "dct_compress.h"

static void stripNewline(char *s)
{
    s[strcspn(s, "\n")] = '\0';
}

static int isQuit(const char *s)
{
    return (strcmp(s, "q") == 0 || strcmp(s, "quit") == 0);
}

static int getAlgorithm(const char *s)
{
    if (strcmp(s, "1") == 0)
        return 1;   /* Huffman – Text  */
    if (strcmp(s, "2") == 0) 
        return 2;   /* Bit Plane – BMP */
    if (strcmp(s, "3") == 0)
        return 3;  
     
    return -1;
}

/* Returns 1 if the file extension matches the chosen algorithm */
static int validFile(const char *file, int algo)
{
    if (algo == 1) return strstr(file, ".txt")  || strstr(file, ".TXT");
    if (algo == 2) return strstr(file, ".bmp")  || strstr(file, ".BMP");
    if (algo == 3) return strstr(file, ".bmp")  || strstr(file, ".BMP")
                       || strstr(file, ".ppm")  || strstr(file, ".PPM")
                       || strstr(file, ".pam")  || strstr(file, ".PAM");
    return 0;
}

static const char *algoName(int algo)
{
    if (algo == 1) 
        return "Huffman Coding  (Text)";
    if (algo == 2)
        return  "Bit Plane       (BMP) ";
    if (algo == 3) 
         return "DCT             (Image)";

    return "Unknown";
}


int main(void)
{
    char input[MAX_FILENAME];
    char filename[MAX_FILENAME];

    printf("\n+--------------------------------------------+\n");
    printf("|        MULTI-FORMAT COMPRESSION TOOL       |\n");
    printf("+--------------------------------------------+\n");

    while (1)
    {
        /* ── Step 1 : Pick algorithm ── */
        printf("\n+------------------------------------+\n");
        printf("|        SELECT ALGORITHM            |\n");
        printf("+------------------------------------+\n");
        printf("|  1. Huffman Coding   (Text .txt)   |\n");
        printf("|  2. Bit Plane Slice  (BMP  .bmp)   |\n");
        printf("|  3. DCT Compression  (Image)       |\n");
        printf("|  q. Quit                           |\n");
        printf("+------------------------------------+\n"); 
        
        printf("Enter choice: ");

        fgets(input, sizeof(input), stdin);
        stripNewline(input);

        if (isQuit(input)) break;

        int algo = getAlgorithm(input);
        if (algo == -1) {
            printf("\n[!] Invalid choice. Please enter 1, 2, 3 or q.\n");
            continue;
        }

        /* ── Step 2 : Enter filename ── */
        printf("\n+------------------------------------+\n");
        printf("|          ENTER FILE NAME           |\n");
        printf("+------------------------------------+\n");
        printf("|  Algorithm : %-20s|\n", algoName(algo));
        printf("|  (type q to go back)               |\n");
        printf("+------------------------------------+\n");
        printf("Filename: ");

        fgets(filename, sizeof(filename), stdin);
        stripNewline(filename);

        if (isQuit(filename)) continue;

        if (!validFile(filename, algo))
         {
            printf("\n[!] Wrong file type for the selected algorithm.\n");
            continue;
        }

        /* ── Step 3 : Compress (decompression happens automatically inside) ── */
        printf("\n+------------------------------------+\n");
        printf("|         SELECT OPERATION           |\n");
        printf("+------------------------------------+\n");
        printf("|  1. Compress  (auto-decompresses)  |\n");
        printf("|  q. Back to file entry             |\n");
        printf("+------------------------------------+\n");
        printf("Enter choice: ");


        fgets(input, sizeof(input), stdin);
        stripNewline(input);

        if (isQuit(input)) continue;

        if (strcmp(input, "1") != 0) {
            printf("\nInvalid choice!\n");
            continue;
        }

        /* Run compression (each module also writes the decompressed output) */
        if(algo == 1)
                 processTextFile(filename, 1);
        else if (algo == 2) 
                 processBMPFile (filename, 1);
        else if (algo == 3) 
                 processColorFile(filename, 1);

        printf("\n Compression + Decompression completed successfully!\n");
    }

    printf("\n+------------------------------------+\n");
    printf("|             GOODBYE!              |\n");
    printf("+------------------------------------+\n\n");

    return 0;
}
//gcc main.c types.c huffman.c text_compress.c bmp_compress.c color_image.c dct_compress.c -o compressor -lm