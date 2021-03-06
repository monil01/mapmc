#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include "resilience.h"

//#include "backprop.h"
#define ETA 0.3F       //eta value
#define MOMENTUM 0.3F  //momentum value
//I_SIZE should be the same as commandline input (layer_size in facetrain.c)
#ifndef I_SIZE
#define I_SIZE  65537
#endif

#ifndef ENABLE_OPENACC
#define ENABLE_OPENACC 1
#endif

#ifndef R_MODE
#define R_MODE 0
#endif

#ifndef RES_REGION0
#define RES_REGION0 1
#endif
#ifndef RES_REGION1
#define RES_REGION1 0
#endif
#ifndef RES_REGION2
#define RES_REGION2 0
#endif
#ifndef RES_REGION3
#define RES_REGION3 0
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

#if I_SIZE == 65537
    #pragma openarc #define I_SIZE 65537
#elif I_SIZE == 655361
    #pragma openarc #define I_SIZE 655361
#elif I_SIZE == 6553601
    #pragma openarc #define I_SIZE 6553601
#endif
#endif

#define H_SIZE  (16 + 1)
#define O_SIZE  (1 + 1) 
#ifdef _OPENARC_
#pragma openarc #define H_SIZE  (16 + 1)
#pragma openarc #define O_SIZE  (1 + 1) 

#include "ftmacro.h"

#endif


#define DEBUG0
#define DEBUG


////////////////////////////////////////////////////////////////////////////////

extern void bpnn_layerforward1(float l1[I_SIZE], float l2[H_SIZE], float conn[H_SIZE][I_SIZE], int n1, int n2);

extern void bpnn_layerforward2(float l1[H_SIZE], float l2[O_SIZE], float conn[O_SIZE][H_SIZE], int n1, int n2);

extern void bpnn_output_error(float *delta, float *target, float *output, int nj, float *err);

extern void bpnn_hidden_error(float *delta_h, int nh, float *delta_o, int no, float who[O_SIZE][H_SIZE], float *hidden, float *err);

extern void bpnn_adjust_weights1(float delta[O_SIZE], int ndelta, float ly[H_SIZE], int nly, float w[O_SIZE][H_SIZE], float oldw[O_SIZE][H_SIZE]);

extern void bpnn_adjust_weights2(float delta[H_SIZE], int ndelta, float ly[I_SIZE], int nly, float w[H_SIZE][I_SIZE], float oldw[H_SIZE][I_SIZE]);


extern void setup(int argc, char** argv);

extern float squash(float x);

double gettime() {
  struct timeval t;
  gettimeofday(&t,0);
  return t.tv_sec+t.tv_usec*1e-6;
}

int input_n;                  /* number of input units */
int hidden_n;                 /* number of hidden units */
int output_n;                 /* number of output units */

float *input_units;          /* the input units */
float *hidden_units;         /* the hidden units */
float *output_units;         /* the output units */

float *hidden_delta;         /* storage for hidden unit error */
float *output_delta;         /* storage for output unit error */

float *target;               /* storage for target vector */

float (*input_weights)[I_SIZE];       /* weights from input to hidden layer */
float (*hidden_weights)[H_SIZE];      /* weights from hidden to output layer */

                                /*** The next two are for momentum ***/
float (*input_prev_weights)[I_SIZE];  /* previous change on input to hidden wgt */
float (*hidden_prev_weights)[H_SIZE]; /* previous change on hidden to output wgt */

#if VERIFICATION == 1
//Verification related variables
float *hidden_units_CPU;         /* the hidden units */
float *output_units_CPU;         /* the output units */

float *hidden_delta_CPU;    
float *output_delta_CPU;    


float (*input_weights_CPU)[I_SIZE];       /* weights from input to hidden layer */
float (*hidden_weights_CPU)[H_SIZE];      /* weights from hidden to output layer */

                                  /*** The next two are for momentum ***/
float (*input_prev_weights_CPU)[I_SIZE];  /* previous change on input to hidden wgt */
float (*hidden_prev_weights_CPU)[H_SIZE]; /* previous change on hidden to output wgt */
#endif


////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////
int
main( int argc, char** argv) 
{
	double start_time, end_time;
	start_time = gettime();
	setup(argc, argv);
	end_time = gettime();
	printf ("Total time = %lf sec\n", end_time - start_time);
	return 0;
}


void bpnn_train_kernel(float *eo, float *eh)
{
  int in, hid, out;
  float out_err, hid_err;
  int j, k;
  float sum;
  float new_dw;
#ifdef DEBUG0
  double start_time0, end_time0;
#endif
#ifdef DEBUG
  double start_time, end_time;
#endif
#if VERIFICATION == 1
  //Verification related variables
	int i;
    float out_err_CPU, hid_err_CPU;
    double deltaL2Norm = 0;
    double nonAccL2Norm = 0;
    double L2Norm;
    double d;
#endif
#if (R_MODE == 2) || (R_MODE == 3) 
  long count0 = 0;
  long count1 = 0;
  long count2 = 0;
  long count3 = 0;
#endif
  
  in = input_n;
  hid = hidden_n;
  out = output_n;   
   
  printf("Performing CPU computation\n");
#ifdef DEBUG0
  start_time0 = gettime();
#endif
#ifdef DEBUG
  start_time = gettime();
#endif
  input_units[0] = 1.0F;
  hidden_units[0] = 1.0F;

#if VERIFICATION == 1
    for(i=0; i<(hid+1); i++) {
        for(j=0 ; j <(in+1); j++) {
            input_weights_CPU[i][j]= input_weights[i][j];
            input_prev_weights_CPU[i][j]= input_prev_weights[i][j];
        }   
    }   

    for(i=0; i<(out+1); i++) {
        for(j=0 ; j <(hid+1); j++) {
            hidden_weights_CPU[i][j]= hidden_weights[i][j];
            hidden_prev_weights_CPU[i][j]= hidden_prev_weights[i][j];
        }   
    }   
#endif

#if ENABLE_OPENACC == 1
#pragma acc data \
copy(input_weights[0:H_SIZE][0:I_SIZE]) \
copy(hidden_weights[0:O_SIZE][0:H_SIZE]) \
copyin(input_prev_weights[0:H_SIZE][0:I_SIZE]) \
copyin(input_units[0:I_SIZE]) \
copyin(hidden_prev_weights[0:O_SIZE][0:H_SIZE]) \
create(hidden_units[0:H_SIZE])
#endif
{
#ifdef DEBUG
  end_time = gettime();
  printf("CUDA Initialization time %lf sec\n", end_time - start_time);
  start_time = gettime();
#endif
  //bpnn_layerforward1(input_units, hidden_units,(float (*)[I_SIZE])input_weights, in, hid);
    /*** Set up thresholding unit ***/
  /*** For each unit in second layer ***/
  for (j = 1; j <= hid; j++) {

    /*** Compute weighted sum of its inputs ***/
    sum = 0.0F;
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
  	#pragma acc kernels loop gang worker independent reduction(+:sum)
#endif
    for (k = 0; k <= in; k++) { 
      sum += input_weights[j][k] * input_units[k]; 
    }   
    //hidden_units[j] = squash(sum);
    hidden_units[j] = (1.0F / (1.0F + expf(-sum)));
  }

#ifdef DEBUG
  end_time = gettime();
  printf("bpnn_layerforward1() execution time %lf sec\n", end_time - start_time);
  start_time = gettime();
#endif
  //bpnn_layerforward2(hidden_units, output_units, (float (*)[H_SIZE])hidden_weights, hid, out);
    /*** Set up thresholding unit ***/
  //hidden_units[0] = 1.0F;
#if ENABLE_OPENACC == 1
  #pragma acc update device(hidden_units[0:H_SIZE])
#endif
  /*** For each unit in second layer ***/
  for (j = 1; j <= out; j++) {

    /*** Compute weighted sum of its inputs ***/
    sum = 0.0F;
    //hidden_units[0] = 1.0F;
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
    #pragma acc kernels loop gang worker independent reduction(+:sum)
#endif
    for (k = 0; k <= hid; k++) {
      sum += hidden_weights[j][k] * hidden_units[k];
    }
    //output_units[j] = squash(sum);
    output_units[j] = (1.0F / (1.0F + expf(-sum)));
  }

#ifdef DEBUG
  end_time = gettime();
  printf("bpnn_layerforward2() execution time %lf sec\n", end_time - start_time);
#endif
  bpnn_output_error(output_delta, target, output_units, out, &out_err);
  bpnn_hidden_error(hidden_delta, hid, output_delta, out, hidden_weights, hidden_units, &hid_err);  
#ifdef DEBUG
  start_time = gettime();
#endif
  //bpnn_adjust_weights1(output_delta, out, hidden_units, hid, (float (*)[H_SIZE])hidden_weights, (float (*)[H_SIZE])hidden_prev_weights);
  //hidden_units[0] = 1.0F;
  	//hidden_units[0] = 1.0F;
#if R_MODE == 0
	#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION2) ftdata(FTVAR2) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#elif R_MODE == 1
	#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION2) ftkind(FTKIND) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#elif R_MODE == 2
	#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION2) ftprofile(count2) ftdata(FTVAR2) num_faults(0) num_ftbits(0)
#elif R_MODE == 3
	#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION2) ftprofile(count2) ftkind(FTKIND) num_faults(0) num_ftbits(0)
#elif R_MODE == 4
	#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION2) ftpredict(FTCNT2) ftdata(FTVAR2) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#elif R_MODE == 5
	#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION2) ftpredict(FTCNT2) ftkind(FTKIND) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#endif
#if ENABLE_OPENACC == 1
    #pragma acc kernels loop gang worker independent \
    copyin(output_delta[0:O_SIZE]) private(new_dw)
#endif
  for (k = 0; k <= hid; k++) {
  	for (j = 1; j <= out; j++) {
      new_dw = ((ETA * output_delta[j] * hidden_units[k]) + (MOMENTUM * hidden_prev_weights[j][k]));
      hidden_weights[j][k] += new_dw;
      hidden_prev_weights[j][k] = new_dw;
    }
  }

#ifdef DEBUG
  end_time = gettime();
  printf("bpnn_adjust_weights1() execution time %lf sec\n", end_time - start_time);
  start_time = gettime();
#endif
  //bpnn_adjust_weights2(hidden_delta, hid, input_units, in, (float (*)[I_SIZE])input_weights, (float (*)[I_SIZE])input_prev_weights);
  //input_units[0] = 1.0F;
  	//input_units[0] = 1.0F;
#if R_MODE == 0
	#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION3) ftdata(FTVAR3) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#elif R_MODE == 1
	#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION3) ftkind(FTKIND) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#elif R_MODE == 2
	#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION3) ftprofile(count3) ftdata(FTVAR3) num_faults(0) num_ftbits(0)
#elif R_MODE == 3
	#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION3) ftprofile(count3) ftkind(FTKIND) num_faults(0) num_ftbits(0)
#elif R_MODE == 4
	#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION3) ftpredict(FTCNT3) ftdata(FTVAR3) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#elif R_MODE == 5
	#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION3) ftpredict(FTCNT3) ftkind(FTKIND) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#endif
#if ENABLE_OPENACC == 1
  	#pragma acc kernels loop gang worker independent \
  	copyin(hidden_delta[0:H_SIZE]) private(new_dw)
#endif
  for (k = 0; k <= in; k++) {
  	for (j = 1; j <= hid; j++) {
      new_dw = ((ETA * hidden_delta[j] * input_units[k]) + (MOMENTUM * input_prev_weights[j][k]));
      input_weights[j][k] += new_dw;
      input_prev_weights[j][k] = new_dw;
    }
  }
}
#ifdef DEBUG
  end_time = gettime();
  printf("bpnn_adjust_weights2() execution time %lf sec\n", end_time - start_time);
#endif
#ifdef DEBUG0
  end_time0 = gettime();
  printf("Elapsed time %lf sec\n", end_time0 - start_time0);
#endif

#if (R_MODE == 2) || (R_MODE == 3) 
  printf("FT profile-count0 = %ld\n", count0);
  printf("FT profile-count1 = %ld\n", count1);
  printf("FT profile-count2 = %ld\n", count2);
  printf("FT profile-count3 = %ld\n", count3);
#endif

#if VERIFICATION == 1

#ifdef DEBUG
  start_time = gettime();
#endif
  //bpnn_layerforward1(input_units, hidden_units_CPU,(float (*)[I_SIZE])input_weights_CPU, in, hid);
    /*** Set up thresholding unit ***/
  /*** For each unit in second layer ***/
  for (j = 1; j <= hid; j++) {

    /*** Compute weighted sum of its inputs ***/
    sum = 0.0F;
    for (k = 0; k <= in; k++) {
      sum += input_weights_CPU[j][k] * input_units[k];
    }
    //hidden_units_CPU[j] = squash(sum);
    hidden_units_CPU[j] = (1.0F / (1.0F + expf(-sum)));
  }

#ifdef DEBUG
  end_time = gettime();
  printf("bpnn_layerforward1() execution time CPU %lf sec\n", end_time - start_time);
  start_time = gettime();
#endif
  //bpnn_layerforward2(hidden_units_CPU, output_units, (float (*)[H_SIZE])hidden_weights_CPU, hid, out);
    /*** Set up thresholding unit ***/
  //hidden_units_CPU[0] = 1.0F;
  /*** For each unit in second layer ***/
  for (j = 1; j <= out; j++) {

    /*** Compute weighted sum of its inputs ***/
    sum = 0.0F;
    //hidden_units_CPU[0] = 1.0F;
    for (k = 0; k <= hid; k++) {
      sum += hidden_weights_CPU[j][k] * hidden_units_CPU[k];
    }
    //output_units[j] = squash(sum);
    output_units_CPU[j] = (1.0F / (1.0F + expf(-sum)));
  }

#ifdef DEBUG
  end_time = gettime();
  printf("bpnn_layerforward2() execution time CPU %lf sec\n", end_time - start_time);
#endif
  bpnn_output_error(output_delta_CPU, target, output_units_CPU, out, &out_err_CPU);
  bpnn_hidden_error(hidden_delta_CPU, hid, output_delta_CPU, out, hidden_weights_CPU, hidden_units_CPU, &hid_err_CPU);
#ifdef DEBUG
  start_time = gettime();
#endif
  //bpnn_adjust_weights1(output_delta_CPU, out, hidden_units_CPU, hid, (float (*)[H_SIZE])hidden_weights_CPU, (float (*)[H_SIZE])hidden_prev_weights_CPU);
  //hidden_units_CPU[0] = 1.0F;
    //hidden_units_CPU[0] = 1.0F;
  for (k = 0; k <= hid; k++) {
    for (j = 1; j <= out; j++) {
      new_dw = ((ETA * output_delta_CPU[j] * hidden_units_CPU[k]) + (MOMENTUM * hidden_prev_weights_CPU[j][k]));
      hidden_weights_CPU[j][k] += new_dw;
      hidden_prev_weights_CPU[j][k] = new_dw;
    }
  }

#ifdef DEBUG
  end_time = gettime();
  printf("bpnn_adjust_weights1() execution time CPU %lf sec\n", end_time - start_time);
  start_time = gettime();
#endif
  //bpnn_adjust_weights2(hidden_delta_CPU, hid, input_units, in, (float (*)[I_SIZE])input_weights_CPU, (float (*)[I_SIZE])input_prev_weights_CPU);
  //input_units[0] = 1.0F;
    //input_units[0] = 1.0F;
  for (k = 0; k <= in; k++) {
    for (j = 1; j <= hid; j++) {
      new_dw = ((ETA * hidden_delta_CPU[j] * input_units[k]) + (MOMENTUM * input_prev_weights_CPU[j][k]));
      input_weights_CPU[j][k] += new_dw;
      input_prev_weights_CPU[j][k] = new_dw;
    }
  }
#ifdef DEBUG
  end_time = gettime();
  printf("bpnn_adjust_weights2() execution time CPU %lf sec\n", end_time - start_time);
#endif

    for (i=0; i<(hid+1); i ++ )
    {
        for (j=0; j<(in+1); j ++ )
        {
            d=(input_weights_CPU[i][j]-input_weights[i][j]);
            deltaL2Norm+=(d*d);
            nonAccL2Norm+=(input_weights_CPU[i][j]*input_weights_CPU[i][j]);
            //printf("GPU %f CPU %f\n", input_weights[i][j], input_weights_CPU[i][j]);
        }
    }  

  L2Norm = sqrt(deltaL2Norm / nonAccL2Norm);
   
    if (L2Norm < 1e-9)
    printf("Verification Successful\n");
  else
    printf("Verification Fail\n");

#endif

}
