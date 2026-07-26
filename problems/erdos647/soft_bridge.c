#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Exact bounded experiment for the relaxed-to-full bridge in Erdős #647.
 *
 * For a fixed K and n > K, the full condition splits as
 *
 *   tau(n-j) <= j+2 for 1 <= j <= K
 *   max_{m <= n-K-1} (m + tau(m)) <= n+2.
 *
 * Divisor counts are produced by a segmented multiplicative sieve. The
 * analyzer retains only a short ring of divisor counts and prefix maxima, so
 * memory is O(block_size + sqrt(limit) + max(K)).
 */

enum {
  WINDOW_COUNT = 21,
  MAX_WINDOW = 256,
};
static const uint32_t WINDOWS[WINDOW_COUNT] = {
    1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
    12, 13, 14, 15, 16, 24, 32, 64, 128, 256};
static const uint64_t DEFAULT_LIMIT = UINT64_C(10000000);
static const uint64_t DEFAULT_BLOCK_SIZE = UINT64_C(1048576);

typedef struct {
  uint32_t k;
  uint64_t local_count;
  uint64_t bridge_count;
  int64_t best_defect;
  uint64_t best_n;
  uint64_t best_blocker;
  uint32_t best_blocker_tau;
  uint64_t last_local_n;
  uint64_t tail_local_count;
  int64_t tail_best_defect;
  uint64_t tail_best_n;
  uint64_t tail_best_blocker;
  uint32_t tail_best_blocker_tau;
} WindowStats;

typedef struct {
  uint64_t limit;
  uint64_t block_size;
  uint32_t trace_window;
  bool self_test;
} Options;

static void die(const char *message) {
  fprintf(stderr, "error: %s\n", message);
  exit(EXIT_FAILURE);
}

static void *xcalloc(size_t count, size_t size) {
  void *pointer = calloc(count, size);
  if (pointer == NULL) {
    die("allocation failed");
  }
  return pointer;
}

static uint64_t parse_u64(const char *text, const char *option) {
  char *end = NULL;
  errno = 0;
  unsigned long long value = strtoull(text, &end, 10);
  if (errno != 0 || end == text || *end != '\0') {
    fprintf(stderr, "error: invalid value for %s: %s\n", option, text);
    exit(EXIT_FAILURE);
  }
  return (uint64_t)value;
}

static Options parse_options(int argc, char **argv) {
  Options options = {
      .limit = DEFAULT_LIMIT,
      .block_size = DEFAULT_BLOCK_SIZE,
      .trace_window = 0,
      .self_test = false,
  };

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--limit") == 0 && i + 1 < argc) {
      options.limit = parse_u64(argv[++i], "--limit");
    } else if (strcmp(argv[i], "--block-size") == 0 && i + 1 < argc) {
      options.block_size = parse_u64(argv[++i], "--block-size");
    } else if (strcmp(argv[i], "--trace-window") == 0 && i + 1 < argc) {
      uint64_t value = parse_u64(argv[++i], "--trace-window");
      if (value > MAX_WINDOW) {
        die("--trace-window exceeds the largest analyzed window");
      }
      options.trace_window = (uint32_t)value;
    } else if (strcmp(argv[i], "--self-test") == 0) {
      options.self_test = true;
    } else if (strcmp(argv[i], "--help") == 0) {
      puts("usage: soft_bridge [--limit N] [--block-size N] "
           "[--trace-window K] [--self-test]");
      exit(EXIT_SUCCESS);
    } else {
      fprintf(stderr, "error: unknown or incomplete option: %s\n", argv[i]);
      exit(EXIT_FAILURE);
    }
  }

  if (options.limit < 24) {
    die("--limit must be at least 24");
  }
  if (options.block_size == 0 || options.block_size > SIZE_MAX) {
    die("--block-size is outside the supported range");
  }
  if (options.limit == UINT64_MAX) {
    die("--limit is too large");
  }
  if (options.limit > (uint64_t)INT64_MAX / 2) {
    die("--limit exceeds the analyzer's signed-defect range");
  }
  return options;
}

