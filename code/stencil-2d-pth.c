/* 
 * Author:   Justin LaForge Kyle Wallace
 * 
 * File:     stencil-2d-pth.c
 *
 * Purpose:  Perform stencil simulation using Pthreads for parallization
 *
 * Run:      ./stencil-2d-pth.c -t <num iters> -i <in> -o <out> -p <num process>
 *
 * Input:    Binary file with stencil matrix
 * 
 * Output:   Output stencil matrix binary file
 *
 * Errors:   Usage errors and file permission errors
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include "utilities.c"
 #include <getopt.h>
 #include <time.h>
 #include <string.h>
 #include <pthread.h>
 

 
 void usage(char **argv){
	 printf("Usage: %s -t <num iters> -i <in file> -o <out file> -p <num processes>\n", argv[0]);
 }
 
 // Set arguments
 void setArgs(int argc, char **argv, int *n, char **in, char **out, int *p){
	 int opt;
 
	 while((opt = getopt(argc, argv, "n:i:o:p:")) != -1){
		 switch(opt){
			 case 'n':
				 *n = atoi(optarg);
				 break;
			 case 'i':
				 *in = optarg;
				 break;
			 case 'o':
				 *out = optarg;
				 break;
			 case 'p':
				 *p = atoi(optarg);
				 break;
			 default:
				 usage(argv);
				 exit(1);
		 }
	 }
	 if(*in == NULL ||*out == NULL){
		 perror("Files -i and -o must be provided");
		 exit(EXIT_FAILURE);
	 }
	 
	 
 }
 
 
 int main(int argc, char **argv){
	 // ---- Timer Variables ----
	 double startOvrll=0;
	 double finishOvrll=0;
	 double startWork=0;
	 double finishWork=0;
 
	 GET_TIME(startOvrll);
	 
	 int n=1,NUM_THREADS=1;
	 char *in = NULL;
	 char *out = NULL;
	 
	 //set args
	 setArgs(argc, argv, &n, &in, &out, &NUM_THREADS);
 
	 double *matrix;
	 double *newMatrix;
	 int rows, cols;
 	
	 Read_matrix(in, &matrix, &rows, &cols);
	
	 newMatrix = malloc(rows * cols * sizeof(double));
	if (newMatrix == NULL) {
		fprintf(stderr, "Can't allocate storage\n");
		exit(-1);
	}
	memcpy(newMatrix, matrix, rows * cols * sizeof(double));	 
	 
	 GET_TIME(startWork);

     
    pthread_t threads[NUM_THREADS];
    thread_arg_t targs[NUM_THREADS];
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, NUM_THREADS);

 
    // Create threads
    for (int t = 0; t < NUM_THREADS; t++) {
        targs[t].thread_id = t;
        targs[t].num_threads = NUM_THREADS;
        targs[t].n_iters = n;
        targs[t].rows = rows;
        targs[t].cols = cols;
        targs[t].matrix = matrix;
        targs[t].newMatrix = newMatrix;
        targs[t].barrier = &barrier;
        pthread_create(&threads[t], NULL, pthread_stencil, (void*) &targs[t]);
    }

    // Join threads
    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    // Update pointers after threads
    matrix = targs[0].matrix;
    newMatrix = targs[0].newMatrix;

    pthread_barrier_destroy(&barrier);

    GET_TIME(finishWork);

    write_memory_to_file(matrix, rows, cols, out);

    free(matrix);
    if (newMatrix != NULL && newMatrix != matrix) free(newMatrix);

    GET_TIME(finishOvrll);

    double overAllTime = finishOvrll - startOvrll;
    double workTime = finishWork - startWork;
	double diffTime = overAllTime - workTime;

    FILE *timeFile = fopen("pthTime.csv", "a");
    if (!timeFile) {
        fprintf(stderr, "Error: Unable to open file 'pthTime.csv' for writing.\n");
        return EXIT_FAILURE;
    }

    fprintf(timeFile, "%d,%d,%d,%.6f,%.6f,%.6f,%d\n", n, rows, cols, overAllTime, workTime, diffTime, NUM_THREADS);
    fclose(timeFile);

    return 0;
}