#include "bignum.h"

// definition: fake_base^width = real_base < UINT32_MAX < fake_base^(width+1)
const BignumBase BN_BASE[BN_BASE_MAX + 1] = {
    [0]  =                                              {.last_digit="00" },
    [1]  =                                              {.last_digit="00" },
    [2]  = {.fake_base=2,.width=31,.real_base=2147483648,.last_digit="11" },
    [3]  = {           3,       20,           3486784401,            "22" },
    [4]  = {           4,       15,           1073741824,            "33" },
    [5]  = {           5,       13,           1220703125,            "44" },
    [6]  = {           6,       12,           2176782336,            "55" },
    [7]  = {           7,       11,           1977326743,            "66" },
    [8]  = {           8,       10,           1073741824,            "77" },
    [9]  = {           9,       10,           3486784401,            "88" },
    [10] = {           10,      9,            1000000000,            "99" },
    [11] = {           11,      9,            2357947691,            "aA" },
    [12] = {           12,      8,            429981696,             "bB" },
    [13] = {           13,      8,            815730721,             "cC" },
    [14] = {           14,      8,            1475789056,            "dD" },
    [15] = {           15,      8,            2562890625,            "eE" },
    [16] = {           16,      7,            268435456,             "fF" },
    [17] = {           17,      7,            410338673,             "gG" },
    [18] = {           18,      7,            612220032,             "hH" },
    [19] = {           19,      7,            893871739,             "iI" },
    [20] = {           20,      7,            1280000000,            "jJ" },
    [21] = {           21,      7,            1801088541,            "kK" },
    [22] = {           22,      7,            2494357888,            "lL" },
    [23] = {           23,      7,            3404825447,            "mM" },
    [24] = {           24,      6,            191102976,             "nN" },
    [25] = {           25,      6,            244140625,             "oO" },
    [26] = {           26,      6,            308915776,             "pP" },
    [27] = {           27,      6,            387420489,             "qQ" },
    [28] = {           28,      6,            481890304,             "rR" },
    [29] = {           29,      6,            594823321,             "sS" },
    [30] = {           30,      6,            729000000,             "tT" },
    [31] = {           31,      6,            887503681,             "uU" },
    [32] = {           32,      6,            1073741824,            "vV" },
    [33] = {           33,      6,            1291467969,            "wW" },
    [34] = {           34,      6,            1544804416,            "xX" },
    [35] = {           35,      6,            1838265625,            "yY" },
    [36] = {           36,      6,            2176782336,            "zZ" },
};

const Bignum BN_ZERO = {
    .base = BN_BASE_DEFAULT,
    .signbit = 0,
    .digits_end = (bn_digit_t[1]){0},
    .msd_pos = 0,
    .capacity = 1
};

const Bignum BN_ONE = {
    .base = BN_BASE_DEFAULT,
    .signbit = 0,
    .digits_end = (bn_digit_t[1]){1},
    .msd_pos = 0,
    .capacity = 1
};

bool bn_write(Bignum* b, const char* str) {
    return bni_write_str(b, str, strlen(str), BN_BASE_DEFAULT);
}

bool bn_write2(Bignum* b, const char* str, uint8_t base) {
    return bni_write_str(b, str, strlen(str), base);
}

bool bn_write3(Bignum* b, const char* str, size_t len, uint8_t base) {
    return bni_write_str(b, str, len, base);
}

void bn_copy(Bignum* dest, const Bignum* src) {
    bni_copy(dest, src);
}

bool bn_convert(Bignum* dest, const Bignum* src, uint8_t new_base) {
    if (!bnu_base_valid(new_base)) {
        return false;
    }

    if (src->base == new_base) {
        bni_copy(dest, src);
    } else {
        bni_convert(dest, src, new_base);
    }

    return true;
}

