
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "curvatures.h"
extern "C" {
#include "nrutil.h"    //  Numerical Recipes
}
#define PMODE 0644
#define M_PI 3.141592653589793238462643

/*
The formulas below assume that the surface is floating above a plane, parametrized as the height r above the plane parametrized by theta and h
*/

//
//  The formulas are based on Do Carmo's book:  Differential Geometry of Curves and Surfaces

// the scaleValue parameter can be used to normalize the curvatures relative to some value
void setPrincipalCurvatures(float **unnormalized_r, float **kappa1, float **kappa2, int N, float stepSize, float scaleValue)
{
	// GIVEN r, N, stepSize, sets kappa1 and kappa2

		//  I use "1" and "2" to indicate the first and second variables, namely theta and h.
	//  Thus, 
	//  d1 means d / d omega  
	//  d2 means d / d h
	// 
	//  d1r   is the first  partial derivative dr/dtheta
	//  d11r  is the second partial derivative d^2r / d theta^2
	//  d2r   is the first  partial derivative dr/dh
	//  d22r  is the second partial derivative d^2r / d h^2
	//  d12r  is  d^2r / d h d theta
	//
	//  The notation efg and EFG are from Do Carmo's textbook on differential geometry. 
	//  That is where the formulas come from.

	int    u1, u2, i, j;
	float  e, f, g, E, F, G, Nx, Ny, Nz;
	float  tmp;
	float  **d1r, **d2r, **d11r, **d12r, **d22r,  **gaussK,  **meanH, **r;

	r = matrix(0,N-1,0,N-1);
	d1r =  matrix(0,N-1,0,N-1);
	d11r = matrix(0,N-1,0,N-1);
	d2r =  matrix(0,N-1,0,N-1);
	d22r = matrix(0,N-1,0,N-1);
	d12r = matrix(0,N-1,0,N-1);
	gaussK = matrix(0,N-1,0,N-1);
	meanH  = matrix(0,N-1,0,N-1);

	
	for (int i=0; i < N; i++){
		for (int j=0; j < N; j++){
			if (scaleValue == 0.0f){
				r[i][j] = unnormalized_r[i][j];
			} else {
				r[i][j] = unnormalized_r[i][j] / scaleValue;
			}
		}
	}



	// i denotes the row index (aka parameter 1)
	for (i =0; i < N; i++){
		// j denotes the column index (aka parameter 2)
		for (j=0; j < N; j++){
			if (i == 0){
				d1r[i][j] = r[i+1][j] - r[i][j];
				d11r[i][j] = r[i+2][j] - 2*r[i+1][j] + r[i][j];
			} else if (i == N-1){
				d1r[i][j] = r[i][j] - r[i-1][j];
				d11r[i][j] = r[i][j] - 2*r[i-1][j] + r[i-2][j];
			} else {
				d1r[i][j] = (r[i+1][j] - r[i-1][j]) / 2;
				d11r[i][j] = r[i+1][j] - 2*r[i][j] + r[i-1][j];
			}

			if (j == 0){
				d2r[i][j] = r[i][j+1] - r[i][j];
				d22r[i][j] = r[i][j+2] - 2*r[i][j+1] + r[i][j];
			} else if (j == N-1){
				d2r[i][j] = r[i][j] - r[i][j-1];
				d22r[i][j] = r[i][j] - 2*r[i][j-1] + r[i][j-2];
			} else {
				d2r[i][j] = (r[i][j+1] - r[i][j-1]) / 2;
				d22r[i][j] = r[i][j+1] - 2*r[i][j] + r[i][j-1];
			}

			d1r[i][j] /= stepSize;
			d2r[i][j] /= stepSize;
			d11r[i][j] /= stepSize*stepSize;
			d22r[i][j] /= stepSize*stepSize;
		}
	}

	// i denotes the row index (aka parameter 1)
	for (i =0; i < N; i++){
		// j denotes the column index (aka parameter 2)
		for (j=0; j < N; j++){
			if (j == 0){
				d12r[i][j] = d1r[i][j+1]-d1r[i][j];
			} else if (j == N-1){
				d12r[i][j] = d1r[i][j]-d1r[i][j-1];
			} else {
				d12r[i][j] = (d1r[i][j+1]-d1r[i][j-1])/2;
			}
			d12r[i][j] /= stepSize;
		}
	}



	for (u1 = 0; u1 < N ; u1++) {
		for (u2 = 0; u2 < N; u2++) {



			Nx =    1.0/d1r[u1][u2];
			Ny =    1.0/d2r[u1][u2];
			Nz =   1.0;
			tmp = sqrt(Nx * Nx + Ny * Ny + Nz * Nz);
			Nx =  Nx / tmp;
			Ny =  Ny / tmp;
			Nz =  Nz / tmp;

			//    first fundamental form  (from Do Carmo's book)

			E = r[u1][u2]   *   r[u1][u2] + d1r[u1][u2] * d1r[u1][u2] ;
			F = d1r[u1][u2] * d2r[u1][u2];
			G = d2r[u1][u2] * d2r[u1][u2] + 1;

			//    second fundamental form  (from Do Carmo's book)

			e   = //  <N, x_theta,theta>
				Nx * (0)
				+Ny * (0)
				+Nz * d11r[u1][u2];

			f  = //  <N, x_theta,h>
				Nx * (0)
				+Ny * (0)
				+Nz * d12r[u1][u2];

			g  = //  <N, x_h,h>
				Nx * (0)
				+Ny * (0)
				+Nz * d22r[u1][u2];

			gaussK[u1][u2] = (e*g - f*f) / (E*G - F*F);
			meanH[u1][u2]  = 0.5 * (e*G - 2*f*F + g*E) / (E*G - F*F);

			kappa1[u1][u2]=  meanH[u1][u2]+sqrt(meanH[u1][u2]*meanH[u1][u2]-gaussK[u1][u2]);
			kappa2[u1][u2]=  meanH[u1][u2]-sqrt(meanH[u1][u2]*meanH[u1][u2]-gaussK[u1][u2]);
		}
	}

	free_matrix(d1r, 0,N-1,0,N-1);
	free_matrix(d11r, 0,N-1,0,N-1);
	free_matrix(d2r, 0,N-1,0,N-1);
	free_matrix(d22r, 0,N-1,0,N-1);
	free_matrix(d12r, 0,N-1,0,N-1);
	free_matrix(gaussK, 0,N-1,0,N-1);
	free_matrix(meanH,  0,N-1,0,N-1);
}
