#ifndef TGA_H
#define TGA_H

/******************************************************************************
* Author: Fahim Mannan
* TGA file reader
*/
//http://organicbit.com/closecombat/formats/tga.html
//http://www.ludorg.net/amnesia/TGA_File_Format_Spec.html
//http://www.fileformat.info/format/tga/
//http://tfcduke.developpez.com/tutoriel/format/tga/fichiers/tga_specs.pdf
#define TGA_COLORMAP_TYPE_NONE          0
#define TGA_COLORMAP_TYPE_PALLETE       1

#define TGA_IMG_TYPE_NONE       0
#define TGA_IMG_TYPE_INDEXED    1
#define TGA_IMG_TYPE_RGB        2
#define TGA_IMG_TYPE_GREY       3
#define TGA_IMG_TYPE_RLE        4

typedef struct tTGAHeader {
    unsigned char IDFieldSize;      //Size of ID field following the 18 byte header
    unsigned char ColormapType;     //type of color map 0=none, 1=pallete
    unsigned char ImageType;        //0=none, 1=indexed, 2=rgb, 3=grey, +8=rle packed

    unsigned short         ColormapStart;    //first colormap entry in pallete
    unsigned short         ColormapLength;   //number of bits per pallete
    unsigned char          ColormapBits;     //no. of bits per pallete entry 15,16,24,32
    
    unsigned short         XOffset;          //x origin
    unsigned short         YOffset;          //y origin
    unsigned short         Width;            //image width
    unsigned short         Height;           //image height
    unsigned char          BitCount;         //bits per pixel 8,16,24,32
    unsigned char          Descriptor;       //v:vertical flip h:horizontal flip
                                             //a:alpha format: 00vhaaaa
} TGAHEADER, *PTGAHEADER;

typedef struct tTGAImage {
    TGAHEADER  tgaHeader;
    unsigned char pixel[1];
} TGAIMG, *PTGAIMG, TGAIMAGE, *PTGAIMAGE;

#endif
