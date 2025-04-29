/* 
 * Author:   Justin LaForge Kyle Wallace
 * 
 * File:     stencil-2d-omp.c
 *
 * Purpose:  Perform stencil simulation using OpenMP for parallization
 *
 * Run:      ./stencil-2d-omp.c -t <num iters> -i <in> -o <out> -p <num process>
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
#include <omp.h>
 
 void usage(char **argv){
	 printf("Usage: %s -n <num iters> -i <in file> -o <out file> -v <debug: 0,1,2>\n", argv[0]);
 }
 
 // Set arguments
void setArgs(int argc, char **argv, int *n, char **in, char **out, int *debug){
	int opt;
 
	while((opt = getopt(argc, argv, "n:i:o:v:p:")) != -1){
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
			case 'v':
				*debug = atoi(optarg);
				break;
            case 'p':
                omp_set_num_threads(atoi(optarg));
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

    omp_set_dynamic(0);
	 
	int n=1,debug=0;
	char *in = NULL;
	char *out = NULL;
	 
	//set args
	setArgs(argc, argv, &n, &in, &out, &debug);
 
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
    
    // Loop iterations
    #pragma omp parallel
    {
        for (int o = 1; o <= n; o++) {
            #pragma omp for collapse(2) // Parallelize the nested loops
            for (int i = 1; i < rows - 1; i++) {
                for (int j = 1; j < cols - 1; j++) {
                    newMatrix[i * cols + j] = (
                        matrix[(i - 1) * cols + (j - 1)] + matrix[(i - 1) * cols + j] + matrix[(i - 1) * cols + (j + 1)] +
                        matrix[i * cols + (j - 1)] + matrix[i * cols + j] + matrix[i * cols + (j + 1)] +
                        matrix[(i + 1) * cols + (j - 1)] + matrix[(i + 1) * cols + j] + matrix[(i + 1) * cols + (j + 1)]
                    ) / 9.0;
                }
            }

            #pragma omp single // Ensure only one thread swaps the pointers
            {
                double* temp = matrix;
                matrix = newMatrix;
                newMatrix = temp;

            }
        }
    }
	 
	GET_TIME(finishWork);
 
	write_memory_to_file(matrix, rows, cols, out);

	 
 
	free(matrix);
	if (newMatrix != NULL && newMatrix != matrix) free(newMatrix);

	GET_TIME(finishOvrll);
 
	 
	double overAllTime = finishOvrll-startOvrll;
	double workTime = finishWork-startWork;
  
  
	// Open file to write timing data
	FILE *timeFile = fopen("ompTime.csv", "a");
	if (!timeFile) {
		fprintf(stderr, "Error: Unable to open file 'ompTime' for writing.\n");
		return EXIT_FAILURE;
	}
  
	// Write the values to the file
	fprintf(timeFile, "%d,%d,%d,%f,%f,%d\n", n, rows, cols, overAllTime, workTime, omp_get_max_threads());
  
	// Close the file
	fclose(timeFile);
 
	return 0;
 
}