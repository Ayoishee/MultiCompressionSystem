#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "color_image.h"

static void freeChannels(ColorImage *img)
{
    if (!img)
     {
        return;
    }

    for (int i = 0; i < img->height; i++) 
    {
        if (img->R) 
            free(img->R[i]);
        if (img->G) 
            free(img->G[i]);
        if (img->B) 
            free(img->B[i]);
        if (img->hasAlpha && img->A)
            free(img->A[i]);
    }

    free(img->R);
    free(img->G);
    free(img->B);
    if (img->hasAlpha)
       free(img->A);

    img->R = img->G = img->B = img->A = NULL;
}

static int allocChannels(ColorImage *img)
{
    int h = img->height;//h=row
    int w = img->width;

    //three separate 2D arrays
    img->R = calloc(h, sizeof(int *));//calloc = dynamic memo allo + zero initialization
    img->G = calloc(h, sizeof(int *));//allocate row pointers
    img->B = calloc(h, sizeof(int *));

    if (img->hasAlpha)
        {
          img->A = calloc(h, sizeof(int *));
        }
    else
       {
          img->A = NULL;
        }

    if (img->R==NULL || img->G==NULL || img->B==NULL || (img->hasAlpha && img->A==NULL)) 
    {
        printf("Memory allocation failed (row pointers)\n");
        freeChannels(img);
        return 0;
    }

    for (int i = 0; i < h; i++)
     {
        img->R[i] = malloc(w * sizeof(int));//allocate each row
        img->G[i] = malloc(w * sizeof(int));
        img->B[i] = malloc(w * sizeof(int));

        if (img->hasAlpha)
            img->A[i] = malloc(w * sizeof(int));

        if (!img->R[i] || !img->G[i] || !img->B[i] ||
            (img->hasAlpha && !img->A[i])) 
            {
               printf("Memory allocation failed (pixel data at row %d)\n", i);
               freeChannels(img);
               return 0;
           }
     }

    return 1;
}

static ColorImage *load32BitBMP(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("Error opening BMP file: %s\n", filename);
        return NULL;
    }

    BMPFileHeader fh;
    BMPInfoHeader ih;

    if (fread(&fh, sizeof(BMPFileHeader), 1, fp) != 1 ||
        fread(&ih, sizeof(BMPInfoHeader), 1, fp) != 1) 
        {
            printf("Error reading BMP headers\n");
            fclose(fp);
            return NULL;
       }

    if (fh.bfType != 0x4D42)
     {
        printf("Not a BMP file.\n");
        fclose(fp);
        return NULL;
    }

    if (ih.biBitCount == 8)
     {
        printf("8-bit BMP detected.Please use the grayscale\n");
        fclose(fp);
        return NULL;
    }

    if (ih.biBitCount != 24 && ih.biBitCount != 32) {
        printf("Only 24-bit RGB or 32-bit RGBA BMP supported (%d-bit given)\n",
               ih.biBitCount);
        fclose(fp);
        return NULL;
    }

    ColorImage *img = malloc(sizeof(ColorImage));
    if (!img) {
        printf("Memory allocation failed for image struct\n");
        fclose(fp);
        return NULL;
    }

    memset(img, 0, sizeof(ColorImage));

    img->width    = ih.biWidth;
    if (ih.biHeight > 0)
       {
         img->height = ih.biHeight;
       }
    else
       {
         img->height = -ih.biHeight;
        }

    img->maxVal   = 255;
    img->hasAlpha = (ih.biBitCount == 32);

    if (!allocChannels(img)) 
    {
        free(img);
        fclose(fp);
        return NULL;
    }

    int bpp;

    if (img->hasAlpha)
      { 
         bpp = 4;
      }
    else
     {
         bpp = 3;
     }

    int rowSize = img->width * bpp; //actual data size of one row
    int padding = (4 - (rowSize % 4)) % 4;

    printf("==== Image info ====\n");
    printf("  Dimensions : %dx%d pixels\n", img->width, img->height); 
    if (img->hasAlpha)
        printf("  Bit depth: %d-bit (RGBA)\n", ih.biBitCount);
    else
        printf("  Bit depth: %d-bit (RGB)\n", ih.biBitCount);

    int channels;
    if (img->hasAlpha){
        channels = 4;
    }
    else
       channels = 3;

    printf("  Channels   : %d\n\n", channels);

    fseek(fp, fh.bfOffBits, SEEK_SET);

    unsigned char px[4];
    unsigned char pad[4];
    int bottomUp = (ih.biHeight > 0);

    for (int i = 0; i < img->height; i++)
     {
        int row;

        if (bottomUp)
           row = img->height - 1 - i;
        else
           row = i;

        for (int j = 0; j < img->width; j++)
         {
            if (fread(px, 1, bpp, fp) != (size_t)bpp) 
            {
                printf("Unexpected end of file at row %d, col %d\n", i, j);
                freeChannels(img);
                free(img);
                fclose(fp);
                return NULL;
            }

            img->B[row][j] = px[0];
            img->G[row][j] = px[1];
            img->R[row][j] = px[2];
            if (img->hasAlpha)
                img->A[row][j] = px[3];
        }

        if (padding > 0)
            fread(pad, 1, padding, fp);
    }

    fclose(fp);
    return img;
}

