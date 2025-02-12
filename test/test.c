#include "../src/bignum.h"

int main() {
    // Bignum a0, a1, result;
    // bn_init(&a0, &a1, &result);
    //
    // bn_write(&a0, "5598472", 10);
    //     bn_irshift(&a0, &a0, 1);
    //     bn_irshift(&a0, &a0, 1);
    //     bn_ilshift(&a0, &a0, 1);
    //     bn_ilshift(&a0, &a0, 1);
    // for (int i = 0; i < 30; i++) {
    //     bn_print(&a0, true);
    //     printf("\n");
    //     bn_ilshift(&a0, &a0, 1);
    //     bn_print(&a0, true);
    //     printf("\n");
    //     bn_irshift(&a0, &a0, 1);
    // }

    Bignum a0, a1, result;
    bn_init(&a0, &a1, &result);

    bni_write_parts1(&a0, 0, 0, 10);

    // for (int b = 2; b < 37; b++) {
    //     bn_write(&a0, "1", b);
    //     printf("%d: %d\n", b, (int)bni_cmp_u32(&a0, 1));
    // }

    // bn_write(&a0,              "9999999999", 10);
    // bni_intdiv_u32(&result, &a0, 111111111);
    // printf("a0: ");
    // bn_print(&a0, true, false);
    // printf("\na1_u32: %"PRIu32"_10\nresult: ", 111111111);
    // bn_print(&result, true, false);
    // printf("\n");

    // uint32_t x0 = 1;
    // uint32_t x1 = 57;
    // uint32_t y = 13;
    // uint32_t base = 10;
    // uint32_t out0, out1;
    // bnu_sub_2x1(x0, x1, y, base, &out0, &out1);
    //
    // printf("x: ");
    // bnu_print_digit(x0, base, false, false);
    // bnu_print_digit(x1, base, false, false);
    // printf("\ny: ");
    // bnu_print_digit(y, base, false, false);
    // printf("\no: ");
    // bnu_print_digit(out0, base, false, false);
    // bnu_print_digit(out1, base, false, false);
    // printf("\n");

    return 0;
}
