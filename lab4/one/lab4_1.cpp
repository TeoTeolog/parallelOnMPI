#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <mpi.h>

using namespace std;

int main(int argc, char **argv)
{

    vector<int> gatheredStartResult;
    double start, end;

    MPI_Init(&argc, &argv);

    int proc_rank, proc_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_size);

    int N;
    double A;

    srand(time(NULL) + proc_rank);

    if (proc_rank == 0)
    {
        start = MPI_Wtime();

        N = 1;
        A = static_cast<double>(rand()) / RAND_MAX;
        gatheredStartResult.resize(proc_size);
    }
    else
    {
        N = rand() % 2;
        A = (N == 1) ? static_cast<double>(rand()) / RAND_MAX : 0;
    }

    MPI_Gather(&N, 1, MPI_INT, gatheredStartResult.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (proc_rank == 0)
    {
        for (int i = 0; i < proc_size; i++)
            printf("Proc %d: N=%d ", i, gatheredStartResult[i]);
        printf("\n");
    }

    MPI_Comm newComm;
    int new_proc_size;
    if (N)
    {
        MPI_Comm_split(MPI_COMM_WORLD, N, proc_rank, &newComm);
        vector<double> gatheredNumbers(proc_size);
        MPI_Gather(&A, 1, MPI_DOUBLE, gatheredNumbers.data(), 1, MPI_DOUBLE, 0, newComm);

        MPI_Comm_size(newComm, &new_proc_size);

        if (proc_rank == 0)
        {
            cout << "Received numbers: ";
            for (int i = new_proc_size - 1; i >= 0; i--)
            {
                cout << gatheredNumbers[i] << " ";
            }
            cout << endl;

            end = MPI_Wtime();
            printf("Time spent: %f\n\n\n", end - start);
        }
    }
    else
        MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, proc_rank, &newComm);

    MPI_Finalize();
    return 0;
}
