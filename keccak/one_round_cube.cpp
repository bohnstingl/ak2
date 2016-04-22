#include <iostream>
#include <iomanip>
#include <vector>

#include <stdio.h>

#define KECCAK_ROUNDS 1
#include "keccak.h"
#include "cube.h"

#define IS_SET(x, i) ((x) & (1 << (i)))
#define IS_CLR(x, i) !((x) & (1 << (i)))

#define ST_SET_BIT(st, bit) ((st)[(bit) / 64] |= ((uint64_t) 1 << ((bit) % 64)))
#define ST_CLR_BIT(st, bit) ((st)[(bit) / 64] &= ~((uint64_t) 1 << ((bit) % 64)))

#define IS_ST_BIT(st, bit) ((st)[(bit) / 64] & ((uint64_t) 1 << ((bit) % 64)))

using namespace std;

typedef unsigned int uint;

typedef pair<uint64_t, uint64_t> CubeRange;
/*class Cube: public vector<uint>
{
public:
  CubeRange getEqualCubeRange(uint i, uint n)
  {
    uint64_t step = ((uint64_t) 1 << size()) / n;
    return CubeRange(i * step, (i + 1) * step);
  }
  ;

  class iterator
  {
  public:
    iterator(uint64_t start, Cube &cube) :
        i_(start), t_(start), diff_(0), var_index_(0), cube_(cube)
    {
      memset(st_, 0, 25 * sizeof(uint64_t));
    }

    iterator operator++()
    {
      i_++;
      return *this;
    }
    uint64_t *operator*()
    {
      // BUILD STATE
      diff_ = i_ ^ t_;
      var_index_ = 0;

      while (diff_ != 0)
      {
        if (diff_ & 1)
        {
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

    bool operator==(const iterator& rhs)
    {
      return rhs.i_ == i_;
    }
    bool operator!=(const iterator& rhs)
    {
      return rhs.i_ != i_;
    }

  private:
    uint64_t i_, t_, diff_;
    uint8_t var_index_;
    Cube &cube_;
    uint64_t st_[25];
  };

  iterator begin()
  {
    return iterator(0, *this);
  }

  iterator end()
  {
    return iterator((uint64_t) 1 << size(), *this);
  }
};*/

/*int main()
{
  Cube cube(1);
  cube.push_back(128);

  uint64_t sum[2] = { 0 };

  Cube::cubeIterator it = cube.cubeBegin();
  for (; it != cube.cubeEnd(); ++it)
  {
    uint64_t st[25];
    memcpy(st, *it, 25 * sizeof(uint64_t));

    keccakf(st, 1);
    sum[0] ^= st[0];
    sum[1] ^= st[1];
  }

  cout << (IS_ST_BIT(sum, 45) != 0) << endl;
  cout << (IS_ST_BIT(sum, 85) != 0) << endl;

  return 0;
}*/
