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
  ifstream cubes ("/home/thomas/workspace/AKCpp/src/cubes4.txt");
  //ifstream cubes ("/home/thomas/workspace/AKCpp/src/cubes5_manually.txt");
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

        if(simple && solutions[solIndex].equations.count(keyBit) == 0)
        {
          solutions[solIndex].equations.insert(pair<uint, uint>(keyBit, outputBit));
          solutions[solIndex].constants.insert(pair<uint, uint>(keyBit, constant));
        }
      }
    }
    cubes.close();
  }
}

void recoverKey(unsigned char output[], int solutionIndex)
{
  //Iterate over the equations
  for(auto eq: solutions[solutionIndex].equations)
  {
    uint con = solutions[solutionIndex].constants.find(eq.first)->second;

    uint val;
    if(output[eq.second] == 0x30)
      val = 0;
    else
      val = 1;

    uint res = con ^ val;
    if(res)
      recoveredKey[eq.first / 8] |= 1 << (eq.first % 8);
    //else
    //  recoveredKey[eq.first / 8] &= ~(1 << (eq.first % 8));
  }

  printHexMessage(recoveredKey, 16);
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
    Cube cube(4);
    cube.addArray(solutions[i].cubeVars, solutions[i].cubeVarsLen);

    uint64_t res[2];
    memset(res, 0, 16);
    unsigned char msg[16];

    cube.deriveParallel(key2, res);

    memcpy(msg, res, 16);
    printHexMessage(msg, 16);
    unsigned char binary[129];
    hexToBinReadable(msg, binary, 16);
    recoverKey(binary, i);
    printf("-----\n");

//    unsigned char binary2[129] = "10101000010011001001010000011110000100000100100010000001010000000001001000101001001100001000001101110010100010000100010101111000";
//    recoverKey(binary2, 1);
//    unsigned char binary3[129] = "00000000100000000000000000010010000000000000101000010000000000000000000000100010110000000000111000010100000000100010000000001100";
//    recoverKey(binary3, 2);
//    unsigned char binary4[129] = "11000000001100010100011110011001101000010000011000010011000100011000000100000000010011000011110001010010011000010000001000000001";
//    recoverKey(binary4, 3);
//    unsigned char binary5[129] = "00100000101000000000000000000000010000000000001110010100010001000000000101000000100000000001000000001001000000101101000100000000";
//    recoverKey(binary5, 4);
//    unsigned char binary6[129] = "00000000010010000000111000010000000000000000000001000101000010000010100010000001000000000101000011010000000000010000000010000010";
//    recoverKey(binary6, 5);
//    unsigned char binary7[129] = "00000000000100001101000011110000100000000000001000000000010010000001000000000100010000010000001000000101000000000000000000000011";
//    recoverKey(binary7, 6);
//    exit(0);
  }

  return 0;
}
