/*--------------------------------------------------------------------

  NAS Parallel Benchmarks 2.3 OpenMP C versions - CG

  This benchmark is an OpenMP C version of the NPB CG code.

  The OpenMP C versions are developed by RWCP and derived from the serial
  Fortran versions in "NPB 2.3-serial" developed by NAS.

  Permission to use, copy, distribute and modify this software for any
  purpose with or without fee is hereby granted.
  This software is provided "as is" without express or implied warranty.

  Send comments on the OpenMP C versions to pdp-openmp@rwcp.or.jp

  Information on OpenMP activities at RWCP is available at:

http://pdplab.trc.rwcp.or.jp/pdperf/Omni/

Information on NAS Parallel Benchmarks 2.3 is available at:

http://www.nas.nasa.gov/NAS/NPB/

--------------------------------------------------------------------*/
/*--------------------------------------------------------------------

Authors: M. Yarrow
C. Kuszmaul

OpenMP C version: S. Satoh

--------------------------------------------------------------------*/

/*
   c---------------------------------------------------------------------
   c  Note: please observe that in the routine conj_grad three 
   c  implementations of the sparse matrix-vector multiply have
   c  been supplied.  The default matrix-vector multiply is not
   c  loop unrolled.  The alternate implementations are unrolled
   c  to a depth of 2 and unrolled to a depth of 8.  Please
   c  experiment with these to find the fastest for your particular
   c  architecture.  If reporting timing results, any of these three may
   c  be used without penalty.
   c---------------------------------------------------------------------
 */

#include "npb-C.h"
#include "npbparams.h"

#define DEBUG_PRINT

#define	NZ	NA*(NONZER+1)*(NONZER+1)+NA*(NONZER+2)

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
#ifndef RES_REGION4
#define RES_REGION4 0
#endif
#ifndef RES_REGION5
#define RES_REGION5 0
#endif
#ifndef RES_REGION6
#define RES_REGION6 0
#endif
#ifndef RES_REGION7
#define RES_REGION7 0
#endif
#ifndef RES_REGION8
#define RES_REGION8 0
#endif
#ifndef RES_REGION9
#define RES_REGION9 0
#endif
#ifndef RES_REGION10
#define RES_REGION10 0
#endif
#ifndef RES_REGION11
#define RES_REGION11 0
#endif
#ifndef RES_REGION12
#define RES_REGION12 0
#endif
#ifndef RES_REGION13
#define RES_REGION13 0
#endif
#ifndef RES_REGION14
#define RES_REGION14 0
#endif
#ifndef RES_REGION15
#define RES_REGION15 0
#endif
#ifndef RES_REGION16
#define RES_REGION16 0
#endif
#ifndef RES_REGION17
#define RES_REGION17 0
#endif
#ifndef RES_REGION18
#define RES_REGION18 0
#endif
#ifndef RES_REGION19
#define RES_REGION19 0
#endif
#ifndef RES_REGION20
#define RES_REGION20 0
#endif
#ifndef RES_REGION21
#define RES_REGION21 0
#endif
#ifndef RES_REGION22
#define RES_REGION22 0
#endif
#ifndef RES_REGION23
#define RES_REGION23 0
#endif
#ifndef RES_REGION24
#define RES_REGION24 0
#endif
#ifndef RES_REGION25
#define RES_REGION25 0
#endif
#ifndef RES_REGION26
#define RES_REGION26 0
#endif
#ifndef RES_REGION27
#define RES_REGION27 0
#endif
#ifndef RES_REGION28
#define RES_REGION28 0
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
#ifndef _FTVAR0
#define _FTVAR0 0
#endif
#ifndef _FTVAR1
#define _FTVAR1 0
#endif
#ifndef _FTVAR2
#define _FTVAR2 0
#endif
#ifndef _FTVAR3
#define _FTVAR3 0
#endif
#ifndef _FTVAR4
#define _FTVAR4 0
#endif
#ifndef _FTVAR5
#define _FTVAR5 0
#endif
#ifndef _FTVAR6
#define _FTVAR6 0
#endif
#ifndef _FTVAR7
#define _FTVAR7 0
#endif
#ifndef _FTVAR8
#define _FTVAR8 0
#endif
#ifndef _FTVAR9
#define _FTVAR9 0
#endif
#ifndef _FTVAR10
#define _FTVAR10 0
#endif
#ifndef _FTVAR11
#define _FTVAR11 0
#endif
#ifndef _FTVAR12
#define _FTVAR12 0
#endif
#ifndef _FTVAR13
#define _FTVAR13 0
#endif
#ifndef _FTVAR14
#define _FTVAR14 0
#endif
#ifndef _FTVAR15
#define _FTVAR15 0
#endif
#ifndef _FTVAR16
#define _FTVAR16 0
#endif
#ifndef _FTVAR17
#define _FTVAR17 0
#endif
#ifndef _FTVAR18
#define _FTVAR18 0
#endif
#ifndef _FTVAR19
#define _FTVAR19 0
#endif
#ifndef _FTVAR20
#define _FTVAR20 0
#endif
#ifndef _FTVAR21
#define _FTVAR21 0
#endif
#ifndef _FTVAR22
#define _FTVAR22 0
#endif
#ifndef _FTVAR23
#define _FTVAR23 0
#endif
#ifndef _FTVAR24
#define _FTVAR24 0
#endif
#ifndef _FTVAR25
#define _FTVAR25 0
#endif
#ifndef _FTVAR26
#define _FTVAR26 0
#endif
#ifndef _FTVAR27
#define _FTVAR27 0
#endif
#ifndef _FTVAR28
#define _FTVAR28 0
#endif
#ifndef _FTTHREAD
#define _FTTHREAD 0
#endif


#ifdef _OPENARC_

#include "ftmacro.h"

#pragma openarc #define	NZ	\NA*(\NONZER+1)*(\NONZER+1)+\NA*(\NONZER+2)
#endif

/* global variables */

/* common /partit_size/ */
static int naa;
static int nzz;
static int firstrow;
static int lastrow;
static int firstcol;
static int lastcol;

