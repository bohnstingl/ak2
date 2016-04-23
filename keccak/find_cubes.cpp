#include <iostream>
#include <iomanip>
#include "cube.h"

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

int main() {
    srand(time(NULL));

    uint64_t sum[2] = { 0 };
    uint64_t key[2] = { 0 };
    memset(key, 0, 16);
    uint loop_var;
    bool found;

    for (uint amount = 0; amount < 1000; amount ++) {
        Cube cube;
        cube.keccak_rounds_ = 4;
        cube.randomCube((1 << cube.keccak_rounds_) - 1, 128, 256);
        //cube.push_back(128);
        uint64_t coefficients[129][2] = { 0 };
        memset(coefficients, 0, 129 * 2 * 8);
        do {
            //cout << "Trying cube..." << endl;
            //cube.printVariables();

            //cout << "determining coefficients" << flush;
            for (int key_bit = -1; key_bit < 128; key_bit++) {
                memset(key, 0, 16);
                //memset(sum, 0, 16);
                if (key_bit >= 0)
                    ST_SET_BIT(key, key_bit);

                cube.deriveParallel(key, coefficients[key_bit + 1]);
                if (key_bit >= 0) {
                    coefficients[key_bit + 1][0] ^= coefficients[0][0];
                    coefficients[key_bit + 1][1] ^= coefficients[0][1];
                }
                //cout << "." << flush;
            }
            //cout << endl;

            found = 0;
            for (loop_var = 2; loop_var < 129 * 2; loop_var++) { // constant is not enough
                if (coefficients[loop_var / 2][loop_var % 2]) {
                    found = true;
                    break;
                }
            }
            if (found)
                break;

            cube.pop_back();
        } while (cube.size() >= 13); // 1

        if (found) {
            cube.printVariables();
            //printCoefficientsReadable(coefficients);
            printCoefficientsMachine(coefficients);
            cout << "---------------------------------------------------------" << endl;
        }
    }
    
    return 0;
}
