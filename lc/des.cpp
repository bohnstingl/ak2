/******************************************
** DES-Implementation
** Author: B-Con (b-con@b-con.us)
** Copyright/Restrictions: GNU GPL
** Disclaimer: This code is presented "as is" without any garuentees; said author holds
               liability for no problems rendered by the use of this code.
** Details: This code is the implementation of the DES algorithm, as specified by the
            NIST in in publication FIPS PUB 197, availible on the NIST website at
            http://csrc.nist.gov/publications/fips/fips46-3/fips46-3.pdf .
******************************************/
#include <time.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <vector>
#include <thread>

using namespace std;

#define uchar unsigned char
#define uint unsigned int
#define ENCRYPT 1
#define DECRYPT 0
#define ROUNDS 16
#define DATA 65536

// Obtain bit "b" from the left and shift it "c" places from the right
#define BITNUM(a,b,c) (((a[(b)/8] >> (7 - (b%8))) & 0x01) << (c))
#define BITNUMINTR(a,b,c) ((((a) >> (31 - (b))) & 0x00000001) << (c))
#define BITNUMINTL(a,b,c) ((((a) << (b)) & 0x80000000) >> (c))
// This macro converts a 6 bit block with the S-Box row defined as the first and last
// bits to a 6 bit block with the row defined by the first two bits.
#define SBOXBIT(a) (((a) & 0x20) | (((a) & 0x1f) >> 1) | (((a) & 0x01) << 4))

int matches = 0, total = 0;
uint cnt[64];

uchar sbox1[64] = {
   14,  4,  13,  1,   2, 15,  11,  8,   3, 10,   6, 12,   5,  9,   0,  7,
    0, 15,   7,  4,  14,  2,  13,  1,  10,  6,  12, 11,   9,  5,   3,  8,
    4,  1,  14,  8,  13,  6,   2, 11,  15, 12,   9,  7,   3, 10,   5,  0,
   15, 12,   8,  2,   4,  9,   1,  7,   5, 11,   3, 14,  10,  0,   6, 13
};
uchar sbox2[64] = {
   15,  1,   8, 14,   6, 11,   3,  4,   9,  7,   2, 13,  12,  0,   5, 10,
    3, 13,   4,  7,  15,  2,   8, 14,  12,  0,   1, 10,   6,  9,  11,  5,
    0, 14,   7, 11,  10,  4,  13,  1,   5,  8,  12,  6,   9,  3,   2, 15,
   13,  8,  10,  1,   3, 15,   4,  2,  11,  6,   7, 12,   0,  5,  14,  9
};
uchar sbox3[64] = {
   10,  0,   9, 14,   6,  3,  15,  5,   1, 13,  12,  7,  11,  4,   2,  8,
   13,  7,   0,  9,   3,  4,   6, 10,   2,  8,   5, 14,  12, 11,  15,  1,
   13,  6,   4,  9,   8, 15,   3,  0,  11,  1,   2, 12,   5, 10,  14,  7,
    1, 10,  13,  0,   6,  9,   8,  7,   4, 15,  14,  3,  11,  5,   2, 12
};
uchar sbox4[64] = {
    7, 13,  14,  3,   0,  6,   9, 10,   1,  2,   8,  5,  11, 12,   4, 15,
   13,  8,  11,  5,   6, 15,   0,  3,   4,  7,   2, 12,   1, 10,  14,  9,
   10,  6,   9,  0,  12, 11,   7, 13,  15,  1,   3, 14,   5,  2,   8,  4,
    3, 15,   0,  6,  10,  1,  13,  8,   9,  4,   5, 11,  12,  7,   2, 14
};
uchar sbox5[64] = {
    2, 12,   4,  1,   7, 10,  11,  6,   8,  5,   3, 15,  13,  0,  14,  9,
   14, 11,   2, 12,   4,  7,  13,  1,   5,  0,  15, 10,   3,  9,   8,  6,
    4,  2,   1, 11,  10, 13,   7,  8,  15,  9,  12,  5,   6,  3,   0, 14,
   11,  8,  12,  7,   1, 14,   2, 13,   6, 15,   0,  9,  10,  4,   5,  3
};
uchar sbox6[64] = {
   12,  1,  10, 15,   9,  2,   6,  8,   0, 13,   3,  4,  14,  7,   5, 11,
   10, 15,   4,  2,   7, 12,   9,  5,   6,  1,  13, 14,   0, 11,   3,  8,
    9, 14,  15,  5,   2,  8,  12,  3,   7,  0,   4, 10,   1, 13,  11,  6,
    4,  3,   2, 12,   9,  5,  15, 10,  11, 14,   1,  7,   6,  0,   8, 13
};
uchar sbox7[64] = {
    4, 11,   2, 14,  15,  0,   8, 13,   3, 12,   9,  7,   5, 10,   6,  1,
   13,  0,  11,  7,   4,  9,   1, 10,  14,  3,   5, 12,   2, 15,   8,  6,
    1,  4,  11, 13,  12,  3,   7, 14,  10, 15,   6,  8,   0,  5,   9,  2,
    6, 11,  13,  8,   1,  4,  10,  7,   9,  5,   0, 15,  14,  2,   3, 12
};
uchar sbox8[64] = {
   13,  2,   8,  4,   6, 15,  11,  1,  10,  9,   3, 14,   5,  0,  12,  7,
    1, 15,  13,  8,  10,  3,   7,  4,  12,  5,   6, 11,   0, 14,   9,  2,
    7, 11,   4,  1,   9, 12,  14,  2,   0,  6,  10, 13,  15,  3,   5,  8,
    2,  1,  14,  7,   4, 10,   8, 13,  15, 12,   9,  0,   3,  5,   6, 11
};


