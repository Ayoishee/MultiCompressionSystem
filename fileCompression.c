#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MAX_FILENAME 256
#define MAX_TEXT_SIZE 1000000
#define MAX_TREE_NODES 512

typedef enum{
    TEXT_FILE,
    BMP_FILE,
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
    const char *dot=strrchr(fileName,'.');

    if(!dot || dot==fileName) 
    {
        return UNKNOWN_FILE;
    }
    if(strcmp(dot,".txt")==0)
    {
        return TEXT_FILE;
    }
    if(strcmp(dot,".bmp")==0)
    {
        return BMP_FILE;
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
           fprintf(stderr, "Memory allocation failed\n");
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

void compressTextHuffman(const char * inputFile)
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
        fprintf(stderr, "Failed to build Huffman tree\n");
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

    double reduction = (1.0 - ((double)compressedBytes / (double)fileSize)) * 100.0;
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
        char* readCompressed=(char*)malloc(readCompressedLength);
        fread(readCompressed,sizeof(char),readCompressedLength,fp);
        readCompressed[readCodeCount]='\0'; 
        fclose(fp);

        unsigned char* decompressedText=(unsigned char*)malloc(originalSize);
        int decompressedIndex=0;
        char currentCode[256]="";
        int currentCodeIndex=0;
        for(int i=0;i<readCompressedLength;i++)
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
    free(text);
    free(compressed);
    free(decompressedText);
    

}

void compressBMPBitPlane(const char *inputFile)
{
    printf("Detected: BMP ImageFile\n");
    printf("Bit Plane Slicing is used for compression.\n");
    
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
    int height = abs(infoHeader.biHeight);
    int isBottomUp = (infoHeader.biHeight > 0);

    int imageSize = width * height;

    printf("Image info:\n");
    printf(" Dimensions: %dx%d pixels\n", width, height);
    printf(" Bit depth:  8-bit grayscale\n\n");
    
    unsigned char palette[1024];
    fread(palette,1,1024,fp);

    int remainder = width % 4;
    int padding = 4 - remainder;
    padding = padding % 4;

    unsigned char *imageData=(unsigned char*)malloc(imageSize);
      for (int i = 0; i < height; i++)
        {
          int row = isBottomUp ? (height - 1 - i) : i;
          fread(imageData + row * width, 1, width, fp);
          fseek(fp, padding, SEEK_CUR);
        }

    fclose(fp);

    unsigned char *bitPlanes[8];
    for(int plane = 0; plane < 8; plane++)
    {
        bitPlanes[plane] = (unsigned char*)calloc(imageSize, 1);
    }

    for(int i = 0; i < imageSize; i++)
    {
        for(int plane = 0; plane < 8; plane++)
        {
            bitPlanes[plane][i] = (imageData[i] >> plane) & 1;
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

    for(int plane=0;plane<8;plane++)
    {
        int packedSize=(imageSize+7)/8;
        unsigned char *packed=(unsigned char*)calloc(packedSize,1);

        for(int i=0;i<imageSize;i++)
        {
            if(bitPlanes[plane][i])
            {
                int byteIndex = i / 8;
                int bitPosition = 7 - (i % 8);
                unsigned char mask = 1 << bitPosition;
                packed[byteIndex] |= mask;
            }
        }
        fwrite(packed,1,packedSize,fp);
        free(packed);
    }
  fclose(fp);
   long originalSize = fileHeader.bfSize;
   long widthSize = sizeof(int);
   long heightSize = sizeof(int);

   int bitsPerPlane = imageSize;
   int bytesPerPlane = (bitsPerPlane + 7) / 8;
   int numberOfPlanes = 8;
   int planesSize = bytesPerPlane * numberOfPlanes;

   long compressedSize = widthSize + heightSize + planesSize;

    
    printf("File compressed successfully: %s\n", compressedFile);
    printf("Original Size: %ld bytes\n", originalSize);
    printf("Compressed Size: %ld bytes\n", compressedSize);
    printf("Reduction: %.2f%%\n", (1.0 - ((double)compressedSize / originalSize)) * 100.0);

    fp = fopen(compressedFile, "rb");
    int readWidth, readHeight;
    fread(&readWidth, sizeof(int), 1, fp);
    fread(&readHeight, sizeof(int), 1, fp);
    
    int readImageSize = readWidth * readHeight;
    unsigned char *reconstructed = (unsigned char*)calloc(readImageSize, 1);
    
    for(int plane = 0; plane < 8; plane++)
    {
        int packedSize = (readImageSize + 7) / 8;
        unsigned char *packed = (unsigned char*)malloc(packedSize);
        fread(packed, 1, packedSize, fp);
        
        for(int i = 0; i < readImageSize; i++)
        {
            int byteIndex = i / 8;
            int bitPosition = 7 - (i % 8);

            if (packed[byteIndex] & (1 << bitPosition))
               {
                 reconstructed[i] |= (1 << plane);
               }

        }
        
        free(packed);
    }
    fclose(fp);
   
    char decompressedFile[MAX_FILENAME];
    strcpy(decompressedFile, inputFile);
    ext = strrchr(decompressedFile, '.');
    if(ext) *ext = '\0';
    strcat(decompressedFile, "_decompressed.bmp");

    infoHeader.biSizeImage = (width + padding) * height;
    fileHeader.bfOffBits = sizeof(BMPFileHeader)
                      + sizeof(BMPInfoHeader)
                      + 1024;

    fileHeader.bfSize = fileHeader.bfOffBits + infoHeader.biSizeImage;  

    fp = fopen(decompressedFile, "wb");

    infoHeader.biHeight = isBottomUp ? height : -height;

    fwrite(&fileHeader, sizeof(BMPFileHeader), 1, fp);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, fp);
    fwrite(palette, 1, 1024, fp);
    
    unsigned char pad[3] = {0, 0, 0};

    for (int i = 0; i < height; i++)
{
    int row = isBottomUp ? (height - 1 - i) : i;
    fwrite(reconstructed + row * width, 1, width, fp);
    fwrite(pad, 1, padding, fp);
}


    fclose(fp);
    printf("File decompressed successfully: %s\n", decompressedFile);

    free(imageData);
    free(reconstructed);
    for(int plane = 0; plane < 8; plane++)
    {
        free(bitPlanes[plane]);
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
            break;
        case BMP_FILE:
            printf("Processing BMP file: %s\n", fileName);
            compressBMPBitPlane(fileName);
            break;    

        default:
            printf("Unsupported file type for file: %s\n", fileName);
            break;
   }
printf("File processing completed for: %s\n", fileName);
}

int main()
{
    char filename[MAX_FILENAME];
    printf("----------Multi-Formate Compression and decompression Tool----------\n");
    printf("Supported Formates:\n");
    printf("1. Text File Compression\n");
    printf("2. BMP Image file Compression(Only Grayscale)\n");
    printf("Enter the filename: ");

    if(fgets(filename,sizeof(filename),stdin)!=NULL)
    {
        filename[strcspn(filename,"\n")]=0;
        processFile(filename);
        
    }
    return 0;
}