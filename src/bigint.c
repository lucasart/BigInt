#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "bigint.h"

#define max(x, y) ({ \
    typeof(x) _x = (x), _y = (y); \
    _x > _y ? _x : _y; \
})

// Verifes that the invariants are satisfied
static bool big_ok(const BigInt *x)
{
    if (!x || x->count < 1 || x->reserved < x->count || !x->digits)
        return false;

    // Make sure that digits[count..reserved-1] only contains zeros
    for (int i = x->count; i < x->reserved; i++)
        if (x->digits[i])
            return false;

    return true;
}

// Resize to count elements, growing the buffer as needed (in powers of 2), and setting the leading
// digits to zero.
static void big_resize(BigInt *x, int count)
{
    assert(big_ok(x));

    const int oldReserved = x->reserved, oldCount = x->count;
    x->count = count;

    while (x->reserved < x->count)
        x->reserved *= 2;

    if (x->reserved > oldReserved) {
        x->digits = realloc(x->digits, sizeof(long) * (size_t)x->reserved);
        for (int i = oldReserved; i < x->reserved; x->digits[i++] = 0);
    } else if (x->count < oldCount)
        for (int i = x->count; i < oldCount; x->digits[i++] = 0);

    assert(big_ok(x));
}

BigInt big_init()
{
    BigInt x = {
        .digits = calloc(1, sizeof(long)),
        .count = 1,
        .reserved = 1
    };

    assert(big_ok(&x));
    return x;
}

void big_clear(BigInt *x)
{
    assert(big_ok(x));
    free(x->digits);
    *x = (BigInt){0};
}

void big_set(BigInt *x, const BigInt *y)
{
    assert(big_ok(x) && big_ok(y));
    big_resize(x, y->count);

    for (int i = 0; i < y->count; i++)
        x->digits[i] = y->digits[i];

    assert(big_ok(x));
}

void big_set_ui(BigInt *x, unsigned long y)
{
    assert(big_ok(x));
    x->digits[0] = y;
    x->count = 1;
    assert(big_ok(x));
}

void big_set_str(BigInt *x, const char *str, int base)
{
    assert(big_ok(x) && str && 2 <= base && base <= 36);

    big_set_ui(x, 0);

    for (char c = *str; c; c = *(++str)) {
        const char digit = '0' <= c && c <= '9' ? c - '0'
            : 'a' <= c && c <= 'z' ? c - 'a'
            : '\0';
        big_add_ui(x, (unsigned long)digit);
        big_mul(x, base);
    }

    assert(big_ok(x));
}

BigInt big_init_set(const BigInt *y)
{
    BigInt x = big_init();
    big_set(&x, y);
    return x;
}

BigInt big_init_set_ui(unsigned long y)
{
    BigInt x = big_init();
    big_set_ui(&x, y);
    return x;
}

BigInt big_init_set_str(const char *str, int base)
{
    BigInt x = big_init();
    big_set_str(x, str, base);
    return x;
}

unsigned long big_get_ui(const BigInt *x)
{
    assert(big_ok(x) && x->count == 1);
    return x->digits[0];
}

