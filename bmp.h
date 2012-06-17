
#ifndef BMP_H
#define BMP_H


BYTE* ConvertRGBToBMPBuffer ( BYTE* Buffer, int width, int height, long* newsize );

BYTE* ConvertBMPToRGBBuffer ( BYTE* Buffer, int width, int height);

bool LoadBMPIntoDC ( HDC hDC, LPCTSTR bmpfile );

bool SaveBMP ( BYTE* Buffer, int width, int height, long paddedsize, LPCTSTR bmpfile );

BYTE* LoadBMP ( int* width, int* height, long* size, LPCTSTR bmpfile );

#endif