size_t bn_print(const Bignum* b, bool print_base, bool use_uppercase) {
    if (b->digits_end == NULL || b->capacity == 0) {
        fputs("(null)", stdout);
        return 6;
    }

    if (bn_equals_zero(b)) {
        fputs("0", stdout);
        return 1;
    }

    // number of characters printed
    size_t nc = 0;

    if (b->signbit) {
        fputs("-", stdout);
        nc += 1;
    }

    // print leading digit without leading 0s
    nc += bnu_print_digit(b->digits_end[b->msd_pos], b->base,
        false, use_uppercase);

    // does it only have 1 digit?
    if (b->msd_pos == 0) {
        if (print_base) {
            nc += printf("_%u", b->base);
        }
        return nc;
    }

    // print remaining digits with leading 0s
    for (size_t i = b->msd_pos-1; &b->digits_end[i] >= b->digits_end; i--) {
        nc += bnu_print_digit(b->digits_end[i], b->base,
            true, use_uppercase);
    }

    if (print_base) {
        nc += printf("_%u", b->base);
    }

    return nc;
}

// comparison


bool bn_equals(const Bignum* a0, const Bignum* a1) {
    return bn_cmp(a0, a1) == 0;
}

bool bn_equals_zero(const Bignum* a0) {
    return (a0->digits_end[a0->msd_pos] == 0);
}


int bn_cmp(const Bignum* a0, const Bignum* a1) {

    if (bn_equals_zero(a0) && bn_equals_zero(a1)) {
        return 0;
    }

    if (a0->signbit && !a1->signbit) {
        return -1; // a0 < 0 < a1
    } else if (!a0->signbit && a1->signbit) {
        return 1; // a1 < 0 < a0
    }

    // a0, a1 have same sign
    int sign = a0->signbit ? -1 : 1;

    // compare real lengths, accounting for where start is not just capacity
    size_t len0 = bni_real_len(a0);
    size_t len1 = bni_real_len(a1);
    if (len0 > len1) {
        return sign * 1;
    } else if (len0 < len1) {
        return sign * -1;
    }

    // compare digitwise MSD -> LSD
    for (size_t i = a0->msd_pos; &a0->digits_end[i] >= a0->digits_end; i--) {
        if (a0->digits_end[i] > a1->digits_end[i]) {
            return sign * 1;
        } else if (a0->digits_end[i] < a1->digits_end[i]) {
            return sign * -1;
        }
    }

    // all digits are equal
    return 0;
}

// operations

void bn_neg(Bignum* result, const Bignum* a0) {

    Bignum arg0 = *a0;

    // -0 = 0
    if (bn_equals_zero(&arg0)) {
        bni_copy(result, &arg0);
        return;
    }

    // -a0
    bni_neg(result, &arg0);
}

void bn_add(Bignum* result, const Bignum* a0, const Bignum* a1) {

    Bignum arg0 = *a0;
    Bignum arg1 = *a1;

    // 0 + a1 = a1
    if (bn_equals_zero(&arg0)) {
        bni_copy(result, &arg1);
        return;
    }

    // a0 + 0 = a0
    if (bn_equals_zero(&arg1)) {
        bni_copy(result, &arg0);
        return;
    }

    // (-a0) + a1 => a1 - a0
    if (arg0.signbit && !arg1.signbit) {
        arg0.signbit = 0;
        bni_sub(result, &arg1, &arg0);
        return;
    }

    // a0 + (-a1) => a0 - a1
    if (!arg0.signbit && arg1.signbit) {
        arg1.signbit = 0;
        bni_sub(result, &arg0, &arg1);
        return;
    }

    // (-a0) + (-a1) => -(a0 + a1)
    if (arg0.signbit && arg1.signbit) {
        arg0.signbit = 0;
        arg1.signbit = 0;
        bni_add(result, &arg1, &arg0);
        result->signbit = !result->signbit;
        return;
    }

    // a0 + a1
    bni_add(result, &arg0, &arg1);
}

