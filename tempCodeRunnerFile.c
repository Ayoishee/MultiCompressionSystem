#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_FILENAME 256
#define MAX_TEXT_SIZE 1000000
#define MAX_TREE_NODES 512
#define PI 3.14159265358979323846

typedef enum{
    TEXT_FILE,
    BMP_FILE,
    FILE_COLOR,
    UNKNOWN_FILE
}FileType;
 
#pragma pack(push,1)
typedef struct
{
    unsigned short bfType;
    unsigned int bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;
}BMPFileHeader;

typedef struct
{
    unsigned int biSize;
    int biWidth;
    int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;

}BMPInfoHeader;
  
#pragma pack(pop)



FileType detectFileType(const char *fileName)
{
    int i;
    int lastDotIndex = -1;

    for(i = 0; fileName[i] != '\0'; i++)
    {
        if(fileName[i] == '.')
            lastDotIndex = i;  
    }

    if(lastDotIndex == -1 || lastDotIndex == 0){
        return UNKNOWN_FILE;
    }

    const char *ext = fileName + lastDotIndex;

    if(strcmp(ext, ".txt") == 0)
    {
        return TEXT_FILE;
    }
    
    // Check if BMP - need to determine if it's grayscale or color
    if(strcmp(ext, ".bmp") == 0 || strcmp(ext, ".BMP") == 0)
    {
        FILE *fp = fopen(fileName, "rb");
        if(fp) {
            BMPFileHeader fileHeader;
            BMPInfoHeader infoHeader;
            fread(&fileHeader, sizeof(BMPFileHeader), 1, fp);
            fread(&infoHeader, sizeof(BMPInfoHeader), 1, fp);
            fclose(fp);
            
            // 8-bit = grayscale, 24/32-bit = color
            if(infoHeader.biBitCount == 8) {
                return BMP_FILE;
            } else if(infoHeader.biBitCount == 24 || infoHeader.biBitCount == 32) {
                return FILE_COLOR;
            }
        }
        return BMP_FILE; // Default to grayscale if can't read
    }
    
    if(strcmp(ext, ".ppm") == 0 ||
       strcmp(ext, ".pam") == 0 ||
       strcmp(ext, ".jpg") == 0 ||
       strcmp(ext, ".jpeg") == 0)
    {
        return FILE_COLOR;
    }    

    return UNKNOWN_FILE;
}


typedef struct HuffmanNode{

     unsigned char data;
     int freq;
     struct HuffmanNode *left,  *right;

}HuffmanNode;

typedef struct{
  
    unsigned  char character; ;
     char code[256];

    }HuffmanCode;

typedef struct {

     HuffmanNode *nodes[MAX_TREE_NODES];
     int size;

}MinHeap;    

HuffmanNode *createNode(unsigned data,int freq)
{
     HuffmanNode *newNode = (HuffmanNode*)malloc(sizeof(HuffmanNode));
     
        if (!newNode) 
         {
           printf("Memory allocation failed\n");
           exit(EXIT_FAILURE);
         }

     newNode->data = data;
     newNode->freq = freq;
     newNode->left = newNode->right = NULL;

     return newNode;
}

void swapNodes(HuffmanNode **a,HuffmanNode **b)
{
    HuffmanNode *temp= *a;
    *a=*b;
    *b=temp;
}

void minHeapify(MinHeap *heap, int index)
{
    int smallest =index;
    int left=2*index+1;
    int right=2*index+2;

    if(left<heap->size && heap->nodes[left]->freq<heap->nodes[smallest]->freq)
        {
           smallest=left;
        }

    if(right <heap->size && heap->nodes[right]->freq<heap->nodes[smallest]->freq)
       {
           smallest=right;
       }

    if(smallest !=index)
       {
           swapNodes(&heap->nodes[smallest],&heap->nodes[index]);
           minHeapify(heap,smallest);  
       }

}

HuffmanNode* extractMin(MinHeap *heap)
{
    if (heap->size <= 0) 
    {
        return NULL;
    }

    HuffmanNode *root = heap->nodes[0];

    heap->nodes[0] = heap->nodes[heap->size - 1];
    heap->size--;
    if (heap->size > 0)
    {
        minHeapify(heap, 0);
    }

    return root;
   
}

