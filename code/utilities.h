// Function protocols


void Create_stencil(char prompt[], double A[], int m, int n);
void Read_matrix(char* file_name, double **matrix, int *rows, int *cols);
void Print_matrix(double* matrix, int rows, int cols);
void write_memory_to_file(double *A, int rows, int cols, char *fname);


#ifndef _TIMER_H_
#define _TIMER_H_

#include <sys/time.h>

/* The argument now should be a double (not a pointer to a double) */
#define GET_TIME(now) { \
   struct timeval t; \
   gettimeofday(&t, NULL); \
   now = t.tv_sec + t.tv_usec/1000000.0; \
}

#endif


/* Start of Justin's Section */



/* End of Justin's Section */

//--------------------------

/* Start of Kyle's Section */



/* End of Kyle's Section */
