#include <stdio.h>
#include <string.h>
#include "types.h"
#include "text_compress.h"
#include "bmp_compress.h"
#include "dct_compress.h"

/* ───────── Utility Functions ───────── */

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
    if (strcmp(s, "1") == 0) return 1;
    if (strcmp(s, "2") == 0) return 2;
    if (strcmp(s, "3") == 0) return 3;
    return -1;
}

static int validFile(const char *file, int algo)
{
    if (algo == 1)
        return strstr(file, ".txt") || strstr(file, ".TXT");

    if (algo == 2)
        return strstr(file, ".bmp") || strstr(file, ".BMP");

    if (algo == 3)
        return strstr(file, ".bmp") || strstr(file, ".ppm") || strstr(file, ".pam");

    return 0;
}

/* ───────── MAIN PROGRAM ───────── */

int main()
{
    char input[MAX_FILENAME];
    char filename[MAX_FILENAME];

    printf("\n╔════════════════════════════════════════════╗\n");
    printf("║      MULTI-FORMAT COMPRESSION TOOL        ║\n");
    printf("╚════════════════════════════════════════════╝\n");

    while (1)
    {
        /* ── Algorithm Menu ── */
        printf("\n╔════════════════════════════════════╗\n");
        printf("║        SELECT ALGORITHM            ║\n");
        printf("╚════════════════════════════════════╝\n");
        printf("  1. Huffman Coding   (Text)\n");
        printf("  2. Bit Plane        (BMP)\n");
        printf("  3. DCT              (Images)\n");
        printf("  q. Quit\n");
        printf("\nEnter choice: ");

        fgets(input, sizeof(input), stdin);
        stripNewline(input);

        if (isQuit(input))
            break;

        int algo = getAlgorithm(input);
        if (algo == -1)
        {
            printf("\n[!] Invalid choice. Try again.\n");
            continue;
        }

        /* ── File Input ── */
        printf("\n╔════════════════════════════════════╗\n");
        printf("║          ENTER FILE NAME           ║\n");
        printf("╚════════════════════════════════════╝\n");
        printf("  (type q to go back)\n\n");

        printf("Filename: ");
        fgets(filename, sizeof(filename), stdin);
        stripNewline(filename);

        if (isQuit(filename))
            continue;

        if (!validFile(filename, algo))
        {
            printf("\n[!] Invalid file type for selected algorithm.\n");
            continue;
        }

        /* ── Compress Only ── */
        printf("\n╔════════════════════════════════════╗\n");
        printf("║           COMPRESS FILE            ║\n");
        printf("╚════════════════════════════════════╝\n");

        printf("File      : %s\n", filename);
        printf("Operation : Compression\n");

        if (algo == 1)
            processTextFile(filename, 1);
        else if (algo == 2)
            processBMPFile(filename, 1);
        else if (algo == 3)
            processColorFile(filename, 1);

        printf("\n✔ Compression Completed Successfully!\n");
    }

    printf("\n╔════════════════════════════════════╗\n");
    printf("║           GOODBYE!                ║\n");
    printf("╚════════════════════════════════════╝\n");

    return 0;
}