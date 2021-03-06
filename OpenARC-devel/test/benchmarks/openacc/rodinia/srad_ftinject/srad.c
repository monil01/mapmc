// srad.cpp : Defines the entry point for the console application.
//

#define OUTPUT


#define DEBUG
//#define _OPENMP
#define	ITERATION
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include "resilience.h"

#ifndef ENABLE_OPENACC
#define ENABLE_OPENACC 1
#endif

#ifndef R_MODE
#define R_MODE 0
#endif

#ifndef _ROWS_
#define _ROWS_ 1024
#endif
#ifndef _COLS_
#define _COLS_ 1024
#endif

#define _SIZE_I_ (_ROWS_ * _COLS_)

#ifndef RES_REGION0
#define RES_REGION0 1
#endif
#ifndef RES_REGION1
#define RES_REGION1 0
#endif
#ifndef TOTAL_NUM_FAULTS
#define TOTAL_NUM_FAULTS    1
#endif
#ifndef NUM_FAULTYBITS
#define NUM_FAULTYBITS  1
#endif
#ifndef NUM_REPEATS
#define NUM_REPEATS 1
#endif
#ifndef _FTVAR
#define _FTVAR 0
#endif
#ifndef _FTKIND
#define _FTKIND 5
#endif
#ifndef _FTTHREAD
#define _FTTHREAD 0
#endif


#ifdef _OPENARC_

#include "ftmacro.h"

#if _ROWS_ == 1024
#pragma openarc #define _ROWS_ 1024
#elif _ROWS_ == 2048
#pragma openarc #define _ROWS_ 2048
#elif _ROWS_ == 4096
#pragma openarc #define _ROWS_ 4096
#endif

#if _COLS_ == 1024
#pragma openarc #define _COLS_ 1024
#elif _COLS_ == 2048
#pragma openarc #define _COLS_ 2048
#elif _COLS_ == 4096
#pragma openarc #define _COLS_ 4096
#endif

#pragma openarc #define _SIZE_I_ (_ROWS_ * _COLS_)

#endif

int rows, cols, size_I, size_R, niter = 10;
int r1, r2, c1, c2;
float lambda;
int nthreads;

double gettime() {
  struct timeval t;
  gettimeofday(&t,0);
  return t.tv_sec+t.tv_usec*1e-6;
}

void random_matrix(float *I, int rows, int cols);

void usage(int argc, char **argv)
{
	fprintf(stderr, "Usage: %s <rows> <cols> <y1> <y2> <x1> <x2> <no. of threads><lamda> <no. of iter>\n", argv[0]);
	fprintf(stderr, "\t<rows>   - number of rows\n");
	fprintf(stderr, "\t<cols>    - number of cols\n");
	fprintf(stderr, "\t<y1> 	 - y1 value of the speckle\n");
	fprintf(stderr, "\t<y2>      - y2 value of the speckle\n");
	fprintf(stderr, "\t<x1>       - x1 value of the speckle\n");
	fprintf(stderr, "\t<x2>       - x2 value of the speckle\n");
	fprintf(stderr, "\t<no. of threads>  - no. of threads\n");
	fprintf(stderr, "\t<lamda>   - lambda (0,1)\n");
	fprintf(stderr, "\t<no. of iter>   - number of iterations\n");
	
	exit(1);
}

void mainComp(float J[_SIZE_I_], int iN[_ROWS_], int iS[_ROWS_],
int jE[_COLS_], int jW[_COLS_], float dN[_SIZE_I_], float dS[_SIZE_I_],
float dW[_SIZE_I_], float dE[_SIZE_I_], float c[_SIZE_I_])
{
	int iter, k;
  float q0sqr, sum, sum2, tmp, meanROI,varROI ;
	float Jc, G2, L, num, den, qsqr;
	float cN,cS,cW,cE;
	float D;
	int i, j;
#if (R_MODE == 2) || (R_MODE == 3) 
	long count0 = 0;
	long count1 = 0;
#endif
#if ENABLE_OPENACC == 1
#pragma acc data \
    copyin(J[0:_SIZE_I_]), \
    create( dN[0:_SIZE_I_], dS[0:_SIZE_I_], dW[0:_SIZE_I_], \
    dE[0:_SIZE_I_], c[0:_SIZE_I_])
#endif
#ifdef ITERATION
	for (iter=0; iter< niter; iter++){
#endif        
		sum=0; sum2=0;     
		for (i=r1; i<=r2; i++) {
            for (j=c1; j<=c2; j++) {
                tmp   = J[i * cols + j];
                sum  += tmp ;
                sum2 += tmp*tmp;
            }
        }
        meanROI = sum / size_R;
        varROI  = (sum2 / size_R) - meanROI*meanROI;
        q0sqr   = varROI / (meanROI*meanROI);
		
#if R_MODE == 0
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION0) ftdata(FTVAR0) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#elif R_MODE == 1
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION0) ftkind(FTKIND) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#elif R_MODE == 2
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION0) ftprofile(count0) ftdata(FTVAR0) num_faults(0) num_ftbits(0)
#elif R_MODE == 3
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION0) ftprofile(count0) ftkind(FTKIND) num_faults(0) num_ftbits(0)
#elif R_MODE == 4
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION0) ftpredict(FTCNT0) ftdata(FTVAR0) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#elif R_MODE == 5
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION0) ftpredict(FTCNT0) ftkind(FTKIND) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#endif
#if ENABLE_OPENACC == 1
		#pragma acc kernels loop gang worker independent