void insertMinHeap(MinHeap *heap,HuffmanNode* node)
{
    if (heap->size >= MAX_TREE_NODES) {
        fprintf(stderr, "Heap overflow\n");
        exit(EXIT_FAILURE);
    }
    int i = heap->size;
    heap->size++;
   
    while (i > 0) {
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

void buildHuffmanCodes(HuffmanNode* root, char* code, int top, HuffmanCode* huffmanCodes, int* index)
{
    if(root->left==NULL && root->right==NULL)
    {
        code[top]='\0';
        huffmanCodes[*index].character=root->data;
        strcpy(huffmanCodes[*index].code,code);
        (*index)++;
        return;
    }
    if(root->left)      
    {
        code[top]='0';
        buildHuffmanCodes(root->left,code,top+1,huffmanCodes,index);
    }
    if(root->right)
    {
        code[top]='1';
        buildHuffmanCodes(root->right,code,top+1,huffmanCodes,index);
    }
    
}

HuffmanNode* buildHuffmanTree(unsigned char *text, int size,HuffmanCode *codes, int *codeCount)
{
     if (size == 0) 
    {
        return NULL;
    }

    int freq[256]={0};

    for(int i=0;i<size;i++)
    {
        freq[text[i]]++;
    }

    MinHeap* heap=(MinHeap*)malloc(sizeof(MinHeap));
    if (!heap)
        { fprintf(stderr, "Memory allocation failed\n");
          exit(EXIT_FAILURE);
         }

    heap->size = 0;

    for(int i=0;i<256;i++)
    {
        if(freq[i]>0)
        {
            insertMinHeap(heap,createNode((unsigned char)i,freq[i]));
        }
    }
       if (heap->size == 1)
        {
          HuffmanNode* only = extractMin(heap);
          HuffmanNode* dummy = createNode('\0', 0);
          HuffmanNode* parent = createNode('\0', only->freq + dummy->freq);
          parent->left = only;
          parent->right = dummy;
          insertMinHeap(heap, parent);
       }


    while(heap->size>1)
    {
        HuffmanNode* left=extractMin(heap);
        HuffmanNode* right=extractMin(heap);

        HuffmanNode* internalNode=createNode('\0',left->freq+right->freq);
        internalNode->left=left;
        internalNode->right=right;

        insertMinHeap(heap,internalNode);
    }

    HuffmanNode* root=extractMin(heap);

    char code[256];
    *codeCount=0;
    buildHuffmanCodes(root,code,0,codes,codeCount);

     if (root)
      {
          buildHuffmanCodes(root, code, 0, codes, codeCount);
       }

    free(heap);
    return root;
}

void freeHuffmanTree(HuffmanNode* root)
{
    if (!root) 
         return;
    freeHuffmanTree(root->left);
    freeHuffmanTree(root->right);
    free(root);
}

void compressTextHuffman(const char *inputFile)
{
    printf("Detected: Text File\n");
    printf("HUFFMAN CODING IS USED FOR COMPRESSION\n");
    printf("\n=== TEXT FILE COMPRESSION ===\n");
    
    FILE *fp=fopen(inputFile,"rb");
    if(fp==NULL)
    {
        printf("Error opening file: %s\n",inputFile);
        return;
    }

    fseek(fp,0,SEEK_END);
    long fileSize=ftell(fp);
    fseek(fp,0,SEEK_SET);

     if (fileSize == 0)
      {
        printf("Input file is empty. Nothing to compress.\n");
        fclose(fp);
        return;
     }

    unsigned char *text=(unsigned char*)malloc(fileSize);
    fread(text,1,fileSize,fp);
    fclose(fp);

    HuffmanCode huffmanCodes[256];
    int codeCount=0;
    HuffmanNode* root=buildHuffmanTree(text,fileSize,huffmanCodes,&codeCount);
    
     if (!root) 
     {
        printf("Failed to build Huffman tree\n");
        free(text);
        return;
    }

   char compressedFile[MAX_FILENAME];
   strcpy(compressedFile, inputFile);
   char *ext=strrchr(compressedFile,'.');
   if(ext)*ext='\0';
   strcat(compressedFile,"_compressed.huff");

   fp= fopen(compressedFile,"wb");

    fwrite(&codeCount,sizeof(int),1,fp);

    for(int i=0;i<codeCount;i++){
        fwrite(&huffmanCodes[i].character,sizeof(unsigned char),1,fp);
        int codeLength=strlen(huffmanCodes[i].code);
        fwrite(&codeLength,sizeof(int),1,fp);
        fwrite(huffmanCodes[i].code,sizeof(char),codeLength,fp);
    }

    fwrite(&fileSize,sizeof(long),1,fp);

      char* compressed=(char*)malloc(fileSize*256);
      int compressedIndex=0;

       for(int i=0; i<fileSize;i++)
          {
            for(int j=0;j<codeCount;j++)
               {
                   if(text[i]==huffmanCodes[j].character)
                    {
                       for(int k=0;k<strlen(huffmanCodes[j].code);k++)
                         {
                          compressed[compressedIndex++]=huffmanCodes[j].code[k];
                         }
                break;
            }
        }
    }   

    fwrite(&compressedIndex,sizeof(int),1,fp);
    fwrite(compressed,sizeof(char),compressedIndex,fp);
    fclose(fp);

    printf("File compressed successfully: %s\n",compressedFile);
    printf("Original Size: %ld bytes\n",fileSize);

    int fullBytes = compressedIndex / 8;
    int leftoverBits = compressedIndex % 8;

    int extraByte = 0;
    if(leftoverBits != 0)
              {
               extraByte = 1;
               }

    int compressedBytes = fullBytes + extraByte;

    printf("Compressed Size: %d bytes\n", compressedBytes);

    double fractionRemaining = (double)compressedBytes / (double)fileSize;
    double reduction = (1.0 - fractionRemaining) * 100.0;

    printf("Reduction: %.2f%%\n", reduction);

    printf("=== DECOMPRESSION ===\n");

    fp=fopen(compressedFile,"rb");

    int readCodeCount;
    fread(&readCodeCount,sizeof(int),1,fp);

    HuffmanCode readCodes[256];

    for(int i=0;i<readCodeCount;i++)
    {
        fread(&readCodes[i].character,sizeof(unsigned char),1,fp);

        int codeLength;

        fread(&codeLength,sizeof(int),1,fp);
        fread(readCodes[i].code,sizeof(char),codeLength,fp);

        readCodes[i].code[codeLength]='\0'; 
    }

        long originalSize;
        fread(&originalSize,sizeof(long),1,fp);

        int readCompressedLength;
        fread(&readCompressedLength,sizeof(int),1,fp);

        char* readCompressed=(char*)malloc(readCompressedLength+1);
        fread(readCompressed,sizeof(char),readCompressedLength,fp);
        readCompressed[readCompressedLength]='\0'; 
        fclose(fp);

        unsigned char* decompressedText=(unsigned char*)malloc(originalSize+1);
        int decompressedIndex=0;
        char currentCode[256]="";
        int currentCodeIndex=0;

        for(int i=0;i<readCompressedLength && decompressedIndex < originalSize;i++)
        {
            currentCode[currentCodeIndex++]=readCompressed[i];
            currentCode[currentCodeIndex]='\0';

            for(int j=0;j<readCodeCount;j++)
            {
                if(strcmp(currentCode,readCodes[j].code)==0)
                {
                    decompressedText[decompressedIndex++]=readCodes[j].character;
                    currentCodeIndex=0;
                    currentCode[0]='\0';
                    break;
                }
            }
        }

    char decompressedFile[MAX_FILENAME];
    strcpy(decompressedFile, inputFile);
    ext = strrchr(decompressedFile, '.');
    if (ext) *ext = '\0';
    strcat(decompressedFile, "_decompressed.txt");
    
    fp = fopen(decompressedFile, "wb");
    fwrite(decompressedText, sizeof(unsigned char), decompressedIndex, fp);
    fclose(fp);

    printf("File decompressed successfully: %s\n", decompressedFile);
    printf("Decompressed %d bytes (expected %ld)\n", decompressedIndex, originalSize);
    

    free(text);
    free(compressed);
    free(readCompressed);
    free(decompressedText);

}

void compressBMPBitPlaneMSB(const char *inputFile)
{
    printf("\n=== BMP IMAGE COMPRESSION (MSB) ===\n");
    printf("Detected: BMP ImageFile\n");
    printf("Bit Plane Slicing(MSB) is used for compression.\n");
    
    FILE *fp=fopen(inputFile,"rb");

    if(fp==NULL)
    {
        printf("Error opening file: %s\n",inputFile);
        return; 
    }

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    fread(&fileHeader,sizeof(BMPFileHeader),1,fp);
    fread(&infoHeader,sizeof(BMPInfoHeader),1,fp);  
    

    if(fileHeader.bfType != 0x4D42)
    {
        printf("Not a BMP file.\n");
        fclose(fp);
        return;
    }

    if(infoHeader.biBitCount != 8)
    {
        printf("Only 8-bit grayscale BMP images are supported\n");
        printf("(Your image is %d-bit)\n", infoHeader.biBitCount);
        fclose(fp);
        return;
    }

    int width = infoHeader.biWidth;
    int height;
    int isBottomUp;

    if (infoHeader.biHeight > 0)
     {
         height = infoHeader.biHeight;  
         isBottomUp = 1;
      } 
      else
       {
         height = -infoHeader.biHeight; 
         isBottomUp = 0;
        }

    int imageSize=width*height;


    printf("Image info:\n");
    printf(" Dimensions: %dx%d pixels\n", width, height);
    printf(" Bit depth:  8-bit grayscale\n\n");
    
    printf(" Total pixels: %d\n\n", imageSize);

    
    unsigned char palette[1024];
    fread(palette,1,1024,fp);
    fseek(fp, fileHeader.bfOffBits, SEEK_SET);

    unsigned char *imageData=(unsigned char*)malloc(imageSize);

    int remainder = width % 4;
    int missing   = 4 - remainder;
    int rowPadding = missing % 4;

    unsigned char padBuffer[4];

    printf("Reading Image...\n");
    long startPos = ftell(fp);

for (int i = 0; i < height; i++)
    {
      int row;

       if (isBottomUp)
            {
               row = height - 1 - i;
             }
        else
            {
               row = i;
             }

       size_t bytesRead = fread(imageData + row * width, 1, width, fp);
       if (bytesRead < width)
                  {
                     printf("WARNING");
                     printf("   Expected %d bytes, got %zu.\n", width, bytesRead);
                     break; 
                  }
            if (rowPadding > 0)
                    {
                       fread(padBuffer, 1, rowPadding, fp);
                      }
                     }
    
    long endPos = ftell(fp);
    printf("Read finished. Bytes read from pixel array: %ld\n", endPos - startPos);
    
    fclose(fp);

    printf("Step 1: Extracting MSB from each pixel...\n");

    unsigned char *msbPlane = (unsigned char*)malloc(imageSize);
    for(int i = 0; i < imageSize; i++)
    {
        msbPlane[i] = (imageData[i] >> 7) & 1;
    }
    printf("Step 2: Packing MSB bits into bytes...\n");
    int packedSize = imageSize / 8;
      if (imageSize % 8 != 0)
            {
             packedSize++;
             }

    unsigned char *packed = (unsigned char*)calloc(packedSize, 1);
    
    for(int i = 0; i < imageSize; i++)
    {
        if(msbPlane[i])
        {
            int byteIndex = i / 8;        
            int bitIndex  = i % 8;        
            int bitPos    = 7 - bitIndex;
            packed[byteIndex] = packed[byteIndex] | (1 << bitPos);

        }
    }

    char compressedFile[MAX_FILENAME];
    strcpy(compressedFile, inputFile);

    char *ext=strrchr(compressedFile,'.');
    if(ext)*ext='\0';

    strcat(compressedFile,"_compressed.bps");

    fp=fopen(compressedFile,"wb");

     fwrite(&width,sizeof(int),1,fp);
     fwrite(&height,sizeof(int),1,fp);
     fwrite(packed,1,packedSize,fp);

      fclose(fp);

   long originalSize = fileHeader.bfSize;


   long compressedSize = sizeof(int)*2 + packedSize;

    printf("\nCompression Results:\n");
    printf("File compressed successfully: %s\n", compressedFile);
    printf("Original Size: %ld bytes\n", originalSize);
    printf("Compressed Size: %ld bytes\n", compressedSize);
    printf("Reduction: %.2f%%\n", (1.0 - ((double)compressedSize / originalSize)) * 100.0);
    printf("Compression Ratio: %.2f:1\n\n", (double)originalSize / compressedSize);


    printf("Step 3: Decompressing...\n");

    printf("=== DECOMPRESSION ===\n");

    fp = fopen(compressedFile, "rb");
    int readWidth, readHeight;
    fread(&readWidth, sizeof(int), 1, fp);
    fread(&readHeight, sizeof(int), 1, fp);
    
    int readImageSize = readWidth * readHeight;
   int readPackedSize = readImageSize / 8;
         if (readImageSize % 8 != 0)
             {
               readPackedSize++;
              }



    unsigned char *readPacked = (unsigned char*)malloc(readPackedSize);
    fread(readPacked, 1, readPackedSize, fp);
    fclose(fp);


    unsigned char *unpackedMSB = (unsigned char*)malloc(readImageSize);
    for(int i = 0; i < readImageSize; i++)
    {
        int byteIndex = i / 8;      
        int bitIndex  = i % 8;       
        int bitPos    = 7 - bitIndex;
        unsigned char value = readPacked[byteIndex];

          unpackedMSB[i] = (value >> bitPos) & 1;

    }

    unsigned char *reconstructed = (unsigned char*)calloc(readImageSize, 1);
    for(int i = 0; i < readImageSize; i++)
    {
        reconstructed[i] = unpackedMSB[i] ? 128 : 0;
    }

    char decompressedFile[MAX_FILENAME];
    strcpy(decompressedFile, inputFile);

    ext = strrchr(decompressedFile, '.');
    if(ext) *ext = '\0';
    strcat(decompressedFile, "_decompressed.bmp");


    infoHeader.biSizeImage = (width + rowPadding) * height;

    fileHeader.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader)+ 1024;

    fileHeader.bfSize = fileHeader.bfOffBits + infoHeader.biSizeImage;  

   if (isBottomUp)
           {
              infoHeader.biHeight = height;   
           }
   else
           {
               infoHeader.biHeight = -height;  
           }


    fp = fopen(decompressedFile, "wb");
    
    fwrite(&fileHeader, sizeof(BMPFileHeader), 1, fp);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, fp);
    fwrite(palette, 1, 1024, fp);
    
    unsigned char pad[3] = {0, 0, 0};
    for (int i = 0; i < height; i++)
    {
        int row;

       if (isBottomUp)
            {
               row = height - 1 - i;
             }
        else
            {
               row = i;
             }
        fwrite(reconstructed + row * width, 1, width, fp);
        fwrite(pad, 1, rowPadding, fp);
    }
    fclose(fp);
    
    printf("File decompressed successfully: %s\n", decompressedFile);
    printf("Note: This is lossy compression - image quality is reduced\n");

    free(imageData);
    free(msbPlane);
    free(packed);
    free(readPacked);
    free(unpackedMSB);
    free(reconstructed);
}