void bn_sub(Bignum* result, const Bignum* a0, const Bignum* a1) {

    Bignum arg0 = *a0;
    Bignum arg1 = *a1;

    // 0 - a1 = -a1
    if (bn_equals_zero(&arg0)) {
        bni_copy(result, &arg1);
        result->signbit = !result->signbit;
        return;
    }

    // a0 - 0 = a0
    if (bn_equals_zero(&arg1)) {
        bni_copy(result, &arg0);
        return;
    }

    int cmp = bn_cmp(&arg0, &arg1);

    // a0 - a0 = 0
    if (cmp == 0) {
        bni_write_parts1(result, 0, 0, BN_BASE_DEFAULT);
        return;
    }

    // (-a0) - a1 => -(a0 + a1)
    if (arg0.signbit && !arg1.signbit) {
        arg0.signbit = 0;
        bni_add(result, &arg0, &arg1);
        result->signbit = !result->signbit;
        return;
    }

    // a0 - (-a1) => a0 + a1
    if (!arg0.signbit && arg1.signbit) {
        arg1.signbit = 0;
        bni_add(result, &arg0, &arg1);
        return;
    }

    // (-a0) - (-a1) => a1 - a0
    if (arg0.signbit && arg1.signbit) {
        arg0.signbit = 0;
        arg1.signbit = 0;
        bni_sub(result, &arg1, &arg0);
        return;
    }

    // a0 - a1
    bni_sub(result, &arg0, &arg1);
}

void bn_mul(Bignum* result, const Bignum* a0, const Bignum* a1) {

    Bignum arg0 = *a0;
    Bignum arg1 = *a1;

    if (bn_equals_zero(&arg0) || bn_equals_zero(&arg1)) {
        bni_write_parts1(result, 0, 0, BN_BASE_DEFAULT);
        return;
    }

    // (-a0) * a1 = -(a0 * a1)
    if (arg0.signbit && !arg1.signbit) {
        arg0.signbit = 0;
        bni_mul(result, &arg0, &arg1);
        result->signbit = !result->signbit;
        return;
    }

    // a0 * (-a1) = -(a0 * a1)
    if (!arg0.signbit && arg1.signbit) {
        arg1.signbit = 0;
        bni_mul(result, &arg0, &arg1);
        result->signbit = !result->signbit;
        return;
    }

    // (-a0) * (-a1) = a0 * a1
    if (arg0.signbit && arg1.signbit) {
        arg0.signbit = 0;
        arg1.signbit = 0;
        bni_mul(result, &arg0, &arg1);
        return;
    }

    // a0 * a1
    bni_mul(result, a0, a1);
}

bool bn_divmod(Bignum* result_div, Bignum* result_mod,
            const Bignum* a0,
            const Bignum* a1)
{

    Bignum arg0 = *a0;
    Bignum arg1 = *a1;

    // divmod(a0, 0) = divide/mod by zero error
    if (bn_equals_zero(&arg1)) {
        return false;
    }

    // divmod(0, a0) = [0, 0]
    if (bni_cmp_Nx1(&arg0, 0) == 0) {
        if (result_div != NULL) {
            bni_write_parts1(result_div, 0, 0, arg0.base);
        }
        if (result_mod != NULL) {
            bni_write_parts1(result_mod, 0, 0, arg0.base);
        }
        return true;
    }

    // divmod(a0, 1) = [a0, 0]
    if (bni_cmp_Nx1(&arg1, 1) == 0) {
        if (result_div != NULL) {
            bni_copy(result_div, &arg0);
        }
        if (result_mod != NULL) {
            bni_write_parts1(result_mod, 0, 0, arg0.base);
        }
        return true;
    }

    // divmod(a0, a0) = [1, 0]
    if (bn_cmp(&arg0, &arg1) == 0) {
        if (result_div != NULL) {
            bni_write_parts1(result_div, 0, 1, arg0.base);
        }
        if (result_mod != NULL) {
            bni_write_parts1(result_mod, 0, 0, arg0.base);
        }
        return true;
    }

    // only supports Nx1 division for now
    // TODO MxN
    if (bni_real_len(&arg1) > 1) {
        return false;
    }

    // a0 // a1, a0 % a1
    bni_divqr_Nx1(result_div, result_mod, &arg0, arg1.digits_end[0]);
    return true;
}

uint64_t bni_real_len(const Bignum* b) {
    return b->msd_pos + 1;
}

bool bni_is_valid(const Bignum* b) {
    return b != NULL && b->digits_end != NULL;
}

