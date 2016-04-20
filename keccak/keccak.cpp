#include "keccak.h"
#include "stdio.h"
#include "time.h"
#include "inttypes.h"
#include <stdlib.h>
#include <thread>
#include <iostream>
#define THREADS 8

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

unsigned char key[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

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

void generateRandomData2(unsigned char input[], int amount)
{
  int remain = amount;
  while (remain > 0)
  {
    int toCopy = 0;
    if (remain >= 8)
    {
      toCopy = 8;
    }
    else
    {
      toCopy = remain;
    }
    uint64_t in = rand();
    in = (in << 32) | rand();
    memcpy(input + (amount - remain), &in, toCopy * sizeof(unsigned char));
    remain -= toCopy;
  }
}

void printHexMessage(unsigned char hash[], int len)
{
  int i;
  for (i = 0; i < len; i++)
  {
    printf("%02x ", hash[i]);
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

  if((inlen % 8) == 0)
  {
    rsizw = inlen / 8;
  }
  else
  {
    rsizw = (inlen / 8) + 1;
  }

  rsizw = 21; //9
  rsiz = 168; //72
  memset(st, 0, sizeof(st));

  memcpy(temp, in, inlen);
  temp[inlen++] = 1;
  memset(temp + inlen, 0, rsiz - inlen);
  temp[rsiz - 1] |= 0x80;

  for (i = 0; i < rsizw; i++)
    st[i] ^= ((uint64_t *) temp)[i];

  keccakf(st, KECCAK_ROUNDS);

  memcpy((char*)md, (char*)st, mdlen * sizeof(char));
  //printHexMessage(md, 16);
}

void generateCube(unsigned char cubeIndices[], int variables, unsigned char output[][128], int amount, unsigned long long startIndex)
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

void xorCipher(unsigned char part1[], unsigned char part2[], int len)
{
  int i;
  for(i = 0; i < len; i++)
  {
    part1[i] = part1[i] ^ part2[i];
  }
}

void simpleCheckEquations()
{
  unsigned char testCipher[1600] = "00000101000001000000000010000001000000001010101000000000001000000000010000000100000000010001000010000000000000001010000000000100",
                   testKey[1600] = "00000000100000000100000011000000001000001010000001100000111000000001000010010000010100001101000000110000101100000111000011110000";
  unsigned char checkCipherE[15] = {7, 84, 112, 31, 87},
                checkCipherN[15] = {15, 42, 96, 13, 69, 100},
                checkKeyE[15] = {77, 44, 17, 25, 123},
                checkKeyN[15] = {113, 103, 100, 110, 105, 104};
  unsigned int i, j;

  int matches = 0, total = 0;
  for(i = 0; i < 5; i++)
  {
    if(testCipher[checkCipherE[i]] == testKey[checkKeyE[i]])
    {
      matches++;
    }
    total++;
  }
  for(i = 0; i < 6; i++)
  {
    if(testCipher[checkCipherN[i]] != testKey[checkKeyN[i]])
    {
      matches++;
    }
    total++;
  }
  printf("Matches %d out of %d\n", matches, total);

  printf("%c %c %c\n", "1", testKey[7], testKey[19]);
  printf("%c\n", testCipher[91]);

  printf("%c %c %c\n", testKey[17], testKey[68], testKey[116]);
  printf("%c\n", testCipher[114]);

  printf("%c %c\n", testKey[38], testKey[51]);
  printf("%c\n", testCipher[71]);

  printf("%c %c %c\n", "1", testKey[80], testKey[122]);
  printf("%c\n", testCipher[113]);
}

void hexToBinReadable(unsigned char hex[], unsigned char bin[], int len)
{
  int counter = 0, i, j;
  for(i = 0; i < 16; i++)
  {
    for(j = 0; j < 8; j++)
    {
      if((key[i] & (1 << j)) == 0)
      {
        //bin[counter] = (unsigned char*)"0";
      }
      else
      {
        //bin[counter] = (unsigned char*)"1";
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
  generateCube(cubeIndex, 31, input, 8192, counter);

  for(j = 0; j < 8192; j++)
  {
    //Calculate the MAC of the message and xor it with others
    keccakF(input[j], 128, cipher, 16);
    xorCipher(sum, cipher, 16);
  }

  //Copy the final results
  memcpy(res, sum, 16);
}

int main()
{
  unsigned char input[8192][128];
  unsigned char cubeIndex[31] = {128, 130, 131, 139, 145, 146, 147, 148, 151,
                                 155, 158, 160, 161, 163, 164, 165, 185, 186,
                                 189, 190, 193, 196, 205, 212, 220, 225, 229,
                                 238, 242, 245, 249};
  unsigned char cipher[64] = {0}, sum[16] = {0};
  unsigned int i, j;

  std::thread t[THREADS];

  //First results
  //a0 20 00 81 00 55 00 04 20 20 80 08 01 00 05 20
  //00000101 00000100 00000000 10000001 00000000 10101010 00000000 00100000 00000100 00000100 00000001 00010000 10000000 00000000 10100000 00000100
  //Key
  //00000000 10000000 01000000 11000000 00100000 10100000 01100000 11100000 00010000 10010000 01010000 11010000 00110000 10110000 01110000 11110000

  srand(time(NULL));

  /*keccakF("Keccak-512 Test Hash", 20, cipher, 64);
  printf("\n\nOutput\n");
  printHexMessage(cipher, 16);*/

  unsigned long long counter = 0;

  //Caution. Do not count till the end. Just count to 0xffffffff
  for(i = 0; i < 262144; i += THREADS)
  {
    unsigned char thres[THREADS][16];
    for(j = 0; j < THREADS; j++)
    {
        t[j] = std::thread(threadFunction, cubeIndex, counter, thres[j]);
        counter += 8192;
    }

    for(j = 0; j < THREADS; j++)
    {
        t[j].join();
        xorCipher(sum, thres[j], 16);
        //printHexMessage(sum, 16);
    }
    /*//Generate a part of the cube
    generateCube(cubeIndex, 31, input, 8192, counter);
    counter += 8192;

    for(j = 0; j < 8192; j++)
    {
      //Calculate the MAC of the message and xor it with others
      keccakF(input[j], 128, cipher, 16);
      xor(sum, cipher, 16);
    }
    if(((i + 1) % 5000) == 0)
        printf("It: %6d\n", i);*/
  }

  printf("finished computation\n");
  printHexMessage(sum, 16);

  //Keccak(1024, 576, input, 128, 0, output, 16);
  /*keccak("Keccak-512 Test Hash", 20, output, 64);
  printf("\n\nOutput\n");
  printHexMessage(output, 64);*/

  return 0;
}
