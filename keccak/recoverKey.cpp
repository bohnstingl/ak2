#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>
#include <algorithm>
#include "keccak.h"
#include "cube.h"

#define IS_SET(x, i) ((x) & (1 << (i)))
#define IS_CLR(x, i) !((x) & (1 << (i)))
#define ST_SET_BIT(st, bit) ((st)[(bit) / 64] |= ((uint64_t) 1 << ((bit) % 64)))
#define ST_CLR_BIT(st, bit) ((st)[(bit) / 64] &= ~((uint64_t) 1 << ((bit) % 64)))
#define IS_ST_BIT(st, bit) ((st)[(bit) / 64] & ((uint64_t) 1 << ((bit) % 64)))

using namespace std;

typedef unsigned int uint;
typedef pair<Cube::cubeIterator, Cube::cubeIterator> CubeRange;

void printHexMessage(unsigned char hash[], int len);
void printCoefficientsReadable(uint64_t coefficients[129][2]);

/*void Cube::sumRange(uint64_t result[], uint64_t key[], CubeRange range)
{
  cubeIterator it = range.first;
  uint64_t st[25];

  for (; it != range.second; ++it)
  {
    memcpy(st, *it, 25 * sizeof(uint64_t));
    memcpy(st, key, 2 * sizeof(uint64_t));

    keccakf(st, keccak_rounds_);
    result[0] ^= st[0];
    result[1] ^= st[1];
  }
}

void Cube::deriveParallel(uint64_t key[], uint64_t result[])
{
  uint64_t tempResults[nThreads][2] = { 0 };
  //uint64_t tempResults[nThreads][5] = { 0 }; 320 bit Tag

  uint th;
  vector<thread *> threads;

  for (th = 0; th < nThreads; th++)
  {
    CubeRange range = getEqualCubeRange(th, nThreads);
    thread *newThread = new thread(&Cube::sumRange, this, tempResults[th], key, range);
    threads.push_back(newThread);
  }

  for (th = 0; th < nThreads; th++)
  {
    threads[th]->join();
    delete threads[th];

    result[0] ^= tempResults[th][0];
    result[1] ^= tempResults[th][1];
  }
}

void printCoefficientsMachine(uint64_t coefficients[129][2])
{
  for (uint i = 0; i < 128; i++)
  {
    cout << "" << i << ":" << (IS_ST_BIT(coefficients[0], i) != 0);
    for (uint key_bit = 0; key_bit < 128; key_bit++)
    {
      if (IS_ST_BIT(coefficients[key_bit + 1], i))
        cout << ";" << key_bit << "";
    }
    cout << endl;
  }
}*/

int main()
{
  unsigned char key1[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                            0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
  Cube cube(5);
  uint cube_vars[] = {128, 130, 131, 139, 145, 146, 147, 148, 151,
                      155, 158, 160, 161, 163, 164, 165, 185, 186,
                      189, 190, 193, 196, 205, 212, 220, 225, 229,
                      238, 242, 245, 249};
  cube.addArray(cube_vars, 31);

  srand(time(NULL));

  uint64_t sum[2] = { 0 };
  uint64_t key2[2] = { 0 };
  uint64_t coefficients[129][2];
  memset(coefficients, 0, 129 * 16 * 2);

  //Manually set the coefficients
  coefficients[0][1] |= ((uint64_t)1 << 49) | ((uint64_t) 1 << 39)
                     |  ((uint64_t)1 << 36) | ((uint64_t) 1 << 46)
                     |  ((uint64_t)1 << 41) | ((uint64_t) 1 << 40);
  coefficients[78][0] |= ((uint64_t)1 << 7);
  coefficients[114][0] |= ((uint64_t)1 << 15);
  coefficients[104][0] |= ((uint64_t)1 << 42);
  coefficients[45][1] |= ((uint64_t)1 << 20);
  coefficients[101][1] |= ((uint64_t)1 << 32);
  coefficients[18][1] |= ((uint64_t)1 << 48);
  coefficients[111][0] |= ((uint64_t)1 << 13);
  coefficients[26][0] |= ((uint64_t)1 << 31);
  coefficients[106][1] |= ((uint64_t)1 << 5);
  coefficients[124][1] |= ((uint64_t)1 << 23);
  coefficients[105][1] |= ((uint64_t)1 << 36);

  printCoefficientsReadable(coefficients);

  uint64_t res[2];
  memset(res, 0, 16);

  unsigned char msg[16];

  //coefficients[]

  memcpy(key2, key1, 16);

  cube.deriveParallel(key2, res);

  memcpy(msg, res, 16);
  printHexMessage(msg, 16);
  return 0;
}
