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
 #include <string.h>
 #include <getopt.h>
 #include <mpi.h>
 #include "utilities.c" // NOTE: Ideally, you should include "utilities.h" instead.
 
 void usage(char **argv) {
     printf("Usage: %s -n <num iters> -i <in file> -o <out file>\n", argv[0]);
 }
 
 void setArgs(int argc, char **argv, int *n, char **in, char **out) {
     int opt;
     while ((opt = getopt(argc, argv, "n:i:o:")) != -1) {
         switch (opt) {
             case 'n':
                 *n = atoi(optarg);
                 break;
             case 'i':
                 *in = optarg;
                 break;
             case 'o':
                 *out = optarg;
                 break;
             default:
                 usage(argv);
                 exit(EXIT_FAILURE);
         }
     }
     if (*in == NULL || *out == NULL) {
         fprintf(stderr, "Error: Both input (-i) and output (-o) files must be specified.\n");
         usage(argv);
         exit(EXIT_FAILURE);
     }
 }
 
 int main(int argc, char **argv) {
     MPI_Init(&argc, &argv);
 
     int rank, size;
     MPI_Comm_rank(MPI_COMM_WORLD, &rank);
     MPI_Comm_size(MPI_COMM_WORLD, &size);
 
     // Timer variables
     double startOvrll = 0, finishOvrll = 0, startWork = 0, finishWork = 0;
     MPI_Barrier(MPI_COMM_WORLD);
     startOvrll = MPI_Wtime();
 
     int n = 1;
     char *in = NULL;
     char *out = NULL;
 
     // Parse arguments
     setArgs(argc, argv, &n, &in, &out);
 
     double *matrix = NULL;
     int rows = 0, cols = 0;
 
     if (rank == 0) {
         Read_matrix(in, &matrix, &rows, &cols);
     }
 
     // Broadcast matrix dimensions
     MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
     MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
 
     // Determine local rows
     int local_rows = rows / size;
     int remainder = rows % size;
     if (rank < remainder) {
         local_rows++;
     }
 
     // Allocate space for local matrix (+2 rows for halo exchange)
     double *local_matrix = malloc((local_rows + 2) * cols * sizeof(double));
     double *local_newMatrix = malloc((local_rows + 2) * cols * sizeof(double));
 
     // Scatter data
     int *sendcounts = malloc(size * sizeof(int));
     int *displs = malloc(size * sizeof(int));
 
     int offset = 0;
     for (int i = 0; i < size; i++) {
         int rows_i = rows / size + (i < remainder ? 1 : 0);
         sendcounts[i] = rows_i * cols;
         displs[i] = offset;
         offset += sendcounts[i];
     }
 
     // Initialize local_matrix (shift by one row for halos)
     MPI_Scatterv(matrix, sendcounts, displs, MPI_DOUBLE,
                  local_matrix + cols, local_rows * cols, MPI_DOUBLE,
                  0, MPI_COMM_WORLD);
 
     // Copy initial data
     memcpy(local_newMatrix, local_matrix, (local_rows + 2) * cols * sizeof(double));
 
     MPI_Barrier(MPI_COMM_WORLD);
     startWork = MPI_Wtime();
 
     // Stencil iterations
     for (int iter = 0; iter < n; iter++) {
         MPI_Request requests[4];
         int req_count = 0;
 
         // Exchange ghost rows
         if (rank > 0) {
             MPI_Isend(local_matrix + cols, cols, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &requests[req_count++]);
             MPI_Irecv(local_matrix, cols, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &requests[req_count++]);
         }
         if (rank < size - 1) {
             MPI_Isend(local_matrix + local_rows * cols, cols, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &requests[req_count++]);
             MPI_Irecv(local_matrix + (local_rows + 1) * cols, cols, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &requests[req_count++]);
         }
 
         MPI_Waitall(req_count, requests, MPI_STATUSES_IGNORE);
 
         for (int i = 1; i <= local_rows; i++) {
            int global_row = i - 1 + rank * (rows / size) + (rank < (rows % size) ? rank : (rows % size));
            
            if (global_row == 0 || global_row == rows - 1)
                continue; // Skip updating first and last rows globally
        
            for (int j = 1; j < cols - 1; j++) {
                local_newMatrix[i * cols + j] = (
                    local_matrix[(i - 1) * cols + (j - 1)] + local_matrix[(i - 1) * cols + j] + local_matrix[(i - 1) * cols + (j + 1)] +
                    local_matrix[i * cols + (j - 1)] + local_matrix[i * cols + j] + local_matrix[i * cols + (j + 1)] +
                    local_matrix[(i + 1) * cols + (j - 1)] + local_matrix[(i + 1) * cols + j] + local_matrix[(i + 1) * cols + (j + 1)]
                ) / 9.0;
            }
        }
        

 
         // Swap matrices
         double *temp = local_matrix;
         local_matrix = local_newMatrix;
         local_newMatrix = temp;
     }
 
     MPI_Barrier(MPI_COMM_WORLD);
     finishWork = MPI_Wtime();
 
     // Gather final results
     if (rank == 0) {
         MPI_Gatherv(local_matrix + cols, local_rows * cols, MPI_DOUBLE,
                     matrix, sendcounts, displs, MPI_DOUBLE,
                     0, MPI_COMM_WORLD);
     } else {
         MPI_Gatherv(local_matrix + cols, local_rows * cols, MPI_DOUBLE,
                     NULL, sendcounts, displs, MPI_DOUBLE,
                     0, MPI_COMM_WORLD);
     }
 
     // Cleanup
     free(local_matrix);
     free(local_newMatrix);
     free(sendcounts);
     free(displs);
 
     if (rank == 0) {
         free(matrix);
     }
 
     MPI_Barrier(MPI_COMM_WORLD);
     finishOvrll = MPI_Wtime();
 
     if (rank == 0) {
         double overAllTime = finishOvrll - startOvrll;
         double workTime = finishWork - startWork;
         double diffTime = overAllTime - workTime;
 
         FILE *timeFile = fopen("mpiTime.csv", "a");
         if (timeFile) {
             fprintf(timeFile, "%d,%d,%d,%.6f,%.6f,%.6f,%d\n", n, rows, cols, overAllTime, workTime, diffTime, size);
             fclose(timeFile);
         } else {
             fprintf(stderr, "Error: Unable to open file 'mpiTime.csv' for writing.\n");
         }
     }
 
     MPI_Finalize();
     return 0;
 }
 