#endif
		for (i = 0 ; i < rows ; i++) {
#if ENABLE_OPENACC == 1
			#pragma acc loop gang worker independent
#endif
            for (j = 0; j < cols; j++) { 
		
				k = i * cols + j;
				Jc = J[k];
 
				// directional derivates
				if( i == 0 ) {
                dN[k] = J[j] - Jc;
                dS[k] = J[cols + j] - Jc;
				} else if( i == (rows-1) ) {
                dN[k] = J[(i-1) * cols + j] - Jc;
                dS[k] = J[k] - Jc;
				} else {
                dN[k] = J[(i-1) * cols + j] - Jc;
                dS[k] = J[(i+1) * cols + j] - Jc;
				}
				if( j == 0 ) {	
                dW[k] = J[i * cols] - Jc;
                dE[k] = J[i * cols + 1] - Jc;
				} else if( j == (cols-1) ) {
                dW[k] = J[i * cols + (j-1)] - Jc;
                dE[k] = J[k] - Jc;
				} else {
                dW[k] = J[i * cols + (j-1)] - Jc;
                dE[k] = J[i * cols + (j+1)] - Jc;
				}
			
                G2 = (dN[k]*dN[k] + dS[k]*dS[k] 
                    + dW[k]*dW[k] + dE[k]*dE[k]) / (Jc*Jc);

   		        L = (dN[k] + dS[k] + dW[k] + dE[k]) / Jc;

				num  = (0.5F*G2) - ((1.0F/16.0F)*(L*L)) ;
                den  = 1 + (.25F*L);
                qsqr = num/(den*den);
 
                // diffusion coefficent (equ 33)
                den = (qsqr-q0sqr) / (q0sqr * (1+q0sqr)) ;
                c[k] = 1.0F / (1.0F+den) ;
                
                // saturate diffusion coefficent
                if (c[k] < 0) {c[k] = 0;}
                else if (c[k] > 1) {c[k] = 1;}
   
		}
  
    }
#if R_MODE == 0
    #pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION1) ftdata(FTVAR1) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#elif R_MODE == 1
    #pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION1) ftkind(FTKIND) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#elif R_MODE == 2
    #pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION1) ftprofile(count1) ftdata(FTVAR1) num_faults(0) num_ftbits(0)
#elif R_MODE == 3
    #pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION1) ftprofile(count1) ftkind(FTKIND) num_faults(0) num_ftbits(0)
#elif R_MODE == 4
    #pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION1) ftpredict(FTCNT1) ftdata(FTVAR1) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#elif R_MODE == 5
    #pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION1) ftpredict(FTCNT1) ftkind(FTKIND) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#endif
#if ENABLE_OPENACC == 1
		#pragma acc kernels loop gang worker independent
#endif
		for (i = 0; i < rows; i++) {
#if ENABLE_OPENACC == 1
			#pragma acc loop gang worker independent
#endif
            for (j = 0; j < cols; j++) {        

                // current index
                k = i * cols + j;
                
                // diffusion coefficent
					cN = c[k];
					if( i == (rows-1) ) {
						cS = c[k];
					} else {
						cS = c[(i+1) * cols + j];
					}
					cW = c[k];
					if( j == (cols-1) ) {
						cE = c[k];
					} else {
						cE = c[i * cols + j+1];
					}

                // divergence (equ 58)
                D = cN * dN[k] + cS * dS[k] + cW * dW[k] + cE * dE[k];
                
                // image update (equ 61)
                J[k] = J[k] + 0.25F*lambda*D;
            }
	     }
#if ENABLE_OPENACC == 1
		 #pragma acc update host(J[0:_SIZE_I_])
#endif

#ifdef ITERATION
	}
#endif

#if (R_MODE == 2) || (R_MODE == 3) 
  printf("FT profile-count0 = %ld\n", count0);
  printf("FT profile-count1 = %ld\n", count1);
