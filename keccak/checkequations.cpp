#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>
#include <algorithm>
#include <fstream>
#include <string>
#include <map>
#include "keccak.h"
#include "cube.h"

#define IS_SET(x, i) ((x) & (1 << (i)))
#define IS_CLR(x, i) !((x) & (1 << (i)))
#define ST_SET_BIT(st, bit) ((st)[(bit) / 64] |= ((uint64_t) 1 << ((bit) % 64)))
#define ST_CLR_BIT(st, bit) ((st)[(bit) / 64] &= ~((uint64_t) 1 << ((bit) % 64)))
#define IS_ST_BIT(st, bit) ((st)[(bit) / 64] & ((uint64_t) 1 << ((bit) % 64)))

using namespace std;
void hexToBinReadable(unsigned char hex[], unsigned char bin[], int len);
void mapVarToStateBits(uint64_t st[], vector<uint> indices, uint count);
void fillRandomKey(uint64_t st[]);

typedef unsigned int uint;
typedef pair<Cube::cubeIterator, Cube::cubeIterator> CubeRange;

typedef struct sol2
{
  map<uint, uint> equations;
  map<uint, uint> constants;
  uint cubeVars[31] = {};
  uint cubeVarsLen = 0;
};
sol2 solutions2[1000];
int solIndex2 = 0;
int wrongCounter = 0;

void readEquations2()
{
  string line;
  ifstream cubes ("bin/cubes4rounds2.txt");
  vector<uint> keyBits;
  char cubeIndexLine = 0;
  if (cubes.is_open())
  {
    while(getline(cubes,line))
    {
      if(line.compare("---------------------------------------------------------") == 0)
      {
        solIndex2++;
        cubeIndexLine = 0;
        continue;
      }

      if(!cubeIndexLine)
      {
        char *ptr;
        ptr = strtok((char*)line.c_str(), " ");
        ptr = strtok(NULL, " ");
        uint coeffNr = 0;
        while((ptr = strtok(NULL, " ")) && ptr != NULL && strcmp(ptr, "\r") != 0 && strcmp(ptr, "\n") != 0)
        {
          sscanf(ptr, "%uc", &(solutions2[solIndex2].cubeVars[coeffNr]));
          coeffNr++;
        }
        solutions2[solIndex2].cubeVarsLen = coeffNr;
        cubeIndexLine++;
      }
      else
      {
        char *ptr;
        char counter = 0, simple = 0;
        uint outputBit, keyBit;
        ptr = strtok((char*)line.c_str(), ":");
        sscanf(ptr, "%uc", &outputBit);
        ptr = strtok(NULL, ";");

        uint val, constant;
        sscanf(ptr, "%uc", &constant);
        while((ptr = strtok(NULL, ";")) && ptr != NULL && strcmp(ptr, "\r") != 0 && strcmp(ptr, "\n") != 0)
        {
          if(counter > 0)
          {
            simple = 0;
            break;
          }
          sscanf(ptr, "%uc", &val);

          keyBit = val;
          counter++;
          simple = 1;
        }

        vector<uint>::iterator it = find(keyBits.begin(), keyBits.end(), keyBit);
        if(simple && it == keyBits.end())
        {
          solutions2[solIndex2].equations.insert(pair<uint, uint>(keyBit, outputBit));
          solutions2[solIndex2].constants.insert(pair<uint, uint>(keyBit, constant));
          keyBits.push_back(keyBit);
        }
      }
    }
    printf("%d\n", keyBits.size());
    cubes.close();
  }
}

void checkEquations(uint64_t output[], uint64_t key[2], int solutionIndex)
{
  //Iterate over the equations
  for(auto eq: solutions2[solutionIndex].equations)
  {
    uint con = solutions2[solutionIndex].constants.find(eq.first)->second;

    uint val;
    if(IS_ST_BIT(output, eq.second))
      val = 1;
    else
      val = 0;

    uint keyBit = IS_ST_BIT(key, eq.first) == 0 ? 0 : 1;
    if((val ^ con) != keyBit)
    {
      unsigned char bin[129];
      hexToBinReadable((unsigned char*)key, bin, 16);
      hexToBinReadable((unsigned char*)output, bin, 16);
      wrongCounter++;

      printf("Not the correct: keyBit: %d and outputbit:%d with constant: %d\n", eq.first, eq.second, con);
    }
  }
}

void fillRandomKey(uint64_t st[])
{
  for (int i = 0; i < 4; i++)
    ((uint32_t *) st)[i] = rand();
}

void mapVarToStateBits(uint64_t st[], vector<uint> indices, uint count) {
  for (int i = 0; i < indices.size(); i ++) {
    if (count & (1 << i))
      ST_SET_BIT(st, indices[i]);
    else
      ST_CLR_BIT(st, indices[i]);
  }
}

int main()
{
  srand(time(NULL));

  //Read the file with the cubes inside
  readEquations2();

  printf("Equations read\n");

  for(int i = 0; i < solIndex2; i++)
  {
    printf("Checking solutions %d / %d\n", i, solIndex2);
    for(int z = 0; z < solutions2[i].cubeVarsLen; z++)
      printf("%d ", solutions2[i].cubeVars[z]);
    printf("\n");

    for(int j = 0; j < 100; j++)
    {
      uint64_t res[2], st[25];
      memset(st, 0, 25 * 8);
      memset(res, 0, 16);
      uint64_t key[2];
      fillRandomKey(key);

      for (uint64_t k = 0; k < ((uint64_t) 1 << solutions2[i].cubeVarsLen); k++)
      {
        memset(st, 0, 8 * 25);
        mapVarToStateBits(st, vector<uint>(solutions2[i].cubeVars, solutions2[i].cubeVars + solutions2[i].cubeVarsLen), k);
        memcpy(st, key, 16);

        keccakf(st, 4);

        res[0] ^= st[0];
        res[1] ^= st[1];
      }

      checkEquations(res, key, i);
    }

    printf("%d / 100 wrong!\n", wrongCounter);
    wrongCounter = 0;

  }

  return 0;
}
