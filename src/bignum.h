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
// base 2 through base 36

typedef struct {
    uint32_t base;          // valid range: 2-36
    uint32_t sign;          // 1 means negative
    uint32_t* digits_end;   // pointer to start of buffer/last digit (LSD)
    uint32_t* start;        // pointer to first digit MSD
    uint64_t len;           // total capacity of buffer
} Bignum;

#define BN_BASE_MIN 2
#define BN_BASE_MAX 36
#define BN_BASE_DEFAULT 10

typedef struct {
    uint32_t value;
    uint32_t max_digit_value;
    uint32_t max_power;
} BignumBase;

extern BignumBase bn_bases[BN_BASE_MAX + 1];
extern const char* bn_base_digits;

// each digit is base 10^9 (largest power of 10 that fits in a u32)
#define BN_BASE         1000000000
#define BN_DIGIT_MAX     999999999

// if this is defined, the library will not free any memory
// apc.h uses this - all memory is freed on subprocess exit
// #define BN_NOFREE

// optional custom memory functions
#ifndef BN_MALLOC
#define BN_MALLOC malloc
#endif

#ifndef BN_REALLOC
#define BN_REALLOC realloc
#endif

#ifndef BN_FREE
#define BN_FREE free
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
bool bn_write(Bignum* b, const char* str, uint32_t base);

// write a string with explicit length or return false for parse error
bool bn_write2(Bignum* b, const char* str, size_t len, uint32_t base);

// write a copy of src into dest
void bn_copy(Bignum* dest, const Bignum* src);

// TODO copy src to dest, in base new_base
void bn_convert_base(Bignum* dest, const Bignum* src, uint32_t new_base);

// print a standard-representation of a bignum to the console
// returns # of characters written
uint32_t bn_print(const Bignum* b, bool print_base);

// print all struct fields and allocated digits in memory order (debug print)
void bn_dump(const Bignum* b);

// comparison

// -1 => a0 < a1
// 0 => a0 == a1
// 1 => a0 > a1
int bn_cmp(const Bignum* a0, const Bignum* a1);

// a0 == 0
bool bn_is_zero(const Bignum* a0);

// operations

// result = -a0
void bn_neg(Bignum* result, const Bignum* a0);

// result = a0 + a1
void bn_add(Bignum* result, const Bignum* a0, const Bignum* a1);

// result = a0 - a1
void bn_sub(Bignum* result, const Bignum* a0, const Bignum* a1);

// result = a0 * a1
void bn_mul(Bignum* result, const Bignum* a0, const Bignum* a1);

// internal

// get the real length of the bignum, ignoring leading zeroes
#define bn_rlen(b) \
    (1 + (b)->start - (b)->digits_end)

// allocate space for n_digits, zeroed out
void bn_ialloc(Bignum* out, uint64_t n_digits);

// create a deep copy of src in dest
void bn_icopy(Bignum* dest, const Bignum* src);

// free a single bignum (unless #ifdef BN_NOFREE) and set it to {0}
void bn_ifree(Bignum* out);

// strip leading zeroes, set out->start
void bn_inormalize(Bignum* out);

// write a string value to a bignum
bool bn_iwrite_str(Bignum* out, const char* str, uint32_t base);

// write a base-10 signbit+value to a bignum (sign == 0 => positive, 1 => negative)
void bn_iwrite_parts(Bignum* out, uint32_t sign, uint32_t value);

// print a single digit to stdout
// returns # of characters written
uint32_t bn_iprint_digit(uint32_t digit, uint32_t base, bool print_leading_0s);

// out = a0 << n (in base BN_BASE)
// assumes n > 0
// preserves sign of a0
void bn_ilshift(Bignum* out, const Bignum* a0, uint64_t n);

// out = a0 >> n (in base BN_BASE)
// assumes n > 0
// preserves sign of a0
void bn_irshift(Bignum* out, const Bignum* a0, uint64_t n);

// out = -a0
// assumes a0 != 0
void bn_ineg(Bignum* out, const Bignum* a0);

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

// parse u32 from a string slice or return false for parse error
bool bn_str_to_u32(const char* str, uint32_t start, uint32_t end, uint32_t base, uint32_t* out);

char* bn_strndup(const char* str, int n);

// writes to value_out if it returns true
bool digit_is_valid_in_base(char digit, uint32_t base, uint32_t* value_out);

#endif // BIGNUM_H