static void save32BitBMP(const char *filename, ColorImage *img)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp)
     {
        printf("Error creating BMP file: %s\n", filename);
        return;
    }

    int bpp;

    if (img->hasAlpha)
        bpp = 4;
    else
        bpp = 3;

    int rowSize   = img->width * bpp;
    int padding   = (4 - (rowSize % 4)) % 4;
    int imageSize = (rowSize + padding) * img->height;

    BMPFileHeader fh;
    BMPInfoHeader ih;

    fh.bfType      = 0x4D42;
    fh.bfOffBits   = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
    fh.bfSize      = fh.bfOffBits + imageSize;
    fh.bfReserved1 = 0;
    fh.bfReserved2 = 0;

    ih.biSize          = sizeof(BMPInfoHeader);
    ih.biWidth         = img->width;
    ih.biHeight        = img->height;   /* positive = bottom-up */
    ih.biPlanes        = 1;
    ih.biBitCount      = img->hasAlpha ? 32 : 24;
    ih.biCompression   = 0;
    ih.biSizeImage     = imageSize;
    ih.biXPelsPerMeter = 2835;   //default BMP resolution (X axis)
    ih.biYPelsPerMeter = 2835;   //(Y axis)
    ih.biClrUsed       = 0;
    ih.biClrImportant  = 0;

    fwrite(&fh, sizeof(fh), 1, fp);
    fwrite(&ih, sizeof(ih), 1, fp);

    unsigned char px[4];
    unsigned char pad[3] = {0, 0, 0};

    for (int i = img->height - 1; i >= 0; i--) 
    {
        for (int j = 0; j < img->width; j++) 
        {

            px[0] = (unsigned char)img->B[i][j];
            px[1] = (unsigned char)img->G[i][j];
            px[2] = (unsigned char)img->R[i][j];

            if (img->hasAlpha) {
                px[3] = (unsigned char)img->A[i][j];
                fwrite(px, 1, 4, fp);
            } else {
                fwrite(px, 1, 3, fp);
            }
        }

        if (padding > 0)
            fwrite(pad, 1, padding, fp);
    }

    fclose(fp);
}

/*public access*/ 
ColorImage *loadColorImage(const char *filename)
{
    if (!filename) 
    {
        printf("NULL filename passed to loadColorImage\n");
        return NULL;
    }

   const char *ext = NULL;

   for (int i = 0; filename[i] != '\0'; i++)
      {
        if (filename[i] == '.')
           {
             ext = &filename[i];
           }
       }

    if (!ext)
      {
         printf("No file extension found\n");
         return NULL;
       } 

    if (strcmp(ext, ".bmp") == 0 ||
        strcmp(ext, ".BMP") == 0)
        return load32BitBMP(filename);

    printf("Unsupported image format: %s\n", ext);
    return NULL;
}

void saveColorImage(const char *filename, ColorImage *img)
{
    if (!filename || !img)
     {
        printf("Invalid arguments passed to saveColorImage\n");
        return;
    }

    save32BitBMP(filename, img);
}

void freeColorImage(ColorImage *img)
{
    if (!img) 
      return;

    freeChannels(img);
    free(img);
}