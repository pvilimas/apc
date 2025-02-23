#include "../src/bignum.h"

int main() {

    Bignum a0 = {0},
        a1 = {0},
        a2 = {0},
        result = {0};

     bn_write(&a0, "16384000");
     bn_write(&a1, "7");

     while (!bn_equals_zero(&a0)) {
         bn_print(&a0);
         printf(" - ");
         bn_divmod(&a0, &a2,
                   &a0, &a1);
         bn_print(&a2);
         printf("\n");
    }

    return 0;
}
