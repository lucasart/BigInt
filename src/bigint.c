#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "bigint.h"

// Verifes that the invariants are satisfied
static bool bint_ok(const BigInt *x)
{
    if (x->count < 1 || x->reserved < x->count || !x->digits)
        return false;

    // Make sure that digits[count..reserved-1] only contains zeros
    for (int i = x->count; i < x->reserved; i++)
        if (x->digits[i])
            return false;

    return true;
}

// Resize to count elements, growing the buffer as needed (in powers of 2), and setting the leading
// digits to zero.
static void bint_resize(BigInt *x, int count)
{
    assert(bint_ok(x));

    const int oldReserved = x->reserved, oldCount = x->count;
    x->count = count;

    while (x->reserved < x->count)
        x->reserved *= 2;

    if (x->reserved > oldReserved) {
        x->digits = realloc(x->digits, sizeof(digit_t) * (size_t)x->reserved);
        for (int i = oldReserved; i < x->reserved; x->digits[i++] = 0);
    } else if (x->count < oldCount)
        for (int i = x->count; i < oldCount; x->digits[i++] = 0);

    assert(bint_ok(x));
}

BigInt bint_new(digit_t value)
{
    BigInt x = {
        .digits = malloc(sizeof(digit_t)),
        .count = 1,
        .reserved = 1
    };
    x.digits[0] = value;

    assert(bint_ok(&x));
    return x;
}

void bint_del(BigInt *x)
{
    assert(bint_ok(x));
    free(x->digits);
    *x = (BigInt){0};
}

// Compares a BigInt to a digit
bool bint_eq(const BigInt *x, digit_t y)
{
    assert(bint_ok(x));
    return x->count == 1 && x->digits[0] == y;
}

void bint_copy(BigInt *x, const BigInt *y)
{
    assert(bint_ok(x) && bint_ok(y));
    bint_resize(x, y->count);

    for (int i = 0; i < y->count; i++)
        x->digits[i] = y->digits[i];

    assert(bint_ok(x));
}

// Print a BigInt's raw internals (for debugging)
void bint_print_debug(const BigInt *x)
{
    assert(bint_ok(x));
    putchar('{');

    printf("count: %d, reserved: %d, digits: (", x->count, x->reserved);

    for (int i = 0; i < x->count; i++) {
        printf("%" PRIx64, x->digits[i]);
        putchar(i == x->count - 1 ? ')' : ',');
    }

    putchar('}');
}

// Print a BigInt in given base, with shown in reverse, and returns the number of digits.
unsigned bint_print(const BigInt *x, digit_t base)
{
    static char symbols[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    assert(bint_ok(x) && base < sizeof(symbols));

    BigInt y = bint_new(0);
    bint_copy(&y, x);

    unsigned count = 0;

    while (!bint_eq(&y, 0)) {
        const digit_t r = bint_div(&y, base);
        assert(r < base);
        putchar(symbols[r]);
        count++;
    }

    bint_del(&y);

    return max(1u, count);  // 0 still counts as 1 digit
}

// in-place addition: X += Y
void bint_add(BigInt *x, const BigInt *y)
{
    assert(bint_ok(x) && bint_ok(y));
    bint_resize(x, max(x->count, y->count));

    digit_t carry = 0;

    for (int i = 0; i < x->count; i++) {
        const container_t sum = (container_t)x->digits[i] + (container_t)y->digits[i]
            + (container_t)carry;
        x->digits[i] = (digit_t)sum;
        carry = (digit_t)(sum >> (sizeof(digit_t) * 8));
    }

    if (carry) {
        bint_resize(x, x->count + 1);
        x->digits[x->count - 1] = carry;
    }

    assert(bint_ok(x));
}

// in-place multiplication by digit: X *= y
void bint_mul(BigInt *x, digit_t y)
{
    assert(bint_ok(x));
    digit_t carry = 0;

    for (int i = 0; i < x->count; i++) {
        const container_t product = (container_t)x->digits[i] * (container_t)y
            + (container_t)carry;
        x->digits[i] = (digit_t)product;
        carry = (digit_t)(product >> (sizeof(digit_t) * 8));
    }

    if (carry) {
        bint_resize(x, x->count + 1);
        x->digits[x->count - 1] = carry;
    }

    assert(bint_ok(x));
}

// in-place division by digit: X /= y, returns remainder
digit_t bint_div(BigInt *x, digit_t y)
{
    assert(bint_ok(x) && y);
    digit_t carry = 0;

    for (int i = x->count - 1; i >= 0; i--) {
        const container_t numerator = (container_t)x->digits[i]
            + ((container_t)carry << (sizeof(digit_t) * 8));
        x->digits[i] = (digit_t)(numerator / y);
        carry = (digit_t)(numerator % y);
    }

    if (!x->digits[x->count - 1])
        bint_resize(x, max(1, x->count - 1));

    assert(bint_ok(x));
    return carry;
}
