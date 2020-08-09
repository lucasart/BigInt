#pragma once
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#if UINTPTR_MAX == 0xffffffff
    // 32-bit
    typedef uint32_t digit_t;
    typedef uint64_t container_t;
#else
    // assume 64-bit
    typedef uint64_t digit_t;
    typedef __uint128_t container_t;
#endif

typedef struct {
    digit_t *digits;  // digits in base 2^N, from lowest to highest.
    int count;  // number of digits (in base 2^N). Always >= 1.
    int reserved;  // number of elements allocated for digits[]. Always >= count.
} BigInt;

// Initializing Integers
BigInt big_init(void);
void big_clear(BigInt *x);

// Assignment
void big_set(BigInt *x, const BigInt *y);
void big_set_ui(BigInt *x, digit_t y);
void big_set_str(BigInt *x, const char *str, unsigned base);

// Combined Initialization and Assignment
BigInt big_init_set(const BigInt *y);
BigInt big_init_set_ui(digit_t y);
BigInt big_init_set_str(const char *str, unsigned base);

// Conversion
digit_t big_get_ui(const BigInt *x);
char *big_get_str(const BigInt *x, unsigned base);

// Operations

// addition: r = x + y
void big_add(BigInt *r, const BigInt *x, const BigInt *y);
void big_add_ui(BigInt *r, const BigInt *x, digit_t y);

// substraction: r = x - y
void big_sub(BigInt *r, const BigInt *x, const BigInt *y);
void big_sub_ui(BigInt *r, const BigInt *x, digit_t y);

// multiplication: r = x * y
void big_mul(BigInt *r, const BigInt *x, const BigInt *y);
void big_mul_ui(BigInt *r, const BigInt *x, digit_t y);

// division: q = x / y, r = x % y
void big_div(BigInt *q, BigInt *r, const BigInt *x, const BigInt *y);
digit_t big_div_ui(BigInt *q, const BigInt *x, digit_t y);  // returns r

// Comparison
int big_cmp(const BigInt *x, const BigInt *y);
int big_cmp_ui(const BigInt *x, digit_t y);

// Expose internal representation. Useful for debugging.
void big_debug(const BigInt *x);
