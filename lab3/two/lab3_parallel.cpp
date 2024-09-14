#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <mpi.h>

using namespace std;

void printVector(const vector<int> &vec)
{
    for (int num : vec)
        cout << num << " ";
    cout << endl;
}

void fillArrayRandomly(vector<int> &vec, int n)
{
    for (int i = 0; i < n; i++)
    {
        vec.push_back(rand() % 100);
    }
}

int main()
{
    MPI_Init(NULL, NULL);
    double timeStart, timeEnd;

    int K, ProcRank;

    MPI_Comm_size(MPI_COMM_WORLD, &K);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    if (ProcRank == 0)
    {
        timeStart = MPI_Wtime();
    }

    vector<int> sendArray;
    fillArrayRandomly(sendArray, K + 5);

    vector<int> receivedArray(K + 5);

    MPI_Reduce(sendArray.data(), receivedArray.data(), sendArray.size(), MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (ProcRank == 0)
    {
        cout << "Result: ";
        printVector(receivedArray);

        timeEnd = MPI_Wtime();

        cout << "Time: " << timeEnd - timeStart;
    }

    MPI_Finalize();

    return 0;
}
