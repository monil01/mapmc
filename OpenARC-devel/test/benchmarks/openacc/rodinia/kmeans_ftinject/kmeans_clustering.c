/*****************************************************************************/
/*IMPORTANT:  READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.         */
/*By downloading, copying, installing or using the software you agree        */
/*to this license.  If you do not agree to this license, do not download,    */
/*install, copy or use the software.                                         */
/*                                                                           */
/*                                                                           */
/*Copyright (c) 2005 Northwestern University                                 */
/*All rights reserved.                                                       */

/*Redistribution of the software in source and binary forms,                 */
/*with or without modification, is permitted provided that the               */
/*following conditions are met:                                              */
/*                                                                           */
/*1       Redistributions of source code must retain the above copyright     */
/*        notice, this list of conditions and the following disclaimer.      */
/*                                                                           */
/*2       Redistributions in binary form must reproduce the above copyright   */
/*        notice, this list of conditions and the following disclaimer in the */
/*        documentation and/or other materials provided with the distribution.*/ 
/*                                                                            */
/*3       Neither the name of Northwestern University nor the names of its    */
/*        contributors may be used to endorse or promote products derived     */
/*        from this software without specific prior written permission.       */
/*                                                                            */
/*THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS    */
/*IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED      */
/*TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT AND         */
/*FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL          */
/*NORTHWESTERN UNIVERSITY OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,       */
/*INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES          */
/*(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR          */
/*SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)          */
/*HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,         */
/*STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN    */
/*ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE             */
/*POSSIBILITY OF SUCH DAMAGE.                                                 */
/******************************************************************************/
/*************************************************************************/
/**   File:         kmeans_clustering.c                                 **/
/**   Description:  Implementation of regular k-means clustering        **/
/**                 algorithm                                           **/
/**   Author:  Wei-keng Liao                                            **/
/**            ECE Department, Northwestern University                  **/
/**            email: wkliao@ece.northwestern.edu                       **/
/**                                                                     **/
/**   Edited by: Jay Pisharath                                          **/
/**              Northwestern University.                               **/
/**                                                                     **/
/**   ================================================================  **/
/**																		**/
/**   Edited by: Sang-Ha  Lee											**/
/**				 University of Virginia									**/
/**																		**/
/**   Description:	No longer supports fuzzy c-means clustering;	 	**/
/**					only regular k-means clustering.					**/
/**					Simplified for main functionality: regular k-means	**/
/**					clustering.											**/
/**                                                                     **/
/*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "kmeans.h"
#include <resilience.h>

#ifndef ENABLE_OPENACC
#define ENABLE_OPENACC 1
#endif
#ifndef R_MODE
#define R_MODE 0
#endif

#define RANDOM_MAX 2147483647

#ifndef FLT_MAX
#define FLT_MAX 3.40282347e+38
#endif

#ifndef _NPOINTS
#define _NPOINTS 819200
#endif

#ifndef _UNROLLFAC_
#define _UNROLLFAC_ 100
#endif

#define _NTHREADS	(_NPOINTS/_UNROLLFAC_)

#ifndef RES_REGION0
#define RES_REGION0	1
#endif
#ifndef TOTAL_NUM_FAULTS
#define TOTAL_NUM_FAULTS	1
#endif
#ifndef NUM_FAULTYBITS
#define NUM_FAULTYBITS	1
#endif
#ifndef NUM_REPEATS
#define NUM_REPEATS	1
#endif
#ifndef _FTVAR
#define _FTVAR	0
#endif
#ifndef _FTKIND
#define _FTKIND 5
#endif
#ifndef _FTTHREAD
#define _FTTHREAD 0
#endif


#ifdef _OPENARC_

#include "ftmacro.h"

#if _NPOINTS == 204800
#pragma openarc #define _NPOINTS 204800
#elif _NPOINTS == 494020
#pragma openarc #define _NPOINTS 494020
#elif _NPOINTS == 819200
#pragma openarc #define _NPOINTS 819200
#endif

#if _UNROLLFAC_ == 1
#pragma openarc #define _UNROLLFAC_ 1
#elif _UNROLLFAC_ == 2
#pragma openarc #define _UNROLLFAC_ 2
#elif _UNROLLFAC_ == 4
#pragma openarc #define _UNROLLFAC_ 4
#elif _UNROLLFAC_ == 5
#pragma openarc #define _UNROLLFAC_ 5
#elif _UNROLLFAC_ == 800
#pragma openarc #define _UNROLLFAC_ 800
#elif _UNROLLFAC_ == 10
#pragma openarc #define _UNROLLFAC_ 10
#elif _UNROLLFAC_ == 100
#pragma openarc #define _UNROLLFAC_ 100
#endif

#pragma openarc #define _NATTRIBUTES 34

#pragma openarc #define _NCLUSTERS 5

#pragma openarc #define _NTHREADS (_NPOINTS/_UNROLLFAC_)
#endif

#if (R_MODE == 2) || (R_MODE == 3) 
extern long count0;
#endif

extern double wtime(void);


/*----< kmeans_clustering() >---------------------------------------------*/
PAType kmeans_clustering(float feature[_NPOINTS][_NATTRIBUTES],    /* in: [npoints][nfeatures] */
				int     nfeatures,
				int     npoints,
				int     nclusters,
				float   threshold,
				int    membership[_NPOINTS]) /* out: [npoints] */
{

		int      i, j, k, n=0, index, loop=0;
		int     *new_centers_len;			/* [nclusters]: no. of points in each cluster */
		float  (*new_centers)[_NATTRIBUTES];				/* [nclusters][nfeatures] */
		float  (*clusters)[_NATTRIBUTES];					/* out: [nclusters][nfeatures] */
		float    delta;

		double   timing;

		int      nthreads;
		int    (*partial_new_centers_len)[_NCLUSTERS];
		float (*partial_new_centers)[_NCLUSTERS][_NATTRIBUTES];

		/////////////////////////////////////////////
		// Added for inlining find_nearest_point() //
		/////////////////////////////////////////////
		int index_fnp, i_fnp;
		float max_dist=FLT_MAX;
		int i_ed;

		///////////////////////////////////////////////
		// Added for unrolling of the parallel loop. //
		///////////////////////////////////////////////
		int tid, ii;

		//int ifcond_var = 0;

		//unsigned int itrpos;
		//unsigned int injectFT = 0;

		nthreads = npoints/_UNROLLFAC_;

		/* allocate space for returning variable clusters[] */
		clusters = (float (*)[_NATTRIBUTES])  malloc(nclusters * nfeatures * sizeof(float));

		/* randomly pick cluster centers */
		for (i=0; i<nclusters; i++) {
				//n = (int)rand() % npoints;
				for (j=0; j<nfeatures; j++)
						clusters[i][j] = feature[n][j];
				n++;
		}

		for (i=0; i<npoints; i++)
				membership[i] = -1;

		/* need to initialize new_centers_len and new_centers[0] to all 0 */
		new_centers_len = (int*) calloc(nclusters, sizeof(int));

		new_centers = (float (*)[_NATTRIBUTES])  calloc(nclusters * nfeatures, sizeof(float));

		partial_new_centers_len = (int (*)[_NCLUSTERS])  calloc(nthreads*nclusters, sizeof(int));

		partial_new_centers =(float (*)[_NCLUSTERS][_NATTRIBUTES]) calloc(nthreads*nclusters*nfeatures, sizeof(float));

		//Select outermost loop iteration to insert faults
		//HI_set_srand();
		//itrpos = HI_genrandom_int(20);  //profile says the do loop runs 20 times.

		printf("num of threads = %d\n", nthreads);
#if ENABLE_OPENACC == 1
#pragma acc data copyin (feature[0:_NPOINTS][0:_NATTRIBUTES], membership[0:_NPOINTS]) create(clusters[0:_NCLUSTERS][0:_NATTRIBUTES], \
		   partial_new_centers[0:_NTHREADS][0:_NCLUSTERS][0:_NATTRIBUTES],partial_new_centers_len[0:_NTHREADS][0:_NCLUSTERS]) 
#endif
		do {
				delta = 0.0F;
/*
				if( loop == itrpos ) {
					injectFT = 1;
				} else {
					injectFT = 0;
				}
				int ifcond_var = RES_REGION0 && injectFT;
*/

#if ENABLE_OPENACC == 1
#pragma acc update device(clusters, partial_new_centers, partial_new_centers_len)
#endif
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
#pragma acc kernels loop gang worker independent \
				private(i, index, index_fnp, max_dist)
#endif
				for(tid=0; tid<nthreads; tid++) {
#if ENABLE_OPENACC == 1
#pragma acc loop seq
#endif
						for (ii=0; ii<_UNROLLFAC_; ii++) {
								i = tid + ii*nthreads;
								/* find the index of nestest cluster centers */					
								//index = find_nearest_point(feature[i],
								//        nfeatures,
								//        clusters,
								//        nclusters);				

								max_dist = FLT_MAX;
								/* find the cluster center id with min distance to pt */
								for (i_fnp=0; i_fnp<nclusters; i_fnp++) {
										float dist;
										//dist = euclid_dist_2(feature[i_fnp], clusters[i_fnp], nfeatures);  /* no need square root */
										dist = 0.0F;
										for (i_ed=0; i_ed<nfeatures; i_ed++)
												dist += (feature[i][i_ed]-clusters[i_fnp][i_ed]) * (feature[i][i_ed]-clusters[i_fnp][i_ed]);
										if (dist < max_dist) {
												max_dist = dist;
												index_fnp    = i_fnp;
										}
								}
								index = index_fnp;

								/* if membership changes, increase delta by 1 */
								if (membership[i] != index) delta += 1.0F;

								/* assign the membership to object i */
								membership[i] = index;

								/* update new cluster centers : sum of all objects located
								   within */
								partial_new_centers_len[tid][index]++;				
								for (j=0; j<nfeatures; j++)
										partial_new_centers[tid][index][j] += feature[i][j];
						}
				} /* end of #pragma omp parallel for */
#if ENABLE_OPENACC == 1
				#pragma acc update host(partial_new_centers, partial_new_centers_len)
#endif

				/* let the main thread perform the array reduction */
				for (i=0; i<nclusters; i++) {
						for (j=0; j<nthreads; j++) {
								new_centers_len[i] += partial_new_centers_len[j][i];
								partial_new_centers_len[j][i] = 0;
								for (k=0; k<nfeatures; k++) {
										new_centers[i][k] += partial_new_centers[j][i][k];
										partial_new_centers[j][i][k] = 0.0F;
								}
						}
				}    

				/* replace old cluster centers with new_centers */
				for (i=0; i<nclusters; i++) {
						for (j=0; j<nfeatures; j++) {
								if (new_centers_len[i] > 0)
										clusters[i][j] = new_centers[i][j] / new_centers_len[i];
								new_centers[i][j] = 0.0F;   /* set back to 0 */
						}
						new_centers_len[i] = 0;   /* set back to 0 */
				}

		} while (delta > threshold && loop++ < 500);


		free(new_centers);
		free(new_centers_len);

		return clusters;
}