void compressBMPBitPlaneFull(const char *inputFile)
{
    printf("\n=== BMP IMAGE COMPRESSION (FULL) ===\n");
    printf("Detected: BMP ImageFile\n");
    printf("Bit Plane Slicing (Full Lossless) is used for compression.\n");
    
    FILE *fp=fopen(inputFile,"rb");

    if(fp==NULL)
    {
        printf("Error opening file: %s\n",inputFile);
        return; 
    }

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    fread(&fileHeader,sizeof(BMPFileHeader),1,fp);
    fread(&infoHeader,sizeof(BMPInfoHeader),1,fp);  
    
    if(fileHeader.bfType != 0x4D42)
    {
        printf("Not a BMP file.\n");
        fclose(fp);
        return;
    }

    if(infoHeader.biBitCount != 8)
    {
        printf("Only 8-bit grayscale BMP images are supported\n");
        fclose(fp);
        return;
    }

    int width = infoHeader.biWidth;
    int height;
    int isBottomUp;

    if (infoHeader.biHeight > 0)
     {
         height = infoHeader.biHeight;  
         isBottomUp = 1;
      } 
      else
       {
         height = -infoHeader.biHeight; 
         isBottomUp = 0;
        }
    int imageSize=width*height;

   int remainder = width % 4;
   int missing   = 4 - remainder;
   int rowPadding = missing % 4;

    unsigned char palette[1024];
    fread(palette,1,1024,fp);
    fseek(fp, fileHeader.bfOffBits, SEEK_SET);

    unsigned char *imageData=(unsigned char*)malloc(imageSize);
    for (int i = 0; i < height; i++)
    {
       int row;

       if (isBottomUp)
            {
                row = height - 1 - i;
             }
       else
             {
                 row = i;
             }

        fread(imageData + row * width, 1, width, fp);
        fseek(fp, rowPadding, SEEK_CUR);
    }
    fclose(fp);

    printf("Step 1: Extracting all 8 bit-planes...\n");
    unsigned char *bitPlanes[8];
    for(int plane = 0; plane < 8; plane++)
    {
        bitPlanes[plane] = (unsigned char*)malloc(imageSize);
        for(int i = 0; i < imageSize; i++)
        {
            bitPlanes[plane][i] = (imageData[i] >> plane) & 1;
        }
    }

    printf("Step 2: Packing bits into bytes for each plane...\n");
    int packedSizePerPlane = (imageSize + 7) / 8;
    int totalPackedSize = packedSizePerPlane * 8;
    unsigned char *allPackedData = (unsigned char*)calloc(totalPackedSize, 1);

    for(int plane = 0; plane < 8; plane++)
    {
        for(int i = 0; i < imageSize; i++)
        {
            if(bitPlanes[plane][i])
            {
                int planeOffset = plane * packedSizePerPlane;
                int byteInPlane = i / 8;
                int byteIndex = planeOffset + byteInPlane;

                int bitPos = 7 - (i % 8);
                allPackedData[byteIndex] |= (1 << bitPos);

            }
        }
    }

    char compressedFile[MAX_FILENAME];
    strcpy(compressedFile, inputFile);

    char *ext=strrchr(compressedFile,'.');
    if(ext)*ext='\0';
    strcat(compressedFile,"_full_compressed.bps");

    fp=fopen(compressedFile,"wb");

    fwrite(&width,sizeof(int),1,fp);
    fwrite(&height,sizeof(int),1,fp);
    fwrite(allPackedData,1,totalPackedSize,fp);
    fclose(fp);

    printf("File compressed successfully: %s\n", compressedFile);
    
    long originalSize = fileHeader.bfSize;
    long compressedSize = sizeof(int)*2 + totalPackedSize;
    printf("Original Size: %ld bytes\n", originalSize);
    printf("Compressed Size: %ld bytes\n", compressedSize);
   
    double reduction = 1.0 - (double)compressedSize / originalSize;
    reduction = reduction * 100; 
    printf("Reduction: %.2f%%\n", reduction);

    printf("Step 3: Decompressing...\n");
    fp = fopen(compressedFile, "rb");
    int readWidth, readHeight;
    fread(&readWidth, sizeof(int), 1, fp);
    fread(&readHeight, sizeof(int), 1, fp);
    
    int readImageSize = readWidth * readHeight;
    int readPackedSizePerPlane = (readImageSize + 7)/ 8;
    int readTotalPackedSize = readPackedSizePerPlane * 8;
    
    unsigned char *readPacked = (unsigned char*)malloc(readTotalPackedSize);
    fread(readPacked, 1, readTotalPackedSize, fp);
    fclose(fp);

    unsigned char *reconstructed = (unsigned char*)calloc(readImageSize, 1);
    
    for(int plane = 0; plane < 8; plane++)
    {
        for(int i = 0; i < readImageSize; i++)
        {
            int planeOffset = plane * readPackedSizePerPlane;
            int byteInPlane = i / 8;
            int byteIndex = planeOffset + byteInPlane;

            int bitPos = 7 - (i % 8);

            int value = (readPacked[byteIndex] >> bitPos) & 1;

               if (value == 1)
                       {
                          reconstructed[i] |= (1 << plane);
                        }

        }
    }

    char decompressedFile[MAX_FILENAME];
    strcpy(decompressedFile, inputFile);
    ext = strrchr(decompressedFile, '.');
    if(ext) *ext = '\0';
    strcat(decompressedFile, "_full_decompressed.bmp");

    infoHeader.biSizeImage = (width + rowPadding) * height;
    fileHeader.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader)+ 1024;
    fileHeader.bfSize = fileHeader.bfOffBits + infoHeader.biSizeImage; 

    if (isBottomUp)
         {
           infoHeader.biHeight = height;  
         }
    else
         {
           infoHeader.biHeight = -height;  
         }


    fp = fopen(decompressedFile, "wb");
    fwrite(&fileHeader, sizeof(BMPFileHeader), 1, fp);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, fp);
    fwrite(palette, 1, 1024, fp);
    
    unsigned char pad[3] = {0, 0, 0};

    for (int i = 0; i < height; i++)
    {
        int row;
        if (isBottomUp)
            {
               row = height - 1 - i;
             }
        else
            {
               row = i;
             }

        fwrite(reconstructed + row * width, 1, width, fp);
        fwrite(pad, 1, rowPadding, fp);
    }
    fclose(fp);
    printf("File decompressed successfully: %s\n", decompressedFile);

    free(imageData);
    for(int plane = 0; plane < 8; plane++) 
    free(bitPlanes[plane]);
    free(allPackedData);
    free(readPacked);
    free(reconstructed);
}

