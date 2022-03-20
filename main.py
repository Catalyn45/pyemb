import catalyn
from threading import Thread
from test_module import test

N_MAX = 10000000000

workers = 4
thread_list = []

for i in range(workers):
    thread_list.append(Thread(target=catalyn.compute, args=(N_MAX,)))

def parallel():
    for thread in thread_list:
        thread.start()

    for thread in thread_list:
        thread.join()


def sequential():
    for i in range(workers):
        catalyn.compute(N_MAX)

print("testing sequential")
test(sequential)

print("testing parallel")
test(parallel)

