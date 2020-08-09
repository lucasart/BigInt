#include <stdlib.h>
#include <stdio.h>
#include "bigint.h"

int main(void)
{
    BigInt x = big_init_set_ui(1);

    for (unsigned n = 2; n <= 40; n++) {
        big_mul_ui(&x, &x, n);
        big_debug(&x);

        char *decimal = big_get_str(&x, 10);
        puts(decimal);
        free(decimal);
        decimal = NULL;
    }

    big_clear(&x);
}
