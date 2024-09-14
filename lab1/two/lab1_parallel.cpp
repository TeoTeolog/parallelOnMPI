#include <algorithm>
#include <ctime>
#include <iostream>
#include <mpi.h>
#include <vector>

using namespace std;

const int N = 10000; // количество элементов массива

void bubbleSort(vector<int> &vec)
{
    int n = vec.size();
    for (int i = 0; i < n - 1; i++)
    {
        for (int j = 0; j < n - i - 1; j++)
        {
            if (vec[j] < vec[j + 1])
            {
                swap(vec[j], vec[j + 1]);
            }
        }
    }
}

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

void mergeArrays(vector<int> &finalRes, vector<int> &vec1, vector<int> &vec2)
{
    vector<int> res;
    while (vec1.size() && vec2.size())
    {
        if (vec1.back() < vec2.back())
        {
            res.push_back(vec1.back());
            vec1.pop_back();
        }
        else
        {
            res.push_back(vec2.back());
            vec2.pop_back();
        }
    }
    while (vec1.size())
    {
        res.push_back(vec1.back());
        vec1.pop_back();
    }
    while (vec2.size())
    {
        res.push_back(vec2.back());
        vec2.pop_back();
    }

    finalRes = res;
}

int main()
{

    MPI_Init(NULL, NULL);
    srand(time(0));
    double timeStart, timeEnd;
    int ProcRank, ProcSize;

    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcSize);

    vector<int> globalArray;

    int elementsPerProcess = N / (ProcSize - 1);
    vector<int> localArray(ProcRank == (ProcSize - 1) ? (N % (ProcSize - 1) + elementsPerProcess)
                                                      : elementsPerProcess);

    if (ProcRank == 0)
    {
        fillArrayRandomly(globalArray, N);

        timeStart = MPI_Wtime();

        // printVector(globalArray);

        for (int i = 1; i < ProcSize; i++)
        {
            vector<int> subvector = {globalArray.begin() + (i - 1) * elementsPerProcess,
                                     i != ProcSize - 1
                                         ? (globalArray.begin() + i * elementsPerProcess)
                                         : globalArray.end()};
            MPI_Send(subvector.data(), subvector.size(), MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Recv(localArray.data(),
                 ProcRank == (ProcSize - 1) ? (N % (ProcSize - 1) + elementsPerProcess)
                                            : elementsPerProcess,
                 MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        bubbleSort(localArray);
        MPI_Send(localArray.data(), localArray.size(), MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    if (ProcRank == 0)
    {
        globalArray.clear();
        for (int i = 1; i < ProcSize; i++)
        {
            vector<int> subvector(i == (ProcSize - 1) ? (N % (ProcSize - 1) + elementsPerProcess)
                                                      : elementsPerProcess);
            MPI_Recv(subvector.data(),
                     i == (ProcSize - 1) ? (N % (ProcSize - 1) + elementsPerProcess)
                                         : elementsPerProcess,
                     MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            reverse(globalArray.begin(), globalArray.end());

            mergeArrays(globalArray, globalArray, subvector);
        }

        cout << "Sorted vector: ";
        printVector(globalArray);

        timeEnd = MPI_Wtime();

        cout << "Time: " << timeEnd - timeStart;
    }

    MPI_Finalize();
    return 0;
}