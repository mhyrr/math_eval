#ifndef WOW2_RATIONAL_H
#define WOW2_RATIONAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Exact rational arithmetic.
 *
 * Several WOWII statements carry genuine fractions: average eccentricity and
 * average local independence divide by the order, conjecture 142 multiplies
 * girth by 2/3, conjecture 100 halves a diameter. Evaluating those in floating
 * point would make the sweep's verdict depend on rounding, which is exactly the
 * failure mode Tao names for evolve-style systems gaming sloppy verifiers
 * (POOLS.md section 3). Every quantity in the registry is therefore rational
 * and every comparison is exact.
 *
 * Every operation normalizes to lowest terms with a positive denominator and
 * aborts on overflow rather than wrapping. With at most 63 vertices the values
 * stay tiny; the guard is there so a future caller cannot silently corrupt a
 * verdict.
 */
typedef struct {
  int64_t num;
  int64_t den;
} Rational;

Rational rational_make(int64_t num, int64_t den);
Rational rational_from_int(int64_t value);
Rational rational_add(Rational left, Rational right);
Rational rational_sub(Rational left, Rational right);
Rational rational_mul(Rational left, Rational right);
Rational rational_div(Rational left, Rational right);

/* Returns -1, 0, or 1 as left is less than, equal to, or greater than right. */
int rational_compare(Rational left, Rational right);
int rational_sign(Rational value);
bool rational_is_integer(Rational value);

int64_t rational_floor(Rational value);
int64_t rational_ceil(Rational value);

/* Writes "3" for integers and "7/2" otherwise. */
void rational_format(Rational value, char *buffer, size_t size);

#endif
