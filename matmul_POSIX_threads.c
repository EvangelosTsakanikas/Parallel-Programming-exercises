#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>

#define N 1024
#define M 256        
#define S N/M            
#define NUMBEROFTASKS M*M  
#define NTHREADS 4

int taskID = 0;
int A[N][N], B[N][N], C[N][N];
int readmat(char *fname, int *mat, int n), 
    writemat(char *fname, int *mat, int n);
pthread_mutex_t threadLock = PTHREAD_MUTEX_INITIALIZER;
	
void doTask(int taskToBeExecuted);
void *threadFunction(void *arg);
	
int main()
{
	int i;
	double t;
	struct timeval tv1, tv2;
	pthread_t threadIDs[NTHREADS];
	
	/* Read matrices from files: "A_file", "B_file" 
	 */
	if (readmat("Amat1024", (int *) A, N) < 0) 
		exit( 1 + printf("file problem\n") );
	if (readmat("Bmat1024", (int *) B, N) < 0) 
		exit( 1 + printf("file problem\n") );

	gettimeofday(&tv1, NULL);
	
	for (i = 0; i < NTHREADS; i++)
	{
		pthread_create(&threadIDs[i], NULL, threadFunction, NULL);
	}
	for (i = 0; i < NTHREADS; i++)
	{
		pthread_join(threadIDs[i], NULL);
	}
	
	gettimeofday(&tv2, NULL);
	
	t = (tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec)*1.0E-6;
	printf("time for multiplication : %lf\n", t);
	
	/* Save result in "Cmat1024"
	 */
	writemat("Cmat1024", (int *) C, N);

	return (0);
}

void *threadFunction(void *arg)
{
	int taskToBeExecuted;
	
	while(1)
	{
		pthread_mutex_lock(&threadLock);
		taskToBeExecuted = taskID++;
		pthread_mutex_unlock(&threadLock);
		
		if (taskToBeExecuted >= NUMBEROFTASKS)
		{
			break;
		}
		doTask(taskToBeExecuted);
	}
}

void doTask(int taskToBeExecuted)
{
	int i,j,k,wantedRow,wantedColumn;
	double sum;
	
	wantedRow = taskToBeExecuted / M;
	wantedColumn = taskToBeExecuted % M;
	
	for (i = wantedRow*S; i < (wantedRow+1)*S; i++)
	{
		for (j = wantedColumn*S; j < (wantedColumn+1)*S; j++)
		{
			for (k = 0, sum = 0.0; k < N; k++)
			{
				sum += A[i][k]*B[k][j];
			}
			C[i][j] = sum;
		}
	}
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
