#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "color_image.h"

/* ---- Channel-array allocation helpers ---- */
static void allocChannels(ColorImage *img)
{
    int h = img->height;
    int w = img->width;
    
    //alllo memo for row pointers
    img->R = malloc(h * sizeof(int *));
    img->G = malloc(h * sizeof(int *));
    img->B = malloc(h * sizeof(int *));

     // Allocate alpha channel only if image supports transparency
    if (img->hasAlpha)
        img->A = malloc(h * sizeof(int *));
    else
        img->A = NULL;

    //allo pix memory
    for (int i = 0; i < h; i++)
    {
        img->R[i] = malloc(w * sizeof(int));
        img->G[i] = malloc(w * sizeof(int));
        img->B[i] = malloc(w * sizeof(int));

        // Allo alpha row if needed
        if (img->hasAlpha)
            img->A[i] = malloc(w * sizeof(int));
    }
}

//bmp
static ColorImage *load32BitBMP(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
     { 
         printf("Error opening BMP file: %s\n", filename);
         return NULL;
      }

    BMPFileHeader fh;
    BMPInfoHeader ih;
    fread(&fh, sizeof(BMPFileHeader), 1, fp);
    fread(&ih, sizeof(BMPInfoHeader), 1, fp);

    if (fh.bfType != 0x4D42)
     {
        printf("Not a BMP file.\n"); 
        fclose(fp);
        return NULL;
     }

    if (ih.biBitCount != 32 && ih.biBitCount != 24) 
    {
        printf("Only 24-bit RGB or 32-bit RGBA BMP images are supported (%d-bit given)\n",
               ih.biBitCount);
        fclose(fp); 
        return NULL;
    }

    ColorImage *img = (ColorImage *)malloc(sizeof(ColorImage));

    img->width    = ih.biWidth;
    img->maxVal   = 255;
    //img->hasAlpha = (ih.biBitCount == 32) ? 1 : 0;
    if (ih.biBitCount == 32)
       {
          img->hasAlpha = 1;
       }
    else
       {
          img->hasAlpha = 0;
     }

    int isBottomUp = (ih.biHeight > 0);
   
    if (isBottomUp)
        {
           img->height = ih.biHeight;
         }
    else
         {
            img->height = -ih.biHeight;
         }

    allocChannels(img);

      int bytesPerPixel;

      if (img->hasAlpha == 1)
         {
             bytesPerPixel = 4;
         }
      else
         {
             bytesPerPixel = 3;
         }

    int rowSize       = img->width * bytesPerPixel;
    int rowPadding    = (4 - (rowSize % 4)) % 4;

    printf("====Image info===\n");
    printf("  Dimensions: %dx%d pixels\n",  img->width, img->height);
    printf("  Bit depth: %d-bit (%s)\n",    ih.biBitCount, img->hasAlpha ? "RGBA" : "RGB");
    printf("  Channels: %d\n\n", img->hasAlpha ? 4 : 3);

    fseek(fp, fh.bfOffBits, SEEK_SET);
    unsigned char padBuf[4];

    for (int i = 0; i < img->height; i++)
    {

        int row = 0;
        if (isBottomUp) {
            row = img->height - 1 - i;
        } else {
            row = i;
        }

        for (int j = 0; j < img->width; j++) 
        {
            unsigned char px[4];
            if (img->hasAlpha) {
                fread(px, 1, 4, fp);          /* BGRA */
                img->B[row][j] = px[0];
                img->G[row][j] = px[1];
                img->R[row][j] = px[2];
                img->A[row][j] = px[3];
            } 
            else 
            {
                fread(px, 1, 3, fp);          /* BGR */
                img->B[row][j] = px[0];
                img->G[row][j] = px[1];
                img->R[row][j] = px[2];
            }
        }
        if (rowPadding > 0) 
        {
            fread(padBuf, 1, rowPadding, fp);
        }
    }
    fclose(fp);
    return img;
}

