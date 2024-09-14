#include <stdio.h>
#include <mpi.h>

int main(int argc, char **argv)
{

   double start, end;
   MPI_Init(NULL, NULL);

   int proc_rank, proc_size;
   MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
   MPI_Comm_size(MPI_COMM_WORLD, &proc_size);
   if (proc_rank == 0)
   {
      start = MPI_Wtime();
      printf("Hello from process %3d\n", proc_rank);

      for (int i = 1; i < proc_size; i++)
      {
         MPI_Status Status;
         int RecvRank;
         MPI_Recv(&RecvRank, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &Status);
         printf("Hello from process %3d\n", RecvRank);
      }

      end = MPI_Wtime();
      printf("Time spent: %f\n", end - start);
   }
   else
   {
      MPI_Send(&proc_rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
   }

   MPI_Finalize();
}