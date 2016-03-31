#include <iostream>
#include <iomanip>
#include "crypto++/aes.h"
#include <thread>
#include <ctime>
#include <cstdlib>

using namespace CryptoPP;
using namespace std;

typedef unsigned char uint8_t;

uint8_t mulRijndaelPoly(uint8_t, uint8_t);

#define xtime(x) (((x) << 1) ^ (((x) & 0x80) ? 0x1b : 0))

#define SHIFTED_IND(i) ((((i) >> 2) << 2) + (((i) - ((i) >> 2)) % 4))
#define INV_SHIFTED_IND(i) ((((i) >> 2) << 2) + (((i) + ((i) >> 2)) % 4))

static const uint8_t sbox[256] =   {
        //0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
        0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
        0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
        0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
        0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
        0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
        0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
        0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
        0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
        0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
        0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
        0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
        0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
        0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
        0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
        0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
        0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };

static const uint8_t rsbox[256] =
        { 0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
          0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
          0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
          0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
          0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
          0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
          0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
          0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
          0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
          0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
          0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
          0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
          0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
          0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
          0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
          0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d };

uint8_t mul_lookup[256 * 256];

#define MUL(a, b) (mul_lookup[(((uint8_t) a) << 8) | ((uint8_t) b)])

void build_multiplication_lookup() {
    for (int i = 0; i < 256; i ++) {
        for (int j = 0; j < 256; j ++) {
            mul_lookup[(i << 8) | j] = mulRijndaelPoly(i, j);
        }
    }
}

void aesShiftRows(uint8_t state[]) {
    //first row
    uint8_t temp = state[4];
    state[4] = state[5];
    state[5] = state[6];
    state[6] = state[7];
    state[7] = temp;

    //second row
    temp = state[10];
    state[10] = state[8];
    state[8] = temp;
    temp = state[9];
    state[9] = state[11];
    state[11] = temp;

    //third row
    temp = state[15];
    state[15] = state[14];
    state[14] = state[13];
    state[13] = state[12];
    state[12] = temp;
}

void aesInvShiftRows(uint8_t state[]) {
    //third row
    uint8_t temp = state[12];
    state[12] = state[13];
    state[13] = state[14];
    state[14] = state[15];
    state[15] = temp;

    //second row
    temp = state[10];
    state[10] = state[8];
    state[8] = temp;
    temp = state[9];
    state[9] = state[11];
    state[11] = temp;

    //first row
    temp = state[7];
    state[7] = state[6];
    state[6] = state[5];
    state[5] = state[4];
    state[4] = temp;
}

void aesMixColumns(uint8_t state[])
{
    int i;
    uint8_t a,b,c,d;
    for(i=0;i<4;++i)
    {
        a = state[i];
        b = state[4 + i];
        c = state[8 + i];
        d = state[12 + i];

        state[i] = MUL(a, 0x02) ^ MUL(b, 0x03) ^ MUL(c, 0x01) ^ MUL(d, 0x01);
        state[4 + i] = MUL(a, 0x01) ^ MUL(b, 0x02) ^ MUL(c, 0x03) ^ MUL(d, 0x01);
        state[8 + i] = MUL(a, 0x01) ^ MUL(b, 0x01) ^ MUL(c, 0x02) ^ MUL(d, 0x03);
        state[12 + i] = MUL(a, 0x03) ^ MUL(b, 0x01) ^ MUL(c, 0x01) ^ MUL(d, 0x02);
    }
}

void aesInvMixColumns(uint8_t state[])
{
    int i;
    uint8_t a,b,c,d;
    for(i=0;i<4;++i)
    {
        a = state[i];
        b = state[4 + i];
        c = state[8 + i];
        d = state[12 + i];

        state[i] = MUL(a, 0x0e) ^ MUL(b, 0x0b) ^ MUL(c, 0x0d) ^ MUL(d, 0x09);
        state[4 + i] = MUL(a, 0x09) ^ MUL(b, 0x0e) ^ MUL(c, 0x0b) ^ MUL(d, 0x0d);
        state[8 + i] = MUL(a, 0x0d) ^ MUL(b, 0x09) ^ MUL(c, 0x0e) ^ MUL(d, 0x0b);
        state[12 + i] = MUL(a, 0x0b) ^ MUL(b, 0x0d) ^ MUL(c, 0x09) ^ MUL(d, 0x0e);
    }
}