void key_schedule(uchar key[], uchar schedule[][6], uint mode)
{
   uint i,j,to_gen,C,D,
        key_rnd_shift[ROUNDS]={1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1},
        key_perm_c[28]={56,48,40,32,24,16,8,0,57,49,41,33,25,17,
                        9,1,58,50,42,34,26,18,10,2,59,51,43,35},
        key_perm_d[28]={62,54,46,38,30,22,14,6,61,53,45,37,29,21,
                        13,5,60,52,44,36,28,20,12,4,27,19,11,3},
        key_compression[48]={13,16,10,23,0,4,2,27,14,5,20,9,
                             22,18,11,3,25,7,15,6,26,19,12,1,
                             40,51,30,36,46,54,29,39,50,44,32,47,
                             43,48,38,55,33,52,45,41,49,35,28,31};

   // Permutated Choice #1 (copy the key in, ignoring parity bits).
   for (i = 0, j = 31, C = 0; i < 28; ++i, --j)
      C |= BITNUM(key,key_perm_c[i],j);
   for (i = 0, j = 31, D = 0; i < 28; ++i, --j)
      D |= BITNUM(key,key_perm_d[i],j);

   // Generate the 16 subkeys.
   for (i = 0; i < 16; ++i) {
      C = ((C << key_rnd_shift[i]) | (C >> (28-key_rnd_shift[i]))) & 0xfffffff0;
      D = ((D << key_rnd_shift[i]) | (D >> (28-key_rnd_shift[i]))) & 0xfffffff0;

      // Decryption subkeys are reverse order of encryption subkeys so
      // generate them in reverse if the key schedule is for decryption useage.
      if (mode == DECRYPT)
         to_gen = (ROUNDS - 1) - i;
      else
         to_gen = i;
      // Initialize the array
      for (j = 0; j < 6; ++j)
         schedule[to_gen][j] = 0;
      for (j = 0; j < 24; ++j)
         schedule[to_gen][j/8] |= BITNUMINTR(C,key_compression[j],7 - (j%8));
      for ( ; j < 48; ++j)
         schedule[to_gen][j/8] |= BITNUMINTR(D,key_compression[j] - 28,7 - (j%8));
   }
}

