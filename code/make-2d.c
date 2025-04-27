/* 
 * Authors:   Justin LaForge, Kyle Wallace
 * File:     make-2d.c
 * Purpose:  create a stencil matrix with 1's on 
 * Run:      ./make-2d <file A> <size n>
 * Input:    file A, the output matrix
 *           size n, the size of the output matrix (nxn)
 * Output:   an n by n stencil matrix with 1's on the left and right sides and 0's everywhere else
 * Errors:   Usage errors and file permission errors; 
 * -------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "utilities.c"

int main(int argc, char** argv){
    // Usage statement
    if(argc != 3){
        printf("usage: %s <file A> <size n>\n", argv[0]);
        exit(0);
    }

    // Initiate values
    double* A = NULL;
    int m = atoi(argv[2]);
    int n = atoi(argv[2]);
 
    char *file_name = argv[1];
 
    
    A = malloc(m*n*sizeof(double));
    if (A == NULL) {
       fprintf(stderr, "Can't allocate storage\n");
       exit(-1);
    }
    Create_stencil("A", A, m, n);
 
     // Writing matrix to binary file
     FILE *file = fopen(file_name, "wb");
     if (!file) {
         fprintf(stderr, "Error: Unable to open file for writing.\n");
         free(A);
         return EXIT_FAILURE;
     }
 
     // Write the dimensions
     if (fwrite(&m, sizeof(int), 1, file) != 1 || fwrite(&n, sizeof(int), 1, file) != 1) {
         fprintf(stderr, "Error: Failed to write matrix dimensions.\n");
         fclose(file);
         free(A);
         return EXIT_FAILURE;
     }
 
     // Write the matrix data
     if (fwrite(A, sizeof(double), m * n, file) != (size_t)(m * n)) {
         fprintf(stderr, "Error: Failed to write matrix data.\n");
         fclose(file);
         free(A);
         return EXIT_FAILURE;
     }
 
     fclose(file);
 
 
    free(A);
    return 0;
 }  /* main */
 