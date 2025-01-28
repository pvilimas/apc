#include "bignum.h"

// bn_digits[B] is the highest digit available in base B (B > 1)
const char* bn_digits = "00123456789abcdefghijklmnopqrstuvwxyz";
const char* BN_DIGITS = "00123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// value^max_power == max_digit_value < UINT32_MAX
BignumBase bn_bases[BN_BASE_MAX + 1] = {
    [2]  = {.value = 2, .max_digit_value = 2147483648, .max_power = 31},
    [3]  = {         3,                    3486784401,              20},
    [4]  = {         4,                    1073741824,              15},
    [5]  = {         5,                    1220703125,              13},
    [6]  = {         6,                    2176782336,              12},
    [7]  = {         7,                    1977326743,              11},
    [8]  = {         8,                    1073741824,              10},
    [9]  = {         9,                    3486784401,              10},
    [10] = {         10,                   1000000000,              9},
    [11] = {         11,                   2357947691,              9},
    [12] = {         12,                   429981696,               8},
    [13] = {         13,                   815730721,               8},
    [14] = {         14,                   1475789056,              8},
    [15] = {         15,                   2562890625,              8},
    [16] = {         16,                   268435456,               7},
    [17] = {         17,                   410338673,               7},
    [18] = {         18,                   612220032,               7},
    [19] = {         19,                   893871739,               7},
    [20] = {         20,                   1280000000,              7},
    [21] = {         21,                   1801088541,              7},
    [22] = {         22,                   2494357888,              7},
    [23] = {         23,                   3404825447,              7},
    [24] = {         24,                   191102976,               6},
    [25] = {         25,                   244140625,               6},
    [26] = {         26,                   308915776,               6},
    [27] = {         27,                   387420489,               6},
    [28] = {         28,                   481890304,               6},
    [29] = {         29,                   594823321,               6},
    [30] = {         30,                   729000000,               6},
    [31] = {         31,                   887503681,               6},
    [32] = {         32,                   1073741824,              6},
    [33] = {         33,                   1291467969,              6},
    [34] = {         34,                   1544804416,              6},
    [35] = {         35,                   1838265625,              6},
    [36] = {         36,                   2176782336,              6}
};

bool bn_write(Bignum* b, const char* str, uint32_t base) {
    bool ok = bn_iwrite_str(b, str, base);
    if (ok) {
        bn_inormalize(b);
    }
    return ok;
}

bool bn_write2(Bignum* b, const char* str, size_t len, uint32_t base) {
    char* s2 = strndup(str, len);
    bool ok = bn_write(b, s2, base);
#ifndef BN_NOFREE
    BN_FREE(s2);
#endif
    return ok;
}

void bn_copy(Bignum* dest, const Bignum* src) {
    bn_icopy(dest, src);
}

void bn_convert_base(Bignum* dest, const Bignum* src, uint32_t new_base) {
    if (src->base == new_base) {
        bn_icopy(dest, src);
    } else {
        bn_iconv(dest, src, new_base);
    }
}

uint32_t bn_print(const Bignum* b, bool print_base) {
    if (b->digits_end == NULL) {
        fputs("(null)", stdout);
        return 6;
    }

    if (bn_is_zero(b)) {
        fputs("0", stdout);
        return 1;
    }

    uint32_t nc = 0;

    if (b->sign) {
        fputs("-", stdout);
        nc += 1;
    }

    // print leading digit without leading 0s
    nc += bn_iprint_digit(b->digits_end[b->msd_pos], b->base, false);

    // does it only have 1 digit?
    if (b->msd_pos == 0) {
        if (print_base) {
            nc += printf("_%u", b->base);
        }
        return nc;
    }

    // print remaining digits with leading 0s
    for (uint64_t i = b->msd_pos-1; &b->digits_end[i] >= b->digits_end; i--) {
        nc += bn_iprint_digit(b->digits_end[i], b->base, true);
    }

    if (print_base) {
        nc += printf("_%u", b->base);
    }

    return nc;
}

