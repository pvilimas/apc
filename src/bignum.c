#include "bignum.h"

// write from a string or return false for parse error
bool bn_write(Bignum* b, const char* str) {
    bool ok = bn_iwrite_str(b, str);
    if (ok) {
        bn_inormalize(b);
    }
    return ok;
}

// write from a string w/ explicit length or return false for parse error
bool bn_write2(Bignum* b, const char* str, size_t len) {
    char* s2 = strndup(str, len);
    bool ok = bn_write(b, s2);
#ifndef BN_NOFREE
    BN_FREE_FN(s2);
#endif
    return ok;
}

// write a copy of src into dest
void bn_copy(Bignum* dest, const Bignum* src) {
    bn_icopy(dest, src);
}

// print a standard-representation of a bignum to the console
// returns # of characters written
int bn_print(const Bignum* b) {
    if (b->digits_end == NULL) {
        fputs("(null)", stdout);
        return 6;
    }

    if (bn_is_zero(b)) {
        fputs("0", stdout);
        return 1;
    }

    int nc = 0;
    int n = 0;

    if (b->sign) {
        fputs("-", stdout);
        nc += 1;
    }

    // print leading digit without leading 0s
    printf("%lu%n", (unsigned long) *b->start, &n);
    nc += n;

    // does it only have 1 digit?
    if (b->start == b->digits_end) {
        return nc;
    }

    // print remaining digits, zero-padded to 9 decimal places
    for (const uint32_t* d = b->start - 1; d >= b->digits_end; d--) {
        printf("%0*lu%n", 9, (unsigned long) *d, &n);
        nc += n;
    }

    return nc;
}

// return a standard-representation of a bignum as a string
char* bn_to_str(const Bignum* b) {
    if (b->digits_end == NULL) {
        return bn_strndup("(null)", 6);
    }

    if (bn_is_zero(b)) {
        return bn_strndup("0", 1);
    }

    char* str;
    size_t len = 0;

    // append leading digit without leading 0s
    size_t len_ld = snprintf(NULL, 0, "%s%lu",
        b->sign ? "-" : "",
        (unsigned long) *b->start);
    str = BN_MALLOC_FN((len_ld+1) * sizeof(char));
    memset(str, '\0', len_ld+1);
    snprintf(str, len_ld+1, "%s%lu",
        b->sign ? "-" : "",
        (unsigned long) *b->start);
    len += len_ld;

    // does it only have 1 digit?
    if (b->start == b->digits_end) {
        return str;
    }

    // append remaining digits, zero-padded to 9 decimal places
    for (const uint32_t* d = b->start - 1; d >= b->digits_end; d--) {
        size_t len_d = snprintf(NULL, 0, "%09lu", (unsigned long) *d);
        str = realloc(str, (len+len_d+1) * sizeof(char));
        snprintf(str + len, len_d+1, "%09lu", (unsigned long) *d);
        len += len_d;
    }

    return str;
}

// print all digits in memory order (debug print)
void bn_dump(const Bignum* b) {
    fputs("Bignum<num=", stdout);
    bn_print(b);
    printf(", len=%lu, rlen=%lu>{\n", b->len, bn_rlen(b));
    for (uint64_t i = 0; i < b->len; i++) {
        printf("      [%09llu]\n", (unsigned long long) b->digits_end[i]);
    }
    fputs("}\n", stdout);
}

// comparison

int bn_cmp(const Bignum* a0, const Bignum* a1) {

    if (bn_is_zero(a0) && bn_is_zero(a1)) {
        return 0;
    }

    if (a0->sign && !a1->sign) {
        return -1; // a0 < 0 < a1
    } else if (!a0->sign && a1->sign) {
        return 1; // a1 < 0 < a0
    }

    // a0, a1 have same sign
    int sign = a0->sign ? -1 : 1;

    // compare lengths, accounting for where start is not just b->len
    uint64_t len0 = bn_rlen(a0);
    uint64_t len1 = bn_rlen(a1);
    if (len0 > len1) {
        return sign * 1;
    } else if (len0 < len1) {
        return sign * -1;
    }

    // compare digitwise MSD -> LSD
    for (uint32_t* d0 = a0->start, *d1 = a1->start; d0 >= a0->digits_end && d1 >= a1->digits_end; d0--, d1--) {
        if (*d0 > *d1) {
            return sign * 1;
        } else if (*d0 < *d1) {
            return sign * -1;
        }
    }

    // all digits are equal
    return 0;
}

bool bn_is_zero(const Bignum* a0) {
    return (*a0->start == 0);
}

// operations

void bn_neg(Bignum* result, const Bignum* a0) {

    Bignum arg0 = *a0;

    // -0 = 0
    if (bn_is_zero(&arg0)) {
        bn_icopy(result, &arg0);
        return;
    }

    // -a0
    bn_ineg(result, &arg0);
}

void bn_add(Bignum* result, const Bignum* a0, const Bignum* a1) {

    Bignum arg0 = *a0;
    Bignum arg1 = *a1;

    // 0 + a1 = a1
    if (bn_is_zero(&arg0)) {
        bn_icopy(result, &arg1);
        return;
    }

    // a0 + 0 = a0
    if (bn_is_zero(&arg1)) {
        bn_icopy(result, &arg0);
        return;
    }

    // (-a0) + a1 => a1 - a0
    if (arg0.sign && !arg1.sign) {
        arg0.sign = 0;
        bn_isub(result, &arg1, &arg0);
        return;
    }

    // a0 + (-a1) => a0 - a1
    if (!arg0.sign && arg1.sign) {
        arg1.sign = 0;
        bn_isub(result, &arg0, &arg1);
        return;
    }

    // (-a0) + (-a1) => -(a0 + a1)
    if (arg0.sign && arg1.sign) {
        arg0.sign = 0;
        arg1.sign = 0;
        bn_iadd(result, &arg1, &arg0);
        result->sign = !result->sign;
        return;
    }

    // a0 + a1
    bn_iadd(result, &arg0, &arg1);
}

void bn_sub(Bignum* result, const Bignum* a0, const Bignum* a1) {
    Bignum arg0 = *a0;
    Bignum arg1 = *a1;

    // 0 - a1 = -a1
    if (bn_is_zero(&arg0)) {
        bn_icopy(result, &arg1);
        result->sign = !result->sign;
        return;
    }

    // a0 - 0 = a0
    if (bn_is_zero(&arg1)) {
        bn_icopy(result, &arg0);
        return;
    }

    int cmp = bn_cmp(&arg0, &arg1);

    // a0 - a0 = 0
    if (cmp == 0) {
        bn_iwrite_parts(result, 0, 0);
        return;
    }

    // (-a0) - a1 => -(a0 + a1)
    if (arg0.sign && !arg1.sign) {
        arg0.sign = 0;
        bn_iadd(result, &arg0, &arg1);
        result->sign = !result->sign;
        return;
    }

    // a0 - (-a1) => a0 + a1
    if (!arg0.sign && arg1.sign) {
        arg1.sign = 0;
        bn_iadd(result, &arg0, &arg1);
        return;
    }

    // (-a0) - (-a1) => a1 - a0
    if (arg0.sign && arg1.sign) {
        arg0.sign = 0;
        arg1.sign = 0;
        bn_isub(result, &arg1, &arg0);
        return;
    }

    // a0 - a1
    bn_isub(result, &arg0, &arg1);
}

void bn_mul(Bignum* result, const Bignum* a0, const Bignum* a1) {

    Bignum arg0 = *a0;
    Bignum arg1 = *a1;

    if (bn_is_zero(&arg0) || bn_is_zero(&arg1)) {
        bn_iwrite_parts(result, 0, 0);
        return;
    }

    // (-a0) * a1 = -(a0 * a1)
    if (arg0.sign && !arg1.sign) {
        arg0.sign = 0;
        bn_imul(result, &arg0, &arg1);
        result->sign = !result->sign;
        return;
    }

    // a0 * (-a1) = -(a0 * a1)
    if (!arg0.sign && arg1.sign) {
        arg1.sign = 0;
        bn_imul(result, &arg0, &arg1);
        result->sign = !result->sign;
        return;
    }

    // (-a0) * (-a1) = a0 * a1
    if (arg0.sign && arg1.sign) {
        arg0.sign = 0;
        arg1.sign = 0;
        bn_imul(result, &arg0, &arg1);
        return;
    }

    // a0 * a1
    bn_imul(result, a0, a1);
}

// internal functions assume certain preconditions that they do not check
// for example bn_isub(out, a0, a1) assumes a0 > a1 > 0
// they do free previous allocation

// frees previous and reallocates space for n_digits
// sets out->start
void bn_ialloc(Bignum* out, uint64_t n_digits) {
    bn_ifree(out);
    *out = (Bignum){
        .digits_end = BN_MALLOC_FN(n_digits * sizeof(uint32_t)),
        .len = n_digits
    };
    memset(out->digits_end, 0, n_digits * sizeof(uint32_t));
    out->start = out->digits_end;
}

// create a deep copy of src in dest
void bn_icopy(Bignum* dest, const Bignum* src) {
    Bignum result = {
        .digits_end = BN_MALLOC_FN(src->len * sizeof(uint32_t)),
        .len = src->len,
        .sign = src->sign
    };
    memcpy(result.digits_end, src->digits_end, src->len * sizeof(uint32_t));

    bn_ifree(dest);
    *dest = result;
    bn_inormalize(dest);
}

// frees and zeroes out a single bignum
void bn_ifree(Bignum* out) {
#ifndef BN_NOFREE
    BN_FREE_FN(out->digits_end);
#endif
    *out = (Bignum){0};
}

// sets out->start
// removes leading zeroes
void bn_inormalize(Bignum* out) {

    out->start = out->digits_end + out->len - 1;

    // strip leading zeroes
    while (out->start >= out->digits_end && *out->start == 0) {
        out->start--;
    }
}

// parse str as a base 10 positive integer and put it in out, or return false for parse error
// assumes nothing
bool bn_iwrite_str(Bignum* out, const char* str) {

    if(str == NULL) {
        // null string
        return false;
    }

    int len = strlen(str);
    if (len == 0) {
        // empty string
        return false;
    }

    // handle optional negative sign
    bool is_negative = false;
    if (str[0] == '-') {
        is_negative = true;
        str += 1;
        len -= 1;

        // "-" is not a number
        if (len == 0) {
            return false;
        }
    }

    // must be all base 10 digits
    for (const char* s = str; *s != '\0'; s++) {
        if (!isdigit(*s)) {
            return false;
        }
    }

    // strip leading zeroes and check if it's all 0s
    while (len > 0 && str[0] == '0') {
        str++;
        len--;
    }

    if (len == 0) {
        // number was all "00000"
        bn_iwrite_parts(out, 0, 0);
        return true;
    }

    // break the string into 1 or more chunks
    // a chunk can be up to 9 decimal digits
    int first_chunk_len = len % 9;
    int number_of_extra_chunks = len / 9;
    if (first_chunk_len == 0) {
        first_chunk_len = 9;
        number_of_extra_chunks -= 1;
    }

    size_t l = number_of_extra_chunks + 1;
    Bignum result = {
        .digits_end = BN_MALLOC_FN(l * sizeof(uint32_t)),
        .len = l,
        .sign = (is_negative) ? 1 : 0
    };
    memset(result.digits_end, 0, l * sizeof(uint32_t));

    // [-0, 0] => 0
    if (!strncmp(str, "0", 1)) {
        result.sign = 0;

        bn_ifree(out);
        *out = result;
        return true;
    }

    // first chunk - can be 1-9 characters/decimal digits long
    uint32_t first_digit_value;
    if (!bn_str_to_u32(str, 0, first_chunk_len, &first_digit_value)) {
        // first digit conversion failed
        bn_ifree(&result);
        return false;
    }
    result.digits_end[result.len - 1] = first_digit_value;

    // remaining chunks
    const char* next_chunk = str + first_chunk_len;
    int next_digit_index = result.len - 2;
    while (next_digit_index >= 0) {

        // nth chunk - always 9 digits long
        uint32_t next_digit_value;
        if (!bn_str_to_u32(next_chunk, 0, 9, &next_digit_value)) {
            // nth digit conversion failed
            bn_ifree(&result);
            return false;
        }
        result.digits_end[next_digit_index] = next_digit_value;

        next_chunk += 9;
        next_digit_index -= 1;
    }
    result.start = result.digits_end + result.len - 1;

    bn_ifree(out);
    *out = result;
    return true;
}

// writes a single digit to out, never fails
// assumes nothing
void bn_iwrite_parts(Bignum* out, uint32_t sign, uint32_t value) {
    bool overflow = (value > BN_DIGIT_MAX);

    bn_ialloc(out, overflow ? 2 : 1);

    if (overflow) {
        out->digits_end[1] = value / BN_BASE;
        out->digits_end[0] = value % BN_BASE;
    } else {
        out->digits_end[0] = value;
    }

    out->start = out->digits_end;
    out->sign = sign;
    bn_inormalize(out);
}

// out = a0 << n (in base BN_BASE)
// assumes n > 0
// preserves sign of a0
void bn_ilshift(Bignum* out, const Bignum* a0, uint64_t n) {
    if (bn_is_zero(a0)) {
        bn_icopy(out, a0);
        return;
    }

    uint64_t len = a0->start - a0->digits_end + 1;

    Bignum result = {0};
    bn_ialloc(&result, len + n);

    uint32_t* dest = result.digits_end + n;
    uint32_t* src = a0->digits_end;

    for (uint64_t i = 0; i < len; i++) {
        dest[i] = src[i];
    }

    for (uint64_t i = 0; i < n; i++) {
        result.digits_end[i] = 0;
    }

    result.start = result.digits_end + len + n - 1;

    bn_ifree(out);
    *out = result;
    bn_inormalize(out);
}


// out = a0 >> n (in base BN_BASE)
// assumes n > 0
// preserves sign of a0
void bn_irshift(Bignum* out, const Bignum* a0, uint64_t n) {
    if (bn_is_zero(a0)) {
        bn_icopy(out, a0);
        return;
    }

    uint64_t len0 = a0->start - a0->digits_end + 1;

    if (n >= len0) {
        bn_iwrite_str(out, "0");
        return;
    }

    uint64_t new_len = len0 - n;

    Bignum result = {0};
    bn_ialloc(&result, new_len);

    uint32_t* dest = result.digits_end;
    uint32_t* src = a0->digits_end + n;

    for (uint64_t i = 0; i < new_len; i++) {
        dest[i] = src[i];
    }

    result.start = result.digits_end + new_len - 1;
    result.sign = a0->sign;

    bn_ifree(out);
    *out = result;
    bn_inormalize(out);
}

// out = -a0
// assumes a0 != 0
void bn_ineg(Bignum* out, const Bignum* a0) {

    Bignum result = {0};
    bn_icopy(&result, a0);
    result.sign = !result.sign;

    *out = result;
    bn_inormalize(out);
}

// out = a0 + a1
// assumes a0, a1 > 0
void bn_iadd(Bignum* out, const Bignum* a0, const Bignum* a1) {
    // carry algorithm

    // swap the numbers such that a0->len >= a1->len
    // this makes computation simpler in the loop
    // also guarantees `max(len0, len1) == len0`
    if (bn_rlen(a0) < bn_rlen(a1)) {
        const Bignum* temp = a0;
        a0 = a1;
        a1 = temp;
    }

    Bignum result = {0};
    size_t max_len = 1 + bn_rlen(a0); // +1 for possible carry
    bn_ialloc(&result, max_len);

    // LSD -> MSD
    uint32_t* d0 = a0->digits_end;
    uint32_t* d1 = a1->digits_end;
    uint32_t carry = 0;
    int i = 0;
    while (d0 <= a0->start) {
        uint32_t digit0 = *d0;
        uint32_t digit1 = (d1 <= a1->start) ? *d1 : 0;
        uint32_t sum = digit0 + digit1 + carry;

        carry = sum / BN_BASE;
        result.digits_end[i] = sum % BN_BASE;

        d0 += 1;
        d1 += 1;
        i += 1;
    }

    result.start = result.digits_end + i - 1;

    if (carry != 0) {
        result.start += 1;
        result.digits_end[i] = carry;
    }

    bn_ifree(out);
    *out = result;
    bn_inormalize(out);
}

// out = a0 - a1
// assumes a0, a1 > 0
void bn_isub(Bignum* out, const Bignum* a0, const Bignum* a1) {
    // borrow algorithm

    // a0 < a1 => -(a1 - a0)
    if (bn_cmp(a0, a1) == -1) {
        bn_isub(out, a1, a0);
        out->sign = !out->sign;
        return;
    }

    Bignum result = {0};
    bn_ialloc(&result, a0->len);

    // assume a0 > a1

    // LSD -> MSD
    uint32_t* d0 = a0->digits_end;
    uint32_t* d1 = a1->digits_end;
    uint32_t borrow = 0;
    uint64_t i = 0;
    while (d0 <= a0->start && i < result.len) {

        uint32_t digit0 = *d0;
        uint32_t digit1 = (d1 <= a1->start) ? *d1 : 0;

        int64_t diff = digit0;

        if (diff < borrow) {
            diff += BN_BASE;
            diff -= borrow;
            borrow = 1;
        } else {
            diff -= borrow;
            borrow = 0;
        }

        if (diff < digit1) {
            diff += BN_BASE;
            diff -= digit1;
            borrow = 1;
        } else {
            diff -= digit1;
        }

        result.digits_end[i] = diff;

        d0 += 1;
        d1 += 1;
        i += 1;
    }

    bn_ifree(out);
    *out = result;
    bn_inormalize(out);
}

// out = a0 * a1
// assumes a0, a1 > 0
void bn_imul(Bignum* out, const Bignum* a0, const Bignum* a1) {
    Bignum result = {0};
    bn_ialloc(&result, bn_rlen(a0) + 1 + bn_rlen(a1) + 1);

    uint32_t* r_ptr = result.digits_end;

    for (uint32_t* d0 = a0->digits_end; d0 <= a0->start; d0++) {
        uint64_t carry = 0;
        uint32_t* res_ptr = r_ptr;

        for (uint32_t* d1 = a1->digits_end; d1 <= a1->start; d1++) {
            uint64_t product = (uint64_t)(*d0) * (*d1) + *res_ptr + carry;
            *res_ptr = product % BN_BASE;
            carry = product / BN_BASE;
            res_ptr++;
        }

        if (carry > 0) {
            *res_ptr += carry;
        }
        r_ptr++;
    }

    bn_ifree(out);
    *out = result;
    bn_inormalize(out);
}


// utils

int bn_min(int x, int y) {
    return (x < y) ? x : y;
}

int bn_max(int x, int y) {
    return (x > y) ? x : y;
}

// false => conversion error
// out = u32(str[start:end])
bool bn_str_to_u32(const char* str, uint32_t start, uint32_t end, uint32_t* out) {

    if (end - start > 9) {
        return false;
    }

    char* b = bn_strndup(str + start, end - start);

    char* b_end = NULL;
    unsigned long ul = strtoul(b, &b_end, 10);
#ifndef BN_NOFREE
    BN_FREE_FN(b);
#endif

    if (b_end == NULL) {
        return false;
    }

    *out = (uint32_t)ul;
    return true;
}

char* bn_strndup(const char* str, int n) {
    char* dup = BN_MALLOC_FN((n + 1) * sizeof(char));
    strncpy(dup, str, n);
    return dup;
}

