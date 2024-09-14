#include <iostream>
#include <vector>
#include <random>
#include <mpi.h>

using namespace std;

int getRandomDevider(int N)
{
    vector<int> variantsOfN;

    for (int i = 2; i <= N; i++)
    {
        if (N % i == 0)
            variantsOfN.push_back(i);
    }

    int index = rand() % variantsOfN.size();

    return variantsOfN[index];
}

void syncOutput(int mainProcessRank, int ProcSize, int ProcRank, int cords[2], int N)
{
    if (ProcRank == mainProcessRank)
    {
        cout << "Chosen N: " << N << endl;
        cout << "Process " << ProcRank << ": Coordinates in the topology: (" << cords[0] << ", " << cords[1] << ")" << endl;

        for (int i = 1; i < ProcSize; i++)
        {
            MPI_Status status;
            int data[2] = {-1, -1};
            MPI_Recv(&data, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            cout << "Process " << status.MPI_SOURCE << ": Coordinates in the topology: (" << data[0] << ", " << data[1] << ")" << endl;
        }
    }
    else
    {
        MPI_Send(cords, 2, MPI_INT, mainProcessRank, 0, MPI_COMM_WORLD);
    }
}

int main()
{
    MPI_Init(NULL, NULL);
    double timeStart, timeEnd;

    int ProcRank, ProcSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcSize);

    srand(time(0) + ProcRank * ProcSize);

    int N;

    if (ProcRank == 0)
    {
        timeStart = MPI_Wtime();
        N = getRandomDevider(ProcSize);
    }

    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int dims[2] = {N, ProcSize / N};
    int periods[2] = {0, 0};
    int coords[2];

    MPI_Comm cartComm;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &cartComm);

    MPI_Cart_coords(cartComm, ProcRank, 2, coords);

    syncOutput(0, ProcSize, ProcRank, coords, N);

    if (ProcRank == 0)
    {
        timeEnd = MPI_Wtime();
        cout << "Time: " << timeEnd - timeStart;
    }

    MPI_Finalize();
    return 0;
}