/* common /main_int_mem/ */
static int colidx[NZ+1];	/* colidx[1:NZ] */
static int rowstr[NA+1+1];	/* rowstr[1:NA+1] */
static int iv[2*NA+1+1];	/* iv[1:2*NA+1] */
static int arow[NZ+1];		/* arow[1:NZ] */
static int acol[NZ+1];		/* acol[1:NZ] */

/* common /main_flt_mem/ */
static float v[NA+1+1];	/* v[1:NA+1] */
static float aelt[NZ+1];	/* aelt[1:NZ] */
static float a[NZ+1];		/* a[1:NZ] */
static float x[NA+2+1];	/* x[1:NA+2] */
static float z[NA+2+1];	/* z[1:NA+2] */
static float p[NA+2+1];	/* p[1:NA+2] */
static float q[NA+2+1];	/* q[1:NA+2] */
static float r[NA+2+1];	/* r[1:NA+2] */
static float w[NA+2+1];	/* w[1:NA+2] */

/* common /urando/ */
static float amult;
static float tran;

// Static variables used in conj_grad(). 
static float d, sum, rho, rho0, alpha, beta;

/* function declarations */
static void conj_grad (int colidx[NZ+1], int rowstr[NA+1+1], float x[NA+2+1], float z[NA+2+1],
				float a[NZ+1], float p[NA+2+1], float q[NA+2+1], float r[NA+2+1],
				float w[NA+2+1], float *rnorm);
static void makea(int n, int nz, float a[NZ+1], int colidx[NZ+1], int rowstr[NA+1+1],
				int nonzer, int firstrow, int lastrow, int firstcol,
				int lastcol, float rcond, int arow[NZ+1], int acol[NZ+1],
				float aelt[NZ+1], float v[NA+1+1], int iv[2*NA+1+1], float shift );
static void sparse(float a[NZ+1], int colidx[NZ+1], int rowstr[NA+1+1], int n,
				int arow[NZ+1], int acol[NZ+1], float aelt[NZ+1],
				int firstrow, int lastrow,
				float x[NA+1+1], boolean mark[NA+1], int nzloc[NA+1], int nnza);
static void sprnvc(int n, int nz, float v[], int iv[], int nzloc[],
				int mark[]);
static int icnvrt(float x, int ipwr2);
static void vecset(int n, float v[], int iv[], int *nzv, int i, float val);

/*--------------------------------------------------------------------
  program cg
  --------------------------------------------------------------------*/

