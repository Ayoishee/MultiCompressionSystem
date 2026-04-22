#ifndef HUFFMAN_H
#define HUFFMAN_H

#include "types.h"

typedef struct HuffmanNode
 {
    unsigned char  data;
    int  freq;
    struct HuffmanNode *left, *right;
 } HuffmanNode;
typedef struct 
{
    unsigned char character;
    char          code[256];
} HuffmanCode;
typedef struct 
{
    HuffmanNode *nodes[MAX_TREE_NODES];
    int size;
} MinHeap;

HuffmanNode *createNode(unsigned data, int freq);

void insertMinHeap(MinHeap *heap, HuffmanNode *node);

HuffmanNode *extractMin(MinHeap *heap);

void buildHuffmanCodes(HuffmanNode *root, 
                       char *code, int top,
                       HuffmanCode *huffmanCodes, 
                       int *index);

HuffmanNode *buildHuffmanTree(unsigned char *text, int size,
                               HuffmanCode *codes, int *codeCount);

void freeHuffmanTree(HuffmanNode *root);

#endif /* HUFFMAN_H */
