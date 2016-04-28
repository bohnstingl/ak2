#include "keccak.h"
#include "cube.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>

using namespace std;

void loop(Cube &cube, uint64_t key[], uint64_t result[]) {
  Cube::cubeIterator it = cube.cubeBegin();
  uint64_t st[25];
  for (; it != cube.cubeEnd(); ++ it) {
    memcpy(st, *it, 25 * 8);
    memcpy(st, key, 2 * 8);
    
    keccakf(st, cube.keccak_rounds_);
    
    result[0] ^= st[0];
    result[1] ^= st[1];
  }
}

void handloop(Cube &cube, uint64_t key[], uint64_t result[]) {
  uint64_t st[25];
  for (int i = 0; i < (1 << cube.size()); i ++) {
    memset(st, 0, 25 * 8);
    for (int v = 0; v < cube.size(); v ++) {
      if (IS_SET(i, v))
        ST_SET_BIT(st, cube[v]);
    }
    
    memcpy(st, key, 2 * 8);
    
    keccakf(st, cube.keccak_rounds_);
    
    result[0] ^= st[0];
    result[1] ^= st[1];
  }
}

void test1() {
  uint64_t result[2], key[2];
  memset(key, 0, 16);
  key[1] = 0xFE01AB341002BEFFL;
  memset(result, 0, 16);
  //Cube cube(1);
  Cube::nThreads = 1;
  for (int i = 634; i < 640+128; i ++) {
    Cube cube(1);
    cube.push_back(i / 128 + 128);
    cube.push_back((i % 128) + 128);
    cube.push_back(233);
    if (i == 634) {
      handloop(cube, key, result);
      //cube.deriveParallel(key, result);
    } else {
      uint64_t tRes[2];
      memset(tRes, 0, 16);
      handloop(cube, key, tRes);
      //cube.deriveParallel(key, tRes);
      if (memcmp(result, tRes, 16) != 0) {
        cout << "alert at: " << dec << i << endl;
        cube.printVariables();
        cout << setw(16) << setfill('0') << hex << result[0] << endl;
        cout << setw(16) << setfill('0') << hex << result[1] << endl;
        cout << setw(16) << setfill('0') << hex << tRes[0] << endl;
        cout << setw(16) << setfill('0') << hex << tRes[1] << endl;
      }
    }
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

void testConstantness() {
  uint64_t result[2];
  uint64_t st[25];
  memset(result, 0, 16);
  
  fillRandom(st);
  
  print64(st, 25);
  
  auto xorResult = [&] (uint64_t st[]) -> void {
    result[0] ^= st[0];
    result[1] ^= st[1];
  };
  
  for (int i = 0; i < 4; i ++) {
    uint64_t temp[25];
    memcpy(temp, st, 25 * 8);
    
    if (i & 0x01) 
      ST_SET_BIT(temp, 0);
    else 
      ST_CLR_BIT(temp, 0);
    
    if (i & 0x02)
      ST_SET_BIT(temp, 1);
    else
      ST_CLR_BIT(temp, 1);
    
    keccakf(temp, 1);
    xorResult(temp);
    
    print64(temp, 2);
  }
  
  print64(result, 2);
}

void testAllConstantness() {
  uint64_t st[25];
  uint64_t result[2];
  memset(st, 0, 25 * 8);
  
  fillRandom(st);
  
  auto xorResult = [&] (uint64_t st[]) -> void {
    result[0] ^= st[0];
    result[1] ^= st[1];
  };
  
  for (int ind1 = 3; ind1 < 200; ind1 ++) {
    for (int ind2 = ind1 + 1; ind2 < 1600; ind2 ++) {
      memset(result, 0, 16);
      
      vector<uint> indices(4);
      indices[0] = ind1;
      indices[1] = ind2;
      indices[2] = 0;
      indices[3] = 1;
      
      for (int i = 0; i < (1 << indices.size()); i ++) {
        uint64_t temp[25];
        memcpy(temp, st, 25 * 8);
        mapVarToStateBits(temp, indices, i);
        keccakf(temp, 2);
        xorResult(temp);
      }
      
      if (result[0] || result[1]) {
        cout << "nonzero with: " << indices[0] << " and " << indices[1] << endl;
        print64(result, 2);
        //cin.get();
      }
    }
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

void testCoefficientComputation() {
  uint ind_arr[] = { 136, 138, 142, 145, 164, 166, 180, 194, 195, 197, 202, 206, 213, 232, 253 }; 
  vector<uint> indices(ind_arr, ind_arr + 15);
  
  uint64_t coefficients[129][2];
  memset(coefficients, 0, 129 * 2 * 8);
  
  cubeCoefficients(coefficients, indices, 4);
  for (int i = 0; i < 129; i ++) {
    print64(coefficients[i], 2);
  }
}

void testKeyBitCorrelation(uint64_t coefficients[][2], vector<uint> indices) {
  
  uint64_t st[25];
  uint64_t key[2];
  for (int kb = -1; kb < 128; kb ++) {
    memset(key, 0, 16);
    if (kb >= 0)
      ST_SET_BIT(key, kb);
    
    for (int j = 0; j < 1; j ++) {
      uint64_t res[2];
      memset(res, 0, 16);

      for (uint64_t i = 0; i < ((uint64_t) 1 << indices.size()); i ++) {
        memset(st, 0, 25 * 8);
        memcpy(st, key, 16);

        mapVarToStateBits(st, indices, i);
        keccakf(st, 4);

        res[0] ^= st[0];
        res[1] ^= st[1];
      }

      res[0] ^= coefficients[0][0];
      res[1] ^= coefficients[0][1];

      if (kb >= 0) {
        res[0] ^= coefficients[kb + 1][0];
        res[1] ^= coefficients[kb + 1][1];
      }
      
      if (res[0] || res[1]) {
        cout << "false at " << kb << endl;
        print64(coefficients[kb], 2);
        cout << "diff" << endl;
        print64(res, 2);
      }
    }
  }
}

int main() {
  srand(time(NULL));
  
  //uint ind_arr[] = { 136, 138, 142, 145, 164, 166, 180, 194, 195, 197, 202, 206, 213, 232, 253 }; 
  //vector<uint> indices(ind_arr, ind_arr + 15);
  uint ind_arr[] = { 131, 132, 136, 141, 152, 153, 160, 166, 184, 189, 201, 207, 210, 245}; 
  vector<uint> indices(ind_arr, ind_arr + 14);
  
  uint64_t coefficients[129][2];
  memset(coefficients, 0, 129 * 2 * 8);
  
  cubeCoefficients(coefficients, indices, 4);

  testKeyBitCorrelation(coefficients, indices);

  for (int i = 0; i < 129; i ++) {
    cout << "key bit: " << i - 1 << endl;    
    print64(coefficients[i], 2);
  }
  
  return 0;
}
