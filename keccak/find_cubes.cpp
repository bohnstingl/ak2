#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>
#include <algorithm>

#include "keccak.h"

#define IS_SET(x, i) ((x) & (1 << (i)))
#define IS_CLR(x, i) !((x) & (1 << (i)))

#define ST_SET_BIT(st, bit) ((st)[(bit) / 64] |= ((uint64_t) 1 << ((bit) % 64)))
#define ST_CLR_BIT(st, bit) ((st)[(bit) / 64] &= ~((uint64_t) 1 << ((bit) % 64)))

#define IS_ST_BIT(st, bit) ((st)[(bit) / 64] & ((uint64_t) 1 << ((bit) % 64)))

using namespace std;

typedef unsigned int uint;

class Cube : public vector<uint> {
public:

    Cube() : keccak_rounds_(4) { }

    class cubeIterator {
    public:
        cubeIterator(uint64_t start, Cube &cube) : i_(start), t_(0), diff_(0),
                                               var_index_(0), cube_(cube) {
            memset(st_, 0, 25 * sizeof(uint64_t));
        }

        cubeIterator operator++() { i_++; return *this; }
        uint64_t *operator*() {
            // BUILD STATE
            diff_ = i_ ^ t_;
            var_index_ = 0;

            while (diff_ != 0) {
                if (diff_ & 1) {
                    if (IS_SET(i_, var_index_))
                        ST_SET_BIT(st_, cube_[var_index_]);
                    else
                        ST_CLR_BIT(st_, cube_[var_index_]);
                }
                diff_ >>= 1;
                var_index_++;
            }

            t_ = i_;
            return st_;
        }

        bool operator==(const cubeIterator& rhs) { return rhs.i_ == i_; }
        bool operator!=(const cubeIterator& rhs) { return rhs.i_ != i_; }

    private:
        uint64_t i_, t_, diff_;
        uint8_t var_index_;
        Cube &cube_;
        uint64_t st_[25];
    };

    void printVariables() {
        cout << "Cube variables: ";
        for (uint i = 0; i < size(); i ++) {
            cout << at(i) << " ";
        }
        cout << endl;
    }

    pair<Cube::cubeIterator, Cube::cubeIterator> getEqualCubeRange(uint i, uint n) {
        uint64_t step = ((uint64_t) 1 << size()) / n;
        return pair<Cube::cubeIterator, Cube::cubeIterator>(cubeIterator(i * step, *this), cubeIterator((i + 1) * step, *this));
    };

    void sumRange(uint64_t result[], uint64_t key[], pair<Cube::cubeIterator, Cube::cubeIterator> range);
    void deriveParallel(uint64_t key[], uint64_t result[]);

    void addArray(uint indices[], uint length) {
        for (uint i = 0; i < length; i ++)
            push_back(indices[i]);
    }

    cubeIterator cubeBegin() {
        return cubeIterator(0, *this);
    }

    cubeIterator cubeEnd() {
        return cubeIterator((uint64_t) 1 << size(), *this);
    }
    uint keccak_rounds_;

private:
    static const uint nThreads = 2;
};

typedef pair<Cube::cubeIterator, Cube::cubeIterator> CubeRange;

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
    uint64_t tempResults[nThreads][2] = { 0 };
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

void randomCube(vector<uint> &indices, uint dim, uint min, uint max) {
    uint i = 0, randi;

    if (max - min < dim)
        throw -1;

    for (; i < dim; i ++) {
        randi = rand() % (max - min) + min;
        if (find(indices.begin(), indices.end(), randi) == indices.end())
            indices.push_back(randi);
        else
            i --;
    }

    sort(indices.begin(), indices.end());
}

void printCoefficientsReadable(uint64_t coefficients[129][2]) {
    for (uint i = 0; i < 128; i ++) {
        cout << "x[" << i << "] = " << (IS_ST_BIT(coefficients[0], i) != 0);
        for (uint key_bit = 0; key_bit < 128; key_bit ++) {
            if (IS_ST_BIT(coefficients[key_bit + 1], i))
                cout << " + k[" << key_bit << "]";
        }
        cout << endl;
    }
}

void printCoefficientsMachine(uint64_t coefficients[129][2]) {
    for (uint i = 0; i < 128; i ++) {
        cout << "x[" << i << "]:" << (IS_ST_BIT(coefficients[0], i) != 0);
        for (uint key_bit = 0; key_bit < 128; key_bit ++) {
            if (IS_ST_BIT(coefficients[key_bit + 1], i))
                cout << ";k[" << key_bit << "]";
        }
        cout << endl;
    }
}

int main() {
    Cube cube;

    srand(time(NULL));
    //uint cube_vars2[] = {138, 140, 151, 157, 164, 191, 202, 210, 219, 221, 226, 236, 244};
    uint cube_vars2[] = {128, 130, 131, 139, 145, 146, 147, 148, 151, 155, 158, 160, 161, 163, 164, 165, 185, 186, 189,
                         190, 193, 196, 205, 212, 220, 225, 229, 238, 242, 245, 249};
    cube.keccak_rounds_ = 5;
    cube.addArray(cube_vars2, 31);
    //randomCube(cube, (1 << cube.keccak_rounds_) - 1, 128, 256);

    uint64_t sum[2] = { 0 };
    uint64_t key[2] = { 0 };
    uint64_t coefficients[129][2] = { 0 };
    uint loop_var;
    bool found;

    do {
        cout << "Trying cube..." << endl;
        cube.printVariables();

        cout << "determining coefficients" << flush;
        for (int key_bit = -1; key_bit < 128; key_bit++) {
            memset(key, 0, 16);
            //memset(sum, 0, 16);
            if (key_bit >= 0)
                ST_SET_BIT(key, key_bit);

            cube.deriveParallel(key, coefficients[key_bit + 1]);
            cout << "." << flush;
        }
        cout << endl;

        found = 0;
        for (loop_var = 0; loop_var < 129 * 2; loop_var ++) {
            if (coefficients[loop_var / 2][loop_var % 2]) {
                found = true;
                break;
            }
        }
        if (found)
            break;

        cube.pop_back();
    } while(1);

    printCoefficientsReadable(coefficients);
    printCoefficientsMachine(coefficients);

//#define SEQ
#ifdef SEQ
    Cube::cubeIterator it = cube.cubeBegin();
    for(; it != cube.cubeEnd(); ++ it) {
        uint64_t st[25];
        memcpy(st, *it, 25 * sizeof(uint64_t));

        keccakf(st, cube.keccak_rounds_);
        sum[0] ^= st[0];
        sum[1] ^= st[1];
    }
#else
    /*uint cube_vars[] = {138, 140, 151, 157, 164, 191, 202, 210, 219, 221, 226, 236, 244};
    Cube test;
    test.addArray(cube_vars, 13);
    test.keccak_rounds_ = 4;
    memset(key, 0, 16);
    memset(sum, 0, 16);
    ST_SET_BIT(key, 51);
    test.deriveParallel(key, sum);
    cout << "x[43] = " << (IS_ST_BIT(sum, 43) != 0) << endl;*/
#endif

    //cout << (IS_ST_BIT(sum, 45) != 0) << endl;
    //cout << (IS_ST_BIT(sum, 85) != 0) << endl;
    //cout << "0x" << setw(16) << setfill('0') << hex << sum[0] << endl;
    //cout << "0x" << setw(16) << setfill('0') << hex << sum[1] << endl;

    return 0;
}