static uint32_t *primes_through(uint64_t bound, size_t *prime_count) {
  if (bound > UINT32_MAX) {
    die("sqrt(limit) exceeds the supported prime-sieve range");
  }

  size_t size = (size_t)bound + 1;
  uint8_t *composite = xcalloc(size, sizeof(*composite));
  for (uint64_t p = 2; p * p <= bound; p++) {
    if (composite[p] != 0) {
      continue;
    }
    for (uint64_t multiple = p * p; multiple <= bound; multiple += p) {
      composite[multiple] = 1;
    }
  }

  size_t count = 0;
  for (uint64_t p = 2; p <= bound; p++) {
    if (composite[p] == 0) {
      count++;
    }
  }

  uint32_t *primes = xcalloc(count == 0 ? 1 : count, sizeof(*primes));
  size_t index = 0;
  for (uint64_t p = 2; p <= bound; p++) {
    if (composite[p] == 0) {
      primes[index++] = (uint32_t)p;
    }
  }

  free(composite);
  *prime_count = count;
  return primes;
}

static uint32_t naive_tau(uint64_t n) {
  uint32_t count = 0;
  for (uint64_t d = 1; d <= n / d; d++) {
    if (n % d == 0) {
      count += (d == n / d) ? 1U : 2U;
    }
  }
  return count;
}

static bool expected_small_solution(uint64_t n) {
  switch (n) {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 8:
  case 10:
  case 12:
  case 24:
    return true;
  default:
    return false;
  }
}

static int decimal_bucket(uint64_t n) {
  int bucket = 0;
  while (n >= 10) {
    n /= 10;
    bucket++;
  }
  return bucket;
}

static void initialize_stats(WindowStats stats[WINDOW_COUNT]) {
  for (size_t i = 0; i < WINDOW_COUNT; i++) {
    stats[i].k = WINDOWS[i];
    stats[i].best_defect = INT64_MAX;
    stats[i].tail_best_defect = INT64_MAX;
  }
}

static void update_window_stats(WindowStats *stats, uint64_t n,
                                uint64_t tail_start, uint64_t old_max,
                                uint64_t old_arg, uint32_t old_arg_tau) {
  int64_t defect = (int64_t)old_max - (int64_t)n - 2;
  stats->local_count++;
  stats->last_local_n = n;
  if (defect <= 0) {
    stats->bridge_count++;
  }
  if (defect < stats->best_defect) {
    stats->best_defect = defect;
    stats->best_n = n;
    stats->best_blocker = old_arg;
    stats->best_blocker_tau = old_arg_tau;
  }

  if (n >= tail_start) {
    stats->tail_local_count++;
    if (defect < stats->tail_best_defect) {
      stats->tail_best_defect = defect;
      stats->tail_best_n = n;
      stats->tail_best_blocker = old_arg;
      stats->tail_best_blocker_tau = old_arg_tau;
    }
  }
}

static void print_stats(const WindowStats stats[WINDOW_COUNT],
                        uint64_t tail_start) {
  printf("\nRELAXED WINDOWS (n > 24)\n");
  printf("K\tlocal\tbridged\tbest_defect\tbest_n\tblocker_m\t"
         "tau(blocker)\tlast_local\n");
  for (size_t i = 0; i < WINDOW_COUNT; i++) {
    const WindowStats *row = &stats[i];
    printf("%" PRIu32 "\t%" PRIu64 "\t%" PRIu64 "\t", row->k,
           row->local_count, row->bridge_count);
    if (row->local_count == 0) {
      printf("NA\tNA\tNA\tNA\tNA\n");
    } else {
      printf("%" PRId64 "\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu32
             "\t%" PRIu64 "\n",
             row->best_defect, row->best_n, row->best_blocker,
             row->best_blocker_tau, row->last_local_n);
    }
  }

  printf("\nTAIL RELAXED WINDOWS (n >= %" PRIu64 ")\n", tail_start);
  printf("K\tlocal\tbest_defect\tbest_n\tblocker_m\ttau(blocker)\n");
  for (size_t i = 0; i < WINDOW_COUNT; i++) {
    const WindowStats *row = &stats[i];
    printf("%" PRIu32 "\t%" PRIu64 "\t", row->k, row->tail_local_count);
    if (row->tail_local_count == 0) {
      printf("NA\tNA\tNA\tNA\n");
    } else {
      printf("%" PRId64 "\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu32 "\n",
             row->tail_best_defect, row->tail_best_n,
             row->tail_best_blocker, row->tail_best_blocker_tau);
    }
  }
}

int main(int argc, char **argv) {
  Options options = parse_options(argc, argv);
  uint64_t factor_limit = options.limit - 1;
  uint64_t sqrt_limit = (uint64_t)sqrtl((long double)factor_limit);
  while ((sqrt_limit + 1) <= factor_limit / (sqrt_limit + 1)) {
    sqrt_limit++;
  }
  while (sqrt_limit > factor_limit / sqrt_limit) {
    sqrt_limit--;
  }

  size_t prime_count = 0;
  uint32_t *primes = primes_through(sqrt_limit, &prime_count);
  size_t block_capacity = (size_t)options.block_size;
  uint64_t *remainders = xcalloc(block_capacity, sizeof(*remainders));
  uint32_t *divisor_counts = xcalloc(block_capacity, sizeof(*divisor_counts));

  const size_t ring_size = (size_t)MAX_WINDOW + 2;
  uint32_t tau_ring[MAX_WINDOW + 2] = {0};
  uint64_t prefix_max_ring[MAX_WINDOW + 2] = {0};
  uint64_t prefix_arg_ring[MAX_WINDOW + 2] = {0};
  uint32_t prefix_arg_tau_ring[MAX_WINDOW + 2] = {0};

  WindowStats stats[WINDOW_COUNT] = {0};
  initialize_stats(stats);

  uint64_t prefix_max = 0;
  uint64_t prefix_arg = 0;
  uint32_t prefix_arg_tau = 0;
  uint64_t small_solution_count = 1; /* n = 1, with an empty prefix */
  uint64_t witness_count = 0;
  uint64_t tail_start = options.limit / 10;
  if (tail_start < 25) {
    tail_start = 25;
  }

  int64_t decade_best[20];
  uint64_t decade_best_n[20] = {0};
  for (size_t i = 0; i < 20; i++) {
    decade_best[i] = INT64_MAX;
  }

  printf("ERDOS 647 SOFT-BRIDGE ANALYZER\n");
  printf("limit_n=%" PRIu64 "\n", options.limit);
  printf("block_size=%" PRIu64 "\n", options.block_size);
  printf("prime_count=%zu (through %" PRIu64 ")\n", prime_count, sqrt_limit);
  printf("full_solutions=1");

  for (uint64_t low = 1; low <= factor_limit;) {
    uint64_t high = low + options.block_size - 1;
    if (high < low || high > factor_limit) {
      high = factor_limit;
    }
    size_t length = (size_t)(high - low + 1);

    for (size_t i = 0; i < length; i++) {
      remainders[i] = low + (uint64_t)i;
      divisor_counts[i] = 1;
    }

    for (size_t pi = 0; pi < prime_count; pi++) {
      uint64_t p = primes[pi];
      if (p > high / p) {
        break;
      }
      uint64_t first = ((low + p - 1) / p) * p;
      for (uint64_t value = first; value <= high;) {
        size_t index = (size_t)(value - low);
        uint32_t exponent = 0;
        while (remainders[index] % p == 0) {
          remainders[index] /= p;
          exponent++;
        }
        divisor_counts[index] *= exponent + 1;
        if (high - value < p) {
          break;
        }
        value += p;
      }
    }

    for (size_t i = 0; i < length; i++) {
      uint64_t m = low + (uint64_t)i;
      if (remainders[i] > 1) {
        divisor_counts[i] *= 2;
      }
      uint32_t tau_m = divisor_counts[i];

      if (options.self_test && m <= 10000 && tau_m != naive_tau(m)) {
        fprintf(stderr,
                "self-test failed: tau(%" PRIu64 ")=%" PRIu32
                ", expected %" PRIu32 "\n",
                m, tau_m, naive_tau(m));
        return EXIT_FAILURE;
      }

      size_t slot = (size_t)(m % ring_size);
      tau_ring[slot] = tau_m;

      uint64_t score = m + (uint64_t)tau_m;
      if (score > prefix_max) {
        prefix_max = score;
        prefix_arg = m;
        prefix_arg_tau = tau_m;
      }
      prefix_max_ring[slot] = prefix_max;
      prefix_arg_ring[slot] = prefix_arg;
      prefix_arg_tau_ring[slot] = prefix_arg_tau;

      uint64_t n = m + 1;
      bool full = prefix_max <= n + 2;
      if (full) {
        printf(",%" PRIu64, n);
        if (n <= 24) {
          small_solution_count++;
        }
        if (n > 24) {
          witness_count++;
          printf("\nWITNESS n=%" PRIu64 "\n", n);
        }
      }
      if (options.self_test && n <= 24 && full != expected_small_solution(n)) {
        fprintf(stderr, "self-test failed: wrong full verdict for n=%" PRIu64
                        "\n",
                n);
        return EXIT_FAILURE;
      }

      if (n > 24) {
        int bucket = decimal_bucket(n);
        int64_t excess = (int64_t)prefix_max - (int64_t)n - 2;
        if (bucket < 20 && excess < decade_best[bucket]) {
          decade_best[bucket] = excess;
          decade_best_n[bucket] = n;
        }
      }

      uint32_t available = n - 1 < MAX_WINDOW ? (uint32_t)(n - 1)
                                               : MAX_WINDOW;
      uint32_t first_bad = 0;
      for (uint32_t j = 1; j <= available; j++) {
        size_t tau_slot = (size_t)((n - j) % ring_size);
        if (tau_ring[tau_slot] > j + 2) {
          first_bad = j;
          break;
        }
      }

      for (size_t wi = 0; wi < WINDOW_COUNT; wi++) {
        uint32_t k = stats[wi].k;
        if (n <= 24 || n <= k || (first_bad != 0 && first_bad <= k)) {
          continue;
        }

        uint64_t historical_end = n - k - 1;
        size_t historical_slot = (size_t)(historical_end % ring_size);
        if (options.trace_window == k) {
          int64_t defect =
              (int64_t)prefix_max_ring[historical_slot] - (int64_t)n - 2;
          printf("\nTRACE K=%" PRIu32 " n=%" PRIu64 " defect=%" PRId64
                 " blocker_m=%" PRIu64 " blocker_tau=%" PRIu32 "\n",
                 k, n, defect, prefix_arg_ring[historical_slot],
                 prefix_arg_tau_ring[historical_slot]);
        }
        update_window_stats(
            &stats[wi], n, tail_start, prefix_max_ring[historical_slot],
            prefix_arg_ring[historical_slot],
            prefix_arg_tau_ring[historical_slot]);
      }
    }

    if (high == factor_limit) {
      break;
    }
    low = high + 1;
  }

  putchar('\n');
  if (options.self_test && small_solution_count != 10) {
    fprintf(stderr,
            "self-test failed: found %" PRIu64
            " full solutions through 24, expected 10\n",
            small_solution_count);
    return EXIT_FAILURE;
  }
  if (options.self_test) {
    puts("SELF_TEST: PASS (tau(1..10000), known solutions through 24)");
  }

  printf("witnesses_above_24=%" PRIu64 "\n", witness_count);
  printf("\nGLOBAL EXCESS MINIMA BY DECIMAL BUCKET\n");
  printf("bucket\tminimum_R(n-1)-(n+2)\tn\n");
  for (size_t i = 1; i < 20; i++) {
    if (decade_best[i] != INT64_MAX) {
      printf("10^%zu..10^%zu-1\t%" PRId64 "\t%" PRIu64 "\n", i, i + 1,
             decade_best[i], decade_best_n[i]);
    }
  }

  print_stats(stats, tail_start);

  free(divisor_counts);
  free(remainders);
  free(primes);
  return EXIT_SUCCESS;
}