void aesInvMixColumns(uint8_t state[], uint col)
{
    uint8_t a, b, c, d;

    a = state[col];
    b = state[4 + col];
    c = state[8 + col];
    d = state[12 + col];

    state[col] = MUL(a, 0x0e) ^ MUL(b, 0x0b) ^ MUL(c, 0x0d) ^ MUL(d, 0x09);
    state[4 + col] = MUL(a, 0x09) ^ MUL(b, 0x0e) ^ MUL(c, 0x0b) ^ MUL(d, 0x0d);
    state[8 + col] = MUL(a, 0x0d) ^ MUL(b, 0x09) ^ MUL(c, 0x0e) ^ MUL(d, 0x0b);
    state[12 + col] = MUL(a, 0x0b) ^ MUL(b, 0x0d) ^ MUL(c, 0x09) ^ MUL(d, 0x0e);
}

void aesSubBytes(uint8_t state[]) {
    uint8_t i;
    for (i = 0; i < 16; i ++) {
        state[i] = sbox[state[i]];
    }
}

void aesInvSubBytes(uint8_t state[]) {
    uint8_t i;
    for (i = 0; i < 16; i ++) {
        state[i] = rsbox[state[i]];
    }
}

void aesAddKey(uint8_t state[], uint8_t key[]) {
    uint8_t i;
    for (i = 0; i < 16; i ++) {
        state[i] ^= key[i];
    }
}

void aesRound(uint8_t state[], uint8_t key[]) {
    aesSubBytes(state);
    aesShiftRows(state);
    aesMixColumns(state);
    aesAddKey(state, key);
}

void aesInvRound(uint8_t state[], uint8_t key[]) {
    aesAddKey(state, key);
    aesInvMixColumns(state);
    aesInvShiftRows(state);
    aesInvSubBytes(state);
}

