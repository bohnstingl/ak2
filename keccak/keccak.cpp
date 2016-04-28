#include "keccak.h"
#include "stdio.h"
#include "time.h"
#include "inttypes.h"
#include <stdlib.h>
#include <thread>
#include <iostream>
#define THREADS 1

const uint64_t keccakf_rndc[24] = { 0x0000000000000001, 0x0000000000008082,
    0x800000000000808a, 0x8000000080008000, 0x000000000000808b,
    0x0000000080000001, 0x8000000080008081, 0x8000000000008009,
    0x000000000000008a, 0x0000000000000088, 0x0000000080008009,
    0x000000008000000a, 0x000000008000808b, 0x800000000000008b,
    0x8000000000008089, 0x8000000000008003, 0x8000000000008002,
    0x8000000000000080, 0x000000000000800a, 0x800000008000000a,
    0x8000000080008081, 0x8000000000008080, 0x0000000080000001,
    0x8000000080008008 };

const int keccakf_rotc[24] = { 1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 2, 14, 27,
    41, 56, 8, 25, 43, 62, 18, 39, 61, 20, 44 };

const int keccakf_piln[24] = { 10, 7, 11, 17, 18, 3, 5, 16, 8, 21, 24, 4, 15,
    23, 19, 13, 12, 2, 20, 14, 22, 9, 6, 1 };

//unsigned char globalKey[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
//unsigned char globalKey[16] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
unsigned char globalKey[16] = { 0x12, 0x71, 0xe1, 0xfa, 0x44, 0x29, 0x1f, 0xff, 0x63, 0x87, 0xaa, 0xcb, 0xee, 0x99, 0xec, 0x84};

// update the state with given number of rounds