int main(int argc, char **argv) {

		int	i_main, j_main, k_main, it;
		int nthreads = 1;
		float zeta;
		float rnorm;
		float norm_temp11;
		float norm_temp12;
		float t, mflops;
		char classT = 'U';
		boolean verified;
		float zeta_verify_value, epsilon;

		////////////////////////////////////
		// Used for inlining conj_grad(). //
		////////////////////////////////////
		int i, j, k;
		int cgit, cgitmax = 25;

#ifdef DEBUG_PRINT
		FILE *fp;
		if( (fp = fopen("cg.out", "w")) == NULL ) {
			exit(1);
		}
#endif

		firstrow = 1;
		lastrow  = NA;
		firstcol = 1;
		lastcol  = NA;

		if (NA == 1400 && NONZER == 7 && NITER == 15 && SHIFT == 10.0) {
				classT = 'S';
				//	zeta_verify_value = 8.5971775078648;
				zeta_verify_value = 8.379274368286; //serial version value with Single Precision
		} else if (NA == 7000 && NONZER == 8 && NITER == 15 && SHIFT == 12.0) {
				classT = 'W';
				//	zeta_verify_value = 10.362595087124;
				zeta_verify_value = 10.11725139618; //serial version value with Single Precision
		} else if (NA == 14000 && NONZER == 11 && NITER == 15 && SHIFT == 20.0) {
				classT = 'A';
				//	zeta_verify_value = 17.130235054029;
				zeta_verify_value = 18.62915039062; //serial version value with Single Precision
		} else if (NA == 75000 && NONZER == 13 && NITER == 75 && SHIFT == 60.0) {
				classT = 'B';
				//	zeta_verify_value = 22.712745482631;
				zeta_verify_value = 62.42129135132; //serial version value with Single Precision
		} else if (NA == 150000 && NONZER == 15 && NITER == 75 && SHIFT == 110.0) {
				classT = 'C';
				//	zeta_verify_value = 28.973605592845;
				zeta_verify_value = 115.1209869385; //serial version value with Single Precision
		} else {
				classT = 'U';
		}

		printf("\n\n NAS Parallel Benchmarks 2.3 OpenMP C version"
						" - CG Benchmark\n");
		printf(" Size: %10d\n", NA);
		printf(" Iterations: %5d\n", NITER);

		naa = NA;
		nzz = NZ;

		timer_clear(2);
		timer_clear(3);
		timer_clear(4);
		timer_start(2);
		/*--------------------------------------------------------------------
		  c  Initialize random number generator
		  c-------------------------------------------------------------------*/
		// Initial numbers are changed for single precision
		//    tran    = 314159265.0;
		//    amult   = 1220703125.0;
		tran   = 28183.0f;
		amult    = 390625.0f;
		zeta    = randlc( &tran, amult );

		/*--------------------------------------------------------------------
		  c  
		  c-------------------------------------------------------------------*/
		timer_start(4);
		makea(naa, nzz, a, colidx, rowstr, NONZER,
						firstrow, lastrow, firstcol, lastcol, 
						RCOND, arow, acol, aelt, v, iv, SHIFT);
		timer_stop(4);

		timer_start(3);
		/*---------------------------------------------------------------------
		  c  Note: as a result of the above call to makea:
		  c        values of j used in indexing rowstr go from 1 --> lastrow-firstrow+1
		  c        values of colidx which are col indexes go from firstcol --> lastcol
		  c        So:
		  c        Shift the col index vals from actual (firstcol --> lastcol ) 
		  c        to local, i.e., (1 --> lastcol-firstcol+1)
		  c---------------------------------------------------------------------*/
#pragma acc data \
create(x[0:NA+3]) \
create(z[0:NA+3]) \
create(p[0:NA+3]) \
create(q[0:NA+3]) \
create(r[0:NA+3]) \
create(w[0:NA+3]) \
copyin(a[0:NZ+1]) \
copyin(colidx[0:NZ+1]) \
copyin(rowstr[0:NA+2]) 
		{	
				timer_stop(3);
				// R/O Shared scalar: lastrow, firstrow, firstcol
				// R/O Shared arrays: rowstr[NA+1+1]
				// R/W Shared arrays: colidx[NZ+1]
				// R/W Private scalar: j_main, k_main
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION0) ftdata(FTVAR0) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
				for (j_main = 1; j_main <= lastrow - firstrow + 1; j_main++) {
						for (k_main = rowstr[j_main]; k_main < rowstr[j_main+1]; k_main++) {
								colidx[k_main] = colidx[k_main] - firstcol + 1;
						}
				}

				/*--------------------------------------------------------------------
				  c  set starting vector to (1, 1, .... 1)
				  c-------------------------------------------------------------------*/
				// R/W Shared arrays: x[NA+2+1]
				// R/W Private scalar: i_main 
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION1) ftdata(FTVAR1) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
				for (i_main = 1; i_main <= NA+1; i_main++) {
						x[i_main] = 1.0f;
				}
				// R/W Shared scalar: zeta
				zeta  = 0.0f;

				/*-------------------------------------------------------------------
				  c---->
				  c  Do one iteration untimed to init all code and data page tables
				  c---->                    (then reinit, start timing, to niter its)
				  c-------------------------------------------------------------------*/

				for (it = 1; it <= 1; it++) {

						/*--------------------------------------------------------------------
						  c  The call to the conjugate gradient routine:
						  c-------------------------------------------------------------------*/
						//conj_grad (colidx, rowstr, x, z, a, p, q, r, w, &rnorm);
						cgitmax = 25;

						// R/W Shared scalars: rho (function-static)
						rho = 0.0f;

						/*--------------------------------------------------------------------
						  c  Initialize the CG algorithm:
						  c-------------------------------------------------------------------*/
						// R/W Shared arrays: x[NA+2+1], r[NA+2+1]
						// R/W Shared arrays: q[NA+2+1], z[NA+2+1], r[NA+2+1], p[NA+2+1], w[NA+2+1]
						// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION2) ftdata(FTVAR2) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker 
						for (j = 1; j <= NA+1; j++) {
								q[j] = 0.0f;
								z[j] = 0.0f;
								r[j] = x[j];
								p[j] = r[j];
								w[j] = 0.0f;
						}

						/*--------------------------------------------------------------------
						  c  rho = r.r
						  c  Now, obtain the norm of r: First, sum squares of r elements locally...
						  c-------------------------------------------------------------------*/
						// R/O Shared scalars: lastcol, firstcol
						// R/O Shared arrays: x[NA+2+1] 
						// R/W Shared scalars: rho (function-static)
						// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION3) ftdata(FTVAR3) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
						for (j = 1; j <= lastcol-firstcol+1; j++) {
								rho = rho + x[j]*x[j];
						}

						/*--------------------------------------------------------------------
						  c---->
						  c  The conj grad iteration loop
						  c---->
						  c-------------------------------------------------------------------*/
						for (cgit = 1; cgit <= cgitmax; cgit++) {
								// R/W Shared scalars: d, rho, rho0 (function-static)
								{	
										rho0 = rho;
										d = 0.0f;
										rho = 0.0f;
								} /* end single */

								/*--------------------------------------------------------------------
								  c  q = A.p
								  c  The partition submatrix-vector multiply: use workspace w
								  c---------------------------------------------------------------------
								  C
								  C  NOTE: this version of the multiply is actually (slightly: maybe %5) 
								  C        faster on the sp2 on 16 nodes than is the unrolled-by-2 version 
								  C        below.   On the Cray t3d, the reverse is true, i.e., the 
								  C        unrolled-by-two version is some 10% faster.  
								  C        The unrolled-by-8 version below is significantly faster
								  C        on the Cray t3d - overall speed of code is 1.5 times faster.
								 */

								/* rolled version */      
								// R/O Shared scalars: lastrow, firstrow
								// R/O Shared arrays: rowstr[NA+1+1], a[NZ+1], p[NA+2+1], colidx[NZ+1], 
								// R/W Shared arrays: w[NA+2+1]
								// R/W Private scalars: j, k, sum
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION4) ftdata(FTVAR4) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker independent private(sum)
								for (j = 1; j <= lastrow-firstrow+1; j++) {
										sum = 0.0f;
										for (k = rowstr[j]; k < rowstr[j+1]; k++) {
												sum = sum + a[k]*p[colidx[k]];
										}
										w[j] = sum;
								}

								// R/O Shared scalars: lastcol, firstcol
								// R/O Shared arrays: w[NA+2+1]
								// R/W Shared arrays: q[NA+2+1]
								// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION5) ftdata(FTVAR5) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
								for (j = 1; j <= lastcol-firstcol+1; j++) {
										q[j] = w[j];
								}

								/*--------------------------------------------------------------------
								  c  Clear w for reuse...
								  c-------------------------------------------------------------------*/
								// R/O Shared scalars: lastcol, firstcol
								// R/W Shared arrays: w[NA+2+1]
								// R/W Private scalars: j
								/*--------------------------------------------------------------------
								  c  Obtain p.q
								  c-------------------------------------------------------------------*/
								// R/O Shared scalars: lastcol, firstcol
								// R/O Shared arrays: p[NA+2+1], q[NA+2+1]
								// R/W Shared scalars: d (function-static)
								// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION6) ftdata(FTVAR6) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
								for (j = 1; j <= lastcol-firstcol+1; j++) {
										w[j] = 0.0f;
										d = d + p[j]*q[j];
								}

								/*--------------------------------------------------------------------
								  c  Obtain alpha = rho / (p.q)
								  c-------------------------------------------------------------------*/
								// R/O Shared scalars: rho0, d (function-static)
								// R/W Shared scalars: alpha (function-static)
								alpha = rho0 / d;

								/*--------------------------------------------------------------------
								  c  Save a temporary of rho
								  c-------------------------------------------------------------------*/
								/*	rho0 = rho;*/

								/*---------------------------------------------------------------------
								  c  Obtain z = z + alpha*p
								  c  and    r = r - alpha*q
								  c---------------------------------------------------------------------*/
								// R/O Shared scalars: lastcol, firstcol
								// R/O Shared scalars: alpha (function-static)
								// R/O Shared arrays: p[NA+2+1], q[NA+2+1]
								// R/W Shared arrays: z[NA+2+1], r[NA+2+1]
								// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION7) ftdata(FTVAR7) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker 
								for (j = 1; j <= lastcol-firstcol+1; j++) {
										z[j] = z[j] + alpha*p[j];
										r[j] = r[j] - alpha*q[j];
								}

								/*---------------------------------------------------------------------
								  c  rho = r.r
								  c  Now, obtain the norm of r: First, sum squares of r elements locally...
								  c---------------------------------------------------------------------*/
								// R/O Shared scalars: lastcol, firstcol
								// R/O Shared arrays: r[NA+2+1]
								// R/W Shared scalars: rho (function-static)
								// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION8) ftdata(FTVAR8) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
								for (j = 1; j <= lastcol-firstcol+1; j++) {
										rho = rho + r[j]*r[j];
								}

								/*--------------------------------------------------------------------
								  c  Obtain beta:
								  c-------------------------------------------------------------------*/
								// R/O Shared scalars: rho0, rho (function-static)
								// R/W Shared scalars: beta (function-static)
								beta = rho / rho0;

								/*--------------------------------------------------------------------
								  c  p = r + beta*p
								  c-------------------------------------------------------------------*/
								// R/O Shared scalars: lastcol, firstcol
								// R/O Shared scalars: beta (function-static)
								// R/O Shared arrays: r[NA+2+1]
								// R/W Shared arrays: p[NA+2+1]
								// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION9) ftdata(FTVAR9) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
								for (j = 1; j <= lastcol-firstcol+1; j++) {
										p[j] = r[j] + beta*p[j];
								}
						} /* end of do cgit=1,cgitmax */

						/*---------------------------------------------------------------------
						  c  Compute residual norm explicitly:  ||r|| = ||x - A.z||
						  c  First, form A.z
						  c  The partition submatrix-vector multiply
						  c---------------------------------------------------------------------*/
						// R/W Shared scalars: sum (function-static)
						sum = 0.0f;

						// R/O Shared scalars: lastcol, firstcol
						// R/O Shared arrays: rowstr[NA+1+1], a[NZ+1], colidx[NZ+1], z[NA+2+1]
						// R/W Shared arrays: w[NA+2+1]
						// R/W Private scalars: j,d,k
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION10) ftdata(FTVAR10) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker independent private(d)
						for (j = 1; j <= lastrow-firstrow+1; j++) {
								d = 0.0f;
								for (k = rowstr[j]; k <= rowstr[j+1]-1; k++) {
										d = d + a[k]*z[colidx[k]];
								}
								w[j] = d;
						}

						// R/O Shared scalars: lastcol, firstcol
						// R/O Shared arrays: w[NA+2+1]
						// R/W Shared arrays: r[NA+2+1]
						// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION11) ftdata(FTVAR11) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
						for (j = 1; j <= lastcol-firstcol+1; j++) {
								r[j] = w[j];
						}

						/*--------------------------------------------------------------------
						  c  At this point, r contains A.z
						  c-------------------------------------------------------------------*/
						// R/O Shared scalars: lastcol, firstcol
						// R/O Shared arrays: r[NA+2+1], x[NA+2+1]
						// R/W Shared scalars: d, sum (function-static)
						// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION12) ftdata(FTVAR12) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker independent private(d)
						for (j = 1; j <= lastcol-firstcol+1; j++) {
								d = x[j] - r[j];
								sum = sum + d*d;
						}

						// R/O Shared scalars: sum (function-static)
						// R/W Shared scalars: rnorm
						{
								//(*rnorm) = sqrtf(sum);
								rnorm = sqrtf(sum);
						} /* end single */

						/*--------------------------------------------------------------------
						  c  zeta = shift + 1/(x.z)
						  c  So, first: (x.z)
						  c  Also, find norm of z
						  c  So, first: (z.z)
						  c-------------------------------------------------------------------*/
						// R/W Shared scalars: norm_temp11, norm_temp12
						{	
								norm_temp11 = 0.0f;
								norm_temp12 = 0.0f;
						} /* end single */

						// R/O Shared scalars: lastcol, firstcol
						// R/O Shared arrays: x[NA+2+1], z[NA+2+1]
						// R/W Shared scalars: norm_temp11, norm_temp12
						// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION13) ftdata(FTVAR13) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
						for (j_main = 1; j_main <= lastcol-firstcol+1; j_main++) {
								norm_temp11 = norm_temp11 + x[j_main]*z[j_main];
								norm_temp12 = norm_temp12 + z[j_main]*z[j_main];
						}
						// R/w Shared scalars: norm_temp12
						norm_temp12 = 1.0f / sqrtf( norm_temp12 );

						/*--------------------------------------------------------------------
						  c  Normalize z to obtain x
						  c-------------------------------------------------------------------*/
						// R/O Shared scalars: lastcol, firstcol, norm_temp12
						// R/O Shared arrays: z[NA+2+1]
						// R/W Shared arrays: x[NA+2+1]
						// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION14) ftdata(FTVAR14) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
						for (j_main = 1; j_main <= lastcol-firstcol+1; j_main++) {
								x[j_main] = norm_temp12*z[j_main];
						}

				} /* end of do one iteration untimed */

				/*--------------------------------------------------------------------
				  c  set starting vector to (1, 1, .... 1)
				  c-------------------------------------------------------------------*/
				// R/W Shared arrays: x[NA+2+1]
				// R/W Private scalars: i_main
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION15) ftdata(FTVAR15) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
				for (i_main = 1; i_main <= NA+1; i_main++) {
						x[i_main] = 1.0f;
				}
				// R/W Shared scalars: zeta
				zeta  = 0.0f;

