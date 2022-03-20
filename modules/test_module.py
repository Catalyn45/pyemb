from time import time

def test(callback, args=[]):
    start = time()
    callback(*args)
    end = time()

    print(f"time: {end - start} seconds")