static void save32BitBMP(const char *filename, ColorImage *img)
{
    int bytesPerPixel;

      if (img->hasAlpha == 1)
         {
             bytesPerPixel = 4;
         }
      else
         {
             bytesPerPixel = 3;
         }

    int rowSize       = img->width * bytesPerPixel;
    int rowPadding    = (4 - (rowSize % 4)) % 4;
    int imageSize     = (rowSize + rowPadding) * img->height;

    BMPFileHeader fh;
    BMPInfoHeader ih;

    fh.bfType      = 0x4D42;
    fh.bfReserved1 = 0;
    fh.bfReserved2 = 0;
    fh.bfOffBits   = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
    fh.bfSize      = fh.bfOffBits + imageSize;

    ih.biSize          = sizeof(BMPInfoHeader);
    ih.biWidth         = img->width;
    ih.biHeight        = img->height; /* bottom-up */
    ih.biPlanes        = 1;
    ih.biBitCount      = img->hasAlpha ? 32 : 24;
    ih.biCompression   = 0;
    ih.biSizeImage     = imageSize;
    ih.biXPelsPerMeter = 2835;
    ih.biYPelsPerMeter = 2835;
    ih.biClrUsed       = 0;
    ih.biClrImportant  = 0;

    FILE *fp = fopen(filename, "wb");
    if (!fp)
     { 
        printf("Error creating BMP file: %s\n", filename); 
        return; 
    }

    fwrite(&fh, sizeof(BMPFileHeader), 1, fp);
    fwrite(&ih, sizeof(BMPInfoHeader), 1, fp);

    unsigned char pad[3] = {0, 0, 0};
    for (int i = img->height - 1; i >= 0; i--) 
    {
        for (int j = 0; j < img->width; j++) 
        {
            unsigned char px[4];
            if (img->hasAlpha)
             {
                px[0] = (unsigned char)img->B[i][j];
                px[1] = (unsigned char)img->G[i][j];
                px[2] = (unsigned char)img->R[i][j];
                px[3] = (unsigned char)img->A[i][j];
                fwrite(px, 1, 4, fp);
            } 
            else
             {
                px[0] = (unsigned char)img->B[i][j];
                px[1] = (unsigned char)img->G[i][j];
                px[2] = (unsigned char)img->R[i][j];
                fwrite(px, 1, 3, fp);
            }
        }
        if (rowPadding > 0) fwrite(pad, 1, rowPadding, fp);
    }
    fclose(fp);
}


//ppm & pam

static ColorImage *loadPPMImage(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
     {
         printf("Error opening file: %s\n", filename); 
         return NULL;
     }

    ColorImage *img = (ColorImage *)malloc(sizeof(ColorImage));
    char magic[10];

    fscanf(fp, "%s", magic); // Read file type (P3 / P6 / P7)

    // Check if format is supported
    if (strcmp(magic, "P3") != 0 &&
        strcmp(magic, "P6") != 0 &&
        strcmp(magic, "P7") != 0)
    {
        printf("Wrong format!\n");
        fclose(fp);
        free(img);
        return NULL;
    }
        // If P7 has Alpha (RGBA)
       img->hasAlpha = (strcmp(magic, "P7") == 0) ? 1 : 0;

     char c = (char)fgetc(fp);
     while (c == '#' || c == '\n' || c == ' ')
    {
        if (c == '#')
        {
            while (fgetc(fp) != '\n'); // skip comment line
        }
        c = fgetc(fp);
    }
    ungetc(c, fp);

    /* Read image size info */
    if (img->hasAlpha)
     {
        // P7 format (header style text)
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if(strstr(line, "WIDTH")) 
                sscanf(line, "WIDTH %d",  &img->width);
            if(strstr(line, "HEIGHT"))
                sscanf(line, "HEIGHT %d", &img->height);
            if(strstr(line, "MAXVAL")) 
                sscanf(line, "MAXVAL %d", &img->maxVal);
            if(strstr(line, "ENDHDR"))
                 break;
        }
    } else {
         // P3 / P6 format (simple numbers)
        fscanf(fp, "%d %d %d",
               &img->width,
               &img->height,
               &img->maxVal);
    }

    allocChannels(img);  // Allo memo for R G B A

    if (strcmp(magic, "P3") == 0) 
    {
        // ASCII RGB (human readable)
        for (int i = 0; i < img->height; i++)
            for (int j = 0; j < img->width; j++)
                fscanf(fp, "%d %d %d", &img->R[i][j], &img->G[i][j], &img->B[i][j]);

    } 
    else if (strcmp(magic, "P6") == 0) 
    {
        fgetc(fp); /* skip trailing newline in header */
        for (int i = 0; i < img->height; i++)
            for (int j = 0; j < img->width; j++) 
            {
                unsigned char rgb[3];
                fread(rgb, 1, 3, fp);
                img->R[i][j] = rgb[0];
                img->G[i][j] = rgb[1];
                img->B[i][j] = rgb[2];
            }
    } 
    else 
    { 
        /* P7 */
        for (int i = 0; i < img->height; i++)
            for (int j = 0; j < img->width; j++) 
            {
                unsigned char rgba[4];
                fread(rgba, 1, 4, fp);
                img->R[i][j] = rgba[0];
                img->G[i][j] = rgba[1];
                img->B[i][j] = rgba[2];
                img->A[i][j] = rgba[3];
            }
    }

    fclose(fp);
    return img;
}

