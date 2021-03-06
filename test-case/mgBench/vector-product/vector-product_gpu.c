/*
   
    This program calculates result of vector product.
    It generates a vector with the results of two vectors multiplication.
    This program create a csv file with the time execution results for each function(CPU,GPU) in this format: size of vector,cpu time,gpu time.
    
    Author: Kezia Andrade
    Date: 04-06-2015
    version 1.0
    
    Run:
    folder_ipmacc/ipmacc folder_archive/mat-mul-sun-acc.c
    ./a.out
*/

#include <openacc.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int SIZE;

float *a;
float *b;
float *c;

FILE *result;
FILE *out;

// Initialize matrices.
void init(int s) 
{
	int i;
	for (i = 0; i < s; ++i)
	{
        	a[i] = (float)i + 3*i;
        	b[i] = (float)i + 2*i;
	}
}

void print_product(int s)
{
    int i;
    fprintf(result,"\nA:");
    for (i = 0; i < s; ++i)
    {
       	fprintf(result,"%f ", a[i]);
    }
    fprintf(result,"\nB:");
    for (i = 0; i < s; ++i)
    {
       	fprintf(result,"%f ", b[i]);
    }
    fprintf(result,"\nC:");
    for (i = 0; i < s; ++i)
    {
        fprintf(result, "%f", c[i]);
    }
}

void product_GPU(int s)
{
	int i;
	a = (float *) malloc(sizeof(float) * SIZE);
	b = (float *) malloc(sizeof(float) * SIZE);
    	c = (float *) malloc(sizeof(float) * SIZE);
 
	init(SIZE);

	double start, finish, elapsed;
    	start = (double) clock() / (CLOCKS_PER_SEC * 1000);
    	
	#pragma acc data copyin(a[0:s],b[0:s]) copyout(c[0:s])
        {
                #pragma acc kernels
                {
                        #pragma acc loop independent
                        {
				for (i = 0; i < s; ++i)
				{
        				c[i] = a[i] * b[i];
				}
			}
		}
	}
	
	acc_free(acc_deviceptr(a));
    acc_free(acc_deviceptr(b));
    acc_free(acc_deviceptr(c));

	finish = (double) clock() / (CLOCKS_PER_SEC * 1000);
	elapsed = finish - start;

	//print_product(s);

	free(a);
	free(b);
	free(c);

	fprintf(out,"%.6lf;",elapsed);
}

int main(int argc, char *argv[]) {
	
 	if(argc!=2)
    {
        return 1;
	}
    
    SIZE = atoi(argv[1]);
    
	out = fopen("time_gpu.csv","w+");
	fprintf(out,"SIZE of vector;Product GPU time;\n\n");
	result = fopen("result_gpu.txt","w+");
 
    fprintf(out,"%d;",SIZE);
    product_GPU(SIZE);
    fprintf(out,"\n");
	
	fclose(out);
	fclose(result);
	return 0;
}

