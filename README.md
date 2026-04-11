# Multi-Compression System in C

A modular, algorithm-driven **file compression and decompression system** implemented in C, supporting both **text data** and **BMP image formats** through multiple specialized techniques.

---

## Overview

This project presents a unified framework that applies **appropriate compression algorithms based on input file type**, demonstrating practical implementation of core concepts from **data structures**, **algorithm design**, and **digital image processing**.

The system supports both **compression and decompression**, ensuring efficient storage and accurate data reconstruction.

---

## Key Features

* Multi-format support: **Text (.txt)** and **Bitmap Images (.bmp)**
* Integration of multiple compression algorithms within a single system
* Automated selection of compression technique based on file type
* Lossless compression for text data using Huffman Coding
* Modular and maintainable C codebase
* Build automation using Makefile

---

## Algorithms & Techniques

### Huffman Coding (Text Compression)

* Frequency-based encoding using binary tree construction
* Generates optimal prefix codes
* Ensures **lossless compression**

---

### Bit Plane Slicing (8-bit BMP Images)

* Decomposes image into individual bit planes
* Supports:

  * **MSB-based compression**
  * **Full bit-plane slicing**
* Enables efficient grayscale image handling

---

### Discrete Cosine Transform (DCT) (32-bit BMP Images)

* Converts spatial data into frequency components
* Reduces high-frequency information
* Inspired by standard image compression techniques

---

## Project Structure

```
SPL-1/
├── .github/                      # GitHub configuration (copilot-instructions.md)
├── .vscode/                      # VS Code configuration
├── Makefile                      # Build automation file
├── README.md                     # Project documentation
│
├── Main Modules (Core Logic)
│   ├── main.c                    # Entry point of the program
│   ├── huffman.c / .h            # Huffman coding implementation
│   ├── bmp_compress.c / .h       # Bit Plane Slicing logic
│   ├── dct_compress.c / .h       # Discrete Cosine Transform logic
│   ├── text_compress.c / .h      # Text compression interface
│   └── color_image.c / .h        # Image processing utilities
│
├── Data Types & Utilities
│   └── types.c / .h              # Common data structures and definitions
│
├── Build Artifacts (Generated Files)
│   ├── *.o                       # Compiled object files
│   └── compressor                # Final executable
│
└── Test / Data Files
    ├── article.txt               # Sample text for testing
    ├── lena.bmp                  # Sample image (grayscale)
    ├── lena2.bmp                 # Additional test image
    ├── lenaColor.jpg             # Reference image
    └── output2.ppm               # Output test file
```

---

## Build & Execution

### Build using Makefile

```bash
make
```

### Run the Program

```bash
./compressor
```

---

## Technical Highlights

* Implementation of **tree-based encoding (Huffman)**
* Low-level handling of **BMP file formats**
* Application of **frequency-domain transformation (DCT)**
* Clear separation of modules for scalability and maintainability
* Use of **Makefile** for efficient build management

---

## Limitations

* Supports only `.txt` and `.bmp` formats for compression
* Image processing restricted to:

  * 8-bit grayscale BMP
  * 32-bit color BMP
* DCT implementation is simplified and not fully JPEG-compliant

---

## Future Enhancements

* Extend support to additional formats (PNG, JPEG)
* Integrate advanced algorithms (RLE, LZW)
* Improve compression efficiency and performance
* Add graphical user interface (GUI)

---

## Author

**Prottasha Saha Ayoishee**
B.Sc. in Software Engineering
University of Dhaka

---

## Summary

This project demonstrates the integration of multiple compression strategies within a single system, reflecting strong understanding of **algorithm design, file processing, and system-level programming in C**.

---
