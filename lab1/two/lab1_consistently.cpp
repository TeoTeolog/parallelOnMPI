#include <chrono>
#include <ctime>
#include <iostream>
#include <vector>

using namespace std;

const int N = 20000; // количество элементов массива

void bubbleSort(vector<int> &vec)
{
    int n = vec.size();
    for (int i = 0; i < n - 1; i++)
    {
        for (int j = 0; j < n - i - 1; j++)
        {
            if (vec[j] > vec[j + 1])
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

int main()
{
    srand(time(0));
    vector<int> vec;

    fillArrayRandomly(vec, N);

    auto start = chrono::system_clock::now();

    cout << "Unsorted vector: ";
    // printVector(vec);

    bubbleSort(vec);

    cout << "Sorted vector: ";
    // printVector(vec);

    auto end = chrono::system_clock::now();

    chrono::duration<double> elapsed_seconds = end - start;

    cout << "Time: " << elapsed_seconds.count() << "s\n";

    return 0;
}
