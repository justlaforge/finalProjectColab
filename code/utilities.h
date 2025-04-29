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


/* For PThreads */

#define MIN(a,b) ((a)<(b)?(a):(b))
#define BLOCK_LOW(id,p,n) ((id)*(n)/(p))
// given rank = id, give p = # processes (or threads), and given n, number of elements in 1 dimension, it will tell you the
//starting index
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n)-1)
// given rank = id, give p = # processes (or threads), and given n, number of elements in 1 dimension, it will tell you the ending index
#define BLOCK_SIZE(id,p,n) \
(BLOCK_HIGH(id,p,n)-BLOCK_LOW(id,p,n)+1)
// given rank = id, give p = # processes (or threads), and given n, number of elements in 1 dimension, it will tell you number of
//elements that the given thread or process is responsible for
#define BLOCK_OWNER(j,p,n) (((p)*((j)+1)-1)/(n))
// given data index j (from [0, n-1], and given p = # processes (or threads), and given n, number of elements in 1 dimension, it will tell
//you which RANK is responsible for that particular element, j.
#define PTR_SIZE (sizeof(void*))
#define CEILING(i,j) (((i)+(j)-1)/(j))

