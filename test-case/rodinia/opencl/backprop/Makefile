include ../../common/make.config

# C compiler
CC = g++
CC_FLAGS = -g -O3

backprop: backprop.o imagenet.o facetrain.o
	$(CC) $(CC_FLAGS) -lOpenCL backprop_ocl.cpp backprop.o imagenet.o facetrain.o -o backprop -I$(OPENCLINC) -L$(OPENCLLIB)

%.o: %.[ch]
	$(CC) $(CC_FLAGS) $< -c
	
backprop.o: backprop.c 
	$(CC) $(CC_FLAGS) backprop.c -c
	
facetrain.o: facetrain.c
	$(CC) $(CC_FLAGS) facetrain.c -c

imagenet.o: imagenet.c
	$(CC) $(CC_FLAGS) imagenet.c -c

clean:
	rm -f *.o *~ backprop *.linkinfo
