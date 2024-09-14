#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

typedef struct
{
    int nb_proc;
    MPI_Comm grid_comm;
    MPI_Comm row_comm;
    MPI_Comm col_comm;
    int order;
    int pos_row;
    int pos_col;
    int grid_rank;
} GRID_INFO_T;

// Initialization of the processus grid and all the communicators.
void init_grid(GRID_INFO_T * /*out*/);
// Read matrix from a file defined by a string. Result in filling a 2D matrix onto a continuous
// array of int
void read_matrix(GRID_INFO_T * /*in*/, int * /*out*/, int /*in*/);
// Display global matrix
void display_matrix(GRID_INFO_T * /*in*/, int * /*in*/);
// Multiply the first and second matrix (array of int) into the third one
void multiply_matrix(GRID_INFO_T * /*in*/, int * /*in*/, int * /*in*/, int * /*out*/);

// Cannon's algorithm. Same as above
void cannon(GRID_INFO_T * /*in*/, int * /*in*/, int * /*in*/, int * /*out*/);

int main(int argc, char **argv)
{

    GRID_INFO_T grid;

    int *block_A;
    int *block_B;
    int *block_C;
    int *mat_A;
    int *mat_B;
    int *mat_C;

    double timeStart, timeEnd;

    // Initialization MPI and our GRID_INFO_T struct
    MPI_Init(&argc, &argv);
    init_grid(&grid);

    if (grid.grid_rank == 0)
    {
        timeStart = MPI_Wtime();
    }

    if (grid.grid_rank == 0)
    {
        mat_A = (int *)malloc(grid.order * grid.order * sizeof(int));
        mat_B = (int *)malloc(grid.order * grid.order * sizeof(int));
        mat_C = (int *)malloc(grid.order * grid.order * sizeof(int));
        read_matrix(&grid, mat_A, grid.order);
        read_matrix(&grid, mat_B, grid.order);

        int mat_col;
        int mat_row;

        // We make sure that mat_C is initialized correctly. No need for mat_A and mat_B that are
        // filled from files
        for (mat_row = 0; mat_row < grid.order; ++mat_row)
        {
            for (mat_col = 0; mat_col < grid.order; ++mat_col)
            {

                mat_C[mat_row * grid.order + mat_col] = 0.0;
            }
        }
    }

    MPI_Datatype blocktype, type;

    int array_size[2] = {grid.nb_proc, grid.nb_proc};
    int subarray_sizes[2] = {(int)sqrt(grid.nb_proc), (int)sqrt(grid.nb_proc)};
    int array_start[2] = {0, 0};

    MPI_Type_create_subarray(2, array_size, subarray_sizes, array_start, MPI_ORDER_C, MPI_INT,
                             &blocktype);
    MPI_Type_create_resized(blocktype, 0, (int)sqrt(grid.nb_proc) * sizeof(int), &type);
    MPI_Type_commit(&type);

    int i, j;
    int displs[grid.nb_proc];
    int send_counts[grid.nb_proc];

    block_A = (int *)malloc(grid.nb_proc * sizeof(int));
    block_B = (int *)malloc(grid.nb_proc * sizeof(int));
    block_C = (int *)malloc(grid.nb_proc * sizeof(int));

    for (i = 0; i < grid.nb_proc; ++i)
        block_C[i] = 0.0;

    if (grid.grid_rank == 0)
    {
        for (i = 0; i < grid.nb_proc; i++)
        {
            send_counts[i] = 1;
        }

        int disp = 0;
        for (i = 0; i < (int)sqrt(grid.nb_proc); i++)
        {
            for (j = 0; j < (int)sqrt(grid.nb_proc); j++)
            {
                displs[i * (int)sqrt(grid.nb_proc) + j] = disp;
                disp += 1;
            }
            disp += ((grid.nb_proc / (int)sqrt(grid.nb_proc) - 1)) * (int)sqrt(grid.nb_proc);
        }
    }

    MPI_Scatterv(mat_A, send_counts, displs, type, block_A, grid.nb_proc, MPI_INT, 0,
                 MPI_COMM_WORLD);
    MPI_Scatterv(mat_B, send_counts, displs, type, block_B, grid.nb_proc, MPI_INT, 0,
                 MPI_COMM_WORLD);

    int algorithm = 0;

    cannon(&grid, block_A, block_B, block_C);

    MPI_Gatherv(block_C, grid.nb_proc, MPI_INT, mat_C, send_counts, displs, type, 0,
                MPI_COMM_WORLD);

    if (grid.grid_rank == 0)
    {
        timeEnd = MPI_Wtime();

        // Если матрицы не гигансткие - выведем их на экран
        if (grid.order < 10)
        {
            printf("A ");
            display_matrix(&grid, mat_A);
            printf("B ");
            display_matrix(&grid, mat_B);
            printf("C ");
            display_matrix(&grid, mat_C);
        }

        printf("Time: %f\n", timeEnd - timeStart);
    }

    MPI_Finalize();
    return 0;
}

