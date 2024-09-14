#include <iostream>
#include <mpi.h>

using namespace std;

void greating(int n)
{
   printf("Hello from process %3d\n", n);
}

int main()
{

   MPI_Init(NULL, NULL);

   int ProcRank, ProcSize;
   MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
   MPI_Comm_size(MPI_COMM_WORLD, &ProcSize);

   if (ProcRank == 0)
   {
      double timeStart, timeEnd;
      timeStart = MPI_Wtime();

      greating(ProcRank);

      for (int i = 0; i < ProcSize - 1; i++)
      {
         int RecvRank;
         MPI_Recv(&RecvRank, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
         greating(RecvRank);
      }

      timeEnd = MPI_Wtime();

      cout << "Time: " << timeEnd - timeStart;
   }
   else
   {
      MPI_Send(&ProcRank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
   }

   MPI_Finalize();
}