char *big_get_str(const BigInt *x, int base)
{
    static const char symbols[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    assert(big_ok(x) && 2 <= base && base < sizeof(symbols));

    size_t allocated = 16, idx = 0;
    char *str = malloc(allocated);
    BigInt tail = big_init_set(x);

    while (!big_eq(&tail, 0)) {
        str[idx++] = symbols[big_div(&tail, base)];

        if (idx >= allocated) {
            allocated *= 2;
            str = realloc(str, allocated);
        }
    }

    str[idx] = '\0';
    return str;
}

// Compares a BigInt to a digit
bool big_eq(const BigInt *x, unsigned long y)
{
    assert(big_ok(x));
    return x->count == 1 && x->digits[0] == y;
}

void big_debug(const BigInt *x)
{
    assert(big_ok(x));
    putchar('{');

    printf("count: %d, reserved: %d, digits: (", x->count, x->reserved);

    for (int i = 0; i < x->count; i++) {
        printf("%" PRIx64, x->digits[i]);
        putchar(i == x->count - 1 ? ')' : ',');
    }

    putchar('}');
}

// Print a BigInt in given base, with shown in reverse, and returns the number of digits.
unsigned big_print(const BigInt *x, unsigned long base)
{
    assert(big_ok(x) && base < sizeof(symbols));

    BigInt y = big_new(0);
    big_copy(&y, x);

    unsigned count = 0;

    while (!big_eq(&y, 0)) {
        const unsigned long r = big_div(&y, base);
        assert(r < base);
        putchar(symbols[r]);
        count++;
    }

    big_del(&y);

    return max(1u, count);  // 0 still counts as 1 digit
}

void big_add(BigInt *r, const BigInt *x, const BigInt *y)
{
    assert(big_ok(r) && big_ok(x) && big_ok(y));
    big_resize(r, max(x->count, y->count));

    unsigned long carry = 0;

    for (int i = 0; i < r->count; i++) {
        const container_t sum = (container_t)(i < x->count ? x->digits[i] : 0)
            + (container_t)(i < y->count ? y->digits[i] : 0)
            + (container_t)carry;
        r->digits[i] = (unsigned long)sum;
        carry = (unsigned long)(sum >> (sizeof(long) * 8));
    }

    if (carry) {
        big_resize(r, r->count + 1);
        r->digits[r->count - 1] = carry;
    }

    assert(big_ok(r));
}

void big_add_ui(BigInt *r, const BigInt *x, unsigned long y)
{
    assert(big_ok(r) && big_ok(x));

    // TODO: This can done faster. For now, focus on correctness.
    BigInt yBig = big_init_set_ui(y);
    big_add(r, x, &yBig);
    big_del(&yBig);
    big_ok(r);
}

void big_sub(BigInt *r, const BigInt *x, const BigInt *y)
{
    assert(big_ok(r) && big_ok(x) && big_ok(y));
    // TODO
}

void big_sub_ui(BigInt *r, const BigInt *x, unsigned long y)
{
    assert(big_ok(r) && big_ok(x));

    // TODO: This can done faster. For now, focus on correctness.
    BigInt yBig = big_init_set_ui(y);
    big_sub(r, x, &yBig);
    big_del(&yBig);
    big_ok(r);
}

void big_mul(BigInt *r, const BigInt *x, const BigInt *y)
{
    assert(big_ok(r) && big_ok(x) && big_ok(y));
    // TODO
}

void big_mul_ui(BigInt *r, const BigInt *x, unsigned long y)
{
    assert(big_ok(r) && big_ok(x));
    big_resize(r, x->count);

    unsigned long carry = 0;

    for (int i = 0; i < x->count; i++) {
        const container_t product = (container_t)x->digits[i] * (container_t)y
            + (container_t)carry;
        r->digits[i] = (unsigned long)product;
        carry = (unsigned long)(product >> (sizeof(long) * 8));
    }

    if (carry) {
        big_resize(r, r->count + 1);
        r->digits[r->count - 1] = carry;
    }

    assert(big_ok(r));
}

void big_div(BigInt *q, BigInt *r, const BigInt *x, const BigInt *y)
{
    // TODO
}

unsigned long big_div_ui(BigInt *q, const BigInt *x, unsigned long y)
{
    assert(big_ok(q) && big_ok(x) && y);
    big_resize(q, x);

    unsigned long carry = 0;

    for (int i = x->count - 1; i >= 0; i--) {
        const container_t numerator = (container_t)x->digits[i]
            + ((container_t)carry << (sizeof(long) * 8));
        q->digits[i] = (unsigned long)(numerator / y);
        carry = (unsigned long)(numerator % y);
    }

    if (!q->digits[x->count - 1])
        big_resize(q, max(1, q->count - 1));

    assert(big_ok(q));
    return carry;
}
