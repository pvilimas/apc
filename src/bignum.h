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

typedef uint32_t bn_digit_t;

typedef struct {
    bn_digit_t* digits_end;     // allocated pointer to last digit (LSD)
    size_t msd_pos;             // position of first digit (MSD)
    size_t capacity;            // total capacity of digits_end
    uint8_t base;               // valid range: 2-36
    uint8_t signbit;            // 1 means negative
} Bignum;

#define BN_BASE_DEFAULT 10
#define BN_BASE_MIN 2
#define BN_BASE_MAX 36

// definition: fake_base^width = real_base < UINT32_MAX < fake_base^(width+1)
typedef struct {
    uint8_t fake_base;
    uint8_t width;
    uint32_t real_base;
    char last_digit[2];     // lower and uppercase
} BignumBase;

extern const BignumBase bn_bases[BN_BASE_MAX + 1];

// constants

extern const Bignum BN_ZERO;
extern const Bignum BN_ONE;

// if this is defined, the library will not free any memory:
// #define BN_NOFREE
// (apc.h uses this - all memory is freed on subprocess exit)

// optional custom memory functions
// !!! SETTING THESE DOES NOT WORK RIGHT NOW !!!
// TODO figure out how to make this work
#ifndef BN_MALLOC
#define BN_MALLOC malloc
#endif

#ifndef BN_REALLOC
#define BN_REALLOC realloc
#endif

#ifndef BN_FREE
#define BN_FREE free
#endif

// constructors and io

// a bignum must be set to {0} before calling any library functions on it
// note that {0} is not a valid Bignum value
#define bn_new() ((Bignum){0})

// initialize a list of pointers to bignum, all to {0}
// does not allocate or free any memory
#define bn_init(...)

// free a list of pointers to bignum
#define bn_free(...)

// write a base-10 number to a bignum, or return false for parse error
bool bn_write(Bignum* b, const char* str);

// write a number with explicit base or return false for parse error
bool bn_write2(Bignum* b, const char* str, uint8_t base);

// write a number with explicit length and base, or return false for parse error
bool bn_write3(Bignum* b, const char* str, size_t len, uint8_t base);

// copy src to dest
void bn_copy(Bignum* dest, const Bignum* src);

// copy src to dest, converting to new_base
void bn_convert(Bignum* dest, const Bignum* src, uint8_t new_base);

// print a standard-representation of a bignum to the console
// returns # of characters written
size_t bn_print(const Bignum* b, bool print_base, bool use_uppercase);

// comparison methods

// a0 == a1
bool bn_equals(const Bignum* a0, const Bignum* a1);

// a0 == 0
bool bn_equals_zero(const Bignum* a0);

// -1 => a0 < a1
// 0 => a0 == a1
// 1 => a0 > a1
int bn_cmp(const Bignum* a0, const Bignum* a1);

// basic arithmetic methods

// result = -a0
void bn_neg(Bignum* result,
            const Bignum* a0);

// result = a0 + a1
void bn_add(Bignum* result,
            const Bignum* a0,
            const Bignum* a1);

// result = a0 - a1
void bn_sub(Bignum* result,
            const Bignum* a0,
            const Bignum* a1);

// result = a0 * a1
void bn_mul(Bignum* result,
            const Bignum* a0,
            const Bignum* a1);

// result_div = a0 // a1 (integer division)
// result_mod = a0 % a1
// returns false if a1 == 0
// both results can optionally be NULL
bool bn_divmod(Bignum* result_div, Bignum* result_mod,
               const Bignum* a0,
               const Bignum* a1);

// internal

// get the real length of the bignum, ignoring leading zeroes
size_t bni_real_len(const Bignum* b);

// is b a valid bignum or is it NULL or &{0} ?
bool bni_is_valid(const Bignum* b);

// the bni_** functions below assume that all bignum arguments are valid

// free out if it's allocated
// allocate space for n_digits in the specified base, zeroed out
void bni_realloc(Bignum* out, size_t n_digits, uint8_t base);

// create a deep copy of src in dest
void bni_copy(Bignum* dest, const Bignum* src);

// copy src to dest, converting to a different base
// assume new_base in [2,36], new_base != src->base
void bni_convert(Bignum* dest, const Bignum* src, uint8_t new_base);

// free a single bignum (unless #ifdef BN_NOFREE) and set it to {0}
void bni_try_free(Bignum* out);

