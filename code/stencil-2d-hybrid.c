/* 
 * Author:   Justin LaForge Kyle Wallace
 * 
 * File:     stencil-2d-hybrid.c
 *
 * Purpose:  Perform stencil simulation using Pthreads OpenMP and MPI
 *
 * Run:      ./stencil-2d-hybrid.c -t <num iters> -i <in> -o <out> -p <num process>
 *
 * Input:    Binary file with stencil matrix
 * 
 * Output:   Output stencil matrix binary file
 *
 * Errors:   Usage errors and file permission errors
 */
// hybrid-stencil.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <pthread.h>
#include <omp.h>
#include <mpi.h>
#include "utilities.h"    // <--- your clean header with prototypes + pthread_barrier_t

// command-line
void usage(char **argv) {
    fprintf(stderr,
      "Usage: %s -n <iters> -i <in> -o <out> -t <threads> -v <debug>\n",
      argv[0]);
    exit(1);
}
void setArgs(int argc, char **argv,
             int *n, char **in, char **out,
             int *nthreads, int *debug)
{
    int opt;
    while ((opt = getopt(argc, argv, "n:i:o:t:v:")) != -1) {
      switch (opt) {
        case 'n': *n       = atoi(optarg); break;
        case 'i': *in      = optarg;       break;
        case 'o': *out     = optarg;       break;
        case 't': *nthreads= atoi(optarg); break;
        case 'v': *debug   = atoi(optarg); break;
        default: usage(argv);
      }
    }
    if (!*in || !*out) usage(argv);
}

typedef struct {
    int    tid, nthreads;
    int    rows, cols, niters;
    double *matrix, *newMatrix;
    pthread_barrier_t *barrier;
    int debug;
} thread_arg_t;

static void* worker(void *v) {
    thread_arg_t *td = v;
    int tid = td->tid, nt = td->nthreads;
    int R  = td->rows, C = td->cols, N=td->niters;
    double *A = td->matrix, *B = td->newMatrix;
    pthread_barrier_t *bar = td->barrier;
    int debug = td->debug;
    // split rows [1..R-2] among threads:
    int lo = 1 + tid*(R-2)/nt;
    int hi = 1 + ((tid+1)*(R-2)+nt-1)/nt - 1;
    for(int iter=0; iter<N; iter++){
      // exchange MPI ghost rows happens in main before this join
      pthread_barrier_wait(bar);
      // each pthread does its stripe, internally parallelized with OpenMP
      #pragma omp parallel for schedule(static)
      for(int i=lo; i<=hi; i++){
        for(int j=1; j<C-1; j++){
          B[i*C+j] = (
            A[(i-1)*C + (j-1)] + A[(i-1)*C + j] + A[(i-1)*C + (j+1)] +
            A[i*C + (j-1)]     + A[i*C + j]     + A[i*C + (j+1)] +
            A[(i+1)*C + (j-1)] + A[(i+1)*C + j] + A[(i+1)*C + (j+1)]
          )/9.0;
        }
      }
      pthread_barrier_wait(bar);
      if(tid==0){
        // only one thread swaps pointers
        double *tmp = A; td->matrix = A = B; td->newMatrix = B = tmp;
        if(debug>=2){ printf("Iter %d:\n", iter);
          Print_matrix(A,R,C); printf("\n"); }
      }
      pthread_barrier_wait(bar);
    }
    return NULL;
}

