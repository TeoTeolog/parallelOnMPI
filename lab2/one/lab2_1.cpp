#include <iostream>
#include <mpi.h>
#include <string>
#include <vector>
#include <numeric>
#include <algorithm>

using namespace std;

typedef struct message_s
{
    vector<int> routes;
    string payload;

    string stringify()
    {
        string routeStr;
        for (int route : routes)
        {
            routeStr += to_string(route) + " ";
        }

        return routeStr + "|" + payload;
    }

    void parse(const string &str)
    {
        size_t delimiterPos = str.find('|');
        if (delimiterPos != string::npos)
        {
            size_t startPos = 0;
            while (startPos < delimiterPos)
            {
                size_t sepPos = str.find(' ', startPos);
                if (sepPos != string::npos)
                {
                    routes.push_back(stoi(str.substr(startPos, sepPos - startPos)));
                    startPos = sepPos + 1;
                }
                else
                {
                    routes.push_back(stoi(str.substr(startPos, delimiterPos - startPos)));
                    break;
                }
            }

            payload = str.substr(delimiterPos + 1);
        }
    }

} message;

vector<int> generateRoute(int size)
{
    vector<int> route(size - 1);
    iota(route.begin(), route.end(), 1);
    reverse(route.begin(), route.end());
    return route;
}

int main(int argc, char **argv)
{
    double start, end;
    MPI_Init(&argc, &argv);

    int process_size, process_rank;

    MPI_Comm_size(MPI_COMM_WORLD, &process_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

    const int route_length = 3;

    start = MPI_Wtime();

    if (process_rank == 0)
    {
        message msg;
        msg.routes = generateRoute(process_size);
        msg.payload = "Hello, world!";

        string messageStr = msg.stringify();
        cout << "Process " << process_rank << " send message " << msg.payload << endl;
        MPI_Send(messageStr.c_str(), messageStr.size() + 1, MPI_CHAR, msg.routes.back(), 0, MPI_COMM_WORLD);
    }
    else
    {
        char recv_msg[256];
        MPI_Recv(recv_msg, 256, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        message msg;
        msg.parse(recv_msg);

        msg.routes.pop_back();

        if (msg.routes.size() > 0)
        {
            string messageStr = msg.stringify();
            cout << "Process " << process_rank << " received message and send it do dist:" << msg.routes.back() << endl;
            MPI_Send(messageStr.c_str(), messageStr.size() + 1, MPI_CHAR, msg.routes.back(), 0, MPI_COMM_WORLD);
        }
        else
        {

            cout << "Process " << process_rank << " is the final addressee. Message was: " << msg.payload << endl;

            end = MPI_Wtime();
            printf("Time spent: %f\n\n", end - start);
        }
    }

    MPI_Finalize();

    return 0;
}
