#include "apc.h"

// unary operators

// +a0 : Num => Num
Value UnopFn_Plus(Value a0) {
    if (a0.type != V_NUMBER) {
        apc_return(E_VALUE_ERROR);
    }

    Value result = { .type = V_NUMBER };
    bn_copy(&result.number, &a0.number);

    return result;
}

// -a0 : Num => Num
Value UnopFn_Minus(Value a0) {
    if (a0.type != V_NUMBER) {
        apc_return(E_VALUE_ERROR);
    }

    Value result = { .type = V_NUMBER };
    bn_neg(&result.number,
        &a0.number);

    return result;
}

// a0 + a1 : (Num, Num) => Num
Value BinopFn_Add(Value a0, Value a1) {
    if (a0.type != V_NUMBER || a1.type != V_NUMBER) {
        apc_return(E_VALUE_ERROR);
    }

    Value result = { .type = V_NUMBER };
    bn_add(&result.number,
        &a0.number,
        &a1.number);

    return result;
}

// a0 - a1 : (Num, Num) => Num
Value BinopFn_Sub(Value a0, Value a1) {
    if (a0.type != V_NUMBER || a1.type != V_NUMBER) {
        apc_return(E_VALUE_ERROR);
    }

    Value result = { .type = V_NUMBER };
    bn_sub(&result.number,
        &a0.number,
        &a1.number);

    return result;
}

// a0 * a1 : (Num, Num) => Num
Value BinopFn_Mul(Value a0, Value a1) {
    if (a0.type != V_NUMBER || a1.type != V_NUMBER) {
        apc_return(E_VALUE_ERROR);
    }

    Value result = { .type = V_NUMBER };
    bn_mul(&result.number,
        &a0.number,
        &a1.number);

    return result;
}

// a0 / a1 : (Num, Num) => Num
Value BinopFn_Div(Value a0, Value a1) {
    if (a0.type != V_NUMBER || a1.type != V_NUMBER) {
        apc_return(E_VALUE_ERROR);
    }

    Value result = { .type = V_NUMBER };
    bn_divmod(&result.number, NULL,
        &a0.number,
        &a1.number);

    return result;
}

// a0 % a1 : (Num, Num) => Num
Value BinopFn_Mod(Value a0, Value a1) {
    if (a0.type != V_NUMBER || a1.type != V_NUMBER) {
        apc_return(E_VALUE_ERROR);
    }

    Value result = { .type = V_NUMBER };
    bn_divmod(NULL, &result.number,
        &a0.number,
        &a1.number);

    return result;
}

// a0 # a1 : (Num, Num) => Num
Value BinopFn_BaseConv(Value a0, Value b) {
    if (a0.type != V_NUMBER || b.type != V_NUMBER) {
        apc_return(E_VALUE_ERROR);
    }

    if (bni_real_len(&b.number) > 1) {
        // base out of range
        apc_return(E_VALUE_ERROR);
    }

    bn_digit_t base = b.number.digits_end[0];

    Value result = { .type = V_NUMBER };
    if (!bn_convert(&result.number, &a0.number, base)) {
        // base out of range
        apc_return(E_VALUE_ERROR);
    }

    return result;
}