typedef struct{
        int width;
        int height;
        int maxVal;

        int**R;
        int**G;
        int**B;
        int**A;
        int hasAlpha;
        
}ColorImage;


ColorImage*load32BitBMP(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if(fp == NULL) {
        printf("Error opening BMP file: %s\n", filename);
        return NULL;
    }

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    fread(&fileHeader, sizeof(BMPFileHeader), 1, fp);
    fread(&infoHeader, sizeof(BMPInfoHeader), 1, fp);

    if(fileHeader.bfType != 0x4D42) {
        printf("Not a BMP file.\n");
        fclose(fp);
        return NULL;
    }

    if(infoHeader.biBitCount != 32 && infoHeader.biBitCount != 24) {
        printf("Only 24-bit RGB or 32-bit RGBA BMP images are supported\n");
        printf("(Your image is %d-bit)\n", infoHeader.biBitCount);
        fclose(fp);
        return NULL;
    }

    ColorImage *img = (ColorImage*)malloc(sizeof(ColorImage));
    img->width = infoHeader.biWidth;
    
    int height;
    int isBottomUp;
    if (infoHeader.biHeight > 0) {
        height = infoHeader.biHeight;
        isBottomUp = 1;
    } else {
        height = -infoHeader.biHeight;
        isBottomUp = 0;
    }
    img->height = height;
    img->maxVal = 255;
    img->hasAlpha = (infoHeader.biBitCount == 32) ? 1 : 0;

    // Allocate memory
    img->R = (int**)malloc(height * sizeof(int*));
    img->G = (int**)malloc(height * sizeof(int*));
    img->B = (int**)malloc(height * sizeof(int*));
    
    if(img->hasAlpha) {
        img->A = (int**)malloc(height * sizeof(int*));
    } else {
        img->A = NULL;
    }

    for(int i = 0; i < height; i++) {
        img->R[i] = (int*)malloc(img->width * sizeof(int));
        img->G[i] = (int*)malloc(img->width * sizeof(int));
        img->B[i] = (int*)malloc(img->width * sizeof(int));
        if(img->hasAlpha) {
            img->A[i] = (int*)malloc(img->width * sizeof(int));
        }
    }

    // Seek to pixel data
    fseek(fp, fileHeader.bfOffBits, SEEK_SET);

    // Calculate row padding
    int bytesPerPixel = img->hasAlpha ? 4 : 3;
    int rowSize = img->width * bytesPerPixel;
    int rowPadding = (4 - (rowSize % 4)) % 4;

    printf("Image info:\n");
    printf("  Dimensions: %dx%d pixels\n", img->width, img->height);
    printf("  Bit depth: %d-bit (%s)\n", infoHeader.biBitCount, img->hasAlpha ? "RGBA" : "RGB");
    printf("  Channels: %d\n\n", img->hasAlpha ? 4 : 3);

    // Read pixel data
    unsigned char padBuffer[4];
    for(int i = 0; i < height; i++) {
        int row = isBottomUp ? (height - 1 - i) : i;
        
        for(int j = 0; j < img->width; j++) {
            unsigned char pixel[4];
            
            if(img->hasAlpha) {
                // 32-bit: BGRA format
                fread(pixel, 1, 4, fp);
                img->B[row][j] = pixel[0];
                img->G[row][j] = pixel[1];
                img->R[row][j] = pixel[2];
                img->A[row][j] = pixel[3];
            } else {
                // 24-bit: BGR format
                fread(pixel, 1, 3, fp);
                img->B[row][j] = pixel[0];
                img->G[row][j] = pixel[1];
                img->R[row][j] = pixel[2];
            }
        }
        
        if(rowPadding > 0) {
            fread(padBuffer, 1, rowPadding, fp);
        }
    }

    fclose(fp);
    return img;
}

