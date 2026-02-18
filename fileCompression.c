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



char *getFileExtension(const char *filename)
{
    char *ext = strrchr(filename, '.');
    if (ext == NULL)
    {
        return NULL;
    }

    static char lower_ext[10];
    int i = 0;
    ext++;

    while (*ext && i < 9)
    {
        lower_ext[i++] = (char)tolower((unsigned char)*ext++);
    }

    lower_ext[i] = '\0';

    return lower_ext;
}


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
        
    if(strcmp(ext, ".bmp") == 0)
        {
            return BMP_FILE;
        }
    if(strcmp(ext, ".ppm") == 0||
       strcmp(ext,".jpg")==0||
       strcmp(ext,".jpeg")==0)
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
    for(int plane = 0; plane < 8; plane++) free(bitPlanes[plane]);
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

}ColorImage;

ColorImage *loadColorImage(const char *filename){
              FILE *fp = fopen(filename, "r");
              if(fp==NULL)
              {
                printf("Error opening file: %s\n", filename);
                return NULL;
              }

        ColorImage *img=(ColorImage*)malloc(sizeof(ColorImage));
        char magic[3];
        fscanf(fp,"%s",magic);

         char c =fgetc(fp);
         while(c=='#' || c=='\n' || c==' ')
              {
                  if(c=='#')
                     {
                        while(fgetc(fp) !='\n');
                      }
                  c=fgetc(fp);
              }
              ungetc(c,fp);
              fscanf(fp, "%d %d %d", &img->width, &img->height, &img->maxVal);

              img->R=(int**)malloc(img->height*sizeof(int*));
              img->G=(int**)malloc(img->height*sizeof(int*)); 
              img->B=(int**)malloc(img->height*sizeof(int*));

              for (int i=0;i<img->height;i++)
                 {
                   img->R[i]=(int*)malloc(img->width*sizeof(int));
                   img->G[i]=(int*)malloc(img->width*sizeof(int));
                   img->B[i]=(int*)malloc(img->width*sizeof(int));
    
                  for(int j=0;j<img->width;j++)
                    {
                       fscanf(fp,"%d %d %d",&img->R[i][j],&img->G[i][j],&img->B[i][j]);
                     }
                 }
    fclose(fp);
    return img;
}
void saveColorImage(const char * filename, ColorImage* img) {
                FILE *fp = fopen(filename, "w");
                if(fp==NULL)
                {
                    printf("Error opening file: %s\n", filename);
                    return;
                }
    
        fprintf(fp,"P3\n%d %d\n%d\n",img->width,img->height,img->maxVal);
    
        for(int i=0;i<img->height;i++)
        {
            for(int j=0;j<img->width;j++)
            {
                fprintf(fp,"%d %d %d ",img->R[i][j],img->G[i][j],img->B[i][j]);
            }
            fprintf(fp,"\n");
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
    }
    free(img->R);
    free(img->G);
    free(img->B);
    free(img);
}

void processColorImage(const char* inputFile) {
    printf("Detected: COLOR IMAGE\n");
    printf("Using: DCT (Discrete Cosine Transform)\n\n");
    
    ColorImage* img = loadColorImage(inputFile);
    if (img == NULL) return;
    
    int quality = 50;
    
    printf("Applying DCT compression...\n");
    compressChannel(img->R, img->width, img->height, img->maxVal, quality);
    compressChannel(img->G, img->width, img->height, img->maxVal, quality);
    compressChannel(img->B, img->width, img->height, img->maxVal, quality);
    
    char compressedFile[MAX_FILENAME];
    strcpy(compressedFile, inputFile);
    char* ext = strrchr(compressedFile, '.');
    if (ext) *ext = '\0';
    strcat(compressedFile, "_compressed.ppm");
    
    saveColorImage(compressedFile, img);
    printf("Color image compressed using DCT (quality: %d)\n", quality);
    printf("Saved to: %s\n", compressedFile);
    
    char decompressedFile[MAX_FILENAME];
    strcpy(decompressedFile, inputFile);
    ext = strrchr(decompressedFile, '.');
    if (ext) *ext = '\0';
    strcat(decompressedFile, "_decompressed.jpg");
    
    saveColorImage(decompressedFile, img);
    printf("Color image decompressed and saved to: %s\n", decompressedFile);
    
    freeColorImage(img);
}




void processBMPFile(const char* fileName)
{
    int choice;
    printf("\n===========BMP Compression Options========\n");
    printf("1. MSB Bit Plane Compression (Lossy - Higher compression)\n");
    printf("2. Full Bit Plane Compression (Lossless - Preserves quality)\n");
    printf("3. Both methods\n");
    printf("Enter your choice (1-3): ");
    scanf("%d", &choice);
    
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
            printf("\n");
            compressBMPBitPlaneFull(fileName);
            break;
        default:
            printf("Invalid choice.\n");
            break;
    }
}

void processFile(const char* fileName)
{
    printf("File compression and decompression functionality is under development.\n");

    printf("Input file: %s\n", fileName);

    FileType type=detectFileType(fileName);
    switch(type)
    {
        case TEXT_FILE:
            printf("Processing text file: %s\n", fileName);
            compressTextHuffman(fileName);
            printf("===========Processing Done==========");
            break;
        case BMP_FILE:
            processBMPFile(fileName);
            printf("===========Processing Done==========");
            break;  
         case FILE_COLOR:
            processColorImage(fileName);
            break;     

        default:
            printf("Unsupported file type for file: %s\n", fileName);
            break;
   }
  printf("===========Processing Done==========");
}

int main()
{
    char filename[MAX_FILENAME];

    printf(" ============== Multi-Format Compression & Decompression Tool ==============\n");
    printf("Menu:\n");
    printf("Supported File Formats:\n");
    printf("1. Text Files (.txt)\n");
    printf("  2. BMP Image Files (.bmp) - 8-bit Grayscale only\n");
    printf("     - MSB Bit Plane\n");
    printf("     - Full Bit Plane\n\n");
    printf("3. Color images (.jpg) - DCT\n");

    printf("Enter the filename: ");

    if(fgets(filename,sizeof(filename),stdin)!=NULL)
    {
        filename[strcspn(filename,"\n")]=0;
        if(strlen(filename) > 0)
        {
            processFile(filename);
        }
        else
        {
            printf("No filename provided.\n");
        }
    }
    return 0;
}