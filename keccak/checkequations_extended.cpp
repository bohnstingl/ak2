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
void inverseIotaChi(uint64_t st[], int round);
void inverseIotaChiRow(uint64_t row[], int round_to_invert);

typedef unsigned int uint;
typedef pair<Cube::cubeIterator, Cube::cubeIterator> CubeRange;

typedef struct sol
{
  map<uint, uint> equations;
  map<uint, uint> constants;
  uint cubeVars[31] = { };
  uint cubeVarsLen = 0;
};
sol solutions[1000];
int solIndex = 0;
int wrongCounter = 0;

void readEquations(char argv[])
{
  string line;
  string path(argv);
  ifstream cubes(path);
  vector<uint> keyBits;
  char cubeIndexLine = 0;
  if (cubes.is_open())
  {
    while (getline(cubes, line))
    {
      if (line.compare(
          "---------------------------------------------------------") == 0)
      {
        solIndex++;
        cubeIndexLine = 0;
        continue;
      }

      if (!cubeIndexLine)
      {
        char *ptr;
        ptr = strtok((char*) line.c_str(), " ");
        ptr = strtok(NULL, " ");
        uint coeffNr = 0;
        while ((ptr = strtok(NULL, " ")) && ptr != NULL
            && strcmp(ptr, "\r") != 0 && strcmp(ptr, "\n") != 0)
        {
          sscanf(ptr, "%uc", &(solutions[solIndex].cubeVars[coeffNr]));
          coeffNr++;
        }
        solutions[solIndex].cubeVarsLen = coeffNr;
        cubeIndexLine++;
      }
      else
      {
        char *ptr;
        char counter = 0, simple = 0;
        uint outputBit, keyBit;
        ptr = strtok((char*) line.c_str(), ":");
        sscanf(ptr, "%uc", &outputBit);
        ptr = strtok(NULL, ";");

        uint val, constant;
        sscanf(ptr, "%uc", &constant);
        while ((ptr = strtok(NULL, ";")) && ptr != NULL
            && strcmp(ptr, "\r") != 0 && strcmp(ptr, "\n") != 0)
        {
          if (counter > 0)
          {
            simple = 0;
            break;
          }
          sscanf(ptr, "%uc", &val);

          keyBit = val;
          counter++;
          simple = 1;
        }

        vector<uint>::iterator it = find(keyBits.begin(), keyBits.end(),
            keyBit);
        if (simple && it == keyBits.end())
        {
          solutions[solIndex].equations.insert(
              pair<uint, uint>(keyBit, outputBit));
          solutions[solIndex].constants.insert(
              pair<uint, uint>(keyBit, constant));
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
  for (auto eq : solutions[solutionIndex].equations)
  {
    uint con = solutions[solutionIndex].constants.find(eq.first)->second;

    uint val;
    if (IS_ST_BIT(output, eq.second))
      val = 1;
    else
      val = 0;

    uint keyBit = IS_ST_BIT(key, eq.first) == 0 ? 0 : 1;
    if ((val ^ con) != keyBit)
    {
      unsigned char bin[129];
      hexToBinReadable((unsigned char*) key, bin, 16);
      hexToBinReadable((unsigned char*) output, bin, 16);
      wrongCounter++;
      for (int i = 0; i < solutions[solutionIndex].cubeVarsLen; i++)
        printf("%u, ", solutions[solutionIndex].cubeVars[i]);
      printf("\n");
      printf("Not the correct: keyBit: %d and outputbit:%d with constant: %d\n",
          eq.first, eq.second, con);
    }
  }
}

void fillRandomKey(uint64_t st[])
{
  for (int i = 0; i < 4; i++)
    ((uint32_t *) st)[i] = rand();
}

void mapVarToStateBits(uint64_t st[], vector<uint> indices, uint count)
{
  for (int i = 0; i < indices.size(); i++)
  {
    if (count & (1 << i))
      ST_SET_BIT(st, indices[i]);
    else
      ST_CLR_BIT(st, indices[i]);
  }
}

int main(int argc, char * argv[])
{
  srand(time(NULL));

  //Read the file with the cubes inside
  readEquations(argv[1]);

  printf("Equations read\n");

  for (int i = 0; i < solIndex; i++)
  {
    printf("Checking solutions %d / %d\n", i, solIndex);

    for (int j = 0; j < 100; j++)
    {
      uint64_t res[5], st[25];
      memset(st, 0, 25 * 8);
      memset(res, 0, 8 * 5);
      uint64_t key[2];
      fillRandomKey(key);

      for (uint64_t k = 0; k < ((uint64_t) 1 << solutions[i].cubeVarsLen); k++)
      {
        memset(st, 0, 8 * 25);
        mapVarToStateBits(st,
            vector<uint>(solutions[i].cubeVars,
                solutions[i].cubeVars + solutions[i].cubeVarsLen), k);
        memcpy(st, key, 16);

        keccakf(st, 5);

        //Apply the inverse chi and the iota function
        inverseIotaChi(st, 5);

        res[0] ^= st[0];
        res[1] ^= st[1];
        res[2] ^= st[2];
        res[3] ^= st[3];
        res[4] ^= st[4];
      }

      checkEquations(res, key, i);
    }

    printf("%d / 100 wrong!\n", wrongCounter);
    wrongCounter = 0;

  }

  return 0;
}
