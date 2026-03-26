#ifndef BMP_COMPRESS_H
#define BMP_COMPRESS_H

/* ============================================================
 * bmp_compress.h  –  Bit-plane slicing for 8-bit grayscale BMP
 * ============================================================ */

/*
 * operation == 1  →  compress  (input: .bmp 8-bit  →  output: .bps)
 * operation == 2  →  decompress (input: .bps        →  output: _decompressed.bmp)
 */
void processBMPFile(const char *fileName, int operation);

#endif /* BMP_COMPRESS_H */
