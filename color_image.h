#ifndef COLOR_IMAGE_H
#define COLOR_IMAGE_H

/* ============================================================
 * color_image.h  –  Color image load / save / free
 *
 * Supported input formats:
 *   .bmp  (24-bit RGB or 32-bit RGBA)
 *   .ppm  P3 (ASCII) or P6 (binary)
 *   .pam  P7 (binary RGBA)
 *
 * Supported output formats:
 *   .bmp  (24 or 32-bit, matching input)
 *   .ppm  P3 (RGB without alpha)
 *   .pam  P7 (RGBA with alpha)
 * ============================================================ */

#include "types.h"

ColorImage *loadColorImage(const char *filename);
void        saveColorImage(const char *filename, ColorImage *img);
void        freeColorImage(ColorImage *img);

#endif /* COLOR_IMAGE_H */
