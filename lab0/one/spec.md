# Отчет

## Формулировка задания

Задание

## Краткое описание выбранного алгоритма решения задачи

Для решения поставленной задачи был разработан алгоритм, который определяет имеет ли процесс rank == 0. Если нет, то процесс отправляет через коммуникатор процессу с нулевым рангом сообщение с номером своего ранга. Если процесс имеет ранг 0, то в немодицифицированной версии он в цикле принимает сообщения из любого источника и тут же печаетает сообщение с учетом полученной полезной нагрузки. В модифицированной версии алгоритма нулевой процесс принимает сообщения от других процессов только в порядке возрастания.

## Листинг программы

Немодифицированная версия

```cpp
#include <stdio.h>
#include <mpi.h>


int main(int argc, char** argv)
{

   MPI_Init(NULL, NULL);

   int ProcRank, size;
   MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
   MPI_Comm_size(MPI_COMM_WORLD, &size);

   if(ProcRank == 0)
   {
      printf("Hello from process %3d\n", ProcRank);

      for(int i = 1; i < size; i++)
      {
         MPI_Status Status;
         int RecvRank;
         MPI_Recv(&RecvRank, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,  &Status);
         printf("Hello from process %3d\n", RecvRank);
      }
   }
   else{
      MPI_Send (&ProcRank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
   }

   MPI_Finalize();
}
```

Измененная строчка в модифицированной версии:

```cpp
- MPI_Recv(&RecvRank, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,  &Status);
+ MPI_Recv(&RecvRank, 1, MPI_INT, MPI_ANY_SOURCE, i, MPI_COMM_WORLD,  &Status);
```

---

mpic++ lab1_1.cpp -o lab1_1
------------------ mpirun -np 4 ./lab1_1
