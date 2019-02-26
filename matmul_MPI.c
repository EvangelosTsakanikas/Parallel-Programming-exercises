#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <mpi.h>

#define N 1024

int A[N][N], B[N][N], C[N][N], (*boardWithMyResults)[N], (*subA)[N];
int readmat(char *fname, int *mat, int n), 
    writemat(char *fname, int *mat, int n);
	
int main(int argc, char *argv[])
{
	int myid, nproc;
	MPI_Status status;
	int i, j, k, sum;
	int WORK;
	double start, end, startMultiplication, endMultiplication, totalTime, totalCalculationsTime, mallocStartTime, mallocEndTime, mallocTotalTime, taskCalculationsTime = 0.0;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);
	
	if (myid == 0)
	{
		if (readmat("Amat1024", (int *) A, N) < 0) 
			exit( 1 + printf("file problem\n") );
		if (readmat("Bmat1024", (int *) B, N) < 0) 
			exit( 1 + printf("file problem\n") );
		start = MPI_Wtime(); // get starting time
	}
	
	mallocStartTime = MPI_Wtime();
	WORK = N/nproc;
	
	subA = malloc(WORK*N*sizeof(int *));
	boardWithMyResults = malloc(WORK*N*sizeof(int *));
	
	mallocEndTime = MPI_Wtime();
	mallocTotalTime = mallocEndTime - mallocStartTime; // time for malloc and a division(WORK = N/nproc)
	
	MPI_Scatter(A, WORK*N, MPI_INT, subA, WORK*N, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(B, N*N, MPI_INT, 0, MPI_COMM_WORLD);
	
	startMultiplication = MPI_Wtime();
	
	for (i = 0; i < WORK; i++)
	{
		for (j = 0; j < N; j++)
		{
			for (k = sum = 0; k < N; k++)
			{
				sum += subA[i][k]*B[k][j];
			}
			boardWithMyResults[i][j] = sum;
		}
	}
		
	endMultiplication = MPI_Wtime();
	taskCalculationsTime = (endMultiplication - startMultiplication) + mallocTotalTime; // calculations' time
	
	MPI_Reduce(&taskCalculationsTime, &totalCalculationsTime, 1, MPI_DOUBLE, MPI_SUM, 0 ,MPI_COMM_WORLD); // gather time calculations and summarize them 
	MPI_Gather(boardWithMyResults, WORK*N, MPI_INT, C, WORK*N, MPI_INT, 0, MPI_COMM_WORLD);

	if (myid == 0)
	{
		end = MPI_Wtime();
		totalTime = end - start; // total time
		totalCalculationsTime = totalCalculationsTime / (nproc);
		
		printf("time for calculations = %.6lf\n", totalCalculationsTime);
		printf("total time for whole multiplication = %.6lf\n", totalTime);
		printf("time for communications = %.6lf\n", (totalTime - totalCalculationsTime));
		writemat("Cmat1024", (int *) C, N);
	}
	
	free(subA);
	free(boardWithMyResults);
	
	MPI_Finalize();
	
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
