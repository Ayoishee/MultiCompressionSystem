#ifndef COLOR_IMAGE_H
#define COLOR_IMAGE_H

#include "types.h"

ColorImage *loadColorImage(const char *filename);

void saveColorImage(const char *filename, ColorImage *img);
void freeColorImage(ColorImage *img);

#endif 