uint f(uint state, uchar key[])
{
   uchar lrgstate[6],i;
   uint t1,t2;

   // Expantion Permutation
   t1 = BITNUMINTL(state,31,0) | ((state & 0xf0000000) >> 1) | BITNUMINTL(state,4,5) |
        BITNUMINTL(state,3,6) | ((state & 0x0f000000) >> 3) | BITNUMINTL(state,8,11) |
        BITNUMINTL(state,7,12) | ((state & 0x00f00000) >> 5) | BITNUMINTL(state,12,17) |
        BITNUMINTL(state,11,18) | ((state & 0x000f0000) >> 7) | BITNUMINTL(state,16,23);

   t2 = BITNUMINTL(state,15,0) | ((state & 0x0000f000) << 15) | BITNUMINTL(state,20,5) |
        BITNUMINTL(state,19,6) | ((state & 0x00000f00) << 13) | BITNUMINTL(state,24,11) |
        BITNUMINTL(state,23,12) | ((state & 0x000000f0) << 11) | BITNUMINTL(state,28,17) |
        BITNUMINTL(state,27,18) | ((state & 0x0000000f) << 9) | BITNUMINTL(state,0,23);

   lrgstate[0] = (t1 >> 24) & 0x000000ff;
   lrgstate[1] = (t1 >> 16) & 0x000000ff;
   lrgstate[2] = (t1 >> 8) & 0x000000ff;
   lrgstate[3] = (t2 >> 24) & 0x000000ff;
   lrgstate[4] = (t2 >> 16) & 0x000000ff;
   lrgstate[5] = (t2 >> 8) & 0x000000ff;

   // Key XOR
   lrgstate[0] ^= key[0];
   lrgstate[1] ^= key[1];
   lrgstate[2] ^= key[2];
   lrgstate[3] ^= key[3];
   lrgstate[4] ^= key[4];
   lrgstate[5] ^= key[5];

   // S-Box Permutation
   state = (sbox1[SBOXBIT(lrgstate[0] >> 2)] << 28) |
           (sbox2[SBOXBIT(((lrgstate[0] & 0x03) << 4) | (lrgstate[1] >> 4))] << 24) |
           (sbox3[SBOXBIT(((lrgstate[1] & 0x0f) << 2) | (lrgstate[2] >> 6))] << 20) |
           (sbox4[SBOXBIT(lrgstate[2] & 0x3f)] << 16) |
           (sbox5[SBOXBIT(lrgstate[3] >> 2)] << 12) |
           (sbox6[SBOXBIT(((lrgstate[3] & 0x03) << 4) | (lrgstate[4] >> 4))] << 8) |
           (sbox7[SBOXBIT(((lrgstate[4] & 0x0f) << 2) | (lrgstate[5] >> 6))] << 4) |
            sbox8[SBOXBIT(lrgstate[5] & 0x3f)];

   // P-Box Permutation
   state = BITNUMINTL(state,15,0) | BITNUMINTL(state,6,1) | BITNUMINTL(state,19,2) |
           BITNUMINTL(state,20,3) | BITNUMINTL(state,28,4) | BITNUMINTL(state,11,5) |
           BITNUMINTL(state,27,6) | BITNUMINTL(state,16,7) | BITNUMINTL(state,0,8) |
           BITNUMINTL(state,14,9) | BITNUMINTL(state,22,10) | BITNUMINTL(state,25,11) |
           BITNUMINTL(state,4,12) | BITNUMINTL(state,17,13) | BITNUMINTL(state,30,14) |
           BITNUMINTL(state,9,15) | BITNUMINTL(state,1,16) | BITNUMINTL(state,7,17) |
           BITNUMINTL(state,23,18) | BITNUMINTL(state,13,19) | BITNUMINTL(state,31,20) |
           BITNUMINTL(state,26,21) | BITNUMINTL(state,2,22) | BITNUMINTL(state,8,23) |
           BITNUMINTL(state,18,24) | BITNUMINTL(state,12,25) | BITNUMINTL(state,29,26) |
           BITNUMINTL(state,5,27) | BITNUMINTL(state,21,28) | BITNUMINTL(state,10,29) |
           BITNUMINTL(state,3,30) | BITNUMINTL(state,24,31);

   // Return the final state value
   return(state);
}

void printtext(unsigned char hash[])
{
   int i;
   for (i=0; i < 8; i++)
      printf("%02x ",hash[i]);
   printf("\n");
}

void generateRandomData(uchar input[][8], int amount)
{
  for(int i = 0; i < amount; i++)
  {
    uint64_t in = rand();
    in = (in << 32) | rand();
    memcpy(input[i], &in, 8 * sizeof(uchar));
  }
}

#define IS_BIT(x, n) (((x) & ((uint64_t) 1 << (n))) >> (n))
#define SET_BIT(st, bit) ((st)[(bit) / 64] |= ((uint64_t) 1 << ((bit) % 64)))
#define CLR_BIT(st, bit) ((st)[(bit) / 64] &= ~((uint64_t) 1 << ((bit) % 64)))

