/* 
 * Author:   Justin LaForge Kyle Wallace
 * 
 * File:     print-2d.c
 *
 * Purpose:  Print matrix to screen in a nice formatted way. 
 *
 * Run:      ./print_matrix <file_name>
 *
 * Input:    Binary file for matrix to be printed
 * 
 * Output:   Nicely formatted mxn matrix
 *
 * Errors:   Usage errors and file permission errors
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "utilities.c"


int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <binary_file>\n", argv[0]);
        exit(0);
    }
    double *matrix;
    int rows, cols;

    char *file_name = argv[1];

    Read_matrix(file_name, &matrix, &rows, &cols);

    
    // Print the matrix
    Print_matrix(matrix, rows, cols);
    
    free(matrix);
    return EXIT_SUCCESS;
}
