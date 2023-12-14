from ctypes import *
import spar_vec_sucher.c_module
import numpy as np
import time

def main():
    # Call the C function
    
    print(spar_vec_sucher.c_module.__file__)
    print("Starting....")

    mmp = np.memmap("/Users/mege/git/poemai-semantic-index/resources/memmap/memmaptest.bin", dtype='float32', mode='r', shape=(121248, 1536))



    # vector = np.random.random(1536)
    vector = np.array(mmp[0])
    spar_vec_sucher.c_module.search(vector,121248,10, "/Users/mege/git/poemai-semantic-index/resources/memmap/memmaptest.bin")
    spar_vec_sucher.c_module.search(vector,121248,10, "/Users/mege/git/poemai-semantic-index/resources/memmap/memmaptest.bin")
    spar_vec_sucher.c_module.search(vector,121248,10, "/Users/mege/git/poemai-semantic-index/resources/memmap/memmaptest.bin")

    print("---- LAST RUN----")
    start = time.time()
    spar_vec_sucher.c_module.search(vector,121248,10, "/Users/mege/git/poemai-semantic-index/resources/memmap/memmaptest.bin")
    end = time.time()
    print("Time: ", 1000*(end - start) , "ms")

    spar_vec_sucher.c_module.search(vector,121248,10, "/Users/mege/git/poemai-semantic-index/resources/memmap/memmaptest.bin")




if __name__ == "__main__":
    main()