void bni_freealloc(Bignum* out, size_t n_digits, uint8_t base) {
    bni_try_free(out);
    *out = (Bignum){
        .digits_end = BN_MALLOC(n_digits * sizeof(bn_digit_t)),
        .capacity = n_digits
    };
    memset(out->digits_end, 0, n_digits * sizeof(bn_digit_t));
    out->msd_pos = 0;
    out->base = base;
}

void bni_append_zeros(Bignum* out, size_t n) {

    uint64_t new_capacity = out->capacity + n;
    Bignum result = {
        .base = out->base,
        .digits_end = BN_MALLOC(new_capacity * sizeof(bn_digit_t)),
        .capacity = new_capacity,
        .signbit = out->signbit
    };
    memset(result.digits_end, 0, new_capacity);
    memcpy(result.digits_end, out->digits_end,
           out->capacity * sizeof(bn_digit_t));

    // do not bni_normalize(), instead:
    result.msd_pos = out->msd_pos;

    bni_try_free(out);
    *out = result;
}

void bni_copy(Bignum* dest, const Bignum* src) {

    uint64_t real_len = bni_real_len(src);
    Bignum result = {
        .base = src->base,
        .digits_end = BN_MALLOC(real_len * sizeof(bn_digit_t)),
        .capacity = real_len,
        .signbit = src->signbit
    };
    memcpy(result.digits_end, src->digits_end,
           real_len * sizeof(bn_digit_t));

    bni_try_free(dest);
    *dest = result;
    bni_normalize(dest);
}

void bni_convert(Bignum* dest, const Bignum* src, uint8_t new_base) {

    bn_digit_t real_base = BN_BASE[new_base].real_base;

    Bignum result = { .base = new_base };

    Bignum src_copy = {0};
    bni_copy(&src_copy, src);

    Bignum current_dest_digit = {0};

    while (true) {

        // allocate a new digit in the result
        bni_append_zeros(&result, 1);

        // if the division result would be zero, src_copy is the final digit
        if (bni_cmp_Nx1(&src_copy, new_base) == -1) {
            result.digits_end[result.msd_pos] = src_copy.digits_end[0];
            result.msd_pos++;
            break;
        }

        // src_copy /= real_base
        // current_dest_digit = src_copy % real_base
        bni_divqr_Nx1(&src_copy, &current_dest_digit, &src_copy, real_base);

        // append another digit
        result.digits_end[result.msd_pos] =
current_dest_digit.digits_end[current_dest_digit.msd_pos];
        result.msd_pos++;
    }

    bni_try_free(&src_copy);
    bni_try_free(dest);
    *dest = result;
    bni_normalize(dest);
}

void bni_try_free(Bignum* out) {
    if (!bni_is_valid(out)) {
        return;
    }
#ifndef BN_NOFREE
    BN_FREE(out->digits_end);
#endif
    *out = (Bignum){0};
}

void bni_normalize(Bignum* out) {

    // single digit, nothing to do
    if (out->capacity == 1) {
        out->msd_pos = 0;
        return;
    }

    out->msd_pos = out->capacity - 1;

    // strip leading zeroes
    while (out->msd_pos != 0 && out->digits_end[out->msd_pos] == 0) {
        out->msd_pos--;
    }
}