static void savePPMOrPAM(const char *filename, ColorImage *img)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp) 
      { 
        printf("Error opening file: %s\n", filename);
         return; 
        }

    if (img->hasAlpha)
     {
        fprintf(fp, "P7\nWIDTH %d\nHEIGHT %d\nDEPTH 4\nMAXVAL %d\n"
                    "TUPLTYPE RGB_ALPHA\nENDHDR\n",
                img->width, img->height, img->maxVal);
        for (int i = 0; i < img->height; i++)
            for (int j = 0; j < img->width; j++) {
                unsigned char rgba[4] = {
                    (unsigned char)img->R[i][j],
                    (unsigned char)img->G[i][j],
                    (unsigned char)img->B[i][j],
                    (unsigned char)img->A[i][j]
                };
                fwrite(rgba, 1, 4, fp);
            }
    } 
    else
     {
        fprintf(fp, "P3\n%d %d\n%d\n", img->width, img->height, img->maxVal);
        for (int i = 0; i < img->height; i++)
         {
            for (int j = 0; j < img->width; j++)
                fprintf(fp, "%d %d %d ", img->R[i][j], img->G[i][j], img->B[i][j]);
                fprintf(fp, "\n");
        }
    }
    fclose(fp);
}

//public entry point
ColorImage *loadColorImage(const char *filename)
{
    const char *ext = strrchr(filename, '.');
    if (!ext) { printf("No file extension found\n"); return NULL; }

    if (strcmp(ext, ".bmp") == 0 || strcmp(ext, ".BMP") == 0) {
        FILE *fp = fopen(filename, "rb");
        if (fp) {
            BMPFileHeader fh; BMPInfoHeader ih;
            fread(&fh, sizeof(BMPFileHeader), 1, fp);
            fread(&ih, sizeof(BMPInfoHeader), 1, fp);
            fclose(fp);

            if (ih.biBitCount == 24 || ih.biBitCount == 32)
                return load32BitBMP(filename);
            if (ih.biBitCount == 8) {
                printf("This is an 8-bit grayscale BMP.\n"
                       "Please use the BMP compression option (Bit Plane Slicing) instead.\n");
                return NULL;
            }
        }
    }

    if (strcmp(ext, ".jpg")  == 0 || strcmp(ext, ".jpeg") == 0 ||
        strcmp(ext, ".JPG")  == 0 || strcmp(ext, ".JPEG") == 0)
         {
             printf("\nJPEG files cannot be processed directly.\n"
                   "Convert first, e.g.:\n"
                   "convert %s output.ppm\n"
                   " convert %s -depth 8 output.pam\n\n", filename, filename);
        return NULL;
    }

    if (strcmp(ext, ".ppm") == 0 || strcmp(ext, ".PPM") == 0 ||
        strcmp(ext, ".pam") == 0 || strcmp(ext, ".PAM") == 0)
        return loadPPMImage(filename);

    printf("Unsupported image format: %s\n", ext);
    return NULL;
}

void saveColorImage(const char *filename, ColorImage *img)
{
    const char *ext = strrchr(filename, '.');
    if (ext && (strcmp(ext, ".bmp") == 0 || strcmp(ext, ".BMP") == 0))
        save32BitBMP(filename, img);
    else
        savePPMOrPAM(filename, img);
}

void freeColorImage(ColorImage *img)
{
    if (!img) return;
    for (int i = 0; i < img->height; i++) 
    {
        free(img->R[i]); 
        free(img->G[i]); 
        free(img->B[i]);

        if (img->hasAlpha && img->A) 
        {
            free(img->A[i]);
        }
    }
    free(img->R); 
    free(img->G); 
    free(img->B);
    if (img->hasAlpha && img->A)
     free(img->A);
    free(img);
}