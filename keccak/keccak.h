// keccak.h
// 19-Nov-11  Markku-Juhani O. Saarinen <mjos@iki.fi>

#ifndef KECCAK_H
#define KECCAK_H

#include <stdint.h>
#include <string.h>

#ifndef KECCAK_ROUNDS
#define KECCAK_ROUNDS 5
#endif

#ifndef ROTL64
#define ROTL64(x, y) (((x) << (y)) | ((x) >> (64 - (y))))
#endif

// compute a keccak hash (md) of given byte length from "in"
int keccak(const uint8_t *in, int inlen, uint8_t *md, int mdlen);

void chi(uint64_t st[25]);
void inverseChi(uint64_t st[25]);
void thetaRhoPi(uint64_t st[25]);
void inverseIotaChi(uint64_t st[25], int round);
void inverseChiRow(uint64_t row_to_invert[]);
void inverseIotaChiRow(uint64_t row[], int round_to_invert);

// update the state
void keccakf(uint64_t st[25], int norounds);

#endif