void bn_dump(const Bignum* b) {
    if (b->digits_end == NULL) {
        printf("Bignum{\n"
            "  .base=%lu,\n"
            "  .sign=%lu,\n"
            "  .cap=%llu,\n"
            "  rlen=%lu,\n"
            "  .digits=[]}\n",
        (unsigned long)b->base,
        (unsigned long)b->sign,
        (unsigned long long)b->cap,
        (unsigned long)BN_REAL_LEN(b));
    } else {

        printf("Bignum{\n"
            "  .base=%lu,\n"
            "  .sign=%lu,\n"
            "  .cap=%llu,\n"
            "  rlen=%lu,\n"
            "  .digits=[\n"
            "    <start>\n",
        (unsigned long)b->base,
        (unsigned long)b->sign,
        (unsigned long long)b->cap,
        (unsigned long)BN_REAL_LEN(b));

        for (uint64_t i = 0; i <= b->cap; i++) {
            printf(
            "    %0*lu%s,\n",
            bn_bases[b->base].max_power,
            (unsigned long) b->digits_end[i],
            i == b->msd_pos ? " <-- MSD" : "");
        }

        printf(
            "    <end>\n"
            "  ]}\n");
    }
}

// comparison

int32_t bn_cmp(const Bignum* a0, const Bignum* a1) {

    if (bn_is_zero(a0) && bn_is_zero(a1)) {
        return 0;
    }

    if (a0->sign && !a1->sign) {
        return -1; // a0 < 0 < a1
    } else if (!a0->sign && a1->sign) {
        return 1; // a1 < 0 < a0
    }

    // a0, a1 have same sign
    int32_t sign = a0->sign ? -1 : 1;

    // compare real lengths, accounting for where start is not just capacity
    uint64_t len0 = BN_REAL_LEN(a0);
    uint64_t len1 = BN_REAL_LEN(a1);
    if (len0 > len1) {
        return sign * 1;
    } else if (len0 < len1) {
        return sign * -1;
    }

    // compare digitwise MSD -> LSD
    for (uint64_t i = a0->msd_pos; &a0->digits_end[i] >= a0->digits_end; i--) {
        if (a0->digits_end[i] > a1->digits_end[i]) {
            return sign * 1;
        } else if (a0->digits_end[i] < a1->digits_end[i]) {
            return sign * -1;
        }
    }

    // all digits are equal
    return 0;
}

