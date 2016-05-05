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

typedef unsigned int uint;
typedef pair<Cube::cubeIterator, Cube::cubeIterator> CubeRange;

void printHexMessage(unsigned char hash[], int len);
void hexToBinReadable(unsigned char hex[], unsigned char bin[], int len);

typedef struct 
{
  map<uint, uint> equations;
  map<uint, uint> constants;
  uint cubeVars[31] = {};
  uint cubeVarsLen = 0;
} sol;

unsigned char recoveredKey[16];
sol solutions[100000];
vector<int> keyBitSolIndices;
int solIndex = 0;

void fillRandomKey(uint64_t st[])
{
  for (int i = 0; i < 4; i++)
    ((uint32_t *) st)[i] = rand();
}

void readEquations(char argv[])
{
  string line;
  string filePath(argv);
  ifstream cubes (filePath);
  //ifstream cubes ("./cubes.txt");
  vector<uint> keyBits;
  char cubeIndexLine = 0;
  if (cubes.is_open())
  {
    while(getline(cubes,line))
    {
      if(line.compare("---------------------------------------------------------") == 0)
      {
        solIndex++;
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

        if(simple && find(keyBits.begin(), keyBits.end(), keyBit) == keyBits.end())
        {
          solutions[solIndex].equations.insert(pair<uint, uint>(keyBit, outputBit));
          solutions[solIndex].constants.insert(pair<uint, uint>(keyBit, constant));
          keyBits.push_back(keyBit);
          if(find(keyBitSolIndices.begin(), keyBitSolIndices.end(), solIndex) == keyBitSolIndices.end())
            keyBitSolIndices.push_back(solIndex);
        }
      }
    }
    printf("%ld\n", keyBits.size());
    cubes.close();
  }
}

void recoverKey(uint64_t output[], int solutionIndex)
{
  //Iterate over the equations
  for(auto eq: solutions[solutionIndex].equations)
  {
    uint con = solutions[solutionIndex].constants.find(eq.first)->second;

    uint val;
    if(IS_ST_BIT(output, eq.second))
      val = 1;
    else
      val = 0;

    uint res = con ^ val;
    if(res)
      recoveredKey[eq.first / 8] |= 1 << (eq.first % 8);
    else
      recoveredKey[eq.first / 8] &= ~(1 << (eq.first % 8));
  }
}

int main(int argc, char * argv[])
{
  srand(time(NULL));
  uint64_t key2[2];
  memset(recoveredKey, 0, 16);
  memset(key2, 0, 16);
  
  if(argc != 2)
  {
    printf("Missing file:\nUSAGE: ./recoverKey <path to cube file>\n");
    exit(0);
  }

  //Read the file with the cubes inside
  readEquations(argv[1]);

  //Iterate over the solutions.
  //1.) Compute the output of the certain cube
  //2.) Use equations and constants to recover key bits

  for(int k = 0; k < 100; k++)
  {
    fillRandomKey(key2);
    memset(recoveredKey, 0, 16);

    for(unsigned int i = 0; i < keyBitSolIndices.size(); i++)
    {
      int index = keyBitSolIndices.at(i);
      Cube cube(4);
      cube.addArray(solutions[index].cubeVars, solutions[index].cubeVarsLen);

      uint64_t res[2];
      memset(res, 0, 16);

      cube.deriveParallel(key2, res, 0);

      recoverKey(res, index);
    }

    if(memcmp(recoveredKey, key2, 16) != 0)
    {
      printf("Key recovery failed!\n");
      unsigned char bin[129];
      printHexMessage(recoveredKey, 16);
      printHexMessage((unsigned char*)key2, 16);
      hexToBinReadable(recoveredKey, bin, 16);
      hexToBinReadable((unsigned char*)key2, bin, 16);
      exit(0);
    }
    else
    {
      printHexMessage(recoveredKey, 16);
      printHexMessage((unsigned char*)key2, 16);
      printf("Key recovery successful %d / 100\n", k + 1);
    }
  }

  printf("Programm ended successfully!\n");

  return 0;
}
