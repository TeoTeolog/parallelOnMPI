import os
import time


def compile(name):
    bashCommand = f"mpic++ {name}.cpp -o {name}"
    print(bashCommand)
    os.system(bashCommand)


def run(name):
    for i in range(3, 8):
        for j in range(1, 8):
            bashCommand = f"mpirun -np {i} ./{name} {i}"
            if j == 1:
                print("------------------", bashCommand, "\n")
            print("\ntry number:", j, "\n")
            os.system(bashCommand)
            time.sleep(0.1)


def runAmountTimes(name, times, process):
    for j in range(1, times + 1):
        bashCommand = f"mpirun -np {process} ./{name} {process}"
        if j == 1:
            print(bashCommand, "\n")
        print("\ntry number:", j, "\n")
        os.system(bashCommand)
        time.sleep(0.1)


def test(name):
    compile(name)
    time.sleep(1)
    runAmountTimes(name, 4, 2)
    time.sleep(1)
    runAmountTimes(name, 4, 4)
    time.sleep(1)
    runAmountTimes(name, 4, 8)
    time.sleep(1)
    runAmountTimes(name, 4, 16)
    time.sleep(1)
    runAmountTimes(name, 4, 32)


test("lab3_parallel")
