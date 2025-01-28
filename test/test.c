#include "../src/bignum.h"

int main() {
    Bignum a0, a1, result;
    bn_init(&a0, &a1, &result);

    bn_write(&a0, "5598472", 10);
        bn_irshift(&a0, &a0, 1);
        bn_irshift(&a0, &a0, 1);
        bn_ilshift(&a0, &a0, 1);
        bn_ilshift(&a0, &a0, 1);
    for (int i = 0; i < 30; i++) {
        bn_print(&a0, true);
        printf("\n");
        bn_ilshift(&a0, &a0, 1);
        bn_print(&a0, true);
        printf("\n");
        bn_irshift(&a0, &a0, 1);
    }

    return 0;
}