uint32_t linearMask64(uint64_t data, uint64_t indices[], uint32_t count)
{
  uint32_t res = 0;

  for (int i = 0; i < count; i ++)
    res ^= IS_BIT(data, indices[i]);

  return res;
}

uint32_t linearMaskKey(uint8_t key[], uint64_t indices[], uint32_t count)
{
  uint32_t res = 0;

  for (int i = 0; i < count; i ++)
    res ^= IS_BIT(key[indices[i] / 8], indices[i] % 8);

  return res;
}

void des_3_round_probability(uint8_t key[]) {
  uint count = 0;
  uint8_t schedule[ROUNDS][6], input[DATA][8];

  for(unsigned int j = 0; j < 100; j++)
  {
    generateRandomData(input, DATA);
    key_schedule(key, schedule, ENCRYPT);
    for(unsigned int i = 0; i < DATA; i++)
    {
      uint64_t in;
      memcpy(&in, input[i], 8);
      uint32_t p_h = (uint32_t) in, p_l = (uint32_t) (in >> 32);

      //START FEISTEL
      uint32_t state = p_l;
      uint32_t left = p_h, temp = 0;
      temp = state;
      state = left ^ f(state, schedule[0]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[1]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[2]);
      left = temp;

      uint64_t out = state;
      out |= ((uint64_t) left << 32);

      uint64_t outmask[] = {7, 18, 24, 29, 32 + 15};
      uint64_t inmask[] = {7, 18, 24, 29, 32 + 15};
      uint64_t keymask1[] = {22};
      uint64_t keymask2[] = {22};
      count += linearMask64(in, inmask, 5) ^ linearMaskKey(schedule[0], keymask1, 1) ^ linearMaskKey(schedule[2], keymask2, 1) ^ linearMask64(out, outmask, 5);

    }
  }
  printf("probability 3 rounds: %lf\n", (double) count / (65536 * 100));
}

void des_5_round_probability(uint8_t key[]) {
  uint count = 0;
  uint8_t schedule[ROUNDS][6], input[DATA][8];

  for(unsigned int j = 0; j < 100; j++)
  {
    generateRandomData(input, DATA);
    key_schedule(key, schedule, ENCRYPT);
    for(unsigned int i = 0; i < DATA; i++)
    {
      uint64_t in;
      memcpy(&in, input[i], 8);
      uint32_t p_h = (uint32_t) in, p_l = (uint32_t) (in >> 32);

      //START FEISTEL
      uint32_t state = p_l;
      uint32_t left = p_h, temp = 0;
      temp = state;
      state = left ^ f(state, schedule[0]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[1]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[2]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[3]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[4]);
      left = temp;

      uint64_t out = state;
      out |= ((uint64_t) left << 32);

      uint64_t outmask[] = {15, 7 + 32, 18 + 32, 24 + 32, 27 + 32, 28 + 32, 29 + 32, 30 + 32, 31 + 32};
      uint64_t inmask[] = {15, 7 + 32, 18 + 32, 24 + 32, 27 + 32, 28 + 32, 29 + 32, 30 + 32, 31 + 32};
      uint64_t keymask1[] = {42, 43, 45, 46};
      uint64_t keymask2[] = {22};
      uint64_t keymask4[] = {22};
      uint64_t keymask5[] = {42, 43, 45, 46};
      count += linearMask64(in, inmask, 9) ^ linearMaskKey(schedule[0], keymask1, 4) ^ linearMaskKey(schedule[1], keymask2, 1) ^ linearMaskKey(schedule[3], keymask4, 1) ^ linearMaskKey(schedule[4], keymask5, 4) ^ linearMask64(out, outmask, 9);

    }
  }
  printf("probability 5 rounds: %lf\n", (double) count / (65536 * 100));
}