int main(int argc, char **argv){
  MPI_Init(&argc,&argv);
  int rank, size; MPI_Comm_rank(MPI_COMM_WORLD,&rank);
                 MPI_Comm_size(MPI_COMM_WORLD,&size);

  int niters=1, debug=0, nthreads=1;
  char *in=NULL, *out=NULL;
  setArgs(argc,argv,&niters,&in,&out,&nthreads,&debug);

  double *matrix=NULL, *newMatrix=NULL;
  int rows, cols;
  if(rank==0) Read_matrix(in,&matrix,&rows,&cols);
  MPI_Bcast(&rows,1,MPI_INT,0,MPI_COMM_WORLD);
  MPI_Bcast(&cols,1,MPI_INT,0,MPI_COMM_WORLD);

  // compute local block sizes
  int base = rows/size, rem = rows%size;
  int myrows = base + (rank<rem);
  int start_row = rank*(base) + (rank<rem?rank:rem);
  // allocate local + 2 ghost rows
  double *A = malloc((myrows+2)*cols*sizeof(double));
  double *B = malloc((myrows+2)*cols*sizeof(double));
  int *sc = malloc(size*sizeof(int)), *disp = malloc(size*sizeof(int));
  // build counts/displs for Scatterv/Gatherv (no ghosts in global)
  int offset=0;
  for(int r=0;r<size;r++){
    int lr = base + (r<rem);
    sc[r] = lr*cols; disp[r]=offset; offset+=sc[r];
  }
  // scatter main data into A+cols (skip top ghost row)
  MPI_Scatterv(matrix,sc,disp,MPI_DOUBLE,
               A+cols, myrows*cols, MPI_DOUBLE,
               0, MPI_COMM_WORLD);
  // initialize B
  memcpy(B, A, (myrows+2)*cols*sizeof(double));

  // prepare pthreads
  pthread_t *ths = malloc(nthreads*sizeof(pthread_t));
  thread_arg_t *args = malloc(nthreads*sizeof(thread_arg_t));
  pthread_barrier_t bar;
  pthread_barrier_init(&bar,NULL,nthreads);

  for(int t=0;t<nthreads;t++){
    args[t].tid = t; args[t].nthreads=nthreads;
    args[t].rows=myrows+2; args[t].cols=cols;
    args[t].niters=niters; args[t].matrix=A; args[t].newMatrix=B;
    args[t].barrier=&bar; args[t].debug=debug;
    pthread_create(&ths[t],NULL,worker,&args[t]);
  }

  // overall timer
  double t0 = MPI_Wtime(), tw0=0, tw1=0;
  MPI_Barrier(MPI_COMM_WORLD); tw0 = MPI_Wtime();

  for(int iter=0;iter<niters;iter++){
    // exchange ghost rows with neighbors
    MPI_Request reqs[4]; int qc=0;
    // send up, recv top ghost
    if(rank>0){
      MPI_Isend(A+cols,      cols,MPI_DOUBLE,rank-1,0,MPI_COMM_WORLD,&reqs[qc++]);
      MPI_Irecv(A,           cols,MPI_DOUBLE,rank-1,0,MPI_COMM_WORLD,&reqs[qc++]);
    }
    // send down, recv bottom ghost
    if(rank<size-1){
      MPI_Isend(A+myrows*cols,cols,MPI_DOUBLE,rank+1,0,MPI_COMM_WORLD,&reqs[qc++]);
      MPI_Irecv(A+(myrows+1)*cols,cols,MPI_DOUBLE,rank+1,0,MPI_COMM_WORLD,&reqs[qc++]);
    }
    MPI_Waitall(qc,reqs,MPI_STATUSES_IGNORE);

    // let pthreads do their N-point updates
    // they synchronize and swap internally
    // nothing here — just wait for them to finish this iter
    // use MPI_Barrier as a global sync each iter
    MPI_Barrier(MPI_COMM_WORLD);
  }

  MPI_Barrier(MPI_COMM_WORLD); tw1 = MPI_Wtime();
  double workTime = tw1-tw0, overAll = MPI_Wtime()-t0;

  // gather back (skip ghosts)
  MPI_Gatherv(A+cols, myrows*cols, MPI_DOUBLE,
              matrix, sc, disp, MPI_DOUBLE,
              0, MPI_COMM_WORLD);

  if(rank==0){
    write_memory_to_file(matrix,rows,cols,out);
    if(debug){
      printf("Final:\n"); Print_matrix(matrix,rows,cols);
    }
    // log times
    FILE *f = fopen("hybridTime.csv","a");
    fprintf(f,"%d,%d,%d,%.6f,%.6f,%d,%d\n",
      niters,rows,cols,overAll,workTime,size,nthreads);
    fclose(f);
    free(matrix);
  }

  free(A); free(B);
  free(sc); free(disp);
  for(int t=0;t<nthreads;t++) pthread_join(ths[t],NULL);
  pthread_barrier_destroy(&bar);
  free(ths); free(args);

  MPI_Finalize();
  return 0;
}
