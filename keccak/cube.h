#ifndef CUBE_H_
#define CUBE_H_

#include <vector>
#include <thread>
#include <cstring>
#include <algorithm>

#define IS_SET(x, i) ((x) & (1 << (i)))
#define IS_CLR(x, i) !((x) & (1 << (i)))

#define ST_SET_BIT(st, bit) ((st)[(bit) / 64] |= ((uint64_t) 1 << ((bit) % 64)))
#define ST_CLR_BIT(st, bit) ((st)[(bit) / 64] &= ~((uint64_t) 1 << ((bit) % 64)))

#define IS_ST_BIT(st, bit) ((st)[(bit) / 64] & ((uint64_t) 1 << ((bit) % 64)))

class Cube : public std::vector<uint> {
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

    void printVariables();

    std::pair<Cube::cubeIterator, Cube::cubeIterator> getEqualCubeRange(uint i, uint n) {
        uint64_t step = ((uint64_t) 1 << size()) / n;
        return std::pair<Cube::cubeIterator, Cube::cubeIterator>(cubeIterator(i * step, *this), cubeIterator((i + 1) * step, *this));
    };

    void sumRange(uint64_t result[], uint64_t key[], std::pair<Cube::cubeIterator, Cube::cubeIterator> range);
    void deriveParallel(uint64_t key[], uint64_t result[]);
    void randomCube(uint dim, uint min, uint max);

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

typedef std::pair<Cube::cubeIterator, Cube::cubeIterator> CubeRange;

#endif
