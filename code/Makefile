CC = gcc
PROGS= make-2d print-2d stencil-2d stencil-2d-pth stencil-2d-omp stencil-2d-mpi stencil-2d-hybrid
CFLAGS = -std=c99 -Wall -g -Wpedantic 
LFLAGS = -lm -fopenmp -pthread

all: $(PROGS)


utilities.o: utilities.c utilities.h
	$(CC) $(CFLAGS) -c utilities.c


make-2d.o: make-2d.c utilities.h utilities.c 
	$(CC) $(CFLAGS) -c make-2d.c

make-2d: make-2d.o 
	$(CC) -o make-2d ./make-2d.o  $(LFLAGS)


print-2d.o: print-2d.c utilities.h utilities.c 
	$(CC) $(CFLAGS) -c print-2d.c

print-2d: print-2d.o 
	$(CC) -o print-2d ./print-2d.o  $(LFLAGS)

	
stencil-2d.o: stencil-2d.c utilities.h utilities.c 
	$(CC) $(CFLAGS) -c stencil-2d.c

stencil-2d: stencil-2d.o 
	$(CC) -o stencil-2d ./stencil-2d.o  $(LFLAGS)


stencil-2d-pth.o: stencil-2d-pth.c utilities.h utilities.c 
	$(CC) $(CFLAGS) -c stencil-2d-pth.c

stencil-2d-pth: stencil-2d-pth.o 
	$(CC) -o stencil-2d-pth ./stencil-2d-pth.o  $(LFLAGS)

	
stencil-2d-omp.o: stencil-2d-omp.c utilities.h utilities.c 
	$(CC) $(CFLAGS) -c stencil-2d-omp.c

stencil-2d-omp: stencil-2d-omp.o 
	$(CC) -o stencil-2d-omp ./stencil-2d-omp.o  $(LFLAGS)

	
stencil-2d-mpi.o: stencil-2d-mpi.c utilities.h utilities.c 
	$(CC) $(CFLAGS) -c stencil-2d-mpi.c

stencil-2d-mpi: stencil-2d-mpi.o 
	$(CC) -o stencil-2d-mpi ./stencil-2d-mpi.o  $(LFLAGS)

	
stencil-2d-hybrid.o: stencil-2d-hybrid.c utilities.h utilities.c 
	$(CC) $(CFLAGS) -c stencil-2d-hybrid.c

stencil-2d-hybrid: stencil-2d-hybrid.o 
	$(CC) -o stencil-2d-hybrid ./stencil-2d-hybrid.o  $(LFLAGS)


clean: 
	rm -f *.o $(PROGS)