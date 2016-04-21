#include <iostream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <vector>
#include <thread>
#include "keccak.h"

#define TAG_SIZE 16
#define MSG_SIZE 112
#define KEY_SIZE 16
#define N_THREADS 8
void printHexMessage(unsigned char hash[], int len);

using namespace std;

typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned long long uint64;

// permutation function
void f(byte *key, byte *message, byte *tag);

void generateCubeWithKey(byte *key, uint cubeIndices[], int variables, unsigned char output[][128], int amount, unsigned long long startIndex)
{
    unsigned char c = 0x00;
    unsigned int i, j;

    for (i = 0; i < amount; i++)
    {
        //Copy the key into the message
        memcpy(output[i], key, 16);

        //Initialise the constant message
        memset(output[i] + 16, c, 112);

        for(j = 0; j < variables; j++)
        {
            //Split counter into variables bits
            if(((startIndex + i) & (1 << j)) == 0)
            {
                output[i][cubeIndices[j] / 8] &= ~(1 << (cubeIndices[j] % 8));
            }
            else
            {
                output[i][cubeIndices[j] / 8] |= 1 << (cubeIndices[j] % 8);
            }
        }
    }
}

void randomCube(vector<uint> &indices, uint dim, uint min, uint max) {
    uint i = 0, randi;

    if (max - min < dim)
        throw -1;

    for (; i < dim; i ++) {
        randi = rand() % (max - min) + min;
        if (find(indices.begin(), indices.end(), randi) == indices.end())
            indices.push_back(randi);
        else
            i --;
    }

    sort(indices.begin(), indices.end());
}

void sumPartial(byte key[], vector<uint> indices, uint start, uint step, byte result[]) {
    byte partOfCube[step][128];
    byte temp[16] = { 0 };
    memset(result, 0, 16);

    generateCubeWithKey(key, &indices[0], indices.size(), partOfCube, step, start);

    for (uint j = 0; j < step; j++) {
        keccakF(partOfCube[j], 128, temp, 16);
        xorCipher(result, temp, 16);
    }
}

void computeCoefficients(vector<uint> &indices, byte coefficients[][16]) {
    uint i, j;
    uint granularity = 8192;
    byte partOfCube[granularity][128];

    byte result[16] = {0}, sum[16];
    byte key[16];
    byte results[N_THREADS][16];
    thread threads[N_THREADS];

    for (int k = 44; k < 45; k ++) {
        memset(sum, 0, 16);
        memset(key, 0, 16);
        if (k >= 0)
            key[k / 8] = 1 << (k % 8);

        for (i = 0; i < (1 << indices.size()); i += granularity * N_THREADS) {
            for (uint th = 0; th < N_THREADS; th ++) {
                threads[th] = thread(&sumPartial, key, indices, i + th * granularity, granularity, results[th]);
            }
            
            for (uint th = 0; th < N_THREADS; th ++) {
                threads[th].join();
                xorCipher(sum, results[th], 16);
            }
            if (i % (granularity * N_THREADS) == 0)
                cout << ".";
        }
        cout << endl;

        memcpy(coefficients[k + 1], sum, 16);
    }
}

int main() {
    srand(time(NULL));
    vector<uint> cubeIndices;
    byte coefficients[129][16] = {0};
    byte zero[129][16] = {0};

    //randomCube(cubeIndices, 16, 128, 256);
    uint ind[] = //{ 133 ,137 ,141 ,143 ,154 ,156 ,177, 184, 196 ,203 ,209 ,227, 230 };
    {128, 130, 131, 139, 145, 146, 147, 148, 151, 155, 158, 160, 161,163,164,165,185,186,189,190,193,196,205,212,220,225,229,238,242,245,249};
    for (int i = 0; i < 31; i ++)
        cubeIndices.push_back(ind[i]);

    cout << "Random cube variables: ";
    for (int i = 0; i < cubeIndices.size(); i ++)
        cout << cubeIndices[i] << " ";
    cout << endl;

    do {
        //cubeIndices.pop_back();
        computeCoefficients(cubeIndices, coefficients);
        cout << "computed..." << endl;
    } while (memcmp(coefficients, zero, 129 * 16) == 0);
    for (int i = 0; i < 129; i ++) {
        for (int j = 0; j < 16; j ++) {
            cout << setw(2) << setfill('0');
            cout << hex << (int) coefficients[i][j] << " ";
        }
        cout << endl;
    }
    return 0;
}