bool bni_write_str(Bignum* out, const char* str, size_t len, uint8_t base) {

    if (!bnu_base_valid(base) || str == NULL || len == 0) {
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
    for (const char* s = str; s != str + len; s++) {
        if (!bnu_digit_valid(*s, base, NULL)) {
            return false;
        }
    }

    // how many leading zeroes are needed to round off a full digit?

    uint32_t mp = BN_BASE[base].width;

    // avoid wasting space - add 0 bytes if len already rounded off
    uint32_t n_leading_zeroes = (len % mp == 0) ? 0 : (mp - (len % mp));
    uint32_t new_len = len + n_leading_zeroes;

    // if needed, copy the string, padding with leading zeroes

    char* new_str;

    if (n_leading_zeroes == 0) {
        new_str = bnu_strndup(str, len);
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
        .capacity = num_digits,
        .signbit = (is_negative) ? 1 : 0,
    };
    memset(result.digits_end, 0, num_digits * sizeof(uint32_t));

    // loop in reverse order
    uint32_t* dp = &result.digits_end[result.capacity - 1];
    char* sp = new_str;

    while (sp < new_str + len) {

        // mp characters => 1 digit
        if (!bnu_parse_digit(sp, 0, mp, base, dp)) {
            // nth digit conversion failed
            bni_try_free(&result);
            return false;
        }

        sp += mp;
        dp -= 1;
    }

#ifndef BN_NOFREE
    BN_FREE(new_str);
#endif

    bni_try_free(out);
    *out = result;
    bni_normalize(out);
    return true;
}

bool bni_write_parts1(Bignum* out,
                      uint8_t signbit,
                      bn_digit_t d0,
                      uint8_t base)
{
    if (!bnu_digit_in_range(d0, base)) {
        return false;
    }

    bni_freealloc(out, 1, base);
    out->signbit = signbit;
    out->digits_end[0] = d0;
    bni_normalize(out);
    return true;

}

bool bni_write_parts2(Bignum* out,
                      uint8_t signbit,
                      bn_digit_t d0, bn_digit_t d1,
                      uint8_t base)
{
    if (!bnu_digit_in_range(d0, base)
    || !bnu_digit_in_range(d1, base)) {
        return false;
    }

    bni_freealloc(out, 2, base);
    out->signbit = signbit;
    out->digits_end[1] = d0;
    out->digits_end[0] = d1;
    bni_normalize(out);
    return true;

}

void bni_dump(const Bignum* b) {
    if (b->digits_end == NULL) {
        printf("Bignum{\n"
            "  .base=%lu,\n"
            "  .signbit=%lu,\n"
            "  .capacity=%llu,\n"
            "  rlen=%lu,\n"
            "  .digits=[]}\n",
        (unsigned long)b->base,
        (unsigned long)b->signbit,
        (unsigned long long)b->capacity,
        (unsigned long)bni_real_len(b));
    } else {

        printf("Bignum{\n"
            "  .base=%lu,\n"
            "  .signbit=%lu,\n"
            "  .capacity=%llu,\n"
            "  rlen=%lu,\n"
            "  .digits=[\n"
            "    <mem_start>\n",
        (unsigned long)b->base,
        (unsigned long)b->signbit,
        (unsigned long long)b->capacity,
        (unsigned long)bni_real_len(b));

        for (uint64_t i = 0; i < b->capacity; i++) {
            printf(
            "    %0*lu,%s\n",
            BN_BASE[b->base].width,
            (unsigned long) b->digits_end[i],
            i == b->msd_pos ? " <-- MSD" : "");
        }

        printf(
            "    <mem_end>\n"
            "  ]}\n");
    }
}

int bni_cmp_Nx1(const Bignum* a0, bn_digit_t a1) {
    if (bni_real_len(a0) > 1) return 1;
    if (a0->digits_end[0] > a1) return 1;
    if (a0->digits_end[0] < a1) return -1;
    return 0;
}

void bni_lshift(Bignum* out, const Bignum* a0, size_t n) {
    if (bn_equals_zero(a0)) {
        bni_copy(out, a0);
        return;
    }

    size_t rlen0 = bni_real_len(a0);
    size_t new_len = rlen0 + n;

    Bignum result = {0};
    bni_freealloc(&result, new_len, a0->base);

    bn_digit_t* dest = result.digits_end + n;
    bn_digit_t* src = a0->digits_end;

    for (size_t i = 0; i < rlen0; i++) {
        dest[i] = src[i];
    }

    for (size_t i = 0; i < n; i++) {
        result.digits_end[i] = 0;
    }

    result.msd_pos = new_len - 1;
    result.signbit = a0->signbit;

    bni_try_free(out);
    *out = result;
    bni_normalize(out);
}

void bni_rshift(Bignum* out, const Bignum* a0, size_t n) {
    if (bn_equals_zero(a0)) {
        bni_copy(out, a0);
        return;
    }

    size_t rlen0 = bni_real_len(a0);

    if (n >= rlen0) {
        bni_write_parts1(out, 0, 0, BN_BASE_DEFAULT);
        return;
    }

    size_t new_len = rlen0 - n;

    Bignum result = {0};
    bni_freealloc(&result, new_len, a0->base);

    bn_digit_t* dest = result.digits_end;
    bn_digit_t* src = a0->digits_end + n;

    for (size_t i = 0; i < new_len; i++) {
        dest[i] = src[i];
    }

    result.msd_pos = new_len - 1;
    result.signbit = a0->signbit;

    bni_try_free(out);
    *out = result;
    bni_normalize(out);
}

void bni_neg(Bignum* out, const Bignum* a0) {

    Bignum result = {0};
    bni_copy(&result, a0);
    result.signbit = !result.signbit;

    *out = result;
    bni_normalize(out);
}

void bni_add(Bignum* out, const Bignum* a0, const Bignum* a1) {
    // carry algorithm

    if (a0->base != a1->base) {
        printf("bases do not match!!\n");
        return;
    }

    // swap the numbers such that a0->len >= a1->len
    // this makes computation simpler in the loop
    // also guarantees `max(len0, len1) == len0`
    if (bni_real_len(a0) < bni_real_len(a1)) {
        const Bignum* temp = a0;
        a0 = a1;
        a1 = temp;
    }

    Bignum result = {0};
    size_t max_len = 1 + bni_real_len(a0); // +1 for possible carry
    bni_freealloc(&result, max_len, a0->base);

    bn_digit_t real_base = BN_BASE[result.base].real_base;

    // LSD -> MSD
    bn_digit_t* d0 = a0->digits_end;
    bn_digit_t* d1 = a1->digits_end;
    bn_digit_t carry = 0;

    int i = 0;
    while (d0 <= &a0->digits_end[a0->msd_pos]) {
        bn_digit_t digit0 = *d0;
        bn_digit_t digit1 = (d1 <= &a1->digits_end[a1->msd_pos]) ? *d1 : 0;
        bn_digit_t sum = digit0 + digit1 + carry;

        carry = sum / real_base;
        result.digits_end[i] = sum % real_base;

        d0 += 1;
        d1 += 1;
        i += 1;
    }

    result.signbit = 0;
    result.msd_pos = i - 1;

    if (carry != 0) {
        result.msd_pos += 1;
        result.digits_end[i] = carry;
    }

    bni_try_free(out);
    *out = result;
    bni_normalize(out);
}

void bni_sub(Bignum* out, const Bignum* a0, const Bignum* a1) {
    // borrow algorithm

    if (a0->base != a1->base) {
        printf("bases do not match!!\n");
        return;
    }

    // a0 < a1 => -(a1 - a0)
    if (bn_cmp(a0, a1) == -1) {
        bni_sub(out, a1, a0);
        out->signbit = !out->signbit;
        return;
    }

    // now assume a0 > a1

    Bignum result = {0};
    bni_freealloc(&result, a0->capacity, a0->base);

    bn_digit_t real_base = BN_BASE[result.base].real_base;

    // LSD -> MSD
    bn_digit_t* d0 = a0->digits_end;
    bn_digit_t* d1 = a1->digits_end;
    uint8_t borrow = 0;
    size_t i = 0;
    while (d0 <= &a0->digits_end[a0->msd_pos] && i < result.capacity) {

        bn_digit_t digit0 = *d0;
        bn_digit_t digit1 = (d1 <= &a1->digits_end[a1->msd_pos]) ? *d1 : 0;

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

    bni_try_free(out);
    *out = result;
    bni_normalize(out);
}

void bni_mul(Bignum* out, const Bignum* a0, const Bignum* a1) {
    // naive algorithm - optimize later

    if (a0->base != a1->base) {
        printf("bases do not match!!\n");
        return;
    }

    Bignum result = {0};
    bni_freealloc(&result, bni_real_len(a0) + 1 + bni_real_len(a1) + 1,
a0->base);

    bn_digit_t real_base = BN_BASE[result.base].real_base;
    bn_digit_t* r_ptr = result.digits_end;

    for (bn_digit_t* d0 = a0->digits_end; d0 <= &a0->digits_end[a0->msd_pos];
d0++)
    {
        uint64_t carry = 0;
        bn_digit_t* res_ptr = r_ptr;

        for (bn_digit_t* d1 = a1->digits_end; d1 <=
&a1->digits_end[a1->msd_pos]; d1++)
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

    bni_try_free(out);
    *out = result;
    bni_normalize(out);
}

void bni_divqr_Nx1(Bignum* q_out, Bignum* r_out,
                const Bignum* a0,
                bn_digit_t a1)
{
    // naive algorithm - optimize later

    Bignum q_result = {0};
    if (q_out != NULL) {
        bni_freealloc(&q_result, bni_real_len(a0) + 1, a0->base);
    }

    // remainder is always 1 digit
    Bignum r_result = {0};
    if (r_out != NULL) {
        bni_freealloc(&r_result, 1, a0->base);
    }

    if (q_result.capacity == 2) {
        // 1x1
        bnu_divqr_1x1(a0->digits_end[0],
                      a1,
                      !!q_out ? &q_result.digits_end[0] : NULL,
                      !!r_out ? &r_result.digits_end[0] : NULL);
    } else if (q_result.capacity == 3) {
        // 2x1
        bnu_divqr_2x1(a0->digits_end[1], a0->digits_end[0],
                    a1,
                    a0->base,
                    !!q_out ? &q_result.digits_end[1] : NULL,
                    !!q_out ? &q_result.digits_end[0] : NULL,
                    !!r_out ? &r_result.digits_end[0] : NULL);
    } else {
        // 3x1 or more - do it in 2x1 steps

        // prepend with a zero
        Bignum arg0 = {0};
        bni_copy(&arg0, a0);
        bni_append_zeros(&arg0, 1);

        // {d_0}{d_1}{d_2}...{d_n} /% D
        // take 2 digits at a time:

        // q_0 = {0}{d_0} // D
        // r_0 = {0}{d_0} % D
        // q_n = {r_n-1}{d_n} // D
        // r_n = {r_n-1}{d_n} % D

        // final result: {q_0}{q_1}{q_2}...{q_n} R {r_n}

        // bni_dump(a0);
        // bni_dump(&arg0);

        // start of loop
        int64_t i = a0->msd_pos;

        // current quotient and remainder
        bn_digit_t cq0, cq1, cr = 0;

        // current two digits
        Bignum ctd = {0};
        bni_freealloc(&ctd, 2, arg0.base);

        while (i >= 0) {

            // ctd = {r_n-1}{d_n}
            // ctd.digits_end[0] = cr;
            // ctd.digits_end[1] = arg0.digits_end[i];
            // ctd.msd_pos = 1;
            bni_write_parts2(&ctd, 0, cr, arg0.digits_end[i], arg0.base);

            // get q_n and r_n
            bnu_divqr_2x1(ctd.digits_end[1], ctd.digits_end[0],
                            a1,
                            arg0.base,
                            &cq0, &cq1,
                            &cr);
            // printf("digit %"PRIu64" - ctd=[%"PRIu32",%"PRIu32"],"
            //     "cq=[%"PRIu32",%"PRIu32"], cr=%"PRIu32"\n",
            //         i, ctd.digits_end[0], ctd.digits_end[1],
            //        cq0, cq1, cr);

            // append to quotient
            // cq0 guaranteed to be 0?
            if (q_out != NULL) {
                q_result.digits_end[i] = cq1;
            }

            i -= 1;
        }

        // final remainder
        if (r_out != NULL) {
            r_result.digits_end[0] = cr;
        }
    }

    // return result
    if (q_out != NULL) {
        bni_try_free(q_out);
        *q_out = q_result;
        bni_normalize(q_out);
    }
    if (r_out != NULL) {
        bni_try_free(r_out);
        *r_out = r_result;
        bni_normalize(r_out);
    }
}

// utils

uint64_t bnu_min(uint64_t x, uint64_t y) {
    return (x < y) ? x : y;
}

uint64_t bnu_max(uint64_t x, uint64_t y) {
    return (x > y) ? x : y;
}

char* bnu_strndup(const char* str, size_t n) {
    char* dup = BN_MALLOC((n + 1) * sizeof(char));
    strncpy(dup, str, n);
    dup[n] = '\0';
    return dup;
}

void bnu_divqr_1x1(bn_digit_t x,
                    bn_digit_t y,
                    bn_digit_t* q_out, bn_digit_t* r_out)
{

    if (q_out != NULL) {
        *q_out = x / y;
    }
    if (r_out != NULL) {
        *r_out = x % y;
    }
}

void bnu_divqr_2x1(bn_digit_t x0, bn_digit_t x1,
                    bn_digit_t y,
                    uint8_t base,
                    bn_digit_t* q0_out, bn_digit_t* q1_out,
                    bn_digit_t* r_out)
{

    uint64_t x0_64 = x0;
    uint64_t x1_64 = x1;
    uint64_t y_64 = y;
    uint64_t real_base_64 = BN_BASE[base].real_base;

    uint64_t x_64 = (x0_64 * real_base_64) + x1_64;

    // split quotient
    if (q0_out != NULL && q1_out != NULL) {
        uint64_t quotient = x_64 / y_64;
        *q0_out = quotient / real_base_64;
        *q1_out = quotient % real_base_64;
    }
    if (r_out != NULL) {
        uint64_t remainder = x_64 % y_64;
        *r_out = (bn_digit_t)remainder;
    }
}

// false => conversion error
// out = u32(str[start:end], base)
bool bnu_parse_digit(const char* str,
                    size_t start,
                    size_t end,
                    uint8_t base,
                    bn_digit_t* out)
{

    if (end - start > BN_BASE[base].width) {
        return false;
    }

    if (base < BN_BASE_MIN || base > BN_BASE_MAX) {
        return false;
    }

    char* b = bnu_strndup(str + start, end - start);

    char* b_end = NULL;
    unsigned long ul = strtoul(b, &b_end, base);
#ifndef BN_NOFREE
    BN_FREE(b);
#endif

    if (b_end == NULL) {
        return false;
    }

    *out = (bn_digit_t)ul;
    return true;
}

size_t bnu_print_digit(bn_digit_t digit_value,
                         uint8_t base,
                         bool print_leading_zeroes,
                         bool use_uppercase_digits)
{
    bn_digit_t d = digit_value;
    uint32_t n = 0;
    while (d > 0) {
        d /= base;
        n += 1;
    }
    size_t n_digits = BN_BASE[base].width - n;

    size_t nc = 0;

    if (print_leading_zeroes) {
        for (size_t i = 0; i < n_digits; i++) {
            fputc('0', stdout);
            nc += 1;
        }
    }

    if (digit_value / base != 0) {
        nc += bnu_print_digit(digit_value / base,
            base, false, use_uppercase_digits);
    }

    // +1 because BN_BASE is zero indexed
    bn_digit_t adjusted_dv = 1 + (digit_value % base);
    fputc(BN_BASE[adjusted_dv].last_digit[use_uppercase_digits], stdout);
    nc += 1;

    return nc;
}

bool bnu_digit_in_range(bn_digit_t digit, uint8_t base) {
    return digit < BN_BASE[base].real_base;
}

bool bnu_digit_valid(char digit, uint8_t base, bn_digit_t* out) {
    if (base < BN_BASE_MIN || base > BN_BASE_MAX) {
        return false;
    }

    if (!isalnum(digit)) {
        return false;
    }

    for (size_t i = 1; i <= base; i++) {
        if (digit == BN_BASE[i].last_digit[0]
        || digit == BN_BASE[i].last_digit[1]) {
            if (out != NULL) {
                *out = i;
            }
            return true;
        }
    }
    return false;
}

bool bnu_base_valid(uint8_t base) {
    return BN_BASE_MIN <= base && base <= BN_BASE_MAX;
}
