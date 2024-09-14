#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <mpi.h>

using namespace std;

int main()
{

    MPI_Init(NULL, NULL);
    double timeStart, timeEnd;

    int ProcRank, ProcSize;
    MPI_Comm comm;
    int commSize;
    int N;
    double A;

    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcSize);

    srand(time(0) * ProcRank);

    if (ProcRank == 0)
    {
        timeStart = MPI_Wtime();
    }

    if (ProcRank == 0 || ProcRank == 1)
    {
        N = ProcRank + 1;
    }
    else
    {
        N = rand() % 2 + 1;
    }

    A = (rand() % 10000 + 1.1) / 100;

    MPI_Comm_split(MPI_COMM_WORLD, N, ProcRank, &comm);

    double minA = A;
    double maxA = A;

    if (N == 1)
    {
        MPI_Reduce(&A, &minA, 1, MPI_DOUBLE, MPI_MIN, 0, comm);
    }

    if (N == 2)
    {
        MPI_Reduce(&A, &maxA, 1, MPI_DOUBLE, MPI_MAX, 1, comm);
    }

    if (ProcRank == 0)
    {
        cout << "MinA: " << minA << endl;

        MPI_Recv(&maxA, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        cout << "MaxA: " << maxA << endl;

        timeEnd = MPI_Wtime();

        cout << "Time: " << timeEnd - timeStart;
    }
    else if (ProcRank == 1)
    {
        MPI_Comm_size(comm, &ProcSize);
        MPI_Send(&maxA, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
