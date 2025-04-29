/* 
 * Author:   Justin LaForge Kyle Wallace
 * 
 * File:     stencil-2d-mpi.c
 *
 * Purpose:  Perform stencil simulation using MPI for parallization
 *
 * Run:      ./stencil-2d-mpi.c -t <num iters> -i <in> -o <out> -p <num process>
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
 #include <mpi.h>
  
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
            // case 'p':
            // MPI_Comm_size(MPI_COMM_WORLD, &optarg);
            // break;
        default:
            usage(argv);
            exit(1);
    }
}
if (*in == NULL || *out == NULL) {
    perror("Files -i and -o must be provided");
    exit(EXIT_FAILURE);
}
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // ---- Timer Variables ----
    double startOvrll=0;
    double finishOvrll=0;
    double startWork=0;
    double finishWork=0;
    MPI_Barrier(MPI_COMM_WORLD);
    startOvrll = MPI_Wtime();

    int n=1,debug=0;
	char *in = NULL;
	char *out = NULL;

    //set args
    setArgs(argc, argv, &n, &in, &out, &debug);

    double *matrix;
	double *newMatrix;
	int rows, cols;

    if (rank == 0) {
        Read_matrix(in, &matrix, &rows, &cols);
    }

    // Broadcast rows and cols to all processes
    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int local_rows = rows / size;
    int remainder = rows % size;

    if (rank < remainder) {
        local_rows++;
    }

    double *local_matrix = malloc(local_rows * cols * sizeof(double));
    double *local_newMatrix = malloc(local_rows * cols * sizeof(double));

    int *sendcount = malloc(size * sizeof(int));
    int *displacement = malloc(size * sizeof(int));

    int sum = 0;
    for (int i = 0; i < size; i++) {
        sendcount[i] = (rows / size + (i < (rows % size))) * cols;
        displacement[i] = sum;
        sum += sendcount[i];
    }


    if (rank == 0) {
        MPI_Scatterv(matrix, sendcount, displacement, MPI_DOUBLE, local_matrix, local_rows * cols, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    } else {
        MPI_Scatterv(NULL, sendcount, displacement, MPI_DOUBLE, local_matrix, local_rows * cols, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    memcpy(local_newMatrix, local_matrix, local_rows * cols * sizeof(double));

    if (debug == 2 && rank == 0) {
        printf("Iteration 0:\n");
        Print_matrix(matrix, rows, cols);
        printf("\n");
    }

    MPI_Barrier(MPI_COMM_WORLD);
    startWork = MPI_Wtime();

    for (int o = 1; o <= n; o++) {
        for (int i = 1; i < local_rows - 1; i++) {
            for (int j = 1; j < cols - 1; j++) {
                local_newMatrix[i * cols + j] = (
                    local_matrix[(i - 1) * cols + (j - 1)] + local_matrix[(i - 1) * cols + j] + local_matrix[(i - 1) * cols + (j + 1)] +
                    local_matrix[i * cols + (j - 1)] + local_matrix[i * cols + j] + local_matrix[i * cols + (j + 1)] +
                    local_matrix[(i + 1) * cols + (j - 1)] + local_matrix[(i + 1) * cols + j] + local_matrix[(i + 1) * cols + (j + 1)]
                ) / 9.0;
            }
        }

        double *temp = local_matrix;
        local_matrix = local_newMatrix;
        local_newMatrix = temp;

        if (debug == 2 && rank == 0) {
            MPI_Gatherv(local_matrix, local_rows * cols, MPI_DOUBLE, matrix, sendcount, displacement, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            printf("Gathered matrix on rank 0 for iteration %d.\n", o);
            printf("Iteration %d:\n", o);
            Print_matrix(matrix, rows, cols);
            printf("\n");
        } else if (debug == 2) {
            MPI_Gatherv(local_matrix, local_rows * cols, MPI_DOUBLE, NULL, sendcount, displacement, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        }

    }

    MPI_Barrier(MPI_COMM_WORLD);
    finishWork = MPI_Wtime();

    if (rank == 0) {
        MPI_Gatherv(local_matrix, local_rows * cols, MPI_DOUBLE, matrix, sendcount, displacement, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        printf("Gathered matrix on rank 0.\n");
        Print_matrix(matrix, rows, cols);
        printf("\n");
        write_memory_to_file(matrix, rows, cols, out);
    } else {
        MPI_Gatherv(local_matrix, local_rows * cols, MPI_DOUBLE, NULL, sendcount, displacement, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    free(local_matrix);
    free(local_newMatrix);

    if (rank == 0) {
        free(matrix);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    finishOvrll = MPI_Wtime();

    if (rank == 0) {
        double overAllTime = finishOvrll - startOvrll;
        double workTime = finishWork - startWork;

        FILE *timeFile = fopen("mpiTime.csv", "a");
        if (!timeFile) {
            fprintf(stderr, "Error: Unable to open file 'mpiTime.csv' for writing.\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        // Write the values to the file
        fprintf(timeFile, "%d,%d,%d,%f,%f,%d\n", n, rows, cols, overAllTime, workTime, size);
        
        // Close the file
        fclose(timeFile);
    }

    MPI_Finalize();
    return 0;
}
