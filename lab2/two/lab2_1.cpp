#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <mpi.h>

using namespace std;

const int N = 11; // количество элементов массива

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

void fillArrayUpwards(vector<int> &vec, int start, int n)
{
    for (int i = 0; i < n; i++)
    {
        vec.push_back(i + start);
    }
}

void cutHalfArray(vector<int> &res, vector<int> &vec)
{
    int targetSize = vec.size() / 2 + vec.size() % 2;
    reverse(vec.begin(), vec.end());
    while (res.size() != targetSize)
    {
        res.push_back(vec.back());
        vec.pop_back();
    }
    reverse(vec.begin(), vec.end());
}

int main()
{
    MPI_Init(NULL, NULL);
    double timeStart, timeEnd;

    int ProcSize, ProcRank;

    MPI_Comm_size(MPI_COMM_WORLD, &ProcSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    int elementsPerProcess = N / (ProcSize - 1);
    vector<int> localArray(ProcRank == (ProcSize - 1) ? (N % (ProcSize - 1) + elementsPerProcess) : elementsPerProcess);

    if (ProcRank == 0)
    {
        vector<int> originalArray;
        vector<int> resultArray(N);

        fillArrayRandomly(originalArray, N);
        cout << "Start: ";
        printVector(originalArray);

        timeStart = MPI_Wtime();

        int order = 0;
        vector<int> sendTargetArray;
        fillArrayUpwards(sendTargetArray, 1, ProcSize - 1);

        random_device rd;
        mt19937 g(rd());
        shuffle(sendTargetArray.begin(), sendTargetArray.end(), g);

        cout << "Target: ";
        printVector(sendTargetArray);
        cout << endl;

        int countWorkingProcess = 0;
        for (int i = 0; i < sendTargetArray.size() && originalArray.size() > 0; i++, countWorkingProcess++)
        {
            int target = sendTargetArray[i];

            vector<int> sendArray;
            if (i == sendTargetArray.size() - 1)
            {
                sendArray = originalArray;
            }
            else
            {
                cutHalfArray(sendArray, originalArray);
            }

            MPI_Send(sendArray.data(), sendArray.size(), MPI_INT, target, 0, MPI_COMM_WORLD);
        }
        // заглушка
        for (int i = countWorkingProcess; i < sendTargetArray.size(); i++)
        {
            int target = sendTargetArray[i];
            int buffer = -2;
            MPI_Send(&buffer, 1, MPI_INT, target, 0, MPI_COMM_WORLD);
        }
        for (int i = 0; i < countWorkingProcess; i++)
        {
            MPI_Status status;
            int count;
            vector<int> recvArray(N);
            MPI_Recv(recvArray.data(), N, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_INT, &count);
            int source = status.MPI_SOURCE;
            int place = find(sendTargetArray.begin(), sendTargetArray.end(), source) - sendTargetArray.begin();
            int size = N;
            int begin = 0;
            int kostil = N;
            size = (size / 2 + (size % 2));
            for (int i = 0; i < place; i++)
            {
                begin += size;
                kostil -= size;
                size = kostil - size / 2;
            }
            for (int i = 0; i < count; i++)
            {
                resultArray[i + begin] = recvArray[i];
            }
        }

        cout << "Result: ";
        printVector(resultArray);

        timeEnd = MPI_Wtime();

        cout << "Time: " << timeEnd - timeStart;
    }
    else
    {
        MPI_Status status;
        int count;
        vector<int> recvArray(N);
        MPI_Recv(recvArray.data(), N, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &count);
        recvArray.resize(count);
        for (int i = 0; i < count; i++)
        {
            recvArray[i] += 1;
        }

        if (recvArray[0] > -1)
        {
            MPI_Send(recvArray.data(), recvArray.size(), MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();

    return 0;
}
