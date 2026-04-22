#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void convertJPGtoPPM(const char *inputFile, char *outputFile)
{
    strcpy(outputFile, inputFile);

    char *dot = strrchr(outputFile, '.');
    if (dot)
       *dot = '\0';
    strcat(outputFile, ".ppm");

    char command[1024];

    snprintf(command, sizeof(command),
             "convert \"%s\" \"%s\"", inputFile, outputFile);

    printf("\n[INFO] Converting JPG → PPM...\n");
    int result = system(command);

    if (result != 0) {
       // printf("[ERROR] Conversion failed. Install ImageMagick.\n");
        exit(1);
    }

    printf("[OK] Converted to: %s\n", outputFile);
}