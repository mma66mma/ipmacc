include ../../common/make.config
include ../../common/arch.mk

LOCAL_CC = gcc -g -O3 -Wall
CC = $(CUDA_DIR)/bin/nvcc $(ARCH) 

all : nn hurricane_gen

clean :
	rm -rf *.o nn nn_cuda.cu.cpp hurricane_gen *ii *txt

nn : 
	$(CC) nn_cuda.cu -o nn
	#$(CC) -L$(CUDA_DIR)/lib64 -lcudart -o nn nn_cuda.cu.cpp

hurricane_gen : hurricane_gen.c
	$(LOCAL_CC) -L$(CUDA_DIR)/lib64 -lcudart -o $@ $< -lm

#data :
#	mkdir data
#	./gen_dataset.sh
