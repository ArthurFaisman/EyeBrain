#ifndef TGA_HELPER_H
#define TGA_HELPER_H

int ReadTGAFile( char * szFilename, int * width, int *height, unsigned char** pixel);
int WriteTGAFile( char * szFilename, int width, int height, unsigned char *pixel );

#endif
