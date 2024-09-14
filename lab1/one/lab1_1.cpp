#include <mpi.h>
#include <stdio.h>
#include <string.h>

const int MAX_MESSAGE_LENGTH = 100;

int main(int argc, char **argv)
{
    double start, end;
    MPI_Init(&argc, &argv);

    start = MPI_Wtime();

    int proc_rank, proc_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_size);

    printf("Process %d was start!\n", proc_rank);

    char message[MAX_MESSAGE_LENGTH] = {""};
    char received_message[MAX_MESSAGE_LENGTH] = {""};

    int destination = (proc_rank + 1) % proc_size;
    int source = (proc_rank + proc_size - 1) % proc_size;

    sprintf(message, "Hello from process %d To process %d !", proc_rank, destination);
    MPI_Send(message, strlen(message) + 1, MPI_CHAR, destination, 0, MPI_COMM_WORLD);
    printf("Process %d sent: %s\n", proc_rank, message);

    MPI_Recv(received_message, MAX_MESSAGE_LENGTH, MPI_CHAR, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Process %d received: %s\n", proc_rank, received_message);

    sprintf(message, "Message received by process %d.", proc_rank);
    MPI_Send(message, strlen(message) + 1, MPI_CHAR, source, 0, MPI_COMM_WORLD);
    printf("Process %d responded: %s\n", proc_rank, message);

    MPI_Recv(received_message, MAX_MESSAGE_LENGTH, MPI_CHAR, destination, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Process %d cathed response: %s\n", proc_rank, received_message);

    if (proc_rank == 0)
    {

        for (int i = 1; i < proc_size; i++)
        {
            int recv_rank;
            MPI_Recv(&recv_rank, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        end = MPI_Wtime();
        printf("Time spent: %f\n\n", end - start);
    }
    else
    {
        MPI_Send(&proc_rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}