//		} /* end parallel */

		timer_clear( 1 );
		timer_start( 1 );

		/*--------------------------------------------------------------------
		  c---->
		  c  Main Iteration for inverse power method
		  c---->
		  c-------------------------------------------------------------------*/

//#pragma omp parallel private(it,i_main,j_main,k_main)
//		{
				for (it = 1; it <= NITER; it++) {

						/*--------------------------------------------------------------------
						  c  The call to the conjugate gradient routine:
						  c-------------------------------------------------------------------*/
						//conj_grad(colidx, rowstr, x, z, a, p, q, r, w, &rnorm);
						cgitmax = 25;

						// R/W Shared scalars: rho (function-static)
						rho = 0.0f;

						/*--------------------------------------------------------------------
						  c  Initialize the CG algorithm:
						  c-------------------------------------------------------------------*/
						// R/W Shared arrays: x[NA+2+1], r[NA+2+1]
						// R/W Shared arrays: q[NA+2+1], z[NA+2+1], r[NA+2+1], p[NA+2+1], w[NA+2+1]
						// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION16) ftdata(FTVAR16) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
						for (j = 1; j <= NA+1; j++) {
								q[j] = 0.0f;
								z[j] = 0.0f;
								r[j] = x[j];
								p[j] = r[j];
								w[j] = 0.0f;
						}

						/*--------------------------------------------------------------------
						  c  rho = r.r
						  c  Now, obtain the norm of r: First, sum squares of r elements locally...
						  c-------------------------------------------------------------------*/
						// R/O Shared scalars: lastcol, firstcol
						// R/O Shared arrays: x[NA+2+1] 
						// R/W Shared scalars: rho (function-static)
						// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION17) ftdata(FTVAR17) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
						for (j = 1; j <= lastcol-firstcol+1; j++) {
								rho = rho + x[j]*x[j];
						}

						/*--------------------------------------------------------------------
						  c---->
						  c  The conj grad iteration loop
						  c---->
						  c-------------------------------------------------------------------*/
						for (cgit = 1; cgit <= cgitmax; cgit++) {
								// R/W Shared scalars: d, rho, rho0 (function-static)
								{	
										rho0 = rho;
										d = 0.0f;
										rho = 0.0f;
								} /* end single */

								/*--------------------------------------------------------------------
								  c  q = A.p
								  c  The partition submatrix-vector multiply: use workspace w
								  c---------------------------------------------------------------------
								  C
								  C  NOTE: this version of the multiply is actually (slightly: maybe %5) 
								  C        faster on the sp2 on 16 nodes than is the unrolled-by-2 version 
								  C        below.   On the Cray t3d, the reverse is true, i.e., the 
								  C        unrolled-by-two version is some 10% faster.  
								  C        The unrolled-by-8 version below is significantly faster
								  C        on the Cray t3d - overall speed of code is 1.5 times faster.
								 */

								/* rolled version */      
								// R/O Shared scalars: lastrow, firstrow
								// R/O Shared arrays: rowstr[NA+1+1], a[NZ+1], p[NA+2+1], colidx[NZ+1], 
								// R/W Shared arrays: w[NA+2+1]
								// R/W Private scalars: j, k, sum
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION18) ftdata(FTVAR18) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker independent private(sum)
								for (j = 1; j <= lastrow-firstrow+1; j++) {
										sum = 0.0f;
										for (k = rowstr[j]; k < rowstr[j+1]; k++) {
												sum = sum + a[k]*p[colidx[k]];
										}
										w[j] = sum;
								}

								// R/O Shared scalars: lastcol, firstcol
								// R/O Shared arrays: w[NA+2+1]
								// R/W Shared arrays: q[NA+2+1]
								// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION19) ftdata(FTVAR19) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
								for (j = 1; j <= lastcol-firstcol+1; j++) {
										q[j] = w[j];
								}

								/*--------------------------------------------------------------------
								  c  Clear w for reuse...
								  c-------------------------------------------------------------------*/
								// R/O Shared scalars: lastcol, firstcol
								// R/W Shared arrays: w[NA+2+1]
								// R/W Private scalars: j
								/*--------------------------------------------------------------------
								  c  Obtain p.q
								  c-------------------------------------------------------------------*/
								// R/O Shared scalars: lastcol, firstcol
								// R/O Shared arrays: p[NA+2+1], q[NA+2+1]
								// R/W Shared scalars: d (function-static)
								// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION20) ftdata(FTVAR20) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
								for (j = 1; j <= lastcol-firstcol+1; j++) {
										w[j] = 0.0f;
										d = d + p[j]*q[j];
								}

								/*--------------------------------------------------------------------
								  c  Obtain alpha = rho / (p.q)
								  c-------------------------------------------------------------------*/
								// R/O Shared scalars: rho0, d (function-static)
								// R/W Shared scalars: alpha (function-static)
								alpha = rho0 / d;

								/*--------------------------------------------------------------------
								  c  Save a temporary of rho
								  c-------------------------------------------------------------------*/
								/*	rho0 = rho;*/

								/*---------------------------------------------------------------------
								  c  Obtain z = z + alpha*p
								  c  and    r = r - alpha*q
								  c---------------------------------------------------------------------*/
								// R/O Shared scalars: lastcol, firstcol
								// R/O Shared scalars: alpha (function-static)
								// R/O Shared arrays: p[NA+2+1], q[NA+2+1]
								// R/W Shared arrays: z[NA+2+1], r[NA+2+1]
								// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION21) ftdata(FTVAR21) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
								for (j = 1; j <= lastcol-firstcol+1; j++) {
										z[j] = z[j] + alpha*p[j];
										r[j] = r[j] - alpha*q[j];
								}

								/*---------------------------------------------------------------------
								  c  rho = r.r
								  c  Now, obtain the norm of r: First, sum squares of r elements locally...
								  c---------------------------------------------------------------------*/
								// R/O Shared scalars: lastcol, firstcol
								// R/O Shared arrays: r[NA+2+1]
								// R/W Shared scalars: rho (function-static)
								// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION22) ftdata(FTVAR22) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
								for (j = 1; j <= lastcol-firstcol+1; j++) {
										rho = rho + r[j]*r[j];
								}

								/*--------------------------------------------------------------------
								  c  Obtain beta:
								  c-------------------------------------------------------------------*/
								// R/O Shared scalars: rho0, rho (function-static)
								// R/W Shared scalars: beta (function-static)
								beta = rho / rho0;

								/*--------------------------------------------------------------------
								  c  p = r + beta*p
								  c-------------------------------------------------------------------*/
								// R/O Shared scalars: lastcol, firstcol
								// R/O Shared scalars: beta (function-static)
								// R/O Shared arrays: r[NA+2+1]
								// R/W Shared arrays: p[NA+2+1]
								// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION23) ftdata(FTVAR23) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
								for (j = 1; j <= lastcol-firstcol+1; j++) {
										p[j] = r[j] + beta*p[j];
								}
						} /* end of do cgit=1,cgitmax */

						/*---------------------------------------------------------------------
						  c  Compute residual norm explicitly:  ||r|| = ||x - A.z||
						  c  First, form A.z
						  c  The partition submatrix-vector multiply
						  c---------------------------------------------------------------------*/
						// R/W Shared scalars: sum (function-static)
						sum = 0.0f;

						// R/O Shared scalars: lastcol, firstcol
						// R/O Shared arrays: rowstr[NA+1+1], a[NZ+1], colidx[NZ+1], z[NA+2+1]
						// R/W Shared arrays: w[NA+2+1]
						// R/W Private scalars: j,d,k
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION24) ftdata(FTVAR24) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker independent private(d) 
						for (j = 1; j <= lastrow-firstrow+1; j++) {
								d = 0.0f;
								for (k = rowstr[j]; k <= rowstr[j+1]-1; k++) {
										d = d + a[k]*z[colidx[k]];
								}
								w[j] = d;
						}

						// R/O Shared scalars: lastcol, firstcol
						// R/O Shared arrays: w[NA+2+1]
						// R/W Shared arrays: r[NA+2+1]
						// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION25) ftdata(FTVAR25) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
						for (j = 1; j <= lastcol-firstcol+1; j++) {
								r[j] = w[j];
						}

						/*--------------------------------------------------------------------
						  c  At this point, r contains A.z
						  c-------------------------------------------------------------------*/
						// R/O Shared scalars: lastcol, firstcol
						// R/O Shared arrays: r[NA+2+1], x[NA+2+1]
						// R/W Shared scalars: d, sum (function-static)
						// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION26) ftdata(FTVAR26) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker independent private(d)
						for (j = 1; j <= lastcol-firstcol+1; j++) {
								d = x[j] - r[j];
								sum = sum + d*d;
						}

						// R/O Shared scalars: sum (function-static)
						// R/W Shared scalars: rnorm
						{
								//(*rnorm) = sqrtf(sum);
								rnorm = sqrtf(sum);
						} /* end single */

						/*--------------------------------------------------------------------
						  c  zeta = shift + 1/(x.z)
						  c  So, first: (x.z)
						  c  Also, find norm of z
						  c  So, first: (z.z)
						  c-------------------------------------------------------------------*/
						// R/W Shared scalars: norm_temp11, norm_temp12
						{	
								norm_temp11 = 0.0f;
								norm_temp12 = 0.0f;
						} /* end single */

						// R/O Shared scalars: lastcol, firstcol
						// R/O Shared arrays: x[NA+2+1], z[NA+2+1]
						// R/W Shared scalars: norm_temp11, norm_temp12
						// R/W Private scalars: j_main
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION27) ftdata(FTVAR27) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
						for (j_main = 1; j_main <= lastcol-firstcol+1; j_main++) {
								norm_temp11 = norm_temp11 + x[j_main]*z[j_main];
								norm_temp12 = norm_temp12 + z[j_main]*z[j_main];
						}

						// R/O Shared scalars: norm_temp11
						// R/W Shared scalars: norm_temp12, zeta
						{	
								norm_temp12 = 1.0f / sqrtf( norm_temp12 );
								zeta = SHIFT + 1.0f / norm_temp11;
						} /* end single */

						{
								if( it == 1 ) {
										printf("   iteration           ||r||                 zeta\n");
								}
								printf("    %5d       %20.14e%20.13e\n", it, rnorm, zeta);
#ifdef DEBUG_PRINT
								fprintf(fp, "    %5d       %20.14e%20.13e\n", it, rnorm, zeta);
								
#endif
						} /* end master */

						/*--------------------------------------------------------------------
						  c  Normalize z to obtain x
						  c-------------------------------------------------------------------*/
						// R/O Shared scalars: lastcol, firstcol, norm_temp12
						// R/O Shared arrays: z[NA+2+1]
						// R/W Shared arrays: x[NA+2+1]
						// R/W Private scalars: j
