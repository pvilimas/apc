#include "../src/bignum.h"

int main() {

    Bignum a0 = {0},
        a1 = {0},
        result = {0};

    bn_write2(&a0, "100000000110", 16);
    // = 17592186044416_10
    bn_convert(&result, &a0, 10);
    bn_print(&result, true, false);
    printf("\n");

    return 0;
}
