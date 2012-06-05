#ifndef IMG_H
#define IMG_H

/****** Image support module ******/


/** Debugging assertions for interpolation **/
#define NDEBUG

#ifdef NDEBUG
#define ASS_IMG_XY(x,y,xs,ys)
#define ASS_IMG_P(p,xs,ys)
#else
#define ASS_IMG_XY(x,y,xs,ys)	assert(x>=0 && y>=0 && x<xs && y<ys)
#define ASS_IMG_P(p,xs,ys)	assert(p>=0 && p<(xs*ys))
#endif


/** Information about the images **/
typedef struct{
    int XSize,YSize;        /* size of image */
		int ZSize;		/* 1->Monochrome, 3->RGB */
		unsigned char *Data;	/* Image Data, row order */
				/* In RGB mode : RGB RGB RGB RGB ... */
				/* NULL -> No Image loaded */
        } imginfo;


/*** Routines ***/

//void DumpImgInfo(imginfo *II);

// init even without loading any image 
int InitImgInfo(imginfo *II,int xs,int ys,int zs);

// copy image I to Icopy
int CopyImg(imginfo *Icopy, imginfo *I);

// Create a new image 
int CreateImage(int XS,int YS,int ZS,imginfo *I);

void FreeImage(imginfo *I);

// return 0 if ok, -1 if error 
// return the data in PData, and the size in P[XYZ]Size 
// ZSize=1 -> monochrome, ZSize=3 or 4 -> RGB or ARGB 
// If RGB : rgbrgbrgbrgb... 
int LoadImg(char *Name,imginfo *I);

// Save in IRIS Format , If color, Data is RGB RGB RGB ... 
int SaveImage(char *Name,imginfo *I);
int SaveImageNoRLE(char *Name,imginfo *I);

// Bi-linear Interpolation 
// Return 1 if (x,y) are inside the image, 0 otherwise 
int ImgCheck(float x,float y,imginfo *I);

// Bi-linear interpolation 
// Use z=0 in monochrome case 
float InterpoleImg(float x,float y,int iz,imginfo *I);


// Bi-Cubic Interpolation 
// Return 1 if (x,y) are inside the image, 0 otherwise 
int ImgCheckCubic(float x,float y,imginfo *I);

// Bi-Cubic interpolation 
// Use z=0 in monochrome case 
float InterpoleImgCubic(float x,float y,int iz,imginfo *I);

#endif
