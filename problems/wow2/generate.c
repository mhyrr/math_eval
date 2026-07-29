#include "graph.h"

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  MAX_GENERATED_ORDER = 10,
};

/*
 * McKay's published counts of connected unlabelled simple graphs, used to
 * check every intermediate order rather than trusting the augmentation:
 * https://users.cecs.anu.edu.au/~bdm/data/graphs.html
 */
static const uint64_t CONNECTED_UNLABELED_COUNTS[MAX_GENERATED_ORDER + 1] = {
    0, 1, 1, 2, 6, 21, 112, 853, 11117, 261080, 11716571,
};

typedef struct {
  uint64_t *keys;
  size_t capacity;
  size_t count;
} CodeSet;

typedef struct {
  const Graph *graph;
  unsigned vertices[WOW2_MAX_VERTICES];
  unsigned cell_start[WOW2_MAX_VERTICES + 1];
  unsigned cell_count;
  uint64_t best;
  bool have_best;
} CanonicalContext;

typedef struct {
  unsigned old_color;
  unsigned counts[WOW2_MAX_VERTICES];
  unsigned vertex;
} ColorSignature;

static unsigned signature_color_count = 0;

static void die(const char *message) {
  fprintf(stderr, "error: %s\n", message);
  exit(EXIT_FAILURE);
}

static void *xmalloc(size_t size) {
  void *pointer = malloc(size);
  if (pointer == NULL) {
    die("allocation failed");
  }
  return pointer;
}