#pragma openarc resilience ftregion FTTHREAD ftcond(RES_REGION28) ftdata(FTVAR28) num_faults(TOTAL_NUM_FAULTS) num_ftbits(NUM_FAULTYBITS)
#pragma acc kernels loop gang worker
						for (j_main = 1; j_main <= lastcol-firstcol+1; j_main++) {
								x[j_main] = norm_temp12*z[j_main];
						}
				} /* end of main iter inv pow meth */

#if defined(_OPENMP)
				nthreads = omp_get_num_threads();
#endif /* _OPENMP */
		} /* end parallel */

		timer_stop( 1 );
		timer_stop( 2 );

		/*--------------------------------------------------------------------
		  c  End of timed section
		  c-------------------------------------------------------------------*/

		t = timer_read( 1 );

		printf(" Benchmark completed\n");

		//epsilon = 1.0e-10;
		//New value for single precision
		epsilon = 1.0e-6;
		if (classT != 'U') {
				if (fabs(zeta - zeta_verify_value) <= epsilon) {
						verified = TRUE;
						printf(" VERIFICATION SUCCESSFUL\n");
						printf(" Zeta is    %20.12e\n", zeta);
						printf(" Error is   %20.12e\n", zeta - zeta_verify_value);
				} else {
						verified = FALSE;
						printf(" VERIFICATION FAILED\n");
						printf(" Zeta                %20.12e\n", zeta);
						printf(" The correct zeta is %20.12e\n", zeta_verify_value);
				}
		} else {
				verified = FALSE;
				printf(" Problem size unknown\n");
				printf(" NO VERIFICATION PERFORMED\n");
		}

		if ( t != 0.0 ) {
				mflops = (2.0*NITER*NA)
						* (3.0+(NONZER*(NONZER+1)) + 25.0*(5.0+(NONZER*(NONZER+1))) + 3.0 )
						/ t / 1000000.0;
		} else {
				mflops = 0.0;
		}
