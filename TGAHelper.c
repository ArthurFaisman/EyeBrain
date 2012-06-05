#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <memory.h>

#define TGAHDRSIZE 18    //size of TGA Header
int ReadTGAFile(  char * szFilename, int * width, int *height, unsigned char** pixel) {
    unsigned char pBuffer[TGAHDRSIZE]; 
   unsigned char temp;
   int i;
    FILE *fp = NULL;

    if(szFilename == NULL)
        return 0;
    if((fp = fopen(szFilename, "rb")) == NULL)
    {
        printf("Unable to open %s\n", szFilename);
        return 0;
    }
    
    fread(pBuffer, TGAHDRSIZE, 1, fp);


    printf("ID Field Size : %d\n", pBuffer[0]);
    printf("Colormap Type: %d\n", pBuffer[1]);
    printf("Image Type : %d\n", pBuffer[2]);
    printf("Colormap Start: %d\n", pBuffer[3] + pBuffer[4] << 8);
    printf("Colormap Length: %d\n", pBuffer[5] + pBuffer[6] << 8);
    printf("Colormap Bits: %d\n", pBuffer[7]);
    printf("X offset   : %d\n", pBuffer[8] + (pBuffer[9] << 8));
    printf("Y offset   : %d\n", pBuffer[10] + (pBuffer[11] << 8));
    printf("Width      : %d\n", pBuffer[12] + (pBuffer[13] << 8));
    printf("Height     : %d\n", pBuffer[14] + (pBuffer[15] << 8));
    printf("Bitcount   : %d\n", pBuffer[16]);
    printf("Descriptor : %x\n", pBuffer[17]);

    if(pBuffer[2] != 2 || pBuffer[16] != 24) {//consider only 24bpp
        printf("Unsupported format\n");
        return 0;
    }
    *width = pBuffer[12] + (pBuffer[13] << 8);
    *height = pBuffer[14] + (pBuffer[15] << 8);
    
    if(*pixel)
        free(*pixel);
    *pixel = (unsigned char*) malloc((*width) * (*height) * 3);
    
    
    fread(*pixel, (*width) * (*height) * 3, 1, fp);

    //BGR -> RGB
    
    for(i = 0; i < (*width) * (*height) * 3; i += 3)
    {
        temp = *(*pixel + i);
        *(*pixel + i) = *(*pixel + i + 2);
        *(*pixel + i + 2) = temp;
    }
    fclose(fp);
    
    return 1;
}

int WriteTGAFile(char * szFilename, int width, int height, unsigned char *pixel ){
    return 0;
}

