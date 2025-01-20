#include "../src/bignum.h"

int main() {
    Bignum a0, a1, result;
    bn_init(&a0, &a1, &result);

    bn_write(&a0, "771408124");
    bn_write(&a1, "273550044");
    bn_add(&result, &a0, &a1);

    printf("result: "); bn_print(&result); printf("\n");
}