#ifdef DEBUG_PRINT
		fclose(fp);
#endif

		c_print_results("CG", classT, NA, 0, 0, NITER, nthreads, t, 
						mflops, "          floating point", 
						verified, NPBVERSION, COMPILETIME,
						CS1, CS2, CS3, CS4, CS5, CS6, CS7);
        printf("makea() execution time = %12.4f\n", timer_read(4));
        printf("CUDA Initialization time = %12.4f\n", timer_read(3));
        printf("Total execution time = %12.4f\n", timer_read(2));
		return 0;
}

/*---------------------------------------------------------------------
  c       generate the test problem for benchmark 6
  c       makea generates a sparse matrix with a
  c       prescribed sparsity distribution
  c
  c       parameter    type        usage
  c
  c       input
  c
  c       n            i           number of cols/rows of matrix
  c       nz           i           nonzeros as declared array size
  c       rcond        r*8         condition number
  c       shift        r*8         main diagonal shift
  c
  c       output
  c
  c       a            r*8         array for nonzeros
  c       colidx       i           col indices
  c       rowstr       i           row pointers
  c
  c       workspace
  c
  c       iv, arow, acol i
  c       v, aelt        r*8
  c---------------------------------------------------------------------*/
static void makea(
				int n,
				int nz,
				float a[NZ+1],		/* a[1:nz] */
				int colidx[NZ+1],	/* colidx[1:nz] */
				int rowstr[NA+1+1],	/* rowstr[1:n+1] */
				int nonzer,
				int firstrow,
				int lastrow,
				int firstcol,
				int lastcol,
				float rcond,
				int arow[NZ+1],		/* arow[1:nz] */
				int acol[NZ+1],		/* acol[1:nz] */
				float aelt[NZ+1],	/* aelt[1:nz] */
				float v[NA+1+1],		/* v[1:n+1] */
				int iv[2*NA+1+1],		/* iv[1:2*n+1] */
				float shift )
{
		int i, nnza, iouter, ivelt, ivelt1, irow, nzv;

		/*--------------------------------------------------------------------
		  c      nonzer is approximately  (int(sqrt(nnza /n)));
		  c-------------------------------------------------------------------*/

		float size, ratio, scale;
		int jcol;

		size = 1.0f;
		ratio = pow(rcond, (1.0f / (float)n));
		nnza = 0;

		/*---------------------------------------------------------------------
		  c  Initialize colidx(n+1 .. 2n) to zero.
		  c  Used by sprnvc to mark nonzero positions
		  c---------------------------------------------------------------------*/
		// R/O Shared scalars: n
		// R/W Shared arrays: colidx[NZ+1]
		// R/W Private scalars: i
#pragma acc kernels loop gang worker pcopyout(colidx) 
		for (i = 1; i <= n; i++) {
				colidx[n+i] = 0;
		}
		for (iouter = 1; iouter <= n; iouter++) {
				nzv = nonzer;
				sprnvc(n, nzv, v, iv, &(colidx[0]), &(colidx[n]));
				vecset(n, v, iv, &nzv, iouter, 0.5);
				for (ivelt = 1; ivelt <= nzv; ivelt++) {
						jcol = iv[ivelt];
						if (jcol >= firstcol && jcol <= lastcol) {
								scale = size * v[ivelt];
								for (ivelt1 = 1; ivelt1 <= nzv; ivelt1++) {
										irow = iv[ivelt1];
										if (irow >= firstrow && irow <= lastrow) {
												nnza = nnza + 1;
												if (nnza > nz) {
														printf("Space for matrix elements exceeded in"
																		" makea\n");
														printf("nnza, nzmax = %d, %d\n", nnza, nz);
														printf("iouter = %d\n", iouter);
														exit(1);
												}
												acol[nnza] = jcol;
												arow[nnza] = irow;
												aelt[nnza] = v[ivelt1] * scale;
										}
								}
						}
				}
				size = size * ratio;
		}

		/*---------------------------------------------------------------------
		  c       ... add the identity * rcond to the generated matrix to bound
		  c           the smallest eigenvalue from below by rcond
		  c---------------------------------------------------------------------*/
		for (i = firstrow; i <= lastrow; i++) {
				if (i >= firstcol && i <= lastcol) {
						iouter = n + i;
						nnza = nnza + 1;
						if (nnza > nz) {
								printf("Space for matrix elements exceeded in makea\n");
								printf("nnza, nzmax = %d, %d\n", nnza, nz);
								printf("iouter = %d\n", iouter);
								exit(1);
						}
						acol[nnza] = i;
						arow[nnza] = i;
						aelt[nnza] = rcond - shift;
				}
		}

		/*---------------------------------------------------------------------
		  c       ... make the sparse matrix from list of elements with duplicates
		  c           (v and iv are used as  workspace)
		  c---------------------------------------------------------------------*/
		sparse(a, colidx, rowstr, n, arow, acol, aelt,
						firstrow, lastrow, v, &(iv[0]), &(iv[n]), nnza);
}