#endif
}

int main(int argc, char* argv[])
{   
  float *I, *J;
	int *iN,*iS,*jE,*jW;
	float *dN,*dS,*dW,*dE;
	float *c;
	int i, j, k;
	double start_time, end_time;
#ifdef DEBUG
	double start_time1, end_time1;
	double start_time2, end_time2;
#endif
#ifdef OUTPUT
  FILE *fp;
#endif

	if (argc == 10)
	{
		rows = atoi(argv[1]); //number of rows in the domain
		cols = atoi(argv[2]); //number of cols in the domain
		if ((rows%16!=0) || (cols%16!=0)){
			fprintf(stderr, "rows and cols must be multiples of 16\n");
			exit(1);
		}
		if( (rows != _ROWS_) || (cols != _COLS_) ) {
			fprintf(stderr, "Wrong values for _ROWS_ or _COLS_!\n");
			exit(1);
		}
		r1   = atoi(argv[3]); //y1 position of the speckle
		r2   = atoi(argv[4]); //y2 position of the speckle
		c1   = atoi(argv[5]); //x1 position of the speckle
		c2   = atoi(argv[6]); //x2 position of the speckle
		nthreads = atoi(argv[7]); // number of threads
		lambda = atof(argv[8]); //Lambda value
		niter = atoi(argv[9]); //number of iterations
	}
    else{
		usage(argc, argv);
    }

	start_time = gettime();

	size_I = cols * rows;
    size_R = (r2-r1+1)*(c2-c1+1);   

	I = (float *)malloc( size_I * sizeof(float) );
    J = (float *)malloc( size_I * sizeof(float) );
	c  = (float *)malloc(sizeof(float)* size_I) ;

    iN = (int *)malloc(sizeof(unsigned int*) * rows) ;
    iS = (int *)malloc(sizeof(unsigned int*) * rows) ;
    jW = (int *)malloc(sizeof(unsigned int*) * cols) ;
    jE = (int *)malloc(sizeof(unsigned int*) * cols) ;    


	dN = (float *)malloc(sizeof(float)* size_I) ;
    dS = (float *)malloc(sizeof(float)* size_I) ;
    dW = (float *)malloc(sizeof(float)* size_I) ;
    dE = (float *)malloc(sizeof(float)* size_I) ;    
    

    for (i=0; i< rows; i++) {
        iN[i] = i-1;
        iS[i] = i+1;
    }    
    for (j=0; j< cols; j++) {
        jW[j] = j-1;
        jE[j] = j+1;
    }
    iN[0]    = 0;
    iS[rows-1] = rows-1;
    jW[0]    = 0;
    jE[cols-1] = cols-1;
	
	printf("Randomizing the input matrix\n");

#ifdef DEBUG
	start_time2 = gettime();
#endif

    random_matrix(I, rows, cols);

    for (k = 0;  k < size_I; k++ ) {
     	J[k] = (float)exp(I[k]) ;
    }

#ifdef DEBUG
	end_time2 = gettime();
	printf("Input Randomization Time = %lf sec\n", end_time2 - start_time2);
#endif
   
	printf("Start the SRAD main loop\n");

#ifdef DEBUG
	start_time1 = gettime();
#endif

mainComp(J, iN, iS, jE, jW, dN, dS, dW, dE, c);

#ifdef DEBUG
	end_time1 = gettime();
	printf(" Accelerator Elapsed Time = %lf sec\n", end_time1 - start_time1);
#endif

#ifdef OUTPUT
    if( (fp = fopen("sradOutput.txt", "w")) == 0 ) {
      printf("Can not open %s\n", "sradOutput.txt");
      exit(1);
    }
    for( i = 0 ; i < rows ; i++){
    for ( j = 0 ; j < cols ; j++){

         fprintf(fp, "%.5f ", J[i * cols + j]);
   
    }
         fprintf(fp, "\n");
   }
    fclose(fp);
#endif

	printf("Computation Done\n");

	free(I);
	free(J);
	free(iN); free(iS); free(jW); free(jE);
    free(dN); free(dS); free(dW); free(dE);

	free(c);

	end_time = gettime();
	printf("Total Execution Time = %lf sec.\n", end_time - start_time);
	return 0;
}




void random_matrix(float *I, int rows, int cols){
	int i, j;

	srand(7);
	
	for( i = 0 ; i < rows ; i++){
		for ( j = 0 ; j < cols ; j++){
		 I[i * cols + j] = rand()/(float)RAND_MAX ;
		 #ifdef OUTPUT
         //printf("%g ", I[i * cols + j]); 
         #endif 
		}
		 #ifdef OUTPUT
         //printf("\n"); 
         #endif 
	}

}