void save32BitBMP(const char *filename, ColorImage *img) {
    FILE *fp = fopen(filename, "wb");
    if(fp == NULL) {
        printf("Error creating BMP file: %s\n", filename);
        return;
    }

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    int bytesPerPixel = img->hasAlpha ? 4 : 3;
    int rowSize = img->width * bytesPerPixel;
    int rowPadding = (4 - (rowSize % 4)) % 4;
    int imageSize = (rowSize + rowPadding) * img->height;

    // Fill BMP headers
    fileHeader.bfType = 0x4D42;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;
    fileHeader.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
    fileHeader.bfSize = fileHeader.bfOffBits + imageSize;

    infoHeader.biSize = sizeof(BMPInfoHeader);
    infoHeader.biWidth = img->width;
    infoHeader.biHeight = img->height;  // Positive = bottom-up
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = img->hasAlpha ? 32 : 24;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = imageSize;
    infoHeader.biXPelsPerMeter = 2835;
    infoHeader.biYPelsPerMeter = 2835;
    infoHeader.biClrUsed = 0;
    infoHeader.biClrImportant = 0;

    // Write headers
    fwrite(&fileHeader, sizeof(BMPFileHeader), 1, fp);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, fp);

    // Write pixel data (bottom-up)
    unsigned char pad[3] = {0, 0, 0};
    for(int i = img->height - 1; i >= 0; i--) {
        for(int j = 0; j < img->width; j++) {
            unsigned char pixel[4];
            
            if(img->hasAlpha) {
                // 32-bit: BGRA format
                pixel[0] = (unsigned char)img->B[i][j];
                pixel[1] = (unsigned char)img->G[i][j];
                pixel[2] = (unsigned char)img->R[i][j];
                pixel[3] = (unsigned char)img->A[i][j];
                fwrite(pixel, 1, 4, fp);
            } else {
                // 24-bit: BGR format
                pixel[0] = (unsigned char)img->B[i][j];
                pixel[1] = (unsigned char)img->G[i][j];
                pixel[2] = (unsigned char)img->R[i][j];
                fwrite(pixel, 1, 3, fp);
            }
        }
        
        if(rowPadding > 0) {
            fwrite(pad, 1, rowPadding, fp);
        }
    }

    fclose(fp);
}

ColorImage *loadPPMImage(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if(fp==NULL)
    {
        printf("Error opening file: %s\n", filename);
        return NULL;
    }

    ColorImage *img=(ColorImage*)malloc(sizeof(ColorImage));
    char magic[10];
    fscanf(fp,"%s",magic);
    
    if(strcmp(magic, "P3") != 0 && strcmp(magic, "P6") != 0 && strcmp(magic, "P7") != 0) {
        printf("Unsupported PPM format: %s\n", magic);
        printf("Supported: P3 (24-bit ASCII), P6 (24-bit binary), P7 (32-bit PAM)\n");
        fclose(fp);
        free(img);
        return NULL;
    }

    img->hasAlpha = (strcmp(magic, "P7") == 0) ? 1 : 0;

    char c = fgetc(fp);
    while(c=='#' || c=='\n' || c==' ')
    {
        if(c=='#')
        {
            while(fgetc(fp) !='\n');
        }
        c=fgetc(fp);
    }
    ungetc(c,fp);
    
    if(img->hasAlpha) {
        // P7 format has different header
        char line[256];
        while(fgets(line, sizeof(line), fp)) {
            if(strstr(line, "WIDTH")) sscanf(line, "WIDTH %d", &img->width);
            if(strstr(line, "HEIGHT")) sscanf(line, "HEIGHT %d", &img->height);
            if(strstr(line, "MAXVAL")) sscanf(line, "MAXVAL %d", &img->maxVal);
            if(strstr(line, "ENDHDR")) break;
        }
    } else {
        fscanf(fp, "%d %d %d", &img->width, &img->height, &img->maxVal);
    }

    img->R=(int**)malloc(img->height*sizeof(int*));
    img->G=(int**)malloc(img->height*sizeof(int*)); 
    img->B=(int**)malloc(img->height*sizeof(int*));
    
    if(img->hasAlpha) {
        img->A=(int**)malloc(img->height*sizeof(int*));
    } else {
        img->A = NULL;
    }

    for (int i=0;i<img->height;i++)
    {
        img->R[i]=(int*)malloc(img->width*sizeof(int));
        img->G[i]=(int*)malloc(img->width*sizeof(int));
        img->B[i]=(int*)malloc(img->width*sizeof(int));
        if(img->hasAlpha) {
            img->A[i]=(int*)malloc(img->width*sizeof(int));
        }
    }

    if(strcmp(magic, "P3") == 0) {
        // ASCII format (24-bit)
        for(int i=0;i<img->height;i++)
        {
            for(int j=0;j<img->width;j++)
            {
                fscanf(fp,"%d %d %d",&img->R[i][j],&img->G[i][j],&img->B[i][j]);
            }
        }
    } else if(strcmp(magic, "P6") == 0) {
        // P6 Binary format (24-bit)
        fgetc(fp); // Skip last newline
        for(int i=0;i<img->height;i++)
        {
            for(int j=0;j<img->width;j++)
            {
                unsigned char rgb[3];
                fread(rgb, 1, 3, fp);
                img->R[i][j] = rgb[0];
                img->G[i][j] = rgb[1];
                img->B[i][j] = rgb[2];
            }
        }
    } else if(strcmp(magic, "P7") == 0) {
        // P7 Binary format (32-bit RGBA)
        for(int i=0;i<img->height;i++)
        {
            for(int j=0;j<img->width;j++)
            {
                unsigned char rgba[4];
                fread(rgba, 1, 4, fp);
                img->R[i][j] = rgba[0];
                img->G[i][j] = rgba[1];
                img->B[i][j] = rgba[2];
                img->A[i][j] = rgba[3];
            }
        }
    }
    
    fclose(fp);
    return img;
}

ColorImage *loadColorImage(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if(ext == NULL) {
        printf("No file extension found\n");
        return NULL;
    }
    
    // Check if it's a BMP file
    if(strcmp(ext, ".bmp") == 0 || strcmp(ext, ".BMP") == 0) {
        // Check if it's a color BMP (24-bit or 32-bit)
        FILE *fp = fopen(filename, "rb");
        if(fp) {
            BMPFileHeader fileHeader;
            BMPInfoHeader infoHeader;
            fread(&fileHeader, sizeof(BMPFileHeader), 1, fp);
            fread(&infoHeader, sizeof(BMPInfoHeader), 1, fp);
            fclose(fp);
            
            if(infoHeader.biBitCount == 24 || infoHeader.biBitCount == 32) {
                return load32BitBMP(filename);
            } else if(infoHeader.biBitCount == 8) {
                printf("\n⚠️  This is an 8-bit grayscale BMP.\n");
                printf("Please use the BMP compression option (Bit Plane Slicing) instead.\n");
                return NULL;
            }
        }
    }
    
    if(strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0 || 
       strcmp(ext, ".JPG") == 0 || strcmp(ext, ".JPEG") == 0) {
        
        printf("\n╔════════════════════════════════════════════════════════╗\n");
        printf("║         JPEG/JPG FILE DETECTED                         ║\n");
        printf("╚════════════════════════════════════════════════════════╝\n\n");
        
        printf("⚠️  JPEG files cannot be processed directly.\n\n");
        printf("To process JPEG files, convert them first:\n\n");
        
        printf("🔧 FOR 24-BIT RGB:\n");
        printf("   convert %s output.ppm\n", filename);
        printf("   OR convert to 24-bit BMP:\n");
        printf("   convert %s output.bmp\n\n", filename);
        
        printf("🔧 FOR 32-BIT RGBA (with transparency):\n");
        printf("   convert %s -depth 8 output.pam\n", filename);
        printf("   OR convert to 32-bit BMP:\n");
        printf("   convert %s -define bmp:format=bmp32 output.bmp\n\n", filename);
        
        return NULL;
        
    } else if(strcmp(ext, ".ppm") == 0 || strcmp(ext, ".PPM") == 0 ||
              strcmp(ext, ".pam") == 0 || strcmp(ext, ".PAM") == 0) {
        return loadPPMImage(filename);
    }
    
    printf("Unsupported image format: %s\n", ext);
    printf("Supported: .bmp (24/32-bit), .ppm, .pam\n");
    return NULL;
}