static uint64_t mix64(uint64_t value) {
  value += UINT64_C(0x9e3779b97f4a7c15);
  value = (value ^ (value >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
  value = (value ^ (value >> 27)) * UINT64_C(0x94d049bb133111eb);
  return value ^ (value >> 31);
}

static void code_set_init(CodeSet *set, size_t capacity) {
  set->capacity = 1;
  while (set->capacity < capacity) {
    set->capacity *= 2;
  }
  set->count = 0;
  set->keys = xmalloc(set->capacity * sizeof(*set->keys));
  for (size_t i = 0; i < set->capacity; i++) {
    set->keys[i] = UINT64_MAX;
  }
}

static void code_set_destroy(CodeSet *set) {
  free(set->keys);
  set->keys = NULL;
  set->capacity = 0;
  set->count = 0;
}

static void code_set_insert_without_growth(CodeSet *set, uint64_t key) {
  size_t index = (size_t)mix64(key) & (set->capacity - 1);
  while (set->keys[index] != UINT64_MAX) {
    if (set->keys[index] == key) {
      return;
    }
    index = (index + 1) & (set->capacity - 1);
  }
  set->keys[index] = key;
  set->count++;
}

static void code_set_grow(CodeSet *set) {
  CodeSet larger;
  code_set_init(&larger, set->capacity * 2);
  for (size_t i = 0; i < set->capacity; i++) {
    if (set->keys[i] != UINT64_MAX) {
      code_set_insert_without_growth(&larger, set->keys[i]);
    }
  }
  code_set_destroy(set);
  *set = larger;
}

static void code_set_insert(CodeSet *set, uint64_t key) {
  if ((set->count + 1) * 10 >= set->capacity * 7) {
    code_set_grow(set);
  }
  code_set_insert_without_growth(set, key);
}

static int compare_u64(const void *left, const void *right) {
  uint64_t a = *(const uint64_t *)left;
  uint64_t b = *(const uint64_t *)right;
  return (a > b) - (a < b);
}

static uint64_t *code_set_sorted_values(const CodeSet *set) {
  uint64_t *values = xmalloc(set->count * sizeof(*values));
  size_t at = 0;
  for (size_t i = 0; i < set->capacity; i++) {
    if (set->keys[i] != UINT64_MAX) {
      values[at++] = set->keys[i];
    }
  }
  qsort(values, set->count, sizeof(*values), compare_u64);
  return values;
}

static unsigned popcount64(uint64_t value) {
  return (unsigned)__builtin_popcountll(value);
}

static void graph_from_code(Graph *graph, unsigned n, uint64_t code) {
  graph_clear(graph, n);
  unsigned bit = 0;
  for (unsigned j = 1; j < n; j++) {
    for (unsigned i = 0; i < j; i++) {
      if ((code & (UINT64_C(1) << bit)) != 0) {
        graph_add_edge(graph, i, j);
      }
      bit++;
    }
  }
}

static uint64_t graph_code_under_permutation(
    const Graph *graph, const unsigned permutation[WOW2_MAX_VERTICES]) {
  uint64_t code = 0;
  unsigned bit = 0;
  for (unsigned j = 1; j < graph->n; j++) {
    for (unsigned i = 0; i < j; i++) {
      if (graph_has_edge(graph, permutation[i], permutation[j])) {
        code |= UINT64_C(1) << bit;
      }
      bit++;
    }
  }
  return code;
}

static int compare_signature(const void *left, const void *right) {
  const ColorSignature *a = left;
  const ColorSignature *b = right;
  if (a->old_color != b->old_color) {
    return (a->old_color > b->old_color) -
           (a->old_color < b->old_color);
  }
  for (unsigned color = 0; color < signature_color_count; color++) {
    if (a->counts[color] != b->counts[color]) {
      return (a->counts[color] > b->counts[color]) -
             (a->counts[color] < b->counts[color]);
    }
  }
  return 0;
}

static bool same_signature(const ColorSignature *a, const ColorSignature *b,
                           unsigned color_count) {
  if (a->old_color != b->old_color) {
    return false;
  }
  for (unsigned color = 0; color < color_count; color++) {
    if (a->counts[color] != b->counts[color]) {
      return false;
    }
  }
  return true;
}

static unsigned stable_colors(const Graph *graph,
                              unsigned colors[WOW2_MAX_VERTICES]) {
  unsigned degrees[WOW2_MAX_VERTICES];
  unsigned ordered_degrees[WOW2_MAX_VERTICES];
  for (unsigned u = 0; u < graph->n; u++) {
    degrees[u] = popcount64(graph->adj[u]);
    ordered_degrees[u] = degrees[u];
  }
  for (unsigned i = 0; i < graph->n; i++) {
    for (unsigned j = i + 1; j < graph->n; j++) {
      if (ordered_degrees[j] < ordered_degrees[i]) {
        unsigned swap = ordered_degrees[i];
        ordered_degrees[i] = ordered_degrees[j];
        ordered_degrees[j] = swap;
      }
    }
  }

  unsigned color_count = 0;
  unsigned previous_degree = UINT32_MAX;
  for (unsigned i = 0; i < graph->n; i++) {
    if (i == 0 || ordered_degrees[i] != previous_degree) {
      previous_degree = ordered_degrees[i];
      color_count++;
    }
    for (unsigned u = 0; u < graph->n; u++) {
      if (degrees[u] == ordered_degrees[i]) {
        colors[u] = color_count - 1;
      }
    }
  }

  for (;;) {
    ColorSignature signatures[WOW2_MAX_VERTICES] = {0};
    for (unsigned u = 0; u < graph->n; u++) {
      signatures[u].old_color = colors[u];
      signatures[u].vertex = u;
      uint64_t neighbors = graph->adj[u];
      while (neighbors != 0) {
        unsigned v = (unsigned)__builtin_ctzll(neighbors);
        neighbors &= neighbors - 1;
        signatures[u].counts[colors[v]]++;
      }
    }

    signature_color_count = color_count;
    qsort(signatures, graph->n, sizeof(*signatures), compare_signature);
    unsigned new_colors[WOW2_MAX_VERTICES];
    unsigned new_count = 1;
    new_colors[signatures[0].vertex] = 0;
    for (unsigned i = 1; i < graph->n; i++) {
      if (!same_signature(&signatures[i - 1], &signatures[i],
                          color_count)) {
        new_count++;
      }
      new_colors[signatures[i].vertex] = new_count - 1;
    }
    memcpy(colors, new_colors, graph->n * sizeof(*colors));
    if (new_count == color_count) {
      return color_count;
    }
    color_count = new_count;
  }
}

static void canonical_permute_cell(CanonicalContext *context,
                                   unsigned cell_index, unsigned position) {
  unsigned start = context->cell_start[cell_index];
  unsigned end = context->cell_start[cell_index + 1];
  if (position == end) {
    if (cell_index + 1 == context->cell_count) {
      uint64_t code =
          graph_code_under_permutation(context->graph, context->vertices);
      if (!context->have_best || code < context->best) {
        context->best = code;
        context->have_best = true;
      }
      return;
    }
    canonical_permute_cell(context, cell_index + 1,
                           context->cell_start[cell_index + 1]);
    return;
  }

  for (unsigned i = position; i < end; i++) {
    unsigned swap = context->vertices[position];
    context->vertices[position] = context->vertices[i];
    context->vertices[i] = swap;
    canonical_permute_cell(context, cell_index, position + 1);
    swap = context->vertices[position];
    context->vertices[position] = context->vertices[i];
    context->vertices[i] = swap;
  }
  (void)start;
}

static uint64_t canonical_code(const Graph *graph) {
  if (graph->n <= 1) {
    return 0;
  }

  unsigned colors[WOW2_MAX_VERTICES] = {0};
  unsigned color_count = stable_colors(graph, colors);
  CanonicalContext context = {
      .graph = graph,
      .cell_count = color_count,
      .best = 0,
      .have_best = false,
  };

  unsigned at = 0;
  for (unsigned color = 0; color < color_count; color++) {
    context.cell_start[color] = at;
    for (unsigned u = 0; u < graph->n; u++) {
      if (colors[u] == color) {
        context.vertices[at++] = u;
      }
    }
  }
  context.cell_start[color_count] = at;
  canonical_permute_cell(&context, 0, 0);
  return context.best;
}

static uint64_t *generate_order(const uint64_t *parents, size_t parent_count,
                                unsigned n, size_t *result_count) {
  CodeSet children;
  size_t initial_capacity = parent_count < 1024 ? 1024 : parent_count * 4;
  code_set_init(&children, initial_capacity);

  unsigned old_n = n - 1;
  uint64_t subset_limit = UINT64_C(1) << old_n;
  for (size_t parent_index = 0; parent_index < parent_count; parent_index++) {
    Graph parent;
    graph_from_code(&parent, old_n, parents[parent_index]);
    for (uint64_t neighbors = 1; neighbors < subset_limit; neighbors++) {
      Graph child = parent;
      child.n = (uint8_t)n;
      child.adj[old_n] = neighbors;
      uint64_t scan = neighbors;
      while (scan != 0) {
        unsigned u = (unsigned)__builtin_ctzll(scan);
        scan &= scan - 1;
        child.adj[u] |= UINT64_C(1) << old_n;
      }
      code_set_insert(&children, canonical_code(&child));
    }
  }

  uint64_t *result = code_set_sorted_values(&children);
  *result_count = children.count;
  code_set_destroy(&children);
  return result;
}

static unsigned parse_order(const char *text) {
  char *end = NULL;
  errno = 0;
  unsigned long value = strtoul(text, &end, 10);
  if (errno != 0 || end == text || *end != '\0' ||
      value < 1 || value > MAX_GENERATED_ORDER) {
    die("order must be an integer from 1 through 10");
  }
  return (unsigned)value;
}

static void usage(void) {
  puts("usage: wow2-generate [--count-only] [--self-test] ORDER\n"
       "\n"
       "Generate one graph6 record for every connected unlabeled graph of\n"
       "the requested order. The native generator uses canonical vertex\n"
       "augmentation and validates every intermediate count against the\n"
       "published connected-unlabeled counts through order 10.\n"
       "\n"
       "--count-only  validate and report counts without graph6 output\n"
       "--self-test   generate through order 7 and suppress graph6 output");
}

int main(int argc, char **argv) {
  bool count_only = false;
  bool self_test = false;
  unsigned target = 0;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--count-only") == 0) {
      count_only = true;
    } else if (strcmp(argv[i], "--self-test") == 0) {
      self_test = true;
      count_only = true;
      target = 7;
    } else if (strcmp(argv[i], "--help") == 0) {
      usage();
      return EXIT_SUCCESS;
    } else if (target == 0) {
      target = parse_order(argv[i]);
    } else {
      die("unexpected argument");
    }
  }
  if (target == 0) {
    usage();
    return EXIT_FAILURE;
  }

  size_t count = 1;
  uint64_t *codes = xmalloc(sizeof(*codes));
  codes[0] = 0;
  fprintf(stderr, "order=1 connected_unlabeled=1 expected=1\n");

  for (unsigned n = 2; n <= target; n++) {
    size_t next_count = 0;
    uint64_t *next = generate_order(codes, count, n, &next_count);
    free(codes);
    codes = next;
    count = next_count;
    fprintf(stderr,
            "order=%u connected_unlabeled=%zu expected=%" PRIu64 "\n",
            n, count, CONNECTED_UNLABELED_COUNTS[n]);
    if (count != CONNECTED_UNLABELED_COUNTS[n]) {
      free(codes);
      die("canonical generator count disagrees with the known sequence");
    }
  }

  if (!count_only) {
    for (size_t i = 0; i < count; i++) {
      Graph graph;
      char graph6[384];
      graph_from_code(&graph, target, codes[i]);
      if (!graph6_encode(&graph, graph6, sizeof(graph6))) {
        free(codes);
        die("graph6 encoding failed");
      }
      puts(graph6);
    }
  }

  free(codes);
  if (self_test) {
    puts("self-test: PASS");
  }
  return EXIT_SUCCESS;
}
