CC = gcc
CILKCC = /usr/local/OpenCilk-9.0.1-Linux/bin/clang
CFLAGS = -O3

sequential: mmio.c coo2csc.c mergesort.c utils.c sequential_version.c
	$(CC) $(CFLAGS) -c mmio.c coo2csc.c mergesort.c utils.c sequential_version.c
	$(CC) mmio.o coo2csc.o mergesort.o utils.o sequential_version.o -o sequential_version

pthreads: mmio.c coo2csc.c mergesort.c utils.c pthreads_version.c
	$(CC) $(CFLAGS) -c mmio.c coo2csc.c mergesort.c utils.c pthreads_version.c
	$(CC) mmio.o coo2csc.o mergesort.o utils.o pthreads_version.o -o pthreads_version -lpthread

openmp: mmio.c coo2csc.c mergesort.c utils.c omp_version.c
	$(CC) $(CFLAGS) -c mmio.c coo2csc.c mergesort.c utils.c omp_version.c
	$(CC) mmio.o coo2csc.o mergesort.o utils.o omp_version.o -o omp_version -fopenmp

cilk: mmio.c coo2csc.c mergesort.c utils.c cilk_version.c
	$(CILKCC) $(CFLAGS) -c mmio.c coo2csc.c mergesort.c utils.c cilk_version.c
	$(CILKCC) mmio.o coo2csc.o mergesort.o utils.o cilk_version.o -o cilk_version -fcilkplus -lpthread

clean:
	rm seq_version pthreads_version omp_version cilk_version
