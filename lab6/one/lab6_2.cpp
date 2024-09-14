#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <random>

int allocMatrix(int ***mat, int rows, int cols)
{

    int *p = (int *)malloc(sizeof(int *) * rows * cols);
    if (!p)
    {
        return -1;
    }

    *mat = (int **)malloc(rows * sizeof(int *));
    if (!mat)
    {
        free(p);
        return -1;
    }

    for (int i = 0; i < rows; i++)
    {
        (*mat)[i] = &(p[i * cols]);
    }
    return 0;
}

int freeMatrix(int ***mat)
{
    free(&((*mat)[0][0]));
    free(*mat);
    return 0;
}

void matrixMultiply(int **a, int **b, int rows, int cols, int ***c)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            int val = 0;
            for (int k = 0; k < rows; k++)
            {
                val += a[i][k] * b[k][j];
            }
            (*c)[i][j] = val;
        }
    }
}

void printMatrix(int **mat, int size)
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            printf("%d ", mat[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    MPI_Comm cartComm;
    int dim[2], period[2], reorder;
    int coord[2], id;
    int **A = NULL, **B = NULL, **C = NULL;
    int **localA = NULL, **localB = NULL, **localC = NULL;
    int **localARec = NULL, **localBRec = NULL;
    int rows = 0;
    int columns;
    int count = 0;
    int proc_size, proc_rank;
    int procDim;
    int blockDim;
    int left, right, up, down;
    int bCastData[4];
    double start, end;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &proc_size);

    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
    if (proc_rank == 0)
    {
        int n;
        char ch;

        scanf("%d", &rows);
        columns = rows;
        count = rows * columns;

        printf("Col: %d, row: %d\n", columns, rows);

        double sqroot = sqrt(proc_size);
        if ((sqroot - floor(sqroot)) != 0)
        {
            printf("[ERROR] Number of processes must be a perfect square!\n");
            MPI_Abort(MPI_COMM_WORLD, 2);
        }
        int intRoot = (int)sqroot;
        if (rows % intRoot != 0)
        {
            printf("[ERROR] Number of rows must be divisible by %d!\n", intRoot);
            MPI_Abort(MPI_COMM_WORLD, 3);
        }
        procDim = intRoot;
        blockDim = columns / intRoot;

        if (allocMatrix(&A, rows, columns) != 0)
        {
            printf("[ERROR] Matrix alloc for A failed!\n");
            MPI_Abort(MPI_COMM_WORLD, 4);
        }
        if (allocMatrix(&B, rows, columns) != 0)
        {
            printf("[ERROR] Matrix alloc for B failed!\n");
            MPI_Abort(MPI_COMM_WORLD, 5);
        }

        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < columns; j++)
            {
                A[i][j] = rand() % 10;
            }
        }
        printf("A matrix:\n");
        // printMatrix(A, rows);

        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < columns; j++)
            {
                B[i][j] = rand() % 10;
            }
        }
        printf("B matrix:\n");
        // printMatrix(B, rows);

        if (allocMatrix(&C, rows, columns) != 0)
        {
            printf("[ERROR] Matrix alloc for C failed!\n");
            MPI_Abort(MPI_COMM_WORLD, 6);
        }

        start = MPI_Wtime();

        bCastData[0] = procDim;
        bCastData[1] = blockDim;
        bCastData[2] = rows;
        bCastData[3] = columns;
    }

    MPI_Bcast(&bCastData, 4, MPI_INT, 0, MPI_COMM_WORLD);
    procDim = bCastData[0];
    blockDim = bCastData[1];
    rows = bCastData[2];
    columns = bCastData[3];

    dim[0] = procDim;
    dim[1] = procDim;
    period[0] = 1;
    period[1] = 1;
    reorder = 1;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dim, period, reorder, &cartComm);

    allocMatrix(&localA, blockDim, blockDim);
    allocMatrix(&localB, blockDim, blockDim);

    // Create datatype to describe the subarrays of the global array
    int globalSize[2] = {rows, columns};
    int localSize[2] = {blockDim, blockDim};
    int starts[2] = {0, 0};
    MPI_Datatype type, subarrtype;
    MPI_Type_create_subarray(2, globalSize, localSize, starts, MPI_ORDER_C, MPI_INT, &type);
    MPI_Type_create_resized(type, 0, blockDim * sizeof(int), &subarrtype);
    MPI_Type_commit(&subarrtype);

    int *globalptrA = NULL;
    int *globalptrB = NULL;
    int *globalptrC = NULL;
    if (proc_rank == 0)
    {
        globalptrA = &(A[0][0]);
        globalptrB = &(B[0][0]);
        globalptrC = &(C[0][0]);
    }

    // Scatter the array to all processors
    int *sendCounts = (int *)malloc(sizeof(int) * proc_size);
    int *displacements = (int *)malloc(sizeof(int) * proc_size);

    if (proc_rank == 0)
    {
        for (int i = 0; i < proc_size; i++)
        {
            sendCounts[i] = 1;
        }
        int disp = 0;
        for (int i = 0; i < procDim; i++)
        {
            for (int j = 0; j < procDim; j++)
            {
                displacements[i * procDim + j] = disp;
                disp += 1;
            }
            disp += (blockDim - 1) * procDim;
        }
    }

    MPI_Scatterv(globalptrA, sendCounts, displacements, subarrtype, &(localA[0][0]),
                 rows * columns / (proc_size), MPI_INT,
                 0, MPI_COMM_WORLD);
    MPI_Scatterv(globalptrB, sendCounts, displacements, subarrtype, &(localB[0][0]),
                 rows * columns / (proc_size), MPI_INT,
                 0, MPI_COMM_WORLD);

    if (allocMatrix(&localC, blockDim, blockDim) != 0)
    {
        printf("[ERROR] Matrix alloc for localC in rank %d failed!\n", proc_rank);
        MPI_Abort(MPI_COMM_WORLD, 7);
    }

    MPI_Cart_coords(cartComm, proc_rank, 2, coord);
    MPI_Cart_shift(cartComm, 1, coord[0], &left, &right);
    MPI_Sendrecv_replace(&(localA[0][0]), blockDim * blockDim, MPI_INT, left, 1, right, 1, cartComm, MPI_STATUS_IGNORE);
    MPI_Cart_shift(cartComm, 0, coord[1], &up, &down);
    MPI_Sendrecv_replace(&(localB[0][0]), blockDim * blockDim, MPI_INT, up, 1, down, 1, cartComm, MPI_STATUS_IGNORE);

    for (int i = 0; i < blockDim; i++)
    {
        for (int j = 0; j < blockDim; j++)
        {
            localC[i][j] = 0;
        }
    }

    int **multiplyRes = NULL;
    if (allocMatrix(&multiplyRes, blockDim, blockDim) != 0)
    {
        printf("[ERROR] Matrix alloc for multiplyRes in rank %d failed!\n", proc_rank);
        MPI_Abort(MPI_COMM_WORLD, 8);
    }
    for (int k = 0; k < procDim; k++)
    {
        matrixMultiply(localA, localB, blockDim, blockDim, &multiplyRes);

        for (int i = 0; i < blockDim; i++)
        {
            for (int j = 0; j < blockDim; j++)
            {
                localC[i][j] += multiplyRes[i][j];
            }
        }
        // Shift A once (left) and B once (up)
        MPI_Cart_shift(cartComm, 1, 1, &left, &right);
        MPI_Cart_shift(cartComm, 0, 1, &up, &down);
        MPI_Sendrecv_replace(&(localA[0][0]), blockDim * blockDim, MPI_INT, left, 1, right, 1, cartComm, MPI_STATUS_IGNORE);
        MPI_Sendrecv_replace(&(localB[0][0]), blockDim * blockDim, MPI_INT, up, 1, down, 1, cartComm, MPI_STATUS_IGNORE);
    }

    MPI_Gatherv(&(localC[0][0]), rows * columns / proc_size, MPI_INT,
                globalptrC, sendCounts, displacements, subarrtype,
                0, MPI_COMM_WORLD);

    freeMatrix(&localC);
    freeMatrix(&multiplyRes);

    if (proc_rank == 0)
    {
        end = MPI_Wtime();
        printf("Time spent: %f\n\n\n", end - start);

        printf("C is:\n");
        // printMatrix(C, rows);
    }

    MPI_Finalize();

    return 0;
}