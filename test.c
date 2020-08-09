#include <stdlib.h>
#include <stdio.h>
#include "bigint.h"

// SplitMix64 PRNG, based on http://xoroshiro.di.unimi.it/splitmix64.c
static uint64_t prng(uint64_t *state)
{
    uint64_t rnd = (*state += 0x9E3779B97F4A7C15);
    rnd = (rnd ^ (rnd >> 30)) * 0xBF58476D1CE4E5B9;
    rnd = (rnd ^ (rnd >> 27)) * 0x94D049BB133111EB;
    rnd ^= rnd >> 31;
    return rnd;
}

static void benchmark(digit_t a[], digit_t b[], size_t count, digit_t x)
{
    BigInt r = big_init_set_ui(x);

    for (size_t i = 0; i < count; i++) {
        printf("r = "); big_debug(&r);
        printf("a = %lxu, b = %lxu\n", a[i], b[i]);

        // r = a * r + b
        big_mul_ui(&r, &r, a[i]);
        big_add_ui(&r, &r, b[i]);
    }

    big_debug(&r);
    big_clear(&r);
}

int main(void)
{
    enum {COUNT = 10};
    digit_t a[COUNT], b[COUNT];
    uint64_t state = 0;

    for (size_t i = 0; i < COUNT; i++) {
        a[i] = (digit_t)prng(&state);
        b[i] = (digit_t)prng(&state);
    }

    benchmark(a, b, COUNT, (digit_t)prng(&state));
}
