#include <iostream>
#include <vector>
#include <mpi.h>

using namespace std;

int main(int argc, char **argv)
{
    double start, end;
    MPI_Init(&argc, &argv);

    start = MPI_Wtime();

    int proc_size, proc_rank;

    MPI_Comm_size(MPI_COMM_WORLD, &proc_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

    const int K = proc_size;

    vector<int> myNumbers(K);
    for (int i = 0; i < K; i++)
    {
        myNumbers[i] = proc_rank * K + i;
    }

    if (proc_rank == 0)
    {

        cout << "Process " << proc_rank << " create numbers: ";
        for (int i = 0; i < K; ++i)
        {
            cout << myNumbers[i] << " ";
        }
        cout << endl;

        for (int i = 1; i < proc_size; i++)
        {
            vector<int> receivedSortedNumbers(K);
            MPI_Recv(&receivedSortedNumbers[0], K, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            cout << "Process " << i << " create numbers: ";
            for (int i = 0; i < K; ++i)
            {
                cout << receivedSortedNumbers[i] << " ";
            }
            cout << endl;
        }
    }
    else
    {
        MPI_Send(&myNumbers[0], K, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    vector<int> receivedNumbers(K);

    MPI_Alltoall(&myNumbers[0], 1, MPI_INT, &receivedNumbers[0], 1, MPI_INT, MPI_COMM_WORLD);

    if (proc_rank == 0)
    {

        cout << "Process " << proc_rank << " received numbers: ";
        for (int i = 0; i < K; ++i)
        {
            cout << receivedNumbers[i] << " ";
        }
        cout << endl;

        for (int i = 1; i < proc_size; i++)
        {
            vector<int> receivedSortedNumbers(K);
            MPI_Recv(&receivedSortedNumbers[0], K, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            cout << "Process " << i << " received numbers: ";
            for (int i = 0; i < K; ++i)
            {
                cout << receivedSortedNumbers[i] << " ";
            }
            cout << endl;
        }
        end = MPI_Wtime();
        printf("Time spent: %f\n\n\n", end - start);
    }
    else
    {
        MPI_Send(&receivedNumbers[0], K, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}
