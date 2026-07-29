#include "rational.h"

#include <stdio.h>
#include <stdlib.h>

static int64_t checked_mul(int64_t left, int64_t right) {
  int64_t product = 0;
  if (__builtin_mul_overflow(left, right, &product)) {
    abort();
  }
  return product;
}

static int64_t checked_add(int64_t left, int64_t right) {
  int64_t sum = 0;
  if (__builtin_add_overflow(left, right, &sum)) {
    abort();
  }
  return sum;
}

static int64_t greatest_common_divisor(int64_t left, int64_t right) {
  if (left < 0) {
    left = -left;
  }
  if (right < 0) {
    right = -right;
  }
  while (right != 0) {
    int64_t remainder = left % right;
    left = right;
    right = remainder;
  }
  return left;
}

Rational rational_make(int64_t num, int64_t den) {
  if (den == 0) {
    abort();
  }
  if (den < 0) {
    if (num == INT64_MIN || den == INT64_MIN) {
      abort();
    }
    num = -num;
    den = -den;
  }
  int64_t divisor = greatest_common_divisor(num, den);
  if (divisor > 1) {
    num /= divisor;
    den /= divisor;
  }
  Rational value = {.num = num, .den = den};
  return value;
}

Rational rational_from_int(int64_t value) { return rational_make(value, 1); }

Rational rational_add(Rational left, Rational right) {
  int64_t num = checked_add(checked_mul(left.num, right.den),
                            checked_mul(right.num, left.den));
  return rational_make(num, checked_mul(left.den, right.den));
}

Rational rational_sub(Rational left, Rational right) {
  Rational negated = {.num = -right.num, .den = right.den};
  return rational_add(left, negated);
}

Rational rational_mul(Rational left, Rational right) {
  return rational_make(checked_mul(left.num, right.num),
                       checked_mul(left.den, right.den));
}

Rational rational_div(Rational left, Rational right) {
  if (right.num == 0) {
    abort();
  }
  return rational_make(checked_mul(left.num, right.den),
                       checked_mul(left.den, right.num));
}

int rational_compare(Rational left, Rational right) {
  int64_t difference = checked_add(checked_mul(left.num, right.den),
                                   -checked_mul(right.num, left.den));
  return (difference > 0) - (difference < 0);
}

int rational_sign(Rational value) {
  return (value.num > 0) - (value.num < 0);
}

bool rational_is_integer(Rational value) { return value.den == 1; }

int64_t rational_floor(Rational value) {
  int64_t quotient = value.num / value.den;
  if (value.num % value.den != 0 && value.num < 0) {
    quotient--;
  }
  return quotient;
}

int64_t rational_ceil(Rational value) {
  int64_t quotient = value.num / value.den;
  if (value.num % value.den != 0 && value.num > 0) {
    quotient++;
  }
  return quotient;
}

void rational_format(Rational value, char *buffer, size_t size) {
  if (value.den == 1) {
    snprintf(buffer, size, "%lld", (long long)value.num);
  } else {
    snprintf(buffer, size, "%lld/%lld", (long long)value.num,
             (long long)value.den);
  }
}
