#ifndef DCT_COMPRESS_H
#define DCT_COMPRESS_H

/* ============================================================
 * dct_compress.h  –  DCT-based compression for color images
 * ============================================================ */

/*
 * operation == 1  →  compress  (DCT → quantize → IDCT, save result)
 * operation == 2  →  decompress (reload a previously compressed image)
 */
void processColorFile(const char *fileName, int operation);

#endif /* DCT_COMPRESS_H */
