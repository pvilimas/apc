#include "../src/bignum.h"

int main() {
    Bignum a0, a1, result;
    bn_init(&a0, &a1, &result);

    if (!bn_write(&a0, "1101", 2)) {
        printf("failed\n");
        return 0;
    }

    printf("a0: ");
    bn_print(&a0, true);
    printf("\n");

    if (!bn_write(&a1, "0010", 2)) {
        printf("failed\n");
        return 0;
    }

    printf("a1: ");
    bn_print(&a1, true);
    printf("\n");

    bn_iadd(&result, &a0, &a1);

    printf("result: ");
    bn_print(&result, true);
    printf("\n");

    return 0;
}
