#ifndef HUFFMAN_H
#define HUFFMAN_H

/* ============================================================
 * huffman.h  –  Huffman coding data structures & API
 * ============================================================ */

#include "types.h"

/* ---- A single node in the Huffman tree ---- */
typedef struct HuffmanNode {
    unsigned char      data;
    int                freq;
    struct HuffmanNode *left, *right;
} HuffmanNode;

/* ---- Bit-string code for one character ---- */
typedef struct {
    unsigned char character;
    char          code[256];
} HuffmanCode;

/* ---- Min-heap used to build the tree ---- */
typedef struct {
    HuffmanNode *nodes[MAX_TREE_NODES];
    int          size;
} MinHeap;

/* ---- Public API ---- */
HuffmanNode *createNode(unsigned data, int freq);

void         insertMinHeap(MinHeap *heap, HuffmanNode *node);
HuffmanNode *extractMin(MinHeap *heap);

void         buildHuffmanCodes(HuffmanNode *root, char *code, int top,
                               HuffmanCode *huffmanCodes, int *index);

HuffmanNode *buildHuffmanTree(unsigned char *text, int size,
                               HuffmanCode *codes, int *codeCount);

void         freeHuffmanTree(HuffmanNode *root);

#endif /* HUFFMAN_H */