void saveColorImage(const char * filename, ColorImage* img) {
    const char *ext = strrchr(filename, '.');
    
    // Save as BMP if extension is .bmp
    if(ext && (strcmp(ext, ".bmp") == 0 || strcmp(ext, ".BMP") == 0)) {
        save32BitBMP(filename, img);
        return;
    }
    
    // Otherwise save as PPM/PAM
    FILE *fp = fopen(filename, "wb");
    if(fp==NULL)
    {
        printf("Error opening file: %s\n", filename);
        return;
    }

    if(img->hasAlpha) {
        // Save as P7 (32-bit PAM format)
        fprintf(fp,"P7\n");
        fprintf(fp,"WIDTH %d\n", img->width);
        fprintf(fp,"HEIGHT %d\n", img->height);
        fprintf(fp,"DEPTH 4\n");
        fprintf(fp,"MAXVAL %d\n", img->maxVal);
        fprintf(fp,"TUPLTYPE RGB_ALPHA\n");
        fprintf(fp,"ENDHDR\n");
        
        for(int i=0;i<img->height;i++)
        {
            for(int j=0;j<img->width;j++)
            {
                unsigned char rgba[4];
                rgba[0] = (unsigned char)img->R[i][j];
                rgba[1] = (unsigned char)img->G[i][j];
                rgba[2] = (unsigned char)img->B[i][j];
                rgba[3] = (unsigned char)img->A[i][j];
                fwrite(rgba, 1, 4, fp);
            }
        }
    } else {
        // Save as P3 (24-bit ASCII)
        fprintf(fp,"P3\n%d %d\n%d\n",img->width,img->height,img->maxVal);
        
        for(int i=0;i<img->height;i++)
        {
            for(int j=0;j<img->width;j++)
            {
                fprintf(fp,"%d %d %d ",img->R[i][j],img->G[i][j],img->B[i][j]);
            }
            fprintf(fp,"\n");
        }
    }
    
    fclose(fp);
}

double dctCoeff(int x,int y, int u, int v, int N){
    double cu=(u==0)? 1.0/sqrt(2.0) :1.0;
    double cv =(v==0)? 1.0/sqrt(2.0) :1.0;

    return (2.0/N)* cu*cv*cos((2*x+1)*u*PI/(2.0*N))*cos((2*y+1)*v*PI/(2.0*N));
}

void dct2D(int block[8][8],double dctBlock[8][8]){
    int N=8;
    for(int u=0;u<N;u++)
    {
        for(int v=0;v<N;v++)
        {
            double sum=0.0;
            for(int x=0;x<N;x++)
            {
                for(int y=0;y<N;y++)
                {
                    sum+=block[x][y]*dctCoeff(x,y,u,v,N);
                }
            }
            dctBlock[u][v]=sum;
        }
    }
}

void idct2D(double dctBlock[8][8], int block[8][8],int maxVal){
    int N=8;
    for(int x=0;x<N;x++)
    {
        for(int y=0;y<N;y++)
        {
            double sum=0.0;
            for(int u=0;u<N;u++)
            {
                for(int v=0;v<N;v++)
                {
                    sum+=dctBlock[u][v]*dctCoeff(x,y,u,v,N);
                }
            }
            int val = (int)round(sum);
            if (val < 0) val = 0;
            if (val > maxVal) val = maxVal;
            block[x][y] = val;
        }
    }
}

void quantize(double dctBlock[8][8], int quality) {
    int qFactor = (quality > 0) ? quality : 1;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            dctBlock[i][j] = round(dctBlock[i][j] / qFactor);
        }
    }
}

void dequantize(double dctBlock[8][8], int quality) {
    int qFactor = (quality > 0) ? quality : 1;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            dctBlock[i][j] = dctBlock[i][j] * qFactor;
        }
    }
}

void compressChannel(int** channel, int width, int height, int maxVal, int quality) {
    for (int i = 0; i < height; i += 8) {
        for (int j = 0; j < width; j += 8) {
            int block[8][8] = {0};
            double dctBlock[8][8];
            
            for (int x = 0; x < 8 && i + x < height; x++) {
                for (int y = 0; y < 8 && j + y < width; y++) {
                    block[x][y] = channel[i + x][j + y];
                }
            }
            
            dct2D(block, dctBlock);
            quantize(dctBlock, quality);
            dequantize(dctBlock, quality);
            idct2D(dctBlock, block, maxVal);
            
            for (int x = 0; x < 8 && i + x < height; x++) {
                for (int y = 0; y < 8 && j + y < width; y++) {
                    channel[i + x][j + y] = block[x][y];
                }
            }
        }
    }
}

void freeColorImage(ColorImage* img) {
    if (img == NULL) return;
    for (int i = 0; i < img->height; i++) {
        free(img->R[i]);
        free(img->G[i]);
        free(img->B[i]);
        if(img->hasAlpha && img->A) {
            free(img->A[i]);
        }
    }
    free(img->R);
    free(img->G);
    free(img->B);
    if(img->hasAlpha && img->A) {
        free(img->A);
    }
    free(img);
}



