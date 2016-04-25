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

unsigned char globalKey[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

//unsigned char globalKey[16] = { 0x12, 0x71, 0xe1, 0xfa, 0x44, 0x29, 0x1f, 0xff,
//                                0x63, 0x87, 0xaa, 0xcb, 0xee, 0x99, 0xec, 0x84};

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

/*int main()
{
  unsigned char input[8192][128];
  unsigned char cubeIndices1[31] = {128, 130, 131, 139, 145, 146, 147, 148, 151,
                                 155, 158, 160, 161, 163, 164, 165, 185, 186,
                                 189, 190, 193, 196, 205, 212, 220, 225, 229,
                                 238, 242, 245, 249},
                cubeIndices2[31] = {128, 129, 134, 135, 138, 139, 141, 151, 154,
                                 155, 157, 166, 168, 171, 175, 180, 191, 193,
                                 198, 202, 203, 206, 209, 214, 216, 222, 223,
                                 225, 239, 246, 247},
                cubeIndices3[31] = {128, 129, 130, 132, 137, 142, 152, 153, 155,
                                 160, 164, 165, 166, 175, 187, 196, 200, 201,
                                 205, 212, 217, 221, 222, 226, 228, 235, 237,
                                 242, 243, 246, 252},
                cubeIndices4[31] = {128, 129, 135, 137, 140, 145, 150, 152, 162,
                                 163, 164, 166, 170, 175, 179, 181, 186, 187,
                                 198, 202, 209, 216, 220, 221, 222, 230, 234,
                                 240, 241, 245, 248},
                cubeIndices5[31] = {131, 132, 134, 136, 138, 142, 143, 147, 152,
                                 158, 165, 167, 171, 172, 173, 180, 186, 196,
                                 206, 208, 213, 214, 217, 219, 226, 233, 235,
                                 237, 239, 250, 251};
  unsigned char cipher[64] = {0}, sum[16] = {0};
  unsigned int i, j;

  std::thread t[THREADS];

  //results
  //unsigned char res1[16] = {0xa0, 0x20, 0x00, 0x81, 0x00, 0x55, 0x00, 0x04, 0x20, 0x20, 0x80, 0x08, 0x01, 0x00, 0x05, 0x20};
  //00000101 00000100 00000000 10000001 00000000 10101010 00000000 00100000 00000100 00000100 00000001 00010000 10000000 00000000 10100000 00000100

  //unsigned char res2[16] = {0x15, 0x32, 0x29, 0x78, 0x08, 0x12, 0x81, 0x02, 0x48, 0x94, 0x0c, 0xc1, 0x4e, 0x11, 0xa2, 0x1e};
  //10101000 01001100 10010100 00011110 00010000 01001000 10000001 01000000 00010010 00101001 00110000 10000011 01110010 10001000 01000101 01111000

  //unsigned char res3[16] = {0x00, 0x01, 0x00, 0x48, 0x00, 0x50, 0x08, 0x00, 0x00, 0x44, 0x03, 0x70, 0x28, 0x40, 0x04, 0x30};
  //00000000 10000000 00000000 00010010 00000000 00001010 00010000 00000000 00000000 00100010 11000000 00001110 00010100 00000010 00100000 00001100

  //unsigned char res4[16] = {0x03, 0x8c, 0xe2, 0x99, 0x85, 0x60, 0xc8, 0x88, 0x81, 0x00, 0x32, 0x3c, 0x4a, 0x86, 0x40, 0x80};
  //11000000 00110001 01000111 10011001 10100001 00000110 00010011 00010001 10000001 00000000 01001100 00111100 01010010 01100001 00000010 00000001

  //unsigned char res5[16] = {0x04, 0x05, 0x00, 0x00, 0x02, 0xc0, 0x29, 0x22, 0x80, 0x02, 0x01, 0x08, 0x90, 0x40, 0x8b, 0x00};
  //00100000 10100000 00000000 00000000 01000000 00000011 10010100 01000100 00000001 01000000 10000000 00010000 00001001 00000010 11010001 00000000

  //Possible key recover bits
  // 77, 113, 103,  44, 100,  17, 110, 25,
  //105, 123, 104,  50, 109, 101,  15,  4,
  //126,  33,  78, 124,  92,  85,  67, 90,
  // 84,   6,  48,  96,  70,  30,  57, 53,
  // 46, 118,  49,  58,  24,  65,  16, 71,
  // 83,  64,   3, 114,  38, 125,  87, 74,
  // 10,  11, 127, 102,  19,  69,   2

  //  2   3   4   6  10  11  15  16  17  19  24  25  30  33
  // 38  44  46  48  49  50  53  57  58  64  65  67  69  70
  // 71  74  77  78  83  84  85  87  90  92  96 100 101 102
  //103 104 105 109 110 113 114 118 123 124 125 126 127

  //Key
  //00000000 10000000 01000000 11000000 00100000 10100000 01100000 11100000 00010000 10010000 01010000 11010000 00110000 10110000 01110000 11110000

  srand(time(NULL));

  //unsigned char bin[16*8];
  //hexToBinReadable(res5, bin, 16);
  simpleCheckEquations();

  exit(0);

  unsigned long long counter = 0;

  //Caution. Do not count till the end. Just count to 0xffffffff
  for(i = 0; i < 262144; i += THREADS)
  {
    unsigned char thres[THREADS][16];
    for(j = 0; j < THREADS; j++)
    {
        t[j] = std::thread(threadFunction, cubeIndices5, counter, thres[j]);
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