void keccakf(uint64_t st[25], int rounds)
{
  int i, j, round;
  uint64_t t, bc[5];

  for (round = 0; round < rounds; round++)
  {

    // Theta
    for (i = 0; i < 5; i++)
      bc[i] = st[i] ^ st[i + 5] ^ st[i + 10] ^ st[i + 15] ^ st[i + 20];

    for (i = 0; i < 5; i++)
    {
      t = bc[(i + 4) % 5] ^ ROTL64(bc[(i + 1) % 5], 1);
      for (j = 0; j < 25; j += 5)
        st[j + i] ^= t;
    }

    // Rho Pi
    t = st[1];
    for (i = 0; i < 24; i++)
    {
      j = keccakf_piln[i];
      bc[0] = st[j];
      st[j] = ROTL64(t, keccakf_rotc[i]);
      t = bc[0];
    }

    //  Chi
    for (j = 0; j < 25; j += 5)
    {
      for (i = 0; i < 5; i++)
        bc[i] = st[j + i];
      for (i = 0; i < 5; i++)
        st[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
    }

    //  Iota
    st[0] ^= keccakf_rndc[round];
  }
}

void thetaRhoPi(uint64_t st[]) {
  int i, j;
  uint64_t t, bc[5];
  
  // Theta
  for (i = 0; i < 5; i++)
    bc[i] = st[i] ^ st[i + 5] ^ st[i + 10] ^ st[i + 15] ^ st[i + 20];

  for (i = 0; i < 5; i++)
  {
    t = bc[(i + 4) % 5] ^ ROTL64(bc[(i + 1) % 5], 1);
    for (j = 0; j < 25; j += 5)
      st[j + i] ^= t;
  }

  // Rho Pi
  t = st[1];
  for (i = 0; i < 24; i++)
  {
    j = keccakf_piln[i];
    bc[0] = st[j];
    st[j] = ROTL64(t, keccakf_rotc[i]);
    t = bc[0];
  }
}

void inverseIotaChi(uint64_t st[], int round) {
  st[0] ^= keccakf_rndc[round];
  inverseChi(st);
}

void chi(uint64_t st[])
{
  int i, j;
  uint64_t bc[5];
  for (j = 0; j < 25; j += 5)
  {
    for (i = 0; i < 5; i++)
      bc[i] = st[j + i];
    for (i = 0; i < 5; i++)
      st[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
  }
}

void inverseChi(uint64_t st[])
{
  uint64_t row[5];
  for (int r = 0; r < 25; r += 5) {
    memcpy(row, st + r, 5 * 8);
    
    st[r]     = row[0] ^ row[2] ^ row[4] ^ row[1] & row[2] ^ row[1] & row[4] ^ row[3] & row[4] ^ row[1] & row[3] & row[4];
    st[r + 1] = row[0] ^ row[1] ^ row[3] ^ row[0] & row[2] ^ row[0] & row[4] ^ row[2] & row[3] ^ row[0] & row[2] & row[4];
    st[r + 2] = row[1] ^ row[2] ^ row[4] ^ row[0] & row[1] ^ row[1] & row[3] ^ row[3] & row[4] ^ row[0] & row[1] & row[3];
    st[r + 3] = row[0] ^ row[2] ^ row[3] ^ row[0] & row[4] ^ row[1] & row[2] ^ row[2] & row[4] ^ row[1] & row[2] & row[4];
    st[r + 4] = row[1] ^ row[3] ^ row[4] ^ row[0] & row[1] ^ row[0] & row[3] ^ row[2] & row[3] ^ row[0] & row[2] & row[3];
  }
}

// compute a keccak hash (md) of given byte length from "in"

int keccak(const uint8_t *in, int inlen, uint8_t *md, int mdlen)
{
  uint64_t st[25];
  uint8_t temp[144];
  int i, rsiz, rsizw;

  rsiz = 200 - 2 * mdlen;
  rsizw = rsiz / 8;

  memset(st, 0, sizeof(st));

  /*for (; inlen >= rsiz; inlen -= rsiz, in += rsiz)
  {
    for (i = 0; i < rsizw; i++)
      st[i] ^= ((uint64_t *) in)[i];
    keccakf(st, KECCAK_ROUNDS);
  }*/

  // last block and padding
  memcpy(temp, in, inlen);
  temp[inlen++] = 1;
  memset(temp + inlen, 0, rsiz - inlen);
  temp[rsiz - 1] |= 0x80;

  for (i = 0; i < rsizw; i++)
    st[i] ^= ((uint64_t *) temp)[i];

  keccakf(st, KECCAK_ROUNDS);

  memcpy(md, st, mdlen);

  return 0;
}

void printHexMessage(unsigned char hash[], int len)
{
  int i;
  for (i = 0; i < len; i++)
  {
    printf("0x%02x, ", hash[i]);
    if (((i + 1) % 20) == 0)
      printf("\n");
  }
  printf("\n");
}

void keccakF(const uint8_t *in, int inlen, uint8_t *md, int mdlen)
{
  uint64_t st[25];
  uint8_t temp[200] = {0};
  int i, rsiz, rsizw;

  memcpy(temp, in, inlen);

  for (i = 0; i < 16; i++)
    st[i] ^= ((uint64_t *) temp)[i];

  keccakf(st, KECCAK_ROUNDS);

  memcpy((char*)md, (char*)st, mdlen * sizeof(char));
}

void xorCipher(unsigned char part1[], unsigned char part2[], int len)
{
  int i;
  for(i = 0; i < len; i++)
  {
    part1[i] = part1[i] ^ part2[i];
  }
}

void hexToBinReadable(unsigned char hex[], unsigned char bin[], int len)
{
  int counter = 0, i, j;
  for(i = 0; i < len; i++)
  {
    for(j = 0; j < 8; j++)
    {
      if((hex[i] & (1 << j)) == 0)
      {
        bin[counter] = 0x30;
      }
      else
      {
        bin[counter] = 0x31;
      }
      printf("%c", bin[counter]);
      counter++;
    }
  }
  printf("\n");
}

void generateCube(unsigned char cubeIndices[], int variables, unsigned char output[][128], int amount, unsigned long long startIndex)
{
  unsigned char c = 0x00;
  unsigned int i, j;

  for (i = 0; i < amount; i++)
  {
    //Copy the key into the message
    memcpy(output[i], globalKey, 16);

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

void threadFunction(unsigned char *cubeIndex, unsigned long long counter, unsigned char *res)
{
  unsigned char input[8192][128];
  unsigned int i, j;
  unsigned char cipher[64] = {0}, sum[16] = {0};

  //Generate a part of the cube
  generateCube(cubeIndex, 13, input, 8192, counter);

  for(j = 0; j < 8192; j++)
  {
    //Calculate the MAC of the message and xor it with others
    keccakF(input[j], 128, cipher, 16);
    xorCipher(sum, cipher, 16);
  }

  //Copy the final results
  memcpy(res, sum, 16);
}


/*int main()
{
  unsigned char input[8192][128];
  unsigned char cubeIndices1[31] = {131, 142, 152, 167, 168, 174, 178, 188, 190, 205, 211, 223, 235},
                cubeIndices2[31] = {138, 140, 142, 153, 159, 165, 166, 180, 204, 206, 210, 221, 223},
                cubeIndices3[31] = {129, 144, 149, 164, 166, 176, 186, 192, 205, 206, 212, 219, 221},
                cubeIndices4[31] = {132, 138, 141, 145, 149, 153, 154, 178, 183, 207, 209, 235, 245},
                cubeIndices5[31] = {132, 140, 156, 161, 164, 176, 185, 186, 191, 199, 206, 223, 225, 233},
                cubeIndices6[31] = {128, 130, 131, 139, 145, 146, 147, 148, 151,
                                    155, 158, 160, 161, 163, 164, 165, 185, 186,
                                    189, 190, 193, 196, 205, 212, 220, 225, 229,
                                    238, 242, 245, 249};
  unsigned char cipher[64] = {0}, sum[16] = {0};
  unsigned int i, j;

  std::thread t[THREADS];

  //Key
  //00000000 10000000 01000000 11000000 00100000 10100000 01100000 11100000 00010000 10010000 01010000 11010000 00110000 10110000 01110000 11110000

  srand(time(NULL));

  //unsigned char bin[16*8];
  //hexToBinReadable(res5, bin, 16);
  //simpleCheckEquations();

  //exit(0);

  unsigned long long counter = 0;

  //Caution. Do not count till the end. Just count to 0xffffffff
  for(i = 0; i < 1; i++)
  {
    unsigned char thres[THREADS][16];
    for(j = 0; j < THREADS; j++)
    {
        t[j] = std::thread(threadFunction, cubeIndices1, counter, thres[j]);
        counter += 8192;
    }

    for(j = 0; j < THREADS; j++)
    {
        t[j].join();
        xorCipher(sum, thres[j], 16);
        //printHexMessage(sum, 16);
    }
  }

  printf("finished computation\n");
  printHexMessage(sum, 16);

  return 0;
}*/