void processTextFile(const char* fileName, int operation)
{
    if(operation == 1) {
        // COMPRESSION
        compressTextHuffman(fileName);
    } else {
        // DECOMPRESSION
        const char* ext = strrchr(fileName, '.');
        if(!ext || strcmp(ext, ".huff") != 0) {
            printf("\n❌ ERROR: This file is not compressed!\n");
            printf("Please compress the file first (choose operation 1).\n\n");
            return;
        }
        
        printf("\n=== TEXT FILE DECOMPRESSION ===\n");
        
        FILE* fp = fopen(fileName, "rb");
        if(!fp) {
            printf("Error: Cannot open compressed file!\n");
            return;
        }
        
        int readCodeCount;
        fread(&readCodeCount, sizeof(int), 1, fp);
        
        HuffmanCode readCodes[256];
        
        for(int i = 0; i < readCodeCount; i++)
        {
            fread(&readCodes[i].character, sizeof(unsigned char), 1, fp);
            int codeLength;
            fread(&codeLength, sizeof(int), 1, fp);
            fread(readCodes[i].code, sizeof(char), codeLength, fp);
            readCodes[i].code[codeLength] = '\0';
        }
        
        long originalSize;
        fread(&originalSize, sizeof(long), 1, fp);
        
        int readCompressedLength;
        fread(&readCompressedLength, sizeof(int), 1, fp);
        char* readCompressed = (char*)malloc(readCompressedLength + 1);
        fread(readCompressed, sizeof(char), readCompressedLength, fp);
        readCompressed[readCompressedLength] = '\0';
        fclose(fp);
        
        unsigned char* decompressedText = (unsigned char*)malloc(originalSize + 1);
        int decompressedIndex = 0;
        char currentCode[256] = "";
        int currentCodeIndex = 0;
        
        for(int i = 0; i < readCompressedLength && decompressedIndex < originalSize; i++)
        {
            currentCode[currentCodeIndex++] = readCompressed[i];
            currentCode[currentCodeIndex] = '\0';
            
            for(int j = 0; j < readCodeCount; j++)
            {
                if(strcmp(currentCode, readCodes[j].code) == 0)
                {
                    decompressedText[decompressedIndex++] = readCodes[j].character;
                    currentCodeIndex = 0;
                    currentCode[0] = '\0';
                    break;
                }
            }
        }
        
        char decompressedFile[MAX_FILENAME];
        strcpy(decompressedFile, fileName);
        char* dotExt = strrchr(decompressedFile, '.');
        if(dotExt) *dotExt = '\0';
        strcat(decompressedFile, "_decompressed.txt");
        
        fp = fopen(decompressedFile, "wb");
        fwrite(decompressedText, sizeof(unsigned char), decompressedIndex, fp);
        fclose(fp);
        
        printf("✓ File decompressed successfully: %s\n", decompressedFile);
        printf("  Decompressed %d bytes (expected %ld)\n\n", decompressedIndex, originalSize);
        
        free(readCompressed);
        free(decompressedText);
    }
}

void processBMPFile(const char* fileName, int operation)
{
    if(operation == 2) {
        // Check if file is compressed
        const char* ext = strrchr(fileName, '.');
        if(!ext || strcmp(ext, ".bps") != 0) {
            printf("\n❌ ERROR: This file is not compressed!\n");
            printf("Please compress the file first (choose option 1).\n\n");
            return;
        }
    }
    
    if(operation == 1) {
        // COMPRESSION
        int choice;
        printf("\n╔═══════════════════════════════════════════╗\n");
        printf("║     BMP COMPRESSION OPTIONS               ║\n");
        printf("╚═══════════════════════════════════════════╝\n");
        printf("1. MSB Bit Plane Compression (Lossy - Higher compression)\n");
        printf("2. Full Bit Plane Compression (Lossless - Preserves quality)\n");
        printf("3. Both methods\n");
        printf("Enter your choice (1-3): ");
        
        if(scanf("%d", &choice) != 1)
        {
            printf("Invalid input.\n");
            while(getchar() != '\n');
            return;
        }
        
        switch(choice)
        {
            case 1:
                compressBMPBitPlaneMSB(fileName);
                break;
            case 2:
                compressBMPBitPlaneFull(fileName);
                break;
            case 3:
                compressBMPBitPlaneMSB(fileName);
                printf("\n════════════════════════════════════════════\n\n");
                compressBMPBitPlaneFull(fileName);
                break;
            default:
                printf("Invalid choice.\n");
                break;
        }
    } else {
        // DECOMPRESSION
        FILE *fp = fopen(fileName, "rb");
        if(!fp) {
            printf("Error: Cannot open compressed file!\n");
            return;
        }
        
        int width, height;
        fread(&width, sizeof(int), 1, fp);
        fread(&height, sizeof(int), 1, fp);
        fclose(fp);
        
        // Determine if it's MSB or Full based on file size
        FILE *checkFp = fopen(fileName, "rb");
        fseek(checkFp, 0, SEEK_END);
        long fileSize = ftell(checkFp);
        fclose(checkFp);
        
        int imageSize = width * height;
        int msbSize = sizeof(int)*2 + (imageSize + 7) / 8;
        int fullSize = sizeof(int)*2 + ((imageSize + 7) / 8) * 8;
        
        printf("\n=== BMP DECOMPRESSION ===\n");
        
        if(abs(fileSize - msbSize) < abs(fileSize - fullSize)) {
            printf("Detected: MSB Bit Plane compressed file\n\n");
            
            // Read compressed data
            fp = fopen(fileName, "rb");
            int readWidth, readHeight;
            fread(&readWidth, sizeof(int), 1, fp);
            fread(&readHeight, sizeof(int), 1, fp);
            
            int readImageSize = readWidth * readHeight;
            int readPackedSize = (readImageSize + 7) / 8;
            unsigned char *readPacked = (unsigned char*)malloc(readPackedSize);
            fread(readPacked, 1, readPackedSize, fp);
            fclose(fp);
            
            // Unpack
            unsigned char *unpackedMSB = (unsigned char*)malloc(readImageSize);
            for(int i = 0; i < readImageSize; i++)
            {
                int byteIndex = i / 8;
                int bitPos = 7 - (i % 8);
                unpackedMSB[i] = (readPacked[byteIndex] >> bitPos) & 1;
            }
            
            unsigned char *reconstructed = (unsigned char*)calloc(readImageSize, 1);
            for(int i = 0; i < readImageSize; i++)
            {
                reconstructed[i] = unpackedMSB[i] ? 128 : 0;
            }
            
            // Create output filename
            char decompressedFile[MAX_FILENAME];
            strcpy(decompressedFile, fileName);
            char* ext = strrchr(decompressedFile, '.');
            if(ext) *ext = '\0';
            strcat(decompressedFile, "_decompressed.bmp");
            
            // Create BMP headers
            BMPFileHeader fileHeader;
            BMPInfoHeader infoHeader;
            
            int rowPadding = (4 - (readWidth % 4)) % 4;
            infoHeader.biSizeImage = (readWidth + rowPadding) * readHeight;
            fileHeader.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + 1024;
            fileHeader.bfSize = fileHeader.bfOffBits + infoHeader.biSizeImage;
            fileHeader.bfType = 0x4D42;
            fileHeader.bfReserved1 = 0;
            fileHeader.bfReserved2 = 0;
            
            infoHeader.biSize = sizeof(BMPInfoHeader);
            infoHeader.biWidth = readWidth;
            infoHeader.biHeight = readHeight;
            infoHeader.biPlanes = 1;
            infoHeader.biBitCount = 8;
            infoHeader.biCompression = 0;
            infoHeader.biXPelsPerMeter = 2835;
            infoHeader.biYPelsPerMeter = 2835;
            infoHeader.biClrUsed = 0;
            infoHeader.biClrImportant = 0;
            
            // Create grayscale palette
            unsigned char palette[1024];
            for(int i = 0; i < 256; i++) {
                palette[i*4] = i;
                palette[i*4+1] = i;
                palette[i*4+2] = i;
                palette[i*4+3] = 0;
            }
            
            fp = fopen(decompressedFile, "wb");
            fwrite(&fileHeader, sizeof(BMPFileHeader), 1, fp);
            fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, fp);
            fwrite(palette, 1, 1024, fp);
            
            unsigned char pad[3] = {0, 0, 0};
            for(int i = readHeight - 1; i >= 0; i--)
            {
                fwrite(reconstructed + i * readWidth, 1, readWidth, fp);
                fwrite(pad, 1, rowPadding, fp);
            }
            fclose(fp);
            
            printf("✓ File decompressed successfully: %s\n", decompressedFile);
            
            free(readPacked);
            free(unpackedMSB);
            free(reconstructed);
        } else {
            printf("Detected: Full Bit Plane compressed file\n\n");
            
            // Full decompression code
            fp = fopen(fileName, "rb");
            int readWidth, readHeight;
            fread(&readWidth, sizeof(int), 1, fp);
            fread(&readHeight, sizeof(int), 1, fp);
            
            int readImageSize = readWidth * readHeight;
            int readPackedSizePerPlane = (readImageSize + 7) / 8;
            int readTotalPackedSize = readPackedSizePerPlane * 8;
            
            unsigned char *readPacked = (unsigned char*)malloc(readTotalPackedSize);
            fread(readPacked, 1, readTotalPackedSize, fp);
            fclose(fp);
            
            unsigned char *reconstructed = (unsigned char*)calloc(readImageSize, 1);
            
            for(int plane = 0; plane < 8; plane++)
            {
                for(int i = 0; i < readImageSize; i++)
                {
                    int planeOffset = plane * readPackedSizePerPlane;
                    int byteInPlane = i / 8;
                    int byteIndex = planeOffset + byteInPlane;
                    int bitPos = 7 - (i % 8);
                    int value = (readPacked[byteIndex] >> bitPos) & 1;
                    
                    if(value == 1)
                    {
                        reconstructed[i] |= (1 << plane);
                    }
                }
            }
            
            char decompressedFile[MAX_FILENAME];
            strcpy(decompressedFile, fileName);
            char* ext = strrchr(decompressedFile, '.');
            if(ext) *ext = '\0';
            strcat(decompressedFile, "_decompressed.bmp");
            
            BMPFileHeader fileHeader;
            BMPInfoHeader infoHeader;
            
            int rowPadding = (4 - (readWidth % 4)) % 4;
            infoHeader.biSizeImage = (readWidth + rowPadding) * readHeight;
            fileHeader.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + 1024;
            fileHeader.bfSize = fileHeader.bfOffBits + infoHeader.biSizeImage;
            fileHeader.bfType = 0x4D42;
            fileHeader.bfReserved1 = 0;
            fileHeader.bfReserved2 = 0;
            
            infoHeader.biSize = sizeof(BMPInfoHeader);
            infoHeader.biWidth = readWidth;
            infoHeader.biHeight = readHeight;
            infoHeader.biPlanes = 1;
            infoHeader.biBitCount = 8;
            infoHeader.biCompression = 0;
            infoHeader.biXPelsPerMeter = 2835;
            infoHeader.biYPelsPerMeter = 2835;
            infoHeader.biClrUsed = 0;
            infoHeader.biClrImportant = 0;
            
            unsigned char palette[1024];
            for(int i = 0; i < 256; i++) {
                palette[i*4] = i;
                palette[i*4+1] = i;
                palette[i*4+2] = i;
                palette[i*4+3] = 0;
            }
            
            fp = fopen(decompressedFile, "wb");
            fwrite(&fileHeader, sizeof(BMPFileHeader), 1, fp);
            fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, fp);
            fwrite(palette, 1, 1024, fp);
            
            unsigned char pad[3] = {0, 0, 0};
            for(int i = readHeight - 1; i >= 0; i--)
            {
                fwrite(reconstructed + i * readWidth, 1, readWidth, fp);
                fwrite(pad, 1, rowPadding, fp);
            }
            fclose(fp);
            
            printf("✓ File decompressed successfully: %s\n", decompressedFile);
            
            free(readPacked);
            free(reconstructed);
        }
    }
}
void processColorFile(const char* fileName, int operation)
{
    if(operation == 1) {
        // COMPRESSION
        processColorImage(fileName);
    } else {
        // DECOMPRESSION
        const char* ext = strrchr(fileName, '.');
        int isCompressed = 0;
        
        if(ext) {
            char testName[MAX_FILENAME];
            strcpy(testName, fileName);
            if(strstr(testName, "_compressed")) {
                isCompressed = 1;
            }
        }
        
        if(!isCompressed) {
            printf("\n❌ ERROR: This file is not compressed!\n");
            printf("Please compress the file first (choose operation 1).\n\n");
            return;
        }
        
        printf("\n=== COLOR IMAGE DECOMPRESSION ===\n");
        printf("Note: Decompressed file is the same as compressed file for DCT.\n");
        printf("DCT is a lossy algorithm - quality was already reduced during compression.\n\n");
        
        ColorImage* img = loadColorImage(fileName);
        if(img == NULL) {
            return;
        }
        
        char decompressedFile[MAX_FILENAME];
        strcpy(decompressedFile, fileName);
        char* ext2 = strrchr(decompressedFile, '.');
        if(ext2) *ext2 = '\0';
        
        // Remove "_compressed" from filename
        char* compStr = strstr(decompressedFile, "_compressed");
        if(compStr) {
            *compStr = '\0';
        }
        strcat(decompressedFile, "_decompressed");
        
        const char* inputExt = strrchr(fileName, '.');
        if(inputExt) {
            strcat(decompressedFile, inputExt);
        } else {
            strcat(decompressedFile, ".bmp");
        }
        
        saveColorImage(decompressedFile, img);
        printf("✓ File decompressed successfully: %s\n", decompressedFile);
        printf("  Format: %s\n\n", img->hasAlpha ? "32-bit RGBA" : "24-bit RGB");
        
        freeColorImage(img);
    }
}

