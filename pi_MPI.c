#include <stdio.h>
#include <mpi.h>

#define K     10000      /* Intervals */

int main(int argc, char *argv[])
{
	int i, sum, myid, nproc, t, readyTask, NTASK;
	int taskid = 0, stop=-1;
	int N;
	double W, W2, pi = 0.0;
	double mysum = 0.0;
	double temp, start, end, totalTime, totalCalculationsTime, startCalculationsTime, endCalculationsTime, taskCalculationsTime = 0.0;
	MPI_Status status;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);
	
	if (myid == 0)
	{
		printf("Give the size of the interval : \n");
		scanf("%d", &N);
		
		start = MPI_Wtime();
		
		if (N % K == 0)
		{
			NTASK = N/K;
		}
		else
		{
			NTASK = N/K + 1;
		}
		
		W = 1.0 / ((double) N);
	}

	MPI_Bcast(&W, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD); // send W to all tasks
	
	if (myid == 0)
	{
		while(1)
		{
			t = taskid++;
			if (t >= NTASK)
			{
				break;
			}
			MPI_Recv(&readyTask, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status); // receive id from ready task
			MPI_Send(&t, 1, MPI_INT, readyTask, 0, MPI_COMM_WORLD); // send t to ready task
		}
		
		for (i = 1; i < nproc; i++)
		{
			MPI_Send(&stop, 1, MPI_INT, i, 0, MPI_COMM_WORLD); // send a signal to stop tasks
		}
	}
	else
	{
		while(1)
		{
			MPI_Send(&myid, 1, MPI_INT, 0, 0, MPI_COMM_WORLD); // send my ID to task 0
			MPI_Recv(&t, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status); // receive t to start work...
			
			if (t == -1)
			{
				break;
			}
			
			startCalculationsTime = MPI_Wtime();
			
			W2 = W*W;
			for (i = t*K; i < (t + 1)*K; i++)
			{
				mysum += 4*W / (1.0 + (0.5 + (double) i)*(0.5 + (double) i)*W2); 	   
			}
			
			endCalculationsTime = MPI_Wtime();
			taskCalculationsTime += endCalculationsTime - startCalculationsTime;
		}
	}
	
	MPI_Reduce(&taskCalculationsTime, &totalCalculationsTime, 1, MPI_DOUBLE, MPI_SUM, 0 ,MPI_COMM_WORLD); // gather time calculations and summarize them
	MPI_Reduce(&mysum, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD); // gather results and summarize them
	
	if (myid == 0)
	{
		printf("pi = %.10lf\n", pi);
		end = MPI_Wtime();
		totalTime = end - start;
		
		totalCalculationsTime = totalCalculationsTime / (nproc - 1);
		
		printf("time for the whole programm = %.6lf\n", totalTime);
		printf("time for calculations = %.6lf\n", totalCalculationsTime);
		printf("time for communications = %.6lf\n", (totalTime-totalCalculationsTime));
	}
	
	MPI_Finalize();
	return (0);
}


