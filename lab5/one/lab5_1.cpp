#include <iostream>
#include <mpi.h>

using namespace std;

int main(int argc, char **argv)
{
    double start, end;
    MPI_Init(&argc, &argv);

    int proc_rank, proc_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_size);

    srand(time(NULL) + proc_rank);

    if (proc_rank == 0)
    {
        start = MPI_Wtime();
    }

    int dims[] = {proc_size};
    int periods[] = {0};
    MPI_Comm cart_comm;
    MPI_Cart_create(MPI_COMM_WORLD, 1, dims, periods, 0, &cart_comm);

    double original_data = static_cast<double>(rand()) / RAND_MAX;
    double received_data = -1;

    int source, dest;
    MPI_Cart_shift(cart_comm, 0, -1, &source, &dest);

    MPI_Send(&original_data, 1, MPI_DOUBLE, dest, 0, cart_comm);
    MPI_Recv(&received_data, 1, MPI_DOUBLE, source, 0, cart_comm, MPI_STATUS_IGNORE);

    if (proc_rank == 0)
    {
        printf("Process %d: Received data: %f\n", proc_rank, received_data);

        for (int i = 1; i < proc_size - 1; i++)
        {
            MPI_Status Status;
            double data;
            MPI_Recv(&data, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &Status);
            printf("Process %d: Received data: %f\n", Status.MPI_SOURCE, data);
        }

        end = MPI_Wtime();
        printf("Time spent: %f\n\n\n", end - start);
    }
    else if (received_data >= 0)
    {
        MPI_Send(&received_data, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