int main()
{
    int algorithm, operation;
    char filename[MAX_FILENAME];

    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║  MULTI-FORMAT COMPRESSION & DECOMPRESSION TOOL         ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n\n");
    
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║             SELECT COMPRESSION ALGORITHM               ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("1. Huffman Coding (for Text files)\n");
    printf("2. Bit Plane Slicing (for Grayscale BMP images)\n");
    printf("3. DCT - Discrete Cosine Transform (for Color images)\n");
    printf("\nEnter your choice (1-3): ");
    
    if(scanf("%d", &algorithm) != 1 || algorithm < 1 || algorithm > 3) {
        printf("Invalid choice!\n");
        return 1;
    }
    
    while(getchar() != '\n');
    
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║                  ENTER FILENAME                        ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    switch(algorithm) {
        case 1:
            printf("Supported formats: .txt (compression), .huff (decompression)\n");
            break;
        case 2:
            printf("Supported formats: .bmp 8-bit (compression), .bps (decompression)\n");
            break;
        case 3:
            printf("Supported formats: .bmp (24/32-bit), .ppm, .pam\n");
            break;
    }
    
    printf("Enter filename: ");
    if(fgets(filename, sizeof(filename), stdin) != NULL) {
        filename[strcspn(filename, "\n")] = 0;
    } else {
        printf("Error reading filename!\n");
        return 1;
    }
    
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║              SELECT OPERATION                          ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("1. Compress\n");
    printf("2. Decompress\n");
    printf("\nEnter your choice (1-2): ");
    
    if(scanf("%d", &operation) != 1 || operation < 1 || operation > 2) {
        printf("Invalid choice!\n");
        return 1;
    }
    
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║                  PROCESSING FILE                       ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("Algorithm: ");
    switch(algorithm) {
        case 1: printf("Huffman Coding\n"); break;
        case 2: printf("Bit Plane Slicing\n"); break;
        case 3: printf("DCT (Discrete Cosine Transform)\n"); break;
    }
    printf("Operation: %s\n", operation == 1 ? "Compression" : "Decompression");
    printf("File: %s\n", filename);
    
    // Process based on algorithm choice
    switch(algorithm) {
        case 1:
            processTextFile(filename, operation);
            break;
        case 2:
            processBMPFile(filename, operation);
            break;
        case 3:
            processColorFile(filename, operation);
            break;
    }
    
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║              PROCESSING COMPLETE                       ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    return 0;
}