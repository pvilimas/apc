#include "apc.h"

// unary operators

// +a0 : Num => Num
Value UnopFn_Plus(Value a0) {
    if (a0.type != V_NUMBER) {
        apc_return(E_VALUE_ERROR);
    }

    return a0;
}

// -a0 : Num => Num
Value UnopFn_Negate(Value a0) {
    if (a0.type != V_NUMBER) {
        apc_return(E_VALUE_ERROR);
    }

    a0.number.sign = !a0.number.sign;
    return a0;
}

// a0 + a1 : (Num, Num) => Num
Value BinopFn_Add(Value a0, Value a1) {
    if (a0.type != V_NUMBER || a1.type != V_NUMBER) {
        apc_return(E_VALUE_ERROR);
    }

    Value result = { .type = V_NUMBER };
    bn_iadd(&result.number,
           &a0.number,
           &a1.number);

    printf("\nbn_add: ");
    bn_print(&a0.number);
    printf(" + ");
    bn_print(&a1.number);
    printf(" = ");
    bn_print(&result.number);
    printf("\n");

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