// strip leading zeroes, set out->msd_pos
void bni_normalize(Bignum* out);

// write a string value to a bignum, or return false for parse error
// TODO make this accept a length argument to avoid a pointless strdup
// TODO make this assume base is valid, move that check to the write functions
bool bni_write_str(Bignum* out, const char* str, uint8_t base);

// write a 1-digit value in any base to a bignum
// returns false for illegal digit value
bool bni_write_parts1(Bignum* out,
                      uint8_t signbit,
                      bn_digit_t d0,
                      uint8_t base);

// write a 2-digit value in any base to a bignum
// returns false for illegal digit value
bool bni_write_parts2(Bignum* out,
                      uint8_t signbit,
                      bn_digit_t d0, bn_digit_t d1,
                      uint8_t base);

// print all struct fields and allocated digits in memory order (debug print)
void bni_dump(const Bignum* b);

// -1 => a0 < a1
// 0 => a0 == a1
// 1 => a0 > a1
// compares with a single digit
int bni_cmp_Nx1(const Bignum* a0, bn_digit_t a1);

// out = a0 << n (in base a1->base.real_base)
// assumes n > 0, ignores sign
void bni_lshift(Bignum* out, const Bignum* a0, size_t n);

// out = a0 >> n (in base a1->base.real_base)
// assumes n > 0, ignores sign
void bni_rshift(Bignum* out, const Bignum* a0, size_t n);

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

// q_out = a0 // a1 (integer division)
// r_out = a0 % a1 (remainder)
// assumes a0, a1 > 0
void bni_divqr_Nx1(Bignum* q_out, Bignum* r_out,
                   const Bignum* a0, size_t a1);

// utils

uint64_t bnu_min(uint64_t x, uint64_t y);
uint64_t bnu_max(uint64_t x, uint64_t y);
char* bnu_strndup(const char* str, size_t n);

// if q_out and/or r_out == NULL they are ignored, else:
// q_out = x // y (integer divide quotient)
// r_out = x % y (mod aka remainder)
// returns false if it failed because y == 0 (div/mod by zero error)
// (base does not matter for single digit operations)
bool bnu_divqr_1x1(bn_digit_t x,
                    bn_digit_t y,
                    bn_digit_t* q_out, bn_digit_t* r_out);

// same as above except x0-x1 is treated as 2 digits of a number in base <base>
// and quotient also has 2 digits
// will not write to q0 AND q1 if either one is NULL
bool bnu_divqr_2x1(bn_digit_t x0, bn_digit_t x1,
                    bn_digit_t y,
                    uint8_t base,
                    bn_digit_t* q0_out, bn_digit_t* q1_out,
                    bn_digit_t* r_out);

// parse a u32 from a string slice or return false for parse error
bool bnu_parse_digit(const char* str,
                    size_t start,
                    size_t end,
                    uint8_t base,
                    bn_digit_t* out);

// print a single digit to stdout
// returns # of characters written
size_t bnu_print_digit(bn_digit_t digit_value,
                         uint8_t base,
                         bool print_leading_zeroes,
                         bool use_uppercase_digits);

// is a char a valid digit in the specified base?
// writes the digit value to value_out if it returns true and value_out != NULL
bool bnu_digit_valid(char digit, uint8_t base, bn_digit_t* value_out);

// 2 <= base <= 36 ?
bool bnu_base_valid(uint8_t base);

// is a digit small enough to fit in a single-digit bignum without any
// truncation or overflow? in the specified base
bool bnu_digit_in_range(bn_digit_t digit, uint8_t base);

// macros

#undef bn_init
#define bn_init(...) \
    do { \
        size_t _len = sizeof((Bignum*[]){__VA_ARGS__}) / sizeof(Bignum*); \
        Bignum* _bs[] = { __VA_ARGS__ }; \
        for (size_t _i = 0; _i < _len; _i++) { \
            if (_bs[_i]->digits_end != NULL) \
                *(_bs[_i]) = (Bignum){0}; \
        } \
    } while(0)

// free a list of pointers to bignum
#undef bn_free
#define bn_free(...) \
    do { \
        size_t _len = sizeof((Bignum*[]){__VA_ARGS__}) / sizeof(Bignum*); \
        Bignum* _bs[] = { __VA_ARGS__ }; \
        for (size_t _i = 0; _i < _len; _i++) { \
            bni_try_free(_bs[_i]); \
        } \
    } while(0)

#endif // BIGNUM_H
