#include <iostream>
#include <vector>
#include <chrono>

using namespace std;

void multiplyMatrices(const std::vector<std::vector<long long>> &A, const std::vector<std::vector<long long>> &B, std::vector<std::vector<long long>> &C, long long size)
{
    for (long long i = 0; i < size; i++)
    {
        for (long long j = 0; j < size; j++)
        {
            C[i][j] = 0;
            for (long long k = 0; k < size; k++)
            {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void printMatrix(const std::vector<std::vector<long long>> &matrix, long long size)
{
    for (long long i = 0; i < size; i++)
    {
        for (long long j = 0; j < size; j++)
        {
            std::cout << matrix[i][j] << "\t";
        }
        std::cout << std::endl;
    }
}

int main()
{
    long long size;

    // Get the size of the square matrices
    std::cout << "Enter the size of the square matrices: ";
    std::cin >> size;

    // Allocate memory for matrices A, B, and C using vectors
    std::vector<std::vector<long long>> A(size, std::vector<long long>(size));
    std::vector<std::vector<long long>> B(size, std::vector<long long>(size));
    std::vector<std::vector<long long>> C(size, std::vector<long long>(size));
    auto start = chrono::system_clock::now();

    // Initialize matrices A and B with random long longegers
    for (long long i = 0; i < size; i++)
    {
        for (long long j = 0; j < size; j++)
        {
            A[i][j] = rand() % 10;
            B[i][j] = rand() % 10;
        }
    }

    // Multiply matrices A and B to get matrix C
    multiplyMatrices(A, B, C, size);

    // Output the matrices
    // std::cout << "Matrix A:\n";
    // prlong longMatrix(A, size);

    // std::cout << "Matrix B:\n";
    // prlong longMatrix(B, size);

    // std::cout << "Matrix C (Result of A x B):\n";
    // prlong longMatrix(C, size);

    auto end = chrono::system_clock::now();

    chrono::duration<double> elapsed_seconds = end - start;

    cout << "Time: " << elapsed_seconds.count() << "s\n";

    return 0;
}
