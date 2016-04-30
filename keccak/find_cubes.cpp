#include <iostream>
#include <iomanip>
#include "cube.h"
#include "keccak.h"

using namespace std;

typedef unsigned int uint;

void printCoefficientsReadable(uint64_t coefficients[129][2])
{
  for (uint i = 0; i < 128; i++)
  {
    cout << "x[" << i << "] = " << (IS_ST_BIT(coefficients[0], i) != 0);
    for (uint key_bit = 0; key_bit < 128; key_bit++)
    {
      if (IS_ST_BIT(coefficients[key_bit + 1], i))
        cout << " + k[" << key_bit << "]";
    }
    cout << endl;
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
}

void fillRandom(uint64_t st[]) {
  for (int i = 0; i < 25 * 2; i ++)
    ((uint32_t *) st)[i] = rand();
}

void print64(uint64_t dat[], int len) {
  for (int i = 0; i < len; i ++)
    cout << setw(16) << setfill('0') << hex << dat[i] << endl;
  cout << dec << endl;
}

void mapVarToStateBits(uint64_t st[], vector<uint> indices, uint count) {
  for (int i = 0; i < indices.size(); i ++) {
    if (count & (1 << i))
      ST_SET_BIT(st, indices[i]);
    else
      ST_CLR_BIT(st, indices[i]);
  }
}

void cubeCoefficients(uint64_t coefficients[][2], vector<uint> cubeIndices, uint rounds) {
  // THREAD COUNT:
  uint ldThreads = 3;
  //---------------------------------------------------------------------------
  uint nThreads = 1 << ldThreads;
  
  memset(coefficients, 0, 129 * 2 * 8);
  
  // CONSTANT
  auto findConstant = [&] () -> void {
    uint64_t st[25];
    for (uint64_t i = 0; i < ((uint64_t) 1 << cubeIndices.size()); i ++) {
      memset(st, 0, 8 * 25);
      mapVarToStateBits(st, cubeIndices, i);
      
      keccakf(st, rounds);
      
      coefficients[0][0] ^= st[0];
      coefficients[0][1] ^= st[1];
    }
  };
  findConstant();
  
  // KEY COEFFICIENTS
  auto findKeyCoefficient = [&] (uint key_bit) -> void {
    uint64_t st[25];
    for (uint64_t i = 0; i < ((uint64_t) 1 << cubeIndices.size()); i ++) {
      memset(st, 0, 8 * 25);
      mapVarToStateBits(st, cubeIndices, i);
      
      ST_SET_BIT(st, key_bit);
      keccakf(st, rounds);
      
      coefficients[key_bit + 1][0] ^= st[0];
      coefficients[key_bit + 1][1] ^= st[1];
    }
    
    coefficients[key_bit + 1][0] ^= coefficients[0][0];
    coefficients[key_bit + 1][1] ^= coefficients[0][1];
  };
  
  vector<thread> threads;
  for (uint key_bit = 0; key_bit < 128; key_bit += nThreads) {
    for (uint th = 0; th < nThreads; th ++)
      threads.push_back(thread(findKeyCoefficient, key_bit + th));
      
    for (thread &th : threads)
      th.join();
    
    threads.clear();
  }
}

bool checkLinearity(Cube &cube)
{
  uint certainty = 100;
  uint64_t zero[2];

  uint64_t key[2], key2[2], key3[2];
  uint64_t res[2], res2[2], res3[2], res_constant[2];

  auto randomKeys = [&] () -> void {
    for (int i = 0; i < 4; i ++) {
      ((uint32_t *) key)[i] = rand();
      ((uint32_t *) key2)[i] = rand();
    }
  };

  memset(zero, 0, 16);

  for (uint i = 0; i < certainty; i ++)
  {
    randomKeys();
    memset(res, 0, 16);
    memset(res2, 0, 16);
    memset(res3, 0, 16);
    memset(res_constant, 0, 16);
    key3[0] = key[0] ^ key2[0];
    key3[1] = key[1] ^ key2[1];
    
    cube.deriveParallel(key, res, 0);
    cube.deriveParallel(key2, res2, 0);
    cube.deriveParallel(zero, res_constant, 0);

    cube.deriveParallel(key3, res3, 0);
    res[0] ^= res2[0] ^ res_constant[0];
    res[1] ^= res2[1] ^ res_constant[1];

    if (memcmp(res, res3, 16) != 0) {
      //cout << "not linear" << endl;
      return false;
    }
  }
  return true;
}

int main()
{
  srand(time(NULL));

  uint64_t sum[2] = { 0 };
  uint64_t key[2] = { 0 };
  memset(key, 0, 16);
  uint loop_var;
  bool found, linear = true;

  for (uint amount = 0; amount < 5000; amount++)
  {
    Cube cube(4);
    cube.randomCube((1 << cube.keccak_rounds_) - 1, 128, 256);
    //cube.push_back(128);
    //uint ind_arr[] = { 136, 138, 142, 145, 164, 166, 180, 194, 195, 197, 202, 206, 213, 232, 253 };
    //for (uint ll = 0; ll < 15; ll ++)
    //  cube.push_back(ind_arr[ll]);
    uint64_t coefficients[129][2];
    //uint64_t coefficients2[129][2];
    do
    {
      memset(coefficients, 0, 129 * 2 * 8);
      //memset(coefficients2, 0, 129 * 2 * 8);
      //cout << "Trying cube..." << endl;
      //cube.printVariables();

      //cout << "determining coefficients" << flush;
      for (int key_bit = -1; key_bit < 128; key_bit++)
      {
        memset(key, 0, 16);
        //memset(sum, 0, 16);
        if (key_bit >= 0)
          ST_SET_BIT(key, key_bit);

        cube.deriveParallel(key, coefficients[key_bit + 1], 0);
        if (key_bit >= 0)
        {
          coefficients[key_bit + 1][0] ^= coefficients[0][0];
          coefficients[key_bit + 1][1] ^= coefficients[0][1];
        }
        //cout << "." << flush;
      }
      //cout << endl;

      //cubeCoefficients(coefficients2, cube, 4);
      
      //if (memcmp(coefficients, coefficients2, 129 * 2 * 8) != 0)
      //    cout << "not the same coefficients!!" << endl;

      found = 0;
      for (loop_var = 2; loop_var < 129 * 2; loop_var++)
      { // constant is not enough
        if (coefficients[loop_var / 2][loop_var % 2])
        {
          found = true;
          break;
        }
      }

      if (found) 
      {
        linear = checkLinearity(cube);
        break;
      }

      cube.pop_back();
    } while (cube.size() >= 13);//15); // 1

    if (found && linear)
    {
      //cout << cube.size() << endl;
      cube.printVariables();
      //printCoefficientsReadable(coefficients);
      printCoefficientsMachine(coefficients);
      cout << "---------------------------------------------------------"
          << endl;
    }
  }

  return 0;
}
