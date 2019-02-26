// Onoma : Tsakanikas Euaggelos
// AM : 2558

/* Serial program for matrix-matrix product.
 *
 * VVD
 */
 
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#define N 1024

int taskID = 0;
int A[N][N], B[N][N], C[N][N];
int readmat(char *fname, int *mat, int n), 
    writemat(char *fname, int *mat, int n);
	
	
int main()
{
	int i, j, k, sum, Nthreads;
	double t, start, end;

	/* Read matrices from files: "A_file", "B_file" 
	 */
	if (readmat("Amat1024", (int *) A, N) < 0) 
		exit( 1 + printf("file problem\n") );
	if (readmat("Bmat1024", (int *) B, N) < 0) 
		exit( 1 + printf("file problem\n") );
	
	////////////////////////////////////////
	
	start = omp_get_wtime();
	
	for (i = 0; i < N; i++)
	{
		for (j = 0; j < N; j++)
		{
			sum = 0;
			#pragma omp parallel for reduction(+: sum) num_threads(4)
			for (k = 0; k < N; k++)
				sum += A[i][k]*B[k][j];
			C[i][j] = sum;
				
		}
	}
	
	end = omp_get_wtime();
	
	t = end - start;
	printf("time for multiplication : %lf\n", t);
	
	
	////////////////////////////////////////
	
	/* Save result in "Cmat1024"
	 */
	writemat("Cmat1024", (int *) C, N);

	return (0);
}


/* Utilities to read & write matrices from/to files
 * VVD
 */

#define _mat(i,j) (mat[(i)*n + (j)])


int readmat(char *fname, int *mat, int n)
{
	FILE *fp;
	int  i, j;
	
	if ((fp = fopen(fname, "r")) == NULL)
		return (-1);
	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
			if (fscanf(fp, "%d", &_mat(i,j)) == EOF)
			{
				fclose(fp);
				return (-1); 
			};
	fclose(fp);
	return (0);
}


int writemat(char *fname, int *mat, int n)
{
	FILE *fp;
	int  i, j;
	
	if ((fp = fopen(fname, "w")) == NULL)
		return (-1);
	for (i = 0; i < n; i++, fprintf(fp, "\n"))
		for (j = 0; j < n; j++)
			fprintf(fp, " %d", _mat(i, j));
	fclose(fp);
	return (0);
}
