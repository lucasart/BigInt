#pragma once
#include <stdbool.h>
#include <inttypes.h>

typedef uint64_t digit_t;
typedef __uint128_t container_t;

#define min(x, y) ({ \
    typeof(x) _x = (x), _y = (y); \
    _x < _y ? _x : _y; \
})

#define max(x, y) ({ \
    typeof(x) _x = (x), _y = (y); \
    _x > _y ? _x : _y; \
})

typedef struct {
    digit_t *digits;  // digits[k] is the 2^64 factor in the polynomial
    int count;  // number of digits
    int reserved;  // number of elements malloced (>= count due to realloc strategy)
} BigInt;

BigInt bint_new(digit_t v);
void bint_del(BigInt *x);

bool bint_eq(const BigInt *x, digit_t y);
void bint_copy(BigInt *x, const BigInt *y);

unsigned bint_print(const BigInt *x, digit_t base);
void bint_print_debug(const BigInt *x);

void bint_add(BigInt *x, const BigInt *y);
void bint_mul(BigInt *x, digit_t y);
digit_t bint_div(BigInt *x, digit_t y);
