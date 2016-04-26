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
extern unsigned char globalKey[16];

typedef struct sol
{
  map<uint, uint> equations;
  map<uint, uint> constants;
  uint cubeVars[31] = {};
  uint cubeVarsLen = 0;
};

unsigned char recoveredKey[16];
sol solutions[1000];
int solIndex = 0;

void readEquations()
{
  string line;
  //ifstream cubes ("/home/thomas/workspace/AKCpp/src/cubes4_manually.txt");
  ifstream cubes ("/home/thomas/workspace/AKCpp/src/cubes1.txt");
  //ifstream cubes ("/home/thomas/workspace/AKCpp/src/cubes5_manually.txt");
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

        vector<uint>::iterator it = find(keyBits.begin(), keyBits.end(), keyBit);
        if(simple && it == keyBits.end())
        {
          solutions[solIndex].equations.insert(pair<uint, uint>(keyBit, outputBit));
          solutions[solIndex].constants.insert(pair<uint, uint>(keyBit, constant));
          keyBits.push_back(keyBit);
        }
      }
    }
    printf("%d\n", keyBits.size());
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
    /*if(output[eq.second] == 0x30)
      val = 0;
    else
      val = 1;*/

    uint res = con ^ val;
    if(res)
      recoveredKey[eq.first / 8] |= 1 << (eq.first % 8);
    //else
    //  recoveredKey[eq.first / 8] &= ~(1 << (eq.first % 8));
  }

  //printHexMessage(recoveredKey, 16);
}

int main()
{
  srand(time(NULL));
  uint64_t key2[2];
  memset(recoveredKey, 0, 16);
  memcpy(key2, globalKey, 16);

  //Read the file with the cubes inside
  readEquations();

  //Iterate over the solutions.
  //1.) Compute the output of the certain cube
  //2.) Use equations and constants to recover key bits

  for(int i = 0; i < solIndex; i++)
  {
    Cube cube(2);
    cube.addArray(solutions[i].cubeVars, solutions[i].cubeVarsLen);
    //cube.addArray(cubeIndices1, 13);

    uint64_t res[2];
    memcpy(key2, globalKey, 16);
    memset(res, 0, 16);

    cube.deriveParallel(key2, res);
    //printHexMessage((unsigned char*)res, 16);

    recoverKey(res, i);
    //printf("-----\n");
  }

  printHexMessage(recoveredKey, 16);

  return 0;
}