void cannon(GRID_INFO_T *grid, int *block_A, int *block_B, int *block_C)
{
    int sqroot = sqrt(grid->nb_proc);
    int shift_source, shift_dest;
    MPI_Status status;
    int up_rank, down_rank, left_rank, right_rank;
    int i;

    MPI_Cart_shift(grid->grid_comm, 1, -1, &right_rank, &left_rank);
    MPI_Cart_shift(grid->grid_comm, 0, -1, &down_rank, &up_rank);
    MPI_Cart_shift(grid->grid_comm, 1, -grid->pos_row, &shift_source, &shift_dest);

    MPI_Sendrecv_replace(block_A, sqroot * sqroot, MPI_INT, shift_dest, 1, shift_source, 1,
                         grid->grid_comm, &status);

    MPI_Cart_shift(grid->grid_comm, 0, -grid->pos_col, &shift_source, &shift_dest);
    MPI_Sendrecv_replace(block_B, sqroot * sqroot, MPI_INT, shift_dest, 1, shift_source, 1,
                         grid->grid_comm, &status);

    for (i = 0; i < sqroot; i++)
    {
        multiply_matrix(grid, block_A, block_B, block_C);
        MPI_Sendrecv_replace(block_A, grid->nb_proc, MPI_INT, left_rank, 1, right_rank, 1,
                             grid->grid_comm, &status);
        MPI_Sendrecv_replace(block_B, grid->nb_proc, MPI_INT, up_rank, 1, down_rank, 1,
                             grid->grid_comm, &status);
    }

    MPI_Cart_shift(grid->grid_comm, 1, +grid->pos_row, &shift_source, &shift_dest);
    MPI_Sendrecv_replace(block_B, grid->nb_proc, MPI_INT, shift_dest, 1, shift_source, 1,
                         grid->grid_comm, &status);

    MPI_Cart_shift(grid->grid_comm, 0, +grid->pos_col, &shift_source, &shift_dest);
    MPI_Sendrecv_replace(block_B, grid->nb_proc, MPI_INT, shift_dest, 1, shift_source, 1,
                         grid->grid_comm, &status);
}

void multiply_matrix(GRID_INFO_T *grid, int *block_A, int *block_B, int *block_C)
{

    int i, j, k;
    int sqroot = (int)sqrt(grid->nb_proc);

    for (i = 0; i < sqroot; i++)
        for (j = 0; j < sqroot; j++)
            for (k = 0; k < sqroot; k++)
                block_C[i * sqroot + j] += block_A[i * sqroot + k] * block_B[k * sqroot + j];
}

void display_matrix(GRID_INFO_T *grid, int *mat)
{
    int i;
    if (grid->grid_rank == 0)
    {
        printf("matrix: \n");
        int ii, jj;
        for (ii = 0; ii < grid->order; ii++)
        {
            for (jj = 0; jj < grid->order; jj++)
            {
                if (log10((int)mat[ii * grid->order + jj]) + 1 <= 2)
                    printf("%2d ", (int)mat[ii * grid->order + jj]);
                else if (log10((int)mat[ii * grid->order + jj]) + 1 > 2)
                    printf("%4d ", (int)mat[ii * grid->order + jj]);
            }
            printf("\n");
        }
        printf("\n\n");
    }
}

void init_grid(GRID_INFO_T *grid_info)
{
    int rank;
    int dims[2];
    int period[2];
    int coords[2];
    int free_coords[2];

    MPI_Comm_size(MPI_COMM_WORLD, &(grid_info->nb_proc));
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        scanf("%d", &grid_info->order);
    }

    MPI_Bcast(&grid_info->order, 1, MPI_INT, 0, MPI_COMM_WORLD);

    dims[0] = dims[1] = (int)sqrt(grid_info->nb_proc);
    period[0] = period[1] = 1;

    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, period, 1, &(grid_info->grid_comm));
    MPI_Comm_rank(grid_info->grid_comm, &(grid_info->grid_rank));
    MPI_Cart_coords(grid_info->grid_comm, grid_info->grid_rank, 2, coords);
    grid_info->pos_row = coords[0];
    grid_info->pos_col = coords[1];

    free_coords[0] = 0;
    free_coords[1] = 1;
    MPI_Cart_sub(grid_info->grid_comm, free_coords, &(grid_info->row_comm));

    free_coords[0] = 1;
    free_coords[1] = 0;
    MPI_Cart_sub(grid_info->grid_comm, free_coords, &(grid_info->col_comm));
}

void read_matrix(GRID_INFO_T *info, int *mat, int order)
{
    int col;
    int row;
    for (row = 0; row < order; ++row)
    {
        for (col = 0; col < order; ++col)
        {
            mat[row * order + col] = rand() % 10;
        }
    }
}