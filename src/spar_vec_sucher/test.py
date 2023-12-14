from ctypes import *
import spar_vec_sucher.c_module
from spar_vec_sucher.c_module  import SparVecSucher
import numpy as np
import time


def main():
    # svs = SparVecSucher("/Users/mege/git/poemai-semantic-index/resources/memmap/memmaptest.bin",121248)
    svs = SparVecSucher("memmaptest.bin",121248)

    mmp = np.memmap("memmaptest.bin", dtype='float32', mode='r', shape=(121248, 1536))



    # vector = np.random.random(1536)
    vector = np.array(mmp[0])

    print("Starting....")
    for i in range(50):

        svs.search(vector,10)
        vector = np.array(np.random.random(1536), dtype=np.float32)

    
    



def main_old():
    # Call the C function
    
    print(spar_vec_sucher.c_module.__file__)
    print("Starting....")

    mmp = np.memmap("/Users/mege/git/poemai-semantic-index/resources/memmap/memmaptest.bin", dtype='float32', mode='r', shape=(121248, 1536))



    # vector = np.random.random(1536)
    vector = np.array(mmp[0])
    spar_vec_sucher.c_module.search(vector,10,  "/Users/mege/git/poemai-semantic-index/resources/memmap/memmaptest.bin",121248)
    spar_vec_sucher.c_module.search(vector,10,  "/Users/mege/git/poemai-semantic-index/resources/memmap/memmaptest.bin",121248)
    spar_vec_sucher.c_module.search(vector,10,  "/Users/mege/git/poemai-semantic-index/resources/memmap/memmaptest.bin",121248)

    print("---- LAST RUN----")
    start = time.time()
    spar_vec_sucher.c_module.search(vector,10,  "/Users/mege/git/poemai-semantic-index/resources/memmap/memmaptest.bin",121248)
    end = time.time()
    print("Time: ", 1000*(end - start) , "ms")

    spar_vec_sucher.c_module.search(vector,10,  "/Users/mege/git/poemai-semantic-index/resources/memmap/memmaptest.bin",121248)




if __name__ == "__main__":
    main()