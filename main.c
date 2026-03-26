#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "text_compress.h"
#include "bmp_compress.h"
#include "dct_compress.h"

static int isValidFilename(const char *filename, int algorithm)
{
    const char *ext = strrchr(filename, '.');
    if (!ext) return 0;

    switch (algorithm) {
        case 1:
            return (strcmp(ext, ".txt") == 0 ||
                    strcmp(ext, ".TXT") == 0);

        case 2:
            return (strcmp(ext, ".bmp") == 0 ||
                    strcmp(ext, ".BMP") == 0 );
        case 3:
            return (strcmp(ext, ".bmp")  == 0 ||
                    strcmp(ext, ".BMP")  == 0 ||
                    strcmp(ext, ".ppm")  == 0 ||
                    strcmp(ext, ".pam")  == 0);
        default:
            return 0;
    }
}

int main(void)
{
    int  algorithm, operation;
    char input[MAX_FILENAME];
    char filename[MAX_FILENAME];

    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║  MULTI-FORMAT COMPRESSION & DECOMPRESSION TOOL         ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n\n");

    
    for ( ; ; )                 //outer loop
    {    

        printf("╔════════════════════════════════════════════════════════╗\n");
        printf("║             SELECT COMPRESSION ALGORITHM               ║\n");
        printf("╚════════════════════════════════════════════════════════╝\n");

        printf("    1 . Huffman Coding       (for Text files)\n");
        printf("    2 . Bit Plane Slicing    (for Grayscale BMP images)\n");
        printf("    3 . DCT                  (for Color images)\n");
        printf(" Quit . Q 0r q               (exit the program)\n");

        printf("\nEnter your choice (1 / 2 / 3 / quit): ");

        if (fgets(input, sizeof(input), stdin) == NULL) 
               break;

        input[strcspn(input, "\n")] = '\0';

        /* quit */
        if (strcmp(input, "quit") == 0 || strcmp(input, "q") == 0) {
            printf("\n╔════════════════════════════════════════════════════════╗\n");
            printf("║                    GOODBYE!                            ║\n");
            printf("╚════════════════════════════════════════════════════════╝\n");
            break;
        }

       
        if (sscanf(input, "%d", &algorithm) != 1 ||algorithm < 1 || algorithm > 3) 
        {
            printf("\n Invalid choice. Please enter 1, 2, 3 or quit.\n\n");
            continue;
        }

        for ( ; ; ) {
            printf("\n╔════════════════════════════════════════════════════════╗\n");
            printf("║                  ENTER FILENAME                        ║\n");
            printf("╚════════════════════════════════════════════════════════╝\n");

            switch (algorithm) {
                case 1: 
                        printf("  Accepted : .txt \n");       
                        break;
                case 2: 
                        printf("  Accepted : .bmp (8-bit)  |  .bps\n");    
                        break;
                case 3:
                        printf("  Accepted : .bmp (24/32-bit)  |  .ppm  |  .pam\n");
                        break;

            }
            printf("  (type 'quit' to go back to algorithm selection)\n");
            printf("\nEnter filename: ");

            if (fgets(filename, sizeof(filename), stdin) == NULL) goto done;
            filename[strcspn(filename, "\n")] = '\0';

            if (strcmp(filename, "quit") == 0 || strcmp(filename, "q") == 0) 
            {
                printf("\nGoing back to algorithm selection\n\n");
                break;
            }

            if (!isValidFilename(filename, algorithm)) 
            {
                printf("\n  [!] Incorrect format for algorithm %d.\n", algorithm);

                switch (algorithm) {
                    case 1:
                          printf("      Expected: .txt \n");           
                          break;
                    case 2: 
                          printf("      Expected: .bmp (8-bit) \n");     
                          break;
                    case 3: 
                          printf("      Expected: .bmp (24/32-bit) \n");
                          break;
                   }

                printf("      Please try again.\n");
                continue;   
            }

            for ( ; ; ) {

                printf("\n╔════════════════════════════════════════════════════════╗\n");
                printf("║              SELECT OPERATION                          ║\n");
                printf("╚════════════════════════════════════════════════════════╝\n");
                printf("  1. Compress\n");
                printf("  2. Decompress\n");
                printf("  (type 'quit' to go back to filename entry)\n");
                printf("\nEnter your choice (1 / 2 / quit): ");

                if (fgets(input, sizeof(input), stdin) == NULL) goto done;
                input[strcspn(input, "\n")] = '\0';

                if (strcmp(input, "quit") == 0 || strcmp(input, "q") == 0) {
                    printf("\nGoing back to filename entry...\n");
                    break;
                }

                if (sscanf(input, "%d", &operation) != 1 ||
                    operation < 1 || operation > 2) {
                    printf("\n  [!] Invalid choice. Please enter 1, 2 or quit.\n");
                    continue;
                }

                printf("\n╔════════════════════════════════════════════════════════╗\n");
                printf("║                  PROCESSING FILE                       ║\n");
                printf("╚════════════════════════════════════════════════════════╝\n");
                printf("  Algorithm : ");
                switch (algorithm)
                 {
                    case 1:
                           printf("Huffman Coding\n");           
                           break;
                    case 2: 
                           printf("Bit Plane Slicing\n");    
                           break;
                    case 3: 
                           printf("DCT (Discrete Cosine Transform)\n"); 
                           break;
                }
                
                printf("  Operation : %s\n", operation == 1 ? "Compression" : "Decompression");
                printf("  File      : %s\n", filename);

                switch (algorithm) {
                    case 1: processTextFile(filename,  operation); break;
                    case 2: processBMPFile(filename,   operation); break;
                    case 3: processColorFile(filename, operation); break;
                }

                printf("\n╔════════════════════════════════════════════════════════╗\n");
                printf("║              PROCESSING COMPLETE                       ║\n");
                printf("╚════════════════════════════════════════════════════════╝\n\n");

                break; //done – go back to algorithm select 

            } // end operation loop 

        } // end filename loop 

    } // end algorithm loop 

done:
    return 0;
}
