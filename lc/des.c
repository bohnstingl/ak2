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

// Initial (Inv)Permutation step
void IP(uint state[], uchar in[])
{
   state[0] = BITNUM(in,57,31) | BITNUM(in,49,30) | BITNUM(in,41,29) | BITNUM(in,33,28) |
              BITNUM(in,25,27) | BITNUM(in,17,26) | BITNUM(in,9,25) | BITNUM(in,1,24) |
              BITNUM(in,59,23) | BITNUM(in,51,22) | BITNUM(in,43,21) | BITNUM(in,35,20) |
              BITNUM(in,27,19) | BITNUM(in,19,18) | BITNUM(in,11,17) | BITNUM(in,3,16) |
              BITNUM(in,61,15) | BITNUM(in,53,14) | BITNUM(in,45,13) | BITNUM(in,37,12) |
              BITNUM(in,29,11) | BITNUM(in,21,10) | BITNUM(in,13,9) | BITNUM(in,5,8) |
              BITNUM(in,63,7) | BITNUM(in,55,6) | BITNUM(in,47,5) | BITNUM(in,39,4) |
              BITNUM(in,31,3) | BITNUM(in,23,2) | BITNUM(in,15,1) | BITNUM(in,7,0);

   state[1] = BITNUM(in,56,31) | BITNUM(in,48,30) | BITNUM(in,40,29) | BITNUM(in,32,28) |
              BITNUM(in,24,27) | BITNUM(in,16,26) | BITNUM(in,8,25) | BITNUM(in,0,24) |
              BITNUM(in,58,23) | BITNUM(in,50,22) | BITNUM(in,42,21) | BITNUM(in,34,20) |
              BITNUM(in,26,19) | BITNUM(in,18,18) | BITNUM(in,10,17) | BITNUM(in,2,16) |
              BITNUM(in,60,15) | BITNUM(in,52,14) | BITNUM(in,44,13) | BITNUM(in,36,12) |
              BITNUM(in,28,11) | BITNUM(in,20,10) | BITNUM(in,12,9) | BITNUM(in,4,8) |
              BITNUM(in,62,7) | BITNUM(in,54,6) | BITNUM(in,46,5) | BITNUM(in,38,4) |
              BITNUM(in,30,3) | BITNUM(in,22,2) | BITNUM(in,14,1) | BITNUM(in,6,0);
}

void InvIP(uint state[], uchar in[])
{
   in[0] = BITNUMINTR(state[1],7,7) | BITNUMINTR(state[0],7,6) | BITNUMINTR(state[1],15,5) |
           BITNUMINTR(state[0],15,4) | BITNUMINTR(state[1],23,3) | BITNUMINTR(state[0],23,2) |
           BITNUMINTR(state[1],31,1) | BITNUMINTR(state[0],31,0);

   in[1] = BITNUMINTR(state[1],6,7) | BITNUMINTR(state[0],6,6) | BITNUMINTR(state[1],14,5) |
           BITNUMINTR(state[0],14,4) | BITNUMINTR(state[1],22,3) | BITNUMINTR(state[0],22,2) |
           BITNUMINTR(state[1],30,1) | BITNUMINTR(state[0],30,0);

   in[2] = BITNUMINTR(state[1],5,7) | BITNUMINTR(state[0],5,6) | BITNUMINTR(state[1],13,5) |
           BITNUMINTR(state[0],13,4) | BITNUMINTR(state[1],21,3) | BITNUMINTR(state[0],21,2) |
           BITNUMINTR(state[1],29,1) | BITNUMINTR(state[0],29,0);

   in[3] = BITNUMINTR(state[1],4,7) | BITNUMINTR(state[0],4,6) | BITNUMINTR(state[1],12,5) |
           BITNUMINTR(state[0],12,4) | BITNUMINTR(state[1],20,3) | BITNUMINTR(state[0],20,2) |
           BITNUMINTR(state[1],28,1) | BITNUMINTR(state[0],28,0);

   in[4] = BITNUMINTR(state[1],3,7) | BITNUMINTR(state[0],3,6) | BITNUMINTR(state[1],11,5) |
           BITNUMINTR(state[0],11,4) | BITNUMINTR(state[1],19,3) | BITNUMINTR(state[0],19,2) |
           BITNUMINTR(state[1],27,1) | BITNUMINTR(state[0],27,0);

   in[5] = BITNUMINTR(state[1],2,7) | BITNUMINTR(state[0],2,6) | BITNUMINTR(state[1],10,5) |
           BITNUMINTR(state[0],10,4) | BITNUMINTR(state[1],18,3) | BITNUMINTR(state[0],18,2) |
           BITNUMINTR(state[1],26,1) | BITNUMINTR(state[0],26,0);

   in[6] = BITNUMINTR(state[1],1,7) | BITNUMINTR(state[0],1,6) | BITNUMINTR(state[1],9,5) |
           BITNUMINTR(state[0],9,4) | BITNUMINTR(state[1],17,3) | BITNUMINTR(state[0],17,2) |
           BITNUMINTR(state[1],25,1) | BITNUMINTR(state[0],25,0);

   in[7] = BITNUMINTR(state[1],0,7) | BITNUMINTR(state[0],0,6) | BITNUMINTR(state[1],8,5) |
           BITNUMINTR(state[0],8,4) | BITNUMINTR(state[1],16,3) | BITNUMINTR(state[0],16,2) |
           BITNUMINTR(state[1],24,1) | BITNUMINTR(state[0],24,0);
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

void des_crypt_3(uchar in[], uchar out[], uchar key[][6])
{
   uint state[2],idx,t;

   /*Test 3 rounds*/
   uchar r015, l07, l018, l024, l029, l315, r37, r318, r324, r329,
   r17, r118, r124, r129;

   //IP(state,in);
   memcpy(state, in, 8 * sizeof(uchar));

   //state[1] is the right hand side
   //state[0] the left hand

   r015 = (state[1] & (1 << 15)) != 0 ? 1 : 0;
   l07 = (state[0] & (1 << 7)) != 0 ? 1 : 0;
   l018 = (state[0] & (1 << 18)) != 0 ? 1 : 0;
   l024 = (state[0] & (1 << 24)) != 0 ? 1 : 0;
   l029 = (state[0] & (1 << 29)) != 0 ? 1 : 0;
   // Loop 16 times, perform the final loop manually as it doesn't switch sides
   for (idx=0; idx < 3; ++idx)
   {
     if(idx == 1)
     {
       r17 = (state[1] & (1 << 7)) != 0 ? 1 : 0;
       r118 = (state[1] & (1 << 18)) != 0 ? 1 : 0;
       r124 = (state[1] & (1 << 24)) != 0 ? 1 : 0;
       r129 = (state[1] & (1 << 29)) != 0 ? 1 : 0;
      }
      /*if(idx == 2)
      {
        r37 = (state[1] & (1 << 7)) != 0 ? 1 : 0;
        r318 = (state[1] & (1 << 18)) != 0 ? 1 : 0;
        r324 = (state[1] & (1 << 24)) != 0 ? 1 : 0;
        r329 = (state[1] & (1 << 29)) != 0 ? 1 : 0;
        l315 = (state[0] & (1 << 15)) != 0 ? 1 : 0;
      }*/

      t = state[1];
      state[1] = f(state[1],key[idx]) ^ state[0];
      state[0] = t;
   }

   r37 = (state[1] & (1 << 7)) != 0 ? 1 : 0;
   r318 = (state[1] & (1 << 18)) != 0 ? 1 : 0;
   r324 = (state[1] & (1 << 24)) != 0 ? 1 : 0;
   r329 = (state[1] & (1 << 29)) != 0 ? 1 : 0;
   l315 = (state[0] & (1 << 15)) != 0 ? 1 : 0;
   //state[0] = f(state[1],key[15]) ^ state[0];

   //Check
   uchar erg = r015^l315^l07^l018^l024^l029^r37^r318^r324^r329;
   uchar erg2 = r015^l07^l018^l024^l029^r17^r118^r124^r129;
   uchar keybit0 = (key[0][2] & (1 << 6)) != 0 ? 1 : 0;
   uchar keybit1 = (key[1][2] & (1 << 6)) != 0 ? 1 : 0;
   uchar keybit2 = (key[2][2] & (1 << 6)) != 0 ? 1 : 0;
   uchar keybit3 = (key[3][2] & (1 << 6)) != 0 ? 1 : 0;
   uchar keybit = keybit1^keybit3;
   uchar keybit11 = keybit0^keybit2^1;
   //printf("%02x || %02x -> ", erg, keybit);
   //printf("%02x || %02x -> ", erg2, keybit1);

   matches += (erg == keybit) ? 1 : 0;
   total++;

   memcpy(out, state, 8 * sizeof(uchar));
   // Inverse IP
   //InvIP(state,out);
}

void des_crypt_5(uchar in[], uchar out[], uchar key[][6])
{
   uint state[2],idx,t;

   /*Test 5 rounds*/
   uchar l015, r07, r018, r024, r027, r028, r029, r030, r031, r515,
   l57, l518, l524, l527, l528, l529, l530, l531;

   //IP(state,in);
   memcpy(state, in, 8 * sizeof(uchar));

   //state[1] is the right hand side
   //state[0] the left hand

   l015 = (state[0] & (1 << 15)) != 0 ? 1 : 0;
   r07 = (state[1] & (1 << 7)) != 0 ? 1 : 0;
   r018 = (state[1] & (1 << 18)) != 0 ? 1 : 0;
   r024 = (state[1] & (1 << 24)) != 0 ? 1 : 0;
   r027 = (state[1] & (1 << 27)) != 0 ? 1 : 0;
   r028 = (state[1] & (1 << 28)) != 0 ? 1 : 0;
   r029 = (state[1] & (1 << 29)) != 0 ? 1 : 0;
   r030 = (state[1] & (1 << 30)) != 0 ? 1 : 0;
   r031 = (state[1] & (1 << 31)) != 0 ? 1 : 0;
   // Loop 16 times, perform the final loop manually as it doesn't switch sides
   for (idx=0; idx < 5; ++idx)
   {
      t = state[1];
      state[1] = f(state[1],key[idx]) ^ state[0];
      state[0] = t;
   }

   r515 = (state[1] & (1 << 15)) != 0 ? 1 : 0;
   l57 = (state[0] & (1 << 7)) != 0 ? 1 : 0;
   l518 = (state[0] & (1 << 18)) != 0 ? 1 : 0;
   l524 = (state[0] & (1 << 24)) != 0 ? 1 : 0;
   l527 = (state[0] & (1 << 27)) != 0 ? 1 : 0;
   l528 = (state[0] & (1 << 28)) != 0 ? 1 : 0;
   l529 = (state[0] & (1 << 29)) != 0 ? 1 : 0;
   l530 = (state[0] & (1 << 30)) != 0 ? 1 : 0;
   l531 = (state[0] & (1 << 31)) != 0 ? 1 : 0;
   //state[0] = f(state[1],key[15]) ^ state[0];

   //Check
   uchar erg = l015^r07^r018^r024^r027^r028^r029^r030^r031^r515^l57^l518^l524^l527^l528^l529^l530^l531;
   uchar k042 = (key[0][5] & (1 << 2)) != 0 ? 1 : 0;
   uchar k043 = (key[0][5] & (1 << 3)) != 0 ? 1 : 0;
   uchar k045 = (key[0][5] & (1 << 5)) != 0 ? 1 : 0;
   uchar k046 = (key[0][5] & (1 << 6)) != 0 ? 1 : 0;
   uchar k0 = (key[0][2] & (1 << 6)) != 0 ? 1 : 0;
   uchar k1 = (key[1][2] & (1 << 6)) != 0 ? 1 : 0;
   uchar k142 = (key[1][5] & (1 << 2)) != 0 ? 1 : 0;
   uchar k143 = (key[1][5] & (1 << 3)) != 0 ? 1 : 0;
   uchar k145 = (key[1][5] & (1 << 5)) != 0 ? 1 : 0;
   uchar k146 = (key[1][5] & (1 << 6)) != 0 ? 1 : 0;
   uchar k2 = (key[2][2] & (1 << 6)) != 0 ? 1 : 0;
   uchar k3 = (key[3][2] & (1 << 6)) != 0 ? 1 : 0;
   uchar k4 = (key[4][2] & (1 << 6)) != 0 ? 1 : 0;
   uchar k442 = (key[4][5] & (1 << 2)) != 0 ? 1 : 0;
   uchar k443 = (key[4][5] & (1 << 3)) != 0 ? 1 : 0;
   uchar k445 = (key[4][5] & (1 << 5)) != 0 ? 1 : 0;
   uchar k446 = (key[4][5] & (1 << 6)) != 0 ? 1 : 0;
   uchar k542 = (key[5][5] & (1 << 2)) != 0 ? 1 : 0;
   uchar k543 = (key[5][5] & (1 << 3)) != 0 ? 1 : 0;
   uchar k545 = (key[5][5] & (1 << 5)) != 0 ? 1 : 0;
   uchar k546 = (key[5][5] & (1 << 6)) != 0 ? 1 : 0;
   uchar keybit = k042^k043^k045^k046^k442^k443^k445^k446^k1^k3^1;
   uchar keybit11 = k142^k143^k145^k146^k542^k543^k545^k546^k2^k4;
   //printf("%02x || %02x -> ", erg, keybit);

   matches += (erg == keybit) ? 1 : 0;
   total++;

   memcpy(out, state, 8 * sizeof(uchar));
   // Inverse IP
   //InvIP(state,out);
}

/*
 * Results
 * {0.499469, 0.499744, 0.499744, 0.500349, 0.499572, 0.499619, \
0.500147, 0.499566, 0.49974000, 0.499912, 0.500235, 0.499903, \
0.500383, 0.500093, 0.499829, 0.499903, 0.500284, 0.499815, 0.499591, \
0.499888, 0.499689, 0.500065, 0.499997, 0.500106, 0.500435, 0.499777, \
0.500106, 0.499735, 0.500027, 0.499716, 0.500099, 0.500181, 0.500181, \
0.499884, 0.50032, 0.499983, 0.499983, 0.499600, 0.499581, 0.499919, \
0.499493, 0.499986, 0.499795, 0.499915, 0.500062, 0.499893, 0.500865, \
0.500102, 0.500093, 0.500312, 0.499897, 0.499591, 0.499708, 0.500253, \
0.500167, 0.500225, 0.500309, 0.50001, 0.499723, 0.5499978, 0.499914}
 */
void des_crypt_7(uchar in[], uchar out[], uchar key[][6])
{
   uint state[2],idx,t;

   uchar l07, l018, l024, r012, r016, l77, l718, l724, l729, r715;

   //IP(state,in);
   memcpy(state, in, 8 * sizeof(uchar));

   //state[1] is the right hand side
   //state[0] the left hand

   l07 = (state[0] & (1 << 7)) != 0 ? 1 : 0;
   l018 = (state[0] & (1 << 18)) != 0 ? 1 : 0;
   l024 = (state[0] & (1 << 24)) != 0 ? 1 : 0;
   r012 = (state[1] & (1 << 12)) != 0 ? 1 : 0;
   r016 = (state[1] & (1 << 16)) != 0 ? 1 : 0;

   for (idx=0; idx < 7; ++idx)
   {
      t = state[1];
      state[1] = f(state[1],key[idx]) ^ state[0];
      state[0] = t;
   }

   l77 = (state[0] & (1 << 7)) != 0 ? 1 : 0;
   l718 = (state[0] & (1 << 18)) != 0 ? 1 : 0;
   l724 = (state[0] & (1 << 24)) != 0 ? 1 : 0;
   l729 = (state[0] & (1 << 29)) != 0 ? 1 : 0;
   r715 = (state[1] & (1 << 15)) != 0 ? 1 : 0;
   //state[0] = f(state[1],key[15]) ^ state[0];

   //Check
   uchar erg = l07^l018^l024^r012^r016^l77^l718^l724^l729^r715;
   uchar k0 = (key[0][2] & (1 << 6)) != 0 ? 1 : 0;
   uchar k019 = (key[0][2] & (1 << 3)) != 0 ? 1 : 0;
   uchar k023 = (key[0][2] & (1 << 7)) != 0 ? 1 : 0;
   uchar k1 = (key[1][2] & (1 << 6)) != 0 ? 1 : 0;
   uchar k119 = (key[1][2] & (1 << 3)) != 0 ? 1 : 0;
   uchar k123 = (key[1][2] & (1 << 7)) != 0 ? 1 : 0;
   uchar k2 = (key[2][2] & (1 << 6)) != 0 ? 1 : 0;
   uchar k3 = (key[3][2] & (1 << 6)) != 0 ? 1 : 0;
   uchar k344 = (key[3][5] & (1 << 4)) != 0 ? 1 : 0;
   uchar k4 = (key[4][2] & (1 << 6)) != 0 ? 1 : 0;
   uchar k444 = (key[4][5] & (1 << 4)) != 0 ? 1 : 0;
   uchar k5 = (key[5][2] & (1 << 6)) != 0 ? 1 : 0;
   uchar k6 = (key[6][2] & (1 << 6)) != 0 ? 1 : 0;
   uchar k7 = (key[7][2] & (1 << 6)) != 0 ? 1 : 0;
   uchar keybit = k019^k023^k2^k344^k4^k6;
   uchar keybit11 = k119^k123^k3^k444^k5^k7;
   //printf("%02x || %02x -> ", erg, keybit);

   matches += (erg == keybit) ? 1 : 0;
   total++;

   memcpy(out, state, 8 * sizeof(uchar));
   // Inverse IP
   //InvIP(state,out);
}

void des_crypt_8(uchar in[], uchar out[], uchar key[][6])
{
   uint state[2],idx,t;

   memcpy(state, in, 8 * sizeof(uchar));

   for (idx=0; idx < 8; ++idx)
   {
      t = state[1];
      state[1] = f(state[1],key[idx]) ^ state[0];
      state[0] = t;
   }

   memcpy(out, state, 8 * sizeof(uchar));
}

void des_crypt_8_attack(uchar in[][8], uchar out[][8], uchar key[][6])
{
   uint state[2],idx,t;
   uchar counter = 0;
   uchar kguess[6];
   uint64_t ran = rand();
   ran = (ran << 32) | rand();
   memcpy(kguess, &ran, 6 * sizeof(uchar));

   printf("Performing attack\n");

   //Generate round 8 sub key. Bits 42 to 47
   for(uint i = 0; i < 64; i++)
   {
     cnt[i] = 0;
     for(uint j = 0; j < DATA; j++)
     {
       uchar l07, l018, l024, r012, r016, r87, r818, r824, r829, l815;
       kguess[5] = counter << 2;

       memcpy(state, in[j], 8 * sizeof(uchar));

       l07 = (state[0] & (1 << 7)) != 0 ? 1 : 0;
       l018 = (state[0] & (1 << 18)) != 0 ? 1 : 0;
       l024 = (state[0] & (1 << 24)) != 0 ? 1 : 0;
       r012 = (state[1] & (1 << 12)) != 0 ? 1 : 0;
       r016 = (state[1] & (1 << 16)) != 0 ? 1 : 0;

       //We do not need this, because this was computed in advance
       /*for (idx=0; idx < 8; ++idx)
       {
          t = state[1];
          state[1] = f(state[1], key[idx]) ^ state[0];
          state[0] = t;
       }*/

       memcpy(state, out[j], 8 * sizeof(uchar));

       r87 = (state[1] & (1 << 7)) != 0 ? 1 : 0;
       r818 = (state[1] & (1 << 18)) != 0 ? 1 : 0;
       r824 = (state[1] & (1 << 24)) != 0 ? 1 : 0;
       r829 = (state[1] & (1 << 29)) != 0 ? 1 : 0;
       l815 = (state[0] & (1 << 15)) != 0 ? 1 : 0;

       //Recompute the last round
       uchar fs = (f(state[1], kguess) & (1 << 15)) != 0 ? 1 : 0;

       //Check
       uchar erg = l07^l018^l024^r012^r016^r87^r818^r824^r829^l815^fs;
       uchar k0 = (key[0][2] & (1 << 6)) != 0 ? 1 : 0;
       uchar k019 = (key[0][2] & (1 << 3)) != 0 ? 1 : 0;
       uchar k023 = (key[0][2] & (1 << 7)) != 0 ? 1 : 0;
       uchar k1 = (key[1][2] & (1 << 6)) != 0 ? 1 : 0;
       uchar k119 = (key[1][2] & (1 << 3)) != 0 ? 1 : 0;
       uchar k123 = (key[1][2] & (1 << 7)) != 0 ? 1 : 0;
       uchar k2 = (key[2][2] & (1 << 6)) != 0 ? 1 : 0;
       uchar k3 = (key[3][2] & (1 << 6)) != 0 ? 1 : 0;
       uchar k344 = (key[3][5] & (1 << 4)) != 0 ? 1 : 0;
       uchar k4 = (key[4][2] & (1 << 6)) != 0 ? 1 : 0;
       uchar k444 = (key[4][5] & (1 << 4)) != 0 ? 1 : 0;
       uchar k5 = (key[5][2] & (1 << 6)) != 0 ? 1 : 0;
       uchar k6 = (key[6][2] & (1 << 6)) != 0 ? 1 : 0;
       uchar k7 = (key[7][2] & (1 << 6)) != 0 ? 1 : 0;
       uchar keybit = k019^k023^k2^k4^k6^k344;
       uchar keybit11 = k119^k123^k3^k444^k5^k7;
       //printf("%02x || %02x -> ", erg, keybit);

       //matches += (erg == keybit) ? 1 : 0;
       //total++;
       //memcpy(out, state, 8 * sizeof(uchar));
       cnt[i] += (erg == keybit) ? 1 : 0;
     }
     counter++;
   }

   //find the max and the min value
   uint tmin = RAND_MAX, tmax = 0;
   for(uint i = 0; i < 64; i++)
   {
     if(cnt[i] < tmin)
       tmin = cnt[i];
     if(cnt[i] > tmax)
       tmax = cnt[i];
   }

   printf("Hier\n");
}

/************************************/

/*
Output should be:
c95744256a5ed31d
0123456789abcde7
85e813540f0ab405
0123456789abcdef
*/



void printtext(unsigned char hash[])
{
   int i;
   for (i=0; i < 8; i++)
      printf("%02x ",hash[i]);
   printf("\n");
}

void generateRandomDataWithDES(uchar plain[][8], uchar cipher[][8], uchar key[][6], int amount)
{
  for(int i = 0; i < amount; i++)
  {
    uint64_t in = rand();
    in = (in << 32) | rand();
    memcpy(plain[i], &in, 8 * sizeof(uchar));
    des_crypt_8(plain[i], cipher[i], key);
  }
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

int main()
{
   unsigned char key1[8]={0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF},
                 out[8],schedule[ROUNDS][6],
                 input[DATA][8];

   srand(time(NULL));

   for(unsigned int z = 0; z < 5; z++)
   {
     for(unsigned int j = 0; j < 20; j++)
     {
       generateRandomData(input, DATA);
       for(unsigned int i = 0; i < DATA; i++)
       {
         key_schedule(key1,schedule,ENCRYPT);
         //des_crypt_3(input[i], out, schedule);
         des_crypt_5(input[i], out, schedule);
         //des_crypt_7(input[i], out, schedule);
       }
     }
     printf("Found %d matches of %d -> %lf\n", matches, total, (double)matches / (double)total);
     matches = 0;
     total = 0;
  }

   //Generate plain and cipher text pairs
   /*key_schedule(key1,schedule,ENCRYPT);
   uchar plain[DATA][8];
   uchar cipher[DATA][8];

   for(unsigned int j = 0; j < 10; j++)
   {
     generateRandomDataWithDES(plain, cipher, schedule, DATA);
     des_crypt_8_attack(plain, cipher, schedule);
   }*/

   /*exit(0);

   key_schedule(key1,schedule,ENCRYPT);
   des_crypt(text1,out,schedule);
   printtext(out);

   //key_schedule(key1,schedule,DECRYPT);
   //des_crypt(out,text1,schedule);
   //printtext(text1);

   key_schedule(key2,schedule,ENCRYPT);
   des_crypt(text1,out,schedule);
   printtext(out);

   key_schedule(key1,schedule,ENCRYPT);
   des_crypt(text2,out,schedule);
   printtext(out);

   key_schedule(key2,schedule,ENCRYPT);
   des_crypt(text2,out,schedule);
   printtext(out);*/

   //key_schedule(key2,schedule,DECRYPT);
   //des_crypt(out,text2,schedule);
   //printtext(text2);

   return 0;
}
