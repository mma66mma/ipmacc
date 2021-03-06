/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

/*
 * Walsh transforms belong to a class of generalized Fourier transformations. 
 * They have applications in various fields of electrical engineering 
 * and numeric theory. In this sample we demonstrate efficient implementation 
 * of naturally-ordered Walsh transform 
 * (also known as Walsh-Hadamard or Hadamard transform) in CUDA and its 
 * particular application to dyadic convolution computation.
 * Refer to excellent Jorg Arndt's "Algorithms for Programmers" textbook
 * http://www.jjj.de/fxt/fxtbook.pdf (Chapter 22)
 *
 * Victor Podlozhnyuk (vpodlozhnyuk@nvidia.com)
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include <cutil_inline.h>
//#include <shrQATest.h>

void fwtCPU(float *h_Output, float *h_Input, int log2N){
    const int N = 1 << log2N;

    for(int pos = 0; pos < N; pos++)
        h_Output[pos] = h_Input[pos];

    //Cycle through stages with different butterfly strides
    for(int stride = N / 2; stride >= 1; stride >>= 1){
        //Cycle through subvectors of (2 * stride) elements
        for(int base = 0; base < N; base += 2 * stride)
            //Butterfly index within subvector of (2 * stride) size
            for(int j = 0; j < stride; j++){
                int i0 = base + j +      0;
                int i1 = base + j + stride;

                float T1 = h_Output[i0];
                float T2 = h_Output[i1];
                h_Output[i0] = T1 + T2;
                h_Output[i1] = T1 - T2;
            }
    }
}



///////////////////////////////////////////////////////////////////////////////
// Straightforward Walsh Transform: used to test both CPU and GPU FWT
// Slow. Uses doubles because of straightforward accumulation
///////////////////////////////////////////////////////////////////////////////
void slowWTcpu(float *h_Output, float *h_Input, int log2N){
    const int N = 1 << log2N;

    for(int i = 0; i < N; i++){
        double sum = 0;

        for(int j = 0; j < N; j++){
            //Walsh-Hadamar quotent
            double q = 1.0;
            for(int t = i & j; t != 0; t >>= 1)
                if(t & 1) q = -q;

            sum += q * h_Input[j];
        }

        h_Output[i] = (float)sum;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Reference CPU dyadic convolution.
// Extremly slow because of non-linear memory access patterns (cache thrashing)
////////////////////////////////////////////////////////////////////////////////
void dyadicConvolutionCPU(
    float *h_Result,
    float *h_Data,
    float *h_Kernel,
    int log2dataN,
    int log2kernelN
){
    const int   dataN = 1 << log2dataN;
    const int kernelN = 1 << log2kernelN;

#progma acc kernels copyin(h_Data[0:dataN],h_Kernel[0:kernelN],sum) copy(h_Result[0:dataN])
    for(int i = 0; i < dataN; i++){
        double sum = 0;

        for(int j = 0; j < kernelN; j++)
            sum += h_Data[i ^ j] * h_Kernel[j];

        h_Result[i] = (float)sum;
    }
}
////////////////////////////////////////////////////////////////////////////////
// Reference CPU FWT
///////////////////////////////////////////////////////////////////////////////
/*extern "C" void fwtCPU(float *h_Output, float *h_Input, int log2N);
extern "C" void slowWTcpu(float *h_Output, float *h_Input, int log2N);
extern "C" void dyadicConvolutionCPU(
    float *h_Result,
    float *h_Data,
    float *h_Kernel,
    int log2dataN,
    int log2kernelN
);
*/

////////////////////////////////////////////////////////////////////////////////
// GPU FWT
////////////////////////////////////////////////////////////////////////////////
//#include "fastWalshTransform_kernel.cu"



////////////////////////////////////////////////////////////////////////////////
// Data configuration
////////////////////////////////////////////////////////////////////////////////
int log2Kernel = 7;
int log2Data = 23;

int   dataN = 1 << 23 ;
int kernelN = 1 << 7 ;

int   DATA_SIZE = dataN   * sizeof(float);
int KERNEL_SIZE = kernelN  * sizeof(float);




double NOPS = 3.0 * (double)(dataN ) * (double)(kernelN) / 2.0;



////////////////////////////////////////////////////////////////////////////////
// Main program
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]){
    float *h_Data, 
          *h_Kernel,
          *h_ResultCPU, 
          *h_ResultGPU;

    float *d_Data,
 	  *d_Kernel;

	double delta, ref, sum_delta2, sum_ref2, L2norm, gpuTime;

    unsigned int hTimer;
    int i;

//    shrQAStart(argc, argv);

    // use command-line specified CUDA device, otherwise use device with highest Gflops/s
  /*  if( cutCheckCmdLineFlag(argc, (const char**)argv, "device") )
       cutilDeviceInit(argc, argv);
    else
        cudaSetDevice( cutGetMaxGflopsDeviceId() );

    cutilCheckError( cutCreateTimer(&hTimer) );
*/
   	 printf("Initializing data...\n");
       	 printf("...allocating CPU memory\n");
  //      cutilSafeMalloc( h_Kernel    = (float *)malloc(KERNEL_SIZE) );
  //      cutilSafeMalloc( h_Data      = (float *)malloc(DATA_SIZE)   );
   //     cutilSafeMalloc( h_ResultCPU = (float *)malloc(DATA_SIZE));
   //     cutilSafeMalloc( h_ResultGPU = (float *)malloc(DATA_SIZE)   );
   	h_Kernel    = (float *)malloc(KERNEL_SIZE);
	h_Data      = (float *)malloc(DATA_SIZE) ;
        h_ResultCPU = (float *)malloc(DATA_SIZE);
	h_ResultGPU = (float *)malloc(DATA_SIZE);
        printf("...allocating GPU memory\n");
       // cutilSafeCall( cudaMalloc((void **)&d_Kernel, DATA_SIZE) );
       // cutilSafeCall( cudaMalloc((void **)&d_Data,   DATA_SIZE) );

        printf("...generating data\n");
        printf("Data length: %i; kernel length: %i\n", dataN, kernelN);
        srand(2007);
        for (i = 0; i < kernelN; i++)
            h_Kernel[i] = (float)rand() / (float)RAND_MAX;

        for (i = 0; i < dataN; i++)
            h_Data[i] = (float)rand() / (float)RAND_MAX;

      //  cutilSafeCall( memset(d_Kernel, 0, DATA_SIZE) );
       // cutilSafeCall( memcpy(d_Kernel, h_Kernel, KERNEL_SIZE, cudaMemcpyHostToDevice) );
       // cutilSafeCall( memcpy(d_Data,   h_Data,     DATA_SIZE, cudaMemcpyHostToDevice) );

    printf("Running GPU dyadic convolution using Fast Walsh Transform...\n");
   // cutilSafeCall( cutilDeviceSynchronize() );
   // cutilCheckError( cutResetTimer(hTimer) );
   // cutilCheckError( cutStartTimer(hTimer) );
   //     fwtBatchGPU(d_Data, 1, log2Data);
   //     fwtBatchGPU(d_Kernel, 1, log2Data);
   //     modulateGPU(d_Data, d_Kernel, dataN);
   //     fwtBatchGPU(d_Data, 1, log2Data);
  //  cutilSafeCall( cutilDeviceSynchronize() );
  //  cutilCheckError( cutStopTimer(hTimer) );
  //  gpuTime = cutGetTimerValue(hTimer);
  //  printf("GPU time: %f ms; GOP/s: %f\n", gpuTime, NOPS / (gpuTime * 0.001 * 1E+9));

    printf("Reading back GPU results...\n");
   // cutilSafeCall( cudaMemcpy(h_ResultGPU, d_Data, DATA_SIZE, cudaMemcpyDeviceToHost) );

    printf("Running straightforward CPU dyadic convolution...\n");
    dyadicConvolutionCPU(h_ResultCPU, h_Data, h_Kernel, log2Data, log2Kernel);

    printf("Comparing the results...\n");
        sum_delta2 = 0;
        sum_ref2   = 0;
        for(i = 0; i < dataN; i++){
         // delta       = h_ResultCPU[i] - h_ResultGPU[i];
             delta       = h_ResultCPU[i] - 0 ;
	     ref         = h_ResultCPU[i];
       	     sum_delta2 += delta * delta; 
             sum_ref2   += ref * ref;
        }
       L2norm = sqrt(sum_delta2 / sum_ref2);

    printf("Shutting down...\n");
     //   cutilCheckError(  cutDeleteTimer(hTimer) );
      //  cutilSafeCall( cudaFree(d_Data)   );
      //  cutilSafeCall( cudaFree(d_Kernel) );
  //      free(h_ResultGPU);
        free(h_ResultCPU);
        free(h_Data);
        free(h_Kernel);

   // cutilDeviceReset();
    printf("L2 norm: %E\n", L2norm);
   // shrQAFinishExit(argc, (const char **)argv, (L2norm < 1e-6) ? QA_PASSED : QA_FAILED);
}