void des_7_alternate_round_probability(uint8_t key[]) {
  uint count = 0;
  uint8_t schedule[ROUNDS][6], input[DATA][8];

  for(unsigned int j = 0; j < 100; j++)
  {
    generateRandomData(input, DATA);
    key_schedule(key, schedule, ENCRYPT);
    for(unsigned int i = 0; i < DATA; i++)
    {
      uint64_t in;
      memcpy(&in, input[i], 8);
      uint32_t p_h = (uint32_t) in, p_l = (uint32_t) (in >> 32);

      //START FEISTEL
      uint32_t state = p_l;
      uint32_t left = p_h, temp = 0;
      temp = state;
      state = left ^ f(state, schedule[0]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[1]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[2]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[3]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[4]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[5]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[6]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[7]);
      left = temp;

      uint64_t out = state;
      out |= ((uint64_t) left << 32);

      uint32_t f8 = f(left, schedule[7]);

      uint64_t outmask[] = {15, 7 + 32, 18 + 32, 24 + 32, 29 + 32};
      uint64_t inmask[] = {7, 18, 24, 12 + 32, 16 + 32};

      uint64_t keymask1[] = {19, 23};
      uint64_t keymask3[] = {22};
      uint64_t keymask4[] = {44};
      uint64_t keymask5[] = {22};
      uint64_t keymask7[] = {22};

      uint32_t key_xor = linearMaskKey(schedule[0], keymask1, 2);
      key_xor ^= linearMaskKey(schedule[2], keymask3, 1);
      key_xor ^= linearMaskKey(schedule[3], keymask4, 1);
      key_xor ^= linearMaskKey(schedule[4], keymask5, 1);
      key_xor ^= linearMaskKey(schedule[6], keymask7, 1);

      count += linearMask64(in, inmask, 5) ^ key_xor ^ linearMask64(out, outmask, 5) ^ IS_BIT(f8, 15) ^ 1;

    }
  }
  printf("probability 7 rounds: %lf\n", (double) count / (65536 * 100));
}