void printHexState(uint8_t state[]) {
    for (int i = 0; i < 4; i ++) {
        for (int j = 0; j < 4; j ++) {
            std::cout << std::setw(2) << std::hex << (int) state[i * 4 + j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void generateRandomState(uint8_t state[]) {
    uint8_t i = 0;
    for (; i < 16; i ++)
        state[i] = rand() % 0xff;
}

uint modRijndaelPoly(uint p) {
    uint rijn = 0b100011011;
    uint temp = p;
    temp = temp >> 8;
    uint m = 0;
    while (temp) {
        m ++;
        temp = temp >> 1;
    }
    if (! m)
        return p;
    p ^= (rijn << (m - 1));
    return modRijndaelPoly(p);
}

uint8_t mulRijndaelPoly(uint8_t p1, uint8_t p2) {
    uint8_t t = 0;
    uint8_t i = 0;
    while (p2) {
        t ^= p2 & 1 ? p1 << i : 0;
        p2 = p2 >> 1;
        i ++;
    }
    return (uint8_t) modRijndaelPoly(t);
}

uint8_t key_1[16] = {0x32,0x34,0x84,0x4F,0xE3,0xBA,0x1C,0x45,0x9A,0xEE,0xBE,0xFF,0x5A,0xB8,0xFA,0x83};
uint8_t key_2[16] = {0xF2,0x24,0x8F,0xEF,0xC2,0xBA,0x4F,0x90,0x93,0xFE,0xBE,0x21,0x4C,0xA2,0x3A,0x43};
uint8_t key_3[16] = {0x14,0xC3,0xA4,0xCF,0x83,0x7A,0x3B,0x15,0x0A,0xBE,0x31,0x57,0xE3,0x48,0x63,0x13};

uint8_t key_4[16] = {0x22,0x24,0x72,0x12,
                     0xC2,0xBA,0x42,0x90,
                     0x5C,0x21,0xB1,0x21,
                     0x4C,0xA2,0xBA,0x4B};

uint8_t key_5[16] = {0xEE,0xA2,0x3E,0x17,
                     0xCE,0x3A,0x2E,0x90,
                     0x5C,0xB1,0x41,0x81,
                     0x2C,0xAE,0x3A,0x23};

void sub_key_loop(uint col, uint8_t *c_set, uint64_t key_start, uint64_t key_end, std::map<uint64_t, uint> *key_counter) {
    uint sum;
    uint8_t state[16];

    for (uint64_t key = key_start; key < key_end; key++) {
        sum = 0;
        for (int i = 0; i < 256; i++) {
            memcpy(state, &c_set[16 * i], 16);

            //KEY ADDITION AND SHIFT ROWS
            state[col] ^= (uint8_t) (key >> 8);
            state[4 + col] = state[SHIFTED_IND(4 + col)] ^ (uint8_t) (key >> 16);
            state[8 + col] = state[SHIFTED_IND(8 + col)] ^ (uint8_t) (key >> 24);
            state[12 + col] = state[SHIFTED_IND(12 + col)] ^ (uint8_t) (key >> 32);

            state[col] = rsbox[state[col]];
            state[4 + col] = rsbox[state[4 + col]];
            state[8 + col] = rsbox[state[8 + col]];
            state[12 + col] = rsbox[state[12 + col]];

            //aesInvShiftRows(state);
            //aesInvSubBytes(state);

            aesInvMixColumns(state, col);
            //aesInvShiftRows(state);
            state[col] ^= (uint8_t) (key);//0x6a;
            //aesInvSubBytes(state);
            state[col] = rsbox[state[col]];

            sum ^= state[col];
        }

        if (!sum) {
            //std::cout << "found " << std::hex << key << std::endl;
            if (key_counter->find(key) != key_counter->end()) {
                (*key_counter)[key]++;
            }
            else
                (*key_counter)[key] = 1;
        }
    }
}

void recover_key_col(uint col, uint8_t **c_sets, uint n_lambda_sets, uint8_t *key_row) {
    auto cmp = [] (std::pair<const uint64_t, uint>& p1, std::pair<const uint64_t, uint>& p2) { return p1.second < p2.second; };
    std::map<uint64_t, uint> key_counter;
    uint n_threads = 8;

    for (int lambda_set = 0; lambda_set < n_lambda_sets; lambda_set++) {
        uint8_t *c_set = c_sets[lambda_set];
        uint64_t key_start = ((uint64_t) key_5[SHIFTED_IND(12 + col)] << 32) |
                ((uint64_t) key_5[SHIFTED_IND(8 + col)] << 24);// |
                //((uint64_t) key_5[SHIFTED_IND(4 + col)] << 16);
        uint64_t key_end = key_start + ((uint64_t) 1 << 24);
        uint64_t key_step = (key_end - key_start) / n_threads;
        std::thread *threads = new std::thread[n_threads];
        std::map<uint64_t, uint> **key_counters = new std::map<uint64_t, uint>*[n_threads];
        for (uint t = 0; t < n_threads; t ++) {
            key_counters[t] = new std::map<uint64_t, uint>();
            threads[t] = std::thread(sub_key_loop, col, c_set, key_start + t * key_step, key_start + (t + 1) * key_step, key_counters[t]);
        }
        for (uint t = 0; t < n_threads; t ++) {
            threads[t].join();
            std::map<uint64_t, uint>::iterator it = key_counters[t]->begin();
            for (; it != key_counters[t]->end(); it ++) {
                if (key_counter.find(it->first) == key_counter.end())
                    key_counter[it->first] = 1;
                else
                    key_counter[it->first] ++;
            }
            delete key_counters[t];
        }
        delete[] key_counters;
        delete[] threads;
    }

    auto max = std::max_element(key_counter.begin(), key_counter.end(), cmp);
    std::cout << "found key: 0x" << std::hex << (uint64_t) max->first << std::endl;
    //*key = max->first;
}

int main() {
    uint8_t state[16]; // = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};

    std::map<uint8_t, uint> key_counter;

    build_multiplication_lookup();

    srand(time(NULL));

    clock_t t = clock();
    time_t time_t1 = time(NULL);

    uint n_lambda_sets = 4;
    uint8_t **p_sets = new uint8_t*[n_lambda_sets];
    uint8_t **c_sets = new uint8_t*[n_lambda_sets];

    for (int j = 0; j < n_lambda_sets; j ++) {
        uint8_t *p_set = new uint8_t[256 * 16];
        uint8_t *c_set = new uint8_t[256 * 16];
        generateRandomState(state);
        memcpy(p_set, state, 16);
        for (int i = 1; i < 256; i ++) {
            memcpy(p_set + 16 * i, state, 16);
            *(p_set + 16 * i) ^= i;
        }

        memcpy(c_set, p_set, 256 * 16);
        for (int i = 0; i < 256; i ++) {
            aesRound(&c_set[16 * i], key_1);
            aesRound(&c_set[16 * i], key_2);
            aesRound(&c_set[16 * i], key_3);
            aesRound(&c_set[16 * i], key_4);

            //final round
            aesSubBytes(&c_set[16 * i]);
            aesShiftRows(&c_set[16 * i]);
            aesAddKey(&c_set[16 * i], key_5);
        }

        p_sets[j] = p_set;
        c_sets[j] = c_set;
    }

    recover_key_col(1, c_sets, n_lambda_sets, NULL);

    std::cout << "passed time(clock): " << double(clock() - t) / CLOCKS_PER_SEC << " seconds " << std::endl;
    std::cout << "passed time(time): " << std::dec << time(NULL) - time_t1 << " seconds " << std::endl;

    for (int i = 0; i < n_lambda_sets; i ++) {
        delete[] p_sets[i], c_sets[i];
    }
    delete[] p_sets, c_sets;

    return 0;
}