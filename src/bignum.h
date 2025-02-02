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
    uint32_t* digits_end;   // allocated pointer/last digit (LSD)
    uint64_t msd_pos;       // position of first digit (MSD)
    uint64_t cap;           // total capacity of buffer
} Bignum;

#define BN_BASE_DEFAULT 10
#define BN_BASE_MIN 2
#define BN_BASE_MAX 36

typedef struct {
    uint32_t fake_base;
    uint32_t real_base;
    uint32_t width;
    char last_digit[2];
} BignumBase;

extern const BignumBase bn_bases[BN_BASE_MAX + 1];

// if this is defined, the library will not free any memory:
// #define BN_NOFREE
// (apc.h uses this - all memory is freed on subprocess exit)

// optional custom memory functions
// !!! SETTING THESE DOES NOT WORK RIGHT NOW !!!
#ifndef BN_MALLOC
#define BN_MALLOC malloc
#endif

#ifndef BN_REALLOC
#define BN_REALLOC realloc
#endif

#ifndef BN_FREE
#define BN_FREE free
#endif

// a bignum must be initialized to {0} before calling any functions on it
#define bn_new() \
    ((Bignum){0})

// initialize a list of pointers to bignum, all to {0}
// does not allocate any memory
#define bn_init(...) \
    do { \
        uint64_t _len = sizeof((Bignum*[]){__VA_ARGS__}) / sizeof(Bignum*); \
        Bignum* _bs[] = { __VA_ARGS__ }; \
        for (uint64_t _i = 0; _i < _len; _i++) { \
            if (_bs[_i]->digits_end != NULL) \
                *(_bs[_i]) = (Bignum){0}; \
        } \
    } while(0)

// free a list of pointers to bignum
#define bn_free(...) \
    do { \
        uint64_t _len = sizeof((Bignum*[]){__VA_ARGS__}) / sizeof(Bignum*); \
        Bignum* _bs[] = { __VA_ARGS__ }; \
        for (uint64_t _i = 0; _i < _len; _i++) { \
            bn_ifree(_bs[_i]); \
            *(_bs[_i]) = (Bignum){0}; \
        } \
    } while(0)

// io

// write a value from a string to a bignum, or return false for parse error
bool bn_write(Bignum* b, const char* str, uint32_t base);

// write a string with explicit length or return false for parse error
bool bn_write2(Bignum* b, const char* str, uint64_t len, uint32_t base);

// write a copy of src into dest
void bn_copy(Bignum* dest, const Bignum* src);

// copy src to dest, converting to new_base
void bn_convert_base(Bignum* dest, const Bignum* src, uint32_t new_base);

// print a standard-representation of a bignum to the console
// returns # of characters written
uint32_t bn_print(const Bignum* b, bool print_base, bool uppercase);

// print all struct fields and allocated digits in memory order (debug print)
void bn_dump(const Bignum* b);

// comparison

// -1 => a0 < a1
// 0 => a0 == a1
// 1 => a0 > a1
int32_t bn_cmp(const Bignum* a0, const Bignum* a1);

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

// result = a0 / a1
// TODO REALLY does not work rn
// TODO what to do if a1 == 0?
void bn_div(Bignum* result, const Bignum* a0, const Bignum* a1);

// internal methods

// get the real length of the bignum, ignoring leading zeroes
uint64_t bni_real_len(const Bignum* b);

// allocate space for n_digits, zeroed out
void bni_alloc(Bignum* out, uint64_t n_digits, uint32_t base);

// create a deep copy of src in dest
void bni_copy(Bignum* dest, const Bignum* src);

// copy src to dest, converting to a different base
void bni_conv(Bignum* dest, const Bignum* src, uint64_t new_base);

// free a single bignum (unless #ifdef BN_NOFREE) and set it to {0}
void bni_free(Bignum* out);

// strip leading zeroes, set out->start
void bni_normalize(Bignum* out);

// write a string value to a bignum
bool bni_write_str(Bignum* out, const char* str, uint32_t base);

// write a base-10 signbit+value to a bignum
void bni_write_parts(Bignum* out, uint32_t signbit, uint32_t value);

// internal operations

// out = a0 << n (in base a0->base.mdv)
// assumes n > 0, ignores sign
void bni_lshift(Bignum* out, const Bignum* a0, uint64_t n);

// out = a0 >> n (in base a1->base.mdv)
// assumes n > 0, ignores sign
void bni_rshift(Bignum* out, const Bignum* a0, uint64_t n);

// out = -a0
// assumes a0 != 0
void bni_neg(Bignum* out, const Bignum* a0);

// out = a0 + a1
// assumes a0, a1 > 0
void bni_add(Bignum* out, const Bignum* a0, const Bignum* a1);

// out = a0 - a1
// assumes a0, a1 > 0
void bni_sub(Bignum* out, const Bignum* a0, const Bignum* a1);

// out = a0 * a1
// assumes a0, a1 > 0
void bni_mul(Bignum* out, const Bignum* a0, const Bignum* a1);

// out = a0 // a1 (integer division)
// assumes a1 != 0
void bni_intdiv_u32(Bignum* out, const Bignum* a0, uint32_t a1);

// utils

uint64_t bnu_min(uint64_t x, uint64_t y);
uint64_t bnu_max(uint64_t x, uint64_t y);

// parse u32 from a string slice or return false for parse error
bool bnu_str_to_u32(const char* str, uint32_t start, uint32_t end, uint32_t base, uint32_t* out);

// print a single digit to stdout
// returns # of characters written
uint32_t bnu_print_digit(uint32_t digit,
                         uint32_t base,
                         bool print_lz,
                         bool uppercase);

// writes to value_out if it returns true
bool bnu_digit_valid(char digit, uint32_t base, uint32_t* value_out);

char* bnu_strndup(const char* str, int n);

#endif // BIGNUM_H
