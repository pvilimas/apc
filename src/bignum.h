#ifndef BIGNUM_H
#define BIGNUM_H

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// bignum.h - arbitrary precision math library

// currently supports:
// positive and negative integers
// comparing numbers
// + - *

typedef struct {
    uint32_t* digits_end; // pointer to start of buffer/last digit (LSD)
    uint32_t* start; // pointer to first digit MSD
    uint64_t len; // total capacity of buffer
    uint64_t sign; // 1 means negative
} Bignum;

// each digit is base 10^9 (the largest power of 10 that fits in a u32)
#define BN_BASE         1000000000
#define BN_DIGIT_MAX     999999999

// if this is defined, the library will not free any memory
// apc.h uses this - all memory is freed when process exits
#define BN_NOFREE

// optional custom memory functions
#ifndef BN_MALLOC_FN
#define BN_MALLOC_FN malloc
#endif

#ifndef BN_REALLOC_FN
#define BN_REALLOC_FN realloc
#endif

#ifndef BN_FREE_FN
#define BN_FREE_FN free
#endif

// bignums must be initialized to {0} before calling any functions on it
#define bn_new() \
    ((Bignum){0})

// initialize a list of pointers to bignum, all to {0}
// does not allocate any memory
#define bn_init(...) \
    do { \
        int _len = sizeof((Bignum*[]){__VA_ARGS__}) / sizeof(Bignum*); \
        Bignum* _bs[] = { __VA_ARGS__ }; \
        for (int _i = 0; _i < _len; _i++) { \
            if (_bs[_i]->digits_end != NULL) \
                *(_bs[_i]) = (Bignum){0}; \
        } \
    } while(0)

// free a list of pointers to bignum
#define bn_free(...) \
    do { \
        size_t _len = sizeof((Bignum*[]){__VA_ARGS__}) / sizeof(Bignum*); \
        Bignum* _bs[] = { __VA_ARGS__ }; \
        for (size_t _i = 0; _i < _len; _i++) { \
            bn_ifree(_bs[_i]); \
            *(_bs[_i]) = (Bignum){0}; \
        } \
    } while(0)

// io

// write a value from a string to a bignum, or return false for parse error
bool bn_write(Bignum* b, const char* str);

// write a string with explicit length or return false for parse error
bool bn_write2(Bignum* b, const char* str, size_t len);

// print a standard-representation of a bignum to the console
// returns # of characters written
int bn_print(const Bignum* b);

// write a standard-representation of a bignum to a string
char* bn_to_str(const Bignum* b);

// debug print to the console
void bn_dump(const Bignum* b);

// comparison

// -1 => a0 < a1
// 0 => a0 == a1
// 1 => a0 > a1
int bn_cmp(const Bignum* a0, const Bignum* a1);

// a0 == 0
bool bn_is_zero(const Bignum* a0);

// operations

// result = a0 + a1
void bn_add(Bignum* result, const Bignum* a0, const Bignum* a1);

// result = a0 - a1
void bn_sub(Bignum* result, const Bignum* a0, const Bignum* a1);

// result = a0 * a1
void bn_mul(Bignum* result, const Bignum* a0, const Bignum* a1);

// internal

// allocate space for n_digits, zeroed out
void bn_ialloc(Bignum* out, uint64_t n_digits);

// create a deep copy of src in dest
void bn_icopy(Bignum* dest, const Bignum* src);

// free a single bignum (unless #ifdef BN_NOFREE) and set it to {0}
void bn_ifree(Bignum* out);

// strip leading zeroes, set out->start
void bn_inormalize(Bignum* out);

// write a string value to a bignum
bool bn_iwrite_str(Bignum* out, const char* str);

// write a signbit and a value to a bignum (sign is 0 => positive, 1 => negative)
void bn_iwrite_parts(Bignum* out, uint32_t sign, uint32_t value);

// out = a0 << n (in base BN_BASE)
// assumes n > 0
// preserves sign of a0
void bn_ilshift(Bignum* out, const Bignum* a0, uint64_t n);

// out = a0 >> n (in base BN_BASE)
// assumes n > 0
// preserves sign of a0
void bn_irshift(Bignum* out, const Bignum* a0, uint64_t n);

// out = a0 + a1
// assumes a0, a1 > 0
void bn_iadd(Bignum* out, const Bignum* a0, const Bignum* a1);

// out = a0 - a1
// assumes a0, a1 > 0
void bn_isub(Bignum* out, const Bignum* a0, const Bignum* a1);

// out = a0 * a1
// assumes a0, a1 > 0
void bn_imul(Bignum* out, const Bignum* a0, const Bignum* a1);

// utils

int bn_min(int x, int y);
int bn_max(int x, int y);

// parse u32 from a slice of a string and allocate it, return false for parse error
bool bn_str_to_u32(const char* str, uint32_t start, uint32_t end, uint32_t* out);

char* bn_strndup(const char* str, int n);

#endif // BIGNUM_H