void des_7_round_probability(uint8_t key[]) {
  uint count = 0;
  uint8_t schedule[ROUNDS][6], input[DATA][8];

  for(unsigned int j = 0; j < 100; j++)
  {
    generateRandomData(input, DATA);
    key_schedule(key, schedule, ENCRYPT);
    for(unsigned int i = 0; i < DATA; i++)
    {
      uint64_t in;
      memcpy(&in, input[i], 8);
      uint32_t p_h = (uint32_t) in, p_l = (uint32_t) (in >> 32);

      //START FEISTEL
      uint32_t state = p_l;
      uint32_t left = p_h, temp = 0;
      temp = state;
      state = left ^ f(state, schedule[0]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[1]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[2]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[3]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[4]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[5]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[6]);
      left = temp;

      uint64_t out = state;
      out |= ((uint64_t) left << 32);

      uint64_t outmask[] = {7, 18, 24, 29, 32 + 15};
      uint64_t inmask[] = {7, 18, 24, 32 + 12, 32 + 16};

      uint64_t keymask1[] = {19, 23};
      uint64_t keymask3[] = {22};
      uint64_t keymask4[] = {44};
      uint64_t keymask5[] = {22};
      uint64_t keymask7[] = {22};

      uint32_t key_xor = linearMaskKey(schedule[0], keymask1, 2);
      key_xor ^= linearMaskKey(schedule[2], keymask3, 1);
      key_xor ^= linearMaskKey(schedule[3], keymask4, 1);
      key_xor ^= linearMaskKey(schedule[4], keymask5, 1);
      key_xor ^= linearMaskKey(schedule[6], keymask7, 1);

      count += linearMask64(in, inmask, 5) ^ key_xor ^ linearMask64(out, outmask, 5) ^ 1;

    }
  }
  printf("probability 7 rounds: %lf\n", (double) count / (65536 * 100));
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

void attackThread(uint8_t key[], uint32_t k, uint32_t *counterval)
{
  uint8_t schedule[ROUNDS][6], input[DATA][8];
  uint count = 0;

  //Make a guess of the eight round key
  uint8_t schedule7[6];
  memset(schedule7, 0, 6);

  k = 63;
  k = 0b110100;
  for(unsigned int j = 0; j < 6; j++)
  {
    //Set the bits of the key
    if(k & (1 << j))
      SET_BIT((uint64_t*)schedule7, 42 + j);
  }
  //By changing one of the lower bits here the bias changes.
  //The assumption that only bits 42-47 are influencing the result does
  //somehow not hold in this case
  /*schedule7[0] = 0b01010111;
  schedule7[1] = 0b10001000;
  schedule7[2] = 0b00111000;

  schedule7[3] = 0b01101100;
  schedule7[4] = 0b11100101;
  schedule7[5] = 0b10000001;*/

  for(unsigned int j = 0; j < 100; j++)
  {
    generateRandomData(input, DATA);
    key_schedule(key, schedule, ENCRYPT);

    /*printf("%u / %u\n", schedule7[0], schedule[7][0]);
    printf("%u / %u\n", schedule7[1], schedule[7][1]);
    printf("%u / %u\n", schedule7[2], schedule[7][2]);
    printf("%u / %u\n", schedule7[3], schedule[7][3]);
    printf("%u / %u\n", schedule7[4], schedule[7][4]);
    printf("%u / %u\n", schedule7[5], schedule[7][5]);*/

    for(unsigned int i = 0; i < DATA; i++)
    {
      uint64_t in;
      memcpy(&in, input[i], 8);
      uint32_t p_h = (uint32_t) in, p_l = (uint32_t) (in >> 32);

      //START FEISTEL
      uint32_t state = p_l, state2 = 0;
      uint32_t left = p_h, temp = 0;

      temp = state;
      state = left ^ f(state, schedule[0]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[1]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[2]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[3]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[4]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[5]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[6]);
      left = temp;

      temp = state;
      state = left ^ f(state, schedule[7]);
      left = temp;

      //8th round with guessed key
      state2 = f(left, schedule7);

      uint64_t out = state;
      out |= ((uint64_t) left << 32);

      uint64_t outmask[] = {15, 32 + 7, 32 + 18, 32 + 24, 32 + 29};
      uint64_t inmask[] = {7, 18, 24, 32 + 12, 32 + 16};
      uint32_t leftSide = linearMask64(in, inmask, 5) ^ linearMask64(out, outmask, 5) ^ IS_BIT(state2, 15) ^ 1;

      uint64_t keymask1[] = {19, 23};
      uint64_t keymask3[] = {22};
      uint64_t keymask4[] = {44};
      uint64_t keymask5[] = {22};
      uint64_t keymask7[] = {22};

      uint32_t key_xor = linearMaskKey(schedule[0], keymask1, 2);
      key_xor ^= linearMaskKey(schedule[2], keymask3, 1);
      key_xor ^= linearMaskKey(schedule[3], keymask4, 1);
      key_xor ^= linearMaskKey(schedule[4], keymask5, 1);
      key_xor ^= linearMaskKey(schedule[6], keymask7, 1);

      if(leftSide == 0)
        (*counterval)++;

      count += linearMask64(in, inmask, 5) ^ key_xor ^ linearMask64(out, outmask, 5) ^ 1 ^ IS_BIT(state2, 15);
    }
  }
  //printf("probability: %lf\n", (double) count / (65536 * 100));
}

void des_8_round_attack(uint8_t key[]) {
  uint32_t counters[64];
  memset(counters, 0, 64 * sizeof(uint32_t));

  thread* threads[8];
  for(unsigned int k = 0; k < 64; k += 8)
  {
    for(unsigned int i = 0; i < 8; i++)
    {
      thread *newThread = new thread(&attackThread, key, k + i, &(counters[k + i]));
      threads[i] = newThread;
    }

    for(unsigned int i = 0; i < 8; i++)
    {
      threads[i]->join();
      delete threads[i];
    }
  }

  int64_t min = 0xffffff, max = 0;
  int minIndex = 0, maxIndex = 0;
  for(uint32_t i = 0; i < 64; i++)
  {
    if(counters[i] < min)
    {
      min = counters[i];
      minIndex = i;
    }

    if(counters[i] > max)
    {
      max = counters[i];
      maxIndex = i;
    }
  }

  int64_t l = (max - (int64_t)50 * DATA) < 0 ? - (max - (int64_t)50 * DATA) : (max - (int64_t)50 * DATA);
  int64_t r = (int64_t)(min - (int64_t)50 * DATA) < 0 ? - (min - (int64_t)50 * DATA) : (min - (int64_t)50 * DATA);

  uint8_t schedule[ROUNDS][6], input[DATA][8];
  key_schedule(key, schedule, ENCRYPT);

  if(l > r)
  {
    printf("adjust the keyguess based on max index\n");
  }
  else
  {
    printf("adjust the keyguess based on min index\n");
  }
}

int main()
{
   unsigned char key1[8]={0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF};
   srand(time(NULL));
   des_3_round_probability(key1);
   des_5_round_probability(key1);
   des_7_round_probability(key1);

   des_8_round_attack(key1);

   return 0;
}
