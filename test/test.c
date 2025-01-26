#include "../src/bignum.h"

int main() {
    Bignum a0, a1, result;
    bn_init(&a0, &a1, &result);

    if (!bn_write(&a0, "1111", 2)) {
        printf("failed\n");
        return 0;
    }

    printf("a0: ");
    bn_print(&a0);
    printf("\n");

    if (!bn_write(&a1, "1111111", 2)) {
        printf("failed\n");
        return 0;
    }

    printf("a1: ");
    bn_print(&a1);
    printf("\n");

    bn_iadd(&result, &a0, &a1);

    printf("result: ");
    bn_print(&result);
    printf("\n");

    return 0;
}
