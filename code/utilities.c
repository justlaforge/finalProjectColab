#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h> // For log and power
#include "utilities.h"
//#include <mpi.h>

/*-------------------------------------------------------------------
 * Function:   Create_stencil
 * Purpose:    Create a stencil matrix with given input for rows and cols
 * In args:    prompt:  description of matrix
 *             m:       number of rows
 *             n:       number of cols
 * Out arg:    A:       the matrix
 */
void Create_stencil(
    char    prompt[]  /* in  */, 
    double  A[]       /* out */, 
    int     m         /* in  */, 
    int     n         /* in  */) {
    
    int i, j;
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            if(j==0 || j==n-1){
                A[i * n + j] = 1;
            } else {
                A[i * n + j] = 0;
            }
        }
    }
}


/*-------------------------------------------------------------------
 * Function:   readMatrix
 * Purpose:    reads the matrix from binary file into an array for accessing in code
 * In args:    file_name: the file holding the matrix
 *             rows: the number of rows in A and components in y
 *             cols: the number of columns in A components in x
 * Out args:   matrix: where the output matrix is stored
 */

void Read_matrix(char* file_name, double **matrix, int *rows, int *cols){
    FILE *file = fopen(file_name, "rb");
    if (!file) {
        fprintf(stderr, "Error: Unable to open file %s for reading.\n", file_name);
        exit(0);
    }

    if (fread(rows, sizeof(int), 1, file) != 1 || fread(cols, sizeof(int), 1, file) != 1) {
        fprintf(stderr, "Error: Failed to read matrix dimensions.\n");
        fclose(file);
        exit(0);
    }

    if (*rows <= 0 || *cols <= 0) {
        fprintf(stderr, "Error: Invalid matrix dimensions.\n");
        fclose(file);
        exit(0);
    }

    *matrix = (double *)malloc(*rows * *cols * sizeof(double));
    if (!(*matrix)) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        fclose(file);
        exit(0);
    }

    if (fread(*matrix, sizeof(double), *rows * *cols, file) != (size_t)(*rows * *cols)) {
        fprintf(stderr, "Error: Failed to read matrix data.\n");
        free(*matrix);
        fclose(file);
        exit(0);
    }

    fclose(file);
}


/*-------------------------------------------------------------------
 * Function:   Print_matrix
 * Purpose:    Print given matrix to the screen
 * In args:    matrix: the matrix to be printed
 *             rows: the number of rows in A and components in y
 *             cols: the number of columns in A components in x
 */
void Print_matrix(double* matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%6.2f ", matrix[i * cols + j]);
        }
        printf("\n");
    }
}

void write_memory_to_file(double *A, int rows, int cols, char *fname){

     // Writing matrix to binary file
     FILE *file = fopen(fname, "wb");
     if (!file) {
         fprintf(stderr, "Error: Unable to open file for writing.\n");
         exit(EXIT_FAILURE);
     }
 
     // Write the dimensions
     if (fwrite(&rows, sizeof(int), 1, file) != 1 || fwrite(&cols, sizeof(int), 1, file) != 1) {
         fprintf(stderr, "Error: Failed to write matrix dimensions.\n");
         exit(EXIT_FAILURE);
     }
 
     // Write the matrix data
     if (fwrite(A, sizeof(double), rows * cols, file) != (size_t)(rows * cols)) {
         fprintf(stderr, "Error: Failed to write matrix data.\n");
         exit(EXIT_FAILURE);
     }
 
     fclose(file);

}


/* Start of Justin's Section */



/* End of Justin's Section */

//--------------------------

/* Start of Kyle's Section */



/* End of Kyle's Section */