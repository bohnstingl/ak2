#include "cube.h"
#include "keccak.h"

#include <iostream>

using namespace std;

uint Cube::nThreads = 8;

void Cube::sumRange(uint64_t result[], uint64_t key[], CubeRange range) {
    cubeIterator it = range.first;
    uint64_t st[25];

    for (; it != range.second; ++ it) {
        memcpy(st, *it, 25 * sizeof(uint64_t));
        memcpy(st, key, 2 * sizeof(uint64_t));

        keccakf(st, keccak_rounds_);
        result[0] ^= st[0];
        result[1] ^= st[1];
        /* 320 bit Tag
        result[2] ^= st[2];
        result[3] ^= st[3];
        result[4] ^= st[4];
         */
    }
}

void Cube::deriveParallel(uint64_t key[], uint64_t result[]) {
    uint64_t tempResults[nThreads][2];
    memset(tempResults, 0, nThreads * 8 * 2);
    //uint64_t tempResults[nThreads][5] = { 0 }; 320 bit Tag

    uint th;
    vector<thread *> threads;

    for (th = 0; th < nThreads; th ++) {
        CubeRange range = getEqualCubeRange(th, nThreads);
        thread *newThread = new thread(&Cube::sumRange, this, tempResults[th], key, range);
        threads.push_back(newThread);
    }

    for (th = 0; th < nThreads; th ++) {
        threads[th]->join();
        delete threads[th];

        result[0] ^= tempResults[th][0];
        result[1] ^= tempResults[th][1];
        /* 320 bit Tag
        result[2] ^= tempResults[th][2];
        result[3] ^= tempResults[th][3];
        result[4] ^= tempResults[th][4];
         */
    }
}

void Cube::randomCube(uint dim, uint min, uint max) {
    uint i = 0, randi;

    if (max - min < dim)
        throw -1;

    for (; i < dim; i ++) {
        randi = rand() % (max - min) + min;
        if (find(begin(), end(), randi) == end())
            push_back(randi);
        else
            i --;
    }

    sort(begin(), end());
}

void Cube::printVariables() {
    cout << "Cube variables: ";
    for (uint i = 0; i < size(); i ++) {
        cout << at(i) << " ";
    }
    cout << endl;
}