/*---------------------------------------------------
  c       generate a sparse matrix from a list of
  c       [col, row, element] tri
  c---------------------------------------------------*/
static void sparse(
				float a[NZ+1],		/* a[1:*] */
				int colidx[NZ+1],	/* colidx[1:*] */
				int rowstr[NA+1+1],	/* rowstr[1:*] */
				int n,
				int arow[NZ+1],		/* arow[1:*] */
				int acol[NZ+1],		/* acol[1:*] */
				float aelt[NZ+1],	/* aelt[1:*] */
				int firstrow,
				int lastrow,
				float x[NA+1+1],		/* x[1:n] */
				boolean mark[NA+1],	/* mark[1:n] */
				int nzloc[NA+1],	/* nzloc[1:n] */
				int nnza)
/*---------------------------------------------------------------------
  c       rows range from firstrow to lastrow
  c       the rowstr pointers are defined for nrows = lastrow-firstrow+1 values
  c---------------------------------------------------------------------*/
{
		int nrows;
		int i, j, jajp1, nza, k, nzrow;
		float xi;

		/*--------------------------------------------------------------------
		  c    how many rows of result
		  c-------------------------------------------------------------------*/
		nrows = lastrow - firstrow + 1;

		/*--------------------------------------------------------------------
		  c     ...count the number of triples in each row
		  c-------------------------------------------------------------------*/
		// R/O Shared scalars: n
		// R/W Shared arrays: rowstr[NA+1+1], mark[n]
		// R/W Private scalars: j
#pragma acc kernels loop gang worker independent \
		pcopyout(rowstr[0:NA+1+1]) create(mark[0:NA+1]) 
		for (j = 1; j <= n; j++) {
				rowstr[j] = 0;
				mark[j] = FALSE;
		}
		rowstr[n+1] = 0;

		for (nza = 1; nza <= nnza; nza++) {
				j = (arow[nza] - firstrow + 1) + 1;
				rowstr[j] = rowstr[j] + 1;
		}

		rowstr[1] = 1;
		for (j = 2; j <= nrows+1; j++) {
				rowstr[j] = rowstr[j] + rowstr[j-1];
		}

		/*---------------------------------------------------------------------
		  c     ... rowstr(j) now is the location of the first nonzero
		  c           of row j of a
		  c---------------------------------------------------------------------*/

		/*--------------------------------------------------------------------
		  c     ... do a bucket sort of the triples on the row index
		  c-------------------------------------------------------------------*/
		for (nza = 1; nza <= nnza; nza++) {
				j = arow[nza] - firstrow + 1;
				k = rowstr[j];
				a[k] = aelt[nza];
				colidx[k] = acol[nza];
				rowstr[j] = rowstr[j] + 1;
		}

		/*--------------------------------------------------------------------
		  c       ... rowstr(j) now points to the first element of row j+1
		  c-------------------------------------------------------------------*/
		for (j = nrows; j >= 1; j--) {
				rowstr[j+1] = rowstr[j];
		}
		rowstr[1] = 1;

		/*--------------------------------------------------------------------
		  c       ... generate the actual output rows by adding elements
		  c-------------------------------------------------------------------*/
		nza = 0;
		// R/O Shared scalars: n
		// R/W Shared arrays: x[NA+2+1], mark[n]
		// R/W Private scalars: i
#pragma acc kernels loop gang worker pcopyout(x, mark)
		for (i = 1; i <= n; i++) {
				x[i] = 0.0f;
				mark[i] = FALSE;
		}

		jajp1 = rowstr[1];
		for (j = 1; j <= nrows; j++) {
				nzrow = 0;

				/*--------------------------------------------------------------------
				  c          ...loop over the jth row of a
				  c-------------------------------------------------------------------*/
				for (k = jajp1; k < rowstr[j+1]; k++) {
						i = colidx[k];
						x[i] = x[i] + a[k];
						if ( mark[i] == FALSE && x[i] != 0.0f) {
								mark[i] = TRUE;
								nzrow = nzrow + 1;
								nzloc[nzrow] = i;
						}
				}

				/*--------------------------------------------------------------------
				  c          ... extract the nonzeros of this row
				  c-------------------------------------------------------------------*/
				for (k = 1; k <= nzrow; k++) {
						i = nzloc[k];
						mark[i] = FALSE;
						xi = x[i];
						x[i] = 0.0f;
						if (xi != 0.0f) {
								nza = nza + 1;
								a[nza] = xi;
								colidx[nza] = i;
						}
				}
				jajp1 = rowstr[j+1];
				rowstr[j+1] = nza + rowstr[1];
		}
}

/*---------------------------------------------------------------------
  c       generate a sparse n-vector (v, iv)
  c       having nzv nonzeros
  c
  c       mark(i) is set to 1 if position i is nonzero.
  c       mark is all zero on entry and is reset to all zero before exit
  c       this corrects a performance bug found by John G. Lewis, caused by
  c       reinitialization of mark on every one of the n calls to sprnvc
  ---------------------------------------------------------------------*/
static void sprnvc(
				int n,
				int nz,
				float v[],		/* v[1:*] */
				int iv[],		/* iv[1:*] */
				int nzloc[],	/* nzloc[1:n] */
				int mark[] ) 	/* mark[1:n] */
{
		int nn1;
		int nzrow, nzv, ii, i;
		float vecelt, vecloc;

		nzv = 0;
		nzrow = 0;
		nn1 = 1;
		do {
				nn1 = 2 * nn1;
		} while (nn1 < n);

		/*--------------------------------------------------------------------
		  c    nn1 is the smallest power of two not less than n
		  c-------------------------------------------------------------------*/

		while (nzv < nz) {
				vecelt = randlc(&tran, amult);

				/*--------------------------------------------------------------------
				  c   generate an integer between 1 and n in a portable manner
				  c-------------------------------------------------------------------*/
				vecloc = randlc(&tran, amult);
				i = icnvrt(vecloc, nn1) + 1;
				if (i > n) continue;

				/*--------------------------------------------------------------------
				  c  was this integer generated already?
				  c-------------------------------------------------------------------*/
				if (mark[i] == 0) {
						mark[i] = 1;
						nzrow = nzrow + 1;
						nzloc[nzrow] = i;
						nzv = nzv + 1;
						v[nzv] = vecelt;
						iv[nzv] = i;
				}
		}

		for (ii = 1; ii <= nzrow; ii++) {
				i = nzloc[ii];
				mark[i] = 0;
		}
}

/*---------------------------------------------------------------------
 * scale a float precision number x in (0,1) by a power of 2 and chop it
 *---------------------------------------------------------------------*/
static int icnvrt(float x, int ipwr2) {
		return ((int)(ipwr2 * x));
}

/*--------------------------------------------------------------------
  c       set ith element of sparse vector (v, iv) with
  c       nzv nonzeros to val
  c-------------------------------------------------------------------*/
static void vecset(
				int n,
				float v[],	/* v[1:*] */
				int iv[],	/* iv[1:*] */
				int *nzv,
				int i,
				float val)
{
		int k;
		boolean set;

		set = FALSE;
		for (k = 1; k <= *nzv; k++) {
				if (iv[k] == i) {
						v[k] = val;
						set  = TRUE;
				}
		}
		if (set == FALSE) {
				*nzv = *nzv + 1;
				v[*nzv] = val;
				iv[*nzv] = i;
		}
}
