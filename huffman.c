#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "huffman.h"

HuffmanNode *createNode(unsigned data, int freq)
{
    HuffmanNode *node = (HuffmanNode *)malloc(sizeof(HuffmanNode));
    if (node == NULL)
     { 
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE); 
    }

    node->data = (unsigned char)data;
    node->freq = freq;
    node->left = node->right = NULL;

    return node;
}

static void swapNodes(HuffmanNode **a, HuffmanNode **b)
{
    HuffmanNode *temp;
                temp = *a;
                *a = *b; 
                *b = temp;
}

static void minHeapify(MinHeap *heap, int index)
{
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left<heap->size && heap->nodes[left]->freq < heap->nodes[smallest]->freq)
        smallest = left;
    if (right < heap->size && heap->nodes[right]->freq < heap->nodes[smallest]->freq)
        smallest = right;

    if (smallest != index) 
    {
        swapNodes(&heap->nodes[smallest], &heap->nodes[index]);
        minHeapify(heap, smallest);
    }
}

HuffmanNode *extractMin(MinHeap *heap)
{
    if (heap->size <= 0)
      {
        return NULL;
      }

    HuffmanNode *root = heap->nodes[0];
    heap->nodes[0]= heap->nodes[--heap->size];
    if (heap->size > 0)
        minHeapify(heap, 0);

    return root;
}

void insertMinHeap(MinHeap *heap, HuffmanNode *node)
{
    if (heap->size >= MAX_TREE_NODES)
     {
        printf("Heap overflow\n");
        exit(EXIT_FAILURE);
    }

    int i = heap->size++;
    while (i > 0) 
    {
        int parent = (i - 1) / 2;
        if (heap->nodes[parent]->freq <= node->freq)
        {
          break;
        }
        heap->nodes[i] = heap->nodes[parent];
        i = parent;
    }
    heap->nodes[i] = node;
}
//recursive
void buildHuffmanCodes(HuffmanNode *root, char *code, int top,
                       HuffmanCode *huffmanCodes, int *index)
{
    if (root->left == NULL && root->right == NULL)
     {
        code[top] = '\0';
        huffmanCodes[*index].character = root->data;
        strcpy(huffmanCodes[*index].code, code);
        (*index)++;
        return;
    }
    if (root->left)  
    { 
        code[top] = '0'; 
        buildHuffmanCodes(root->left,  code, top + 1, huffmanCodes, index);
     }
    if (root->right)
     {
         code[top] = '1'; 
         buildHuffmanCodes(root->right, code, top + 1, huffmanCodes, index);
      }
}

//tree making
HuffmanNode *buildHuffmanTree(unsigned char *text, int size,
                               HuffmanCode *codes, int *codeCount)
{
    if (size == 0) 
       return NULL;

    int freq[256] = {0};
    for (int i = 0; i < size; i++)
        freq[text[i]]++;

    MinHeap *heap = (MinHeap *)malloc(sizeof(MinHeap));
    if (!heap)
       {
         printf("Memory allocation failed\n");
         exit(EXIT_FAILURE); 
        }
    heap->size = 0;

    for (int i = 0; i < 256; i++)
        if (freq[i] > 0)
            insertMinHeap(heap, createNode((unsigned char)i, freq[i]));

    if (heap->size == 1)
     {
        HuffmanNode *only   = extractMin(heap);
        HuffmanNode *dummy  = createNode('\0', 0);
        HuffmanNode *parent = createNode('\0', only->freq + dummy->freq);

        parent->left = only;
        parent->right = dummy;

        insertMinHeap(heap, parent);
    }

    while (heap->size > 1) 
    {
        HuffmanNode *left  = extractMin(heap);
        HuffmanNode *right = extractMin(heap);

        HuffmanNode *merged = createNode('\0', left->freq + right->freq);
        merged->left  = left;
        merged->right = right;
        insertMinHeap(heap, merged);
    }

    HuffmanNode *root = extractMin(heap);

    char code[256];
    *codeCount = 0;
    buildHuffmanCodes(root, code, 0, codes, codeCount);

    free(heap);
    return root;
}

void freeHuffmanTree(HuffmanNode *root)
{
    if (!root)
       return;

    freeHuffmanTree(root->left);
    freeHuffmanTree(root->right);
    free(root);
}