bool bn_is_zero(const Bignum* a0) {
    return (a0->digits_end[a0->msd_pos] == 0);
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

void bn_ialloc(Bignum* out, uint64_t n_digits, uint32_t base) {
    bn_ifree(out);
    *out = (Bignum){
        .digits_end = BN_MALLOC(n_digits * sizeof(uint32_t)),
        .cap = n_digits
    };
    memset(out->digits_end, 0, n_digits * sizeof(uint32_t));
    out->msd_pos = 0;
    out->base = base;
}

void bn_icopy(Bignum* dest, const Bignum* src) {
    Bignum result = {
        .base = src->base,
        .digits_end = BN_MALLOC(src->cap * sizeof(uint32_t)),
        .cap = src->cap,
        .sign = src->sign,
    };
    memcpy(result.digits_end, src->digits_end, src->cap * sizeof(uint32_t));

    bn_ifree(dest);
    *dest = result;
    bn_inormalize(dest);
}

// assume new_base in [2,36], new_base != src->base
void bn_iconv(Bignum* dest, const Bignum* src, uint64_t new_base) {
    Bignum result;

    // bn_ifree(dest);
    // *dest = result;
    // bn_inormalize(dest);
}

void bn_ifree(Bignum* out) {
#ifndef BN_NOFREE
    BN_FREE(out->digits_end);
#endif
    *out = (Bignum){0};
}

void bn_inormalize(Bignum* out) {

    out->msd_pos = out->cap - 1;

    // strip leading zeroes
    while (out->digits_end + out->msd_pos >= out->digits_end &&
    out->digits_end[out->msd_pos] == 0) {
        out->msd_pos--;
    }
}

bool bn_iwrite_str(Bignum* out, const char* str, uint32_t base) {

    if (base < BN_BASE_MIN || base > BN_BASE_MAX) {
        return false;
    }

    if(str == NULL) {
        // null string
        return false;
    }

    uint32_t len = strlen(str);
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

    // must be all valid digits in the specified base
    for (const char* s = str; *s != '\0'; s++) {
        if (!digit_is_valid_in_base(*s, base, NULL)) {
            return false;
        }
    }

    // [-0, 0] => 0
    // if (!strncmp(str, "0", 1)) {
    //
    //     Bignum result = {
    //         .base = BN_BASE_DEFAULT,
    //         .digits_end = BN_MALLOC(1 * sizeof(uint32_t)),
    //         .len = 1,
    //         .sign = 0
    //     };
    //     result.digits_end[0] = 0;
    //     result.start = result.digits_end;
    //
    //     bn_ifree(out);
    //     *out = result;
    //     return true;
    // }

    // how many leading zeroes are needed to round off a full digit?

    uint32_t mp = bn_bases[base].max_power;

    // avoid wasting space - add 0 bytes if len already rounded off
    uint32_t n_leading_zeroes = (len % mp == 0) ? 0 : (mp - (len % mp));
    uint32_t new_len = len + n_leading_zeroes;

    // if needed, copy the string, padding with leading zeroes

    char* new_str;

    if (n_leading_zeroes == 0) {
        new_str = bn_strndup(str, len);
    } else {
        new_str = BN_MALLOC((new_len + 1) * sizeof(char));
        memset(new_str, 0, new_len + 1);
        memset(new_str, '0', n_leading_zeroes);

        // copy all the "real" digits from the old string
        memcpy(new_str + n_leading_zeroes, str, len);
    }

    // parse the number, mp characters at a time

    // guaranteed: new_len % mp == 0 -- very important
    uint32_t num_digits = new_len / mp;

    Bignum result = {
        .base = base,
        .digits_end = BN_MALLOC(num_digits * sizeof(uint32_t)),
        .cap = num_digits,
        .sign = (is_negative) ? 1 : 0,
    };
    memset(result.digits_end, 0, num_digits * sizeof(uint32_t));

    // loop in reverse order
    // mp characters => 1 digit

    uint32_t* dp = &result.digits_end[result.cap - 1];
    char* sp = new_str;

    while (sp < new_str + len) {

        if (!bn_str_to_u32(sp, 0, mp, base, dp)) {
            // nth digit conversion failed
            bn_ifree(&result);
            return false;
        }

        sp += mp;
        dp -= 1;
    }

#ifndef BN_NOFREE
    BN_FREE(new_str);
#endif

    bn_ifree(out);
    *out = result;
    bn_inormalize(out);
    return true;
}

void bn_iwrite_parts(Bignum* out, uint32_t signbit, uint32_t value) {
    bool is_overflow = (value > bn_bases[BN_BASE_DEFAULT].max_digit_value);

    bn_ialloc(out, is_overflow ? 2 : 1, BN_BASE_DEFAULT);

    if (is_overflow) {
        out->digits_end[1] = value / out->base;
        out->digits_end[0] = value % out->base;
    } else {
        out->digits_end[0] = value;
    }

    out->msd_pos = 0;
    out->sign = signbit;
    bn_inormalize(out);
}

uint32_t bn_iprint_digit(uint32_t digit, uint32_t base, bool leading_0s) {
    uint32_t d = digit;
    uint32_t n = 0;
    while (d > 0) {
        d /= bn_bases[base].value;
        n += 1;
    }
    uint32_t n_digits = bn_bases[base].max_power - n;

    uint32_t nc = 0;

    if (leading_0s) {
        for (uint32_t i = 0; i < n_digits; i++) {
            printf("0");
            nc += 1;
        }
    }

    if (digit / bn_bases[base].value != 0) {
        nc += bn_iprint_digit(digit / bn_bases[base].value, base, false);
    }

    // +1 because bn_bases is zero indexed
    printf("%c", bn_digits[digit % bn_bases[base].value + 1]);
    nc += 1;

    return nc;
}

void bn_ilshift(Bignum* out, const Bignum* a0, uint64_t n) {
    if (bn_is_zero(a0)) {
        bn_icopy(out, a0);
        return;
    }

    uint64_t rlen0 = BN_REAL_LEN(a0);

    uint64_t new_len = rlen0 + n;

    Bignum result = {0};
    bn_ialloc(&result, new_len, a0->base);

    uint32_t* dest = result.digits_end + n;
    uint32_t* src = a0->digits_end;

    for (uint64_t i = 0; i < rlen0; i++) {
        dest[i] = src[i];
    }

    for (uint64_t i = 0; i < n; i++) {
        result.digits_end[i] = 0;
    }

    result.msd_pos = new_len - 1;
    result.sign = a0->sign;

    bn_ifree(out);
    *out = result;
    bn_inormalize(out);
}

void bn_irshift(Bignum* out, const Bignum* a0, uint64_t n) {
    if (bn_is_zero(a0)) {
        bn_icopy(out, a0);
        return;
    }

    uint64_t rlen0 = BN_REAL_LEN(a0);

    if (n >= rlen0) {
        bn_iwrite_parts(out, 0, 0);
        return;
    }

    uint64_t new_len = rlen0 - n;

    Bignum result = {0};
    bn_ialloc(&result, new_len, a0->base);

    uint32_t* dest = result.digits_end;
    uint32_t* src = a0->digits_end + n;

    for (uint64_t i = 0; i < new_len; i++) {
        dest[i] = src[i];
    }

    result.msd_pos = new_len - 1;
    result.sign = a0->sign;

    bn_ifree(out);
    *out = result;
    bn_inormalize(out);
}

void bn_ineg(Bignum* out, const Bignum* a0) {

    Bignum result = {0};
    bn_icopy(&result, a0);
    result.sign = !result.sign;

    *out = result;
    bn_inormalize(out);
}

void bn_iadd(Bignum* out, const Bignum* a0, const Bignum* a1) {
    // carry algorithm

    if (a0->base != a1->base) {
        printf("bases do not match!!\n");
        return;
    }

    // swap the numbers such that a0->len >= a1->len
    // this makes computation simpler in the loop
    // also guarantees `max(len0, len1) == len0`
    if (BN_REAL_LEN(a0) < BN_REAL_LEN(a1)) {
        const Bignum* temp = a0;
        a0 = a1;
        a1 = temp;
    }

    Bignum result = {0};
    size_t max_len = 1 + BN_REAL_LEN(a0); // +1 for possible carry
    bn_ialloc(&result, max_len, a0->base);

    result.base = a0->base;
    uint32_t mdv = bn_bases[result.base].max_digit_value;
    uint32_t real_base = mdv + 1;

    // LSD -> MSD
    uint32_t* d0 = a0->digits_end;
    uint32_t* d1 = a1->digits_end;
    uint32_t carry = 0;

    int i = 0;
    while (d0 <= &a0->digits_end[a0->msd_pos]) {
        uint32_t digit0 = *d0;
        uint32_t digit1 = (d1 <= &a1->digits_end[a1->msd_pos]) ? *d1 : 0;
        uint32_t sum = digit0 + digit1 + carry;

        carry = sum / real_base;
        result.digits_end[i] = sum % real_base;

        d0 += 1;
        d1 += 1;
        i += 1;
    }

    result.sign = 0;
    result.msd_pos = i - 1;

    if (carry != 0) {
        result.msd_pos += 1;
        result.digits_end[i] = carry;
    }

    bn_ifree(out);
    *out = result;
    bn_inormalize(out);
}

void bn_isub(Bignum* out, const Bignum* a0, const Bignum* a1) {
    // borrow algorithm

    if (a0->base != a1->base) {
        printf("bases do not match!!\n");
        return;
    }

    // a0 < a1 => -(a1 - a0)
    if (bn_cmp(a0, a1) == -1) {
        bn_isub(out, a1, a0);
        out->sign = !out->sign;
        return;
    }

    // now assume a0 > a1

    Bignum result = {0};
    bn_ialloc(&result, a0->cap, a0->base);

    uint32_t mdv = bn_bases[result.base].max_digit_value;
    uint32_t real_base = mdv + 1;

    // LSD -> MSD
    uint32_t* d0 = a0->digits_end;
    uint32_t* d1 = a1->digits_end;
    uint32_t borrow = 0;
    uint64_t i = 0;
    while (d0 <= &a0->digits_end[a0->msd_pos] && i < result.cap) {

        uint32_t digit0 = *d0;
        uint32_t digit1 = (d1 <= &a1->digits_end[a1->msd_pos]) ? *d1 : 0;

        int64_t diff = digit0;

        if (diff < borrow) {
            diff += real_base;
            diff -= borrow;
            borrow = 1;
        } else {
            diff -= borrow;
            borrow = 0;
        }

        if (diff < digit1) {
            diff += real_base;
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

void bn_imul(Bignum* out, const Bignum* a0, const Bignum* a1) {
    // naive algorithm - optimize later

    if (a0->base != a1->base) {
        printf("bases do not match!!\n");
        return;
    }

    Bignum result = {0};
    bn_ialloc(&result, BN_REAL_LEN(a0) + 1 + BN_REAL_LEN(a1) + 1, a0->base);

    result.base = a0->base;
    uint32_t mdv = bn_bases[result.base].max_digit_value;
    uint32_t real_base = mdv + 1;

    uint32_t* r_ptr = result.digits_end;

    for (uint32_t* d0 = a0->digits_end; d0 <= &a0->digits_end[a0->msd_pos]; d0++)
    {
        uint64_t carry = 0;
        uint32_t* res_ptr = r_ptr;

        for (uint32_t* d1 = a1->digits_end; d1 <= &a1->digits_end[a1->msd_pos]; d1++)
        {
            uint64_t product = (uint64_t)(*d0) * (*d1) + *res_ptr + carry;
            *res_ptr = product % real_base;
            carry = product / real_base;
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

void bn_iintdiv_u32(Bignum* out, const Bignum* a0, uint32_t a1) {

}

// utils

uint64_t bn_min(uint64_t x, uint64_t y) {
    return (x < y) ? x : y;
}

uint64_t bn_max(uint64_t x, uint64_t y) {
    return (x > y) ? x : y;
}

// false => conversion error
// out = u32(str[start:end], base)
bool bn_str_to_u32(const char* str, uint32_t start, uint32_t end, uint32_t base, uint32_t* out) {

    if (end - start > bn_bases[base].max_power) {
        return false;
    }

    if (base < BN_BASE_MIN || base > BN_BASE_MAX) {
        return false;
    }

    char* b = bn_strndup(str + start, end - start);

    char* b_end = NULL;
    unsigned long ul = strtoul(b, &b_end, base);
#ifndef BN_NOFREE
    BN_FREE(b);
#endif

    if (b_end == NULL) {
        return false;
    }

    *out = (uint32_t)ul;
    return true;
}

char* bn_strndup(const char* str, int n) {
    char* dup = BN_MALLOC((n + 1) * sizeof(char));
    strncpy(dup, str, n);
    dup[n] = '\0';
    return dup;
}

bool digit_is_valid_in_base(char digit, uint32_t base, uint32_t* value_out) {
    if (base < BN_BASE_MIN || base > BN_BASE_MAX) {
        return false;
    }

    if (!isalnum(digit)) {
        return false;
    }

    for (uint32_t i = 1; i <= base; i++) {
        if (digit == bn_digits[i]) {
            if (value_out != NULL) {
                *value_out = i;
            }
            return true;
        }
    }
    return false;
}
