
#include <stdio.h>
#include <string.h>

#include "types.h"

FileType detectFileType(const char *fileName)
{
    int i;
    int lastDotIndex = -1;

    for(i = 0; fileName[i] != '\0'; i++)
        if(fileName[i] == '.')
            lastDotIndex = i;

    if(lastDotIndex == -1 || lastDotIndex == 0)
        return UNKNOWN_FILE;

    const char *ext = fileName + lastDotIndex;

    if(strcmp(ext, ".txt") == 0)
        return TEXT_FILE;

    if(strcmp(ext, ".bmp") == 0 || strcmp(ext, ".BMP") == 0) 
    {
        FILE *fp = fopen(fileName, "rb");
        if (fp) 
        {
            BMPFileHeader fileHeader;
            BMPInfoHeader infoHeader;

            fread(&fileHeader, sizeof(BMPFileHeader),1,fp);
            fread(&infoHeader, sizeof(BMPInfoHeader),1,fp);
            fclose(fp);

            if (infoHeader.biBitCount == 8)     
                      return BMP_FILE;

            if (infoHeader.biBitCount == 24 || infoHeader.biBitCount == 32)      
                     return FILE_COLOR;

        }
        return BMP_FILE; 
    }

        if(strcmp(ext, ".ppm")  == 0 ||
           strcmp(ext, ".jpg")  == 0 || 
           strcmp(ext, ".jpeg") == 0)

        return FILE_COLOR;

    return UNKNOWN_FILE;
}
