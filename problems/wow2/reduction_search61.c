#include "reduction61.h"

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  unsigned order;
  uint64_t iterations;
  unsigned restarts;
  uint64_t seed;
  const char *start_graph6;
} Options;

typedef struct {
  Wow2Reduction61 reduction;
  unsigned residue;
  int diameter;
  unsigned edges;
} Evaluation;

static uint64_t random_state;

static void die(const char *message) {
  fprintf(stderr, "error: %s\n", message);
  exit(EXIT_FAILURE);
}

static uint64_t random_u64(void) {
  uint64_t value = random_state;
  value ^= value >> 12;
  value ^= value << 25;
  value ^= value >> 27;
  random_state = value;
  return value * UINT64_C(2685821657736338717);
}

static unsigned random_below(unsigned limit) {
  return (unsigned)(random_u64() % limit);
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
      .order = 16,
      .iterations = 100000,
      .restarts = 20,
      .seed = UINT64_C(0x61b0ddf00d),
      .start_graph6 = NULL,
  };
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--order") == 0 && i + 1 < argc) {
      options.order = (unsigned)parse_u64(argv[++i], "--order");
    } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
      options.iterations = parse_u64(argv[++i], "--iterations");
    } else if (strcmp(argv[i], "--restarts") == 0 && i + 1 < argc) {
      options.restarts = (unsigned)parse_u64(argv[++i], "--restarts");
    } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
      options.seed = parse_u64(argv[++i], "--seed");
    } else if (strcmp(argv[i], "--start") == 0 && i + 1 < argc) {
      options.start_graph6 = argv[++i];
    } else if (strcmp(argv[i], "--help") == 0) {
      puts("usage: wow2-reduction-search61 [--order N] [--iterations N]\n"
           "       [--restarts N] [--seed N] [--start GRAPH6]\n"
           "\n"
           "Searches for a connected graph of diameter at least four whose\n"
           "WOWII 61 bound is larger than the sum of the component bounds\n"
           "after every one-vertex deletion. Such a graph refutes the\n"
           "candidate induction lemma, not WOWII 61 itself.");
      exit(EXIT_SUCCESS);
    } else {
      die("unknown or incomplete option");
    }
  }
  if (options.order < 5 || options.order > 50) {
    die("--order must be from 5 through 50");
  }
  if (options.iterations == 0 || options.restarts == 0) {
    die("--iterations and --restarts must be positive");
  }
  if (options.seed == 0) {
    options.seed = 1;
  }
  if (options.start_graph6 != NULL) {
    Graph start;
    char error[128];
    if (!graph6_parse(options.start_graph6, &start, error, sizeof(error))) {
      fprintf(stderr, "error: invalid --start graph6: %s\n", error);
      exit(EXIT_FAILURE);
    }
    if (start.n < 5 || start.n > 50 || !graph_connected(&start)) {
      die("--start must be a connected graph of order 5 through 50");
    }
    options.order = start.n;
  }
  return options;
}

static void random_pair(unsigned n, unsigned *left, unsigned *right) {
  *left = random_below(n);
  do {
    *right = random_below(n);
  } while (*left == *right);
  if (*left > *right) {
    unsigned swap = *left;
    *left = *right;
    *right = swap;
  }
}

static void random_connected_graph(Graph *graph, unsigned n) {
  graph_clear(graph, n);
  for (unsigned vertex = 1; vertex < n; vertex++) {
    graph_add_edge(graph, vertex, random_below(vertex));
  }
  unsigned density = 5 + random_below(71);
  for (unsigned left = 0; left < n; left++) {
    for (unsigned right = left + 1; right < n; right++) {
      if (!graph_has_edge(graph, left, right) &&
          random_below(100) < density) {
        graph_add_edge(graph, left, right);
      }
    }
  }
}

static void block_chain_graph(Graph *graph, unsigned n) {
  graph_clear(graph, n);
  unsigned maximum_blocks = n / 2;
  if (maximum_blocks > 8) {
    maximum_blocks = 8;
  }
  unsigned block_count =
      maximum_blocks <= 2
          ? maximum_blocks
          : 2 + random_below(maximum_blocks - 1);
  if (block_count == 0) {
    block_count = 1;
  }

  unsigned start = 0;
  for (unsigned block = 0; block < block_count; block++) {
    unsigned remaining = n - start;
    unsigned remaining_blocks = block_count - block;
    unsigned size = remaining / remaining_blocks;
    if (block + 1 == block_count) {
      size = remaining;
    }
    unsigned end = start + size;
    for (unsigned left = start; left < end; left++) {
      for (unsigned right = left + 1; right < end; right++) {
        if (random_below(100) < 80) {
          graph_add_edge(graph, left, right);
        }
      }
    }
    for (unsigned vertex = start + 1; vertex < end; vertex++) {
      if (!graph_has_edge(graph, start, vertex)) {
        graph_add_edge(graph, start, vertex);
      }
    }
    if (block > 0) {
      graph_add_edge(graph, start - 1, start);
    }
    start = end;
  }
}

static void layered_path_graph(Graph *graph, unsigned n) {
  graph_clear(graph, n);
  unsigned maximum_layers = n < 16 ? n / 2 : 10;
  if (maximum_layers < 4) {
    maximum_layers = 4;
  }
  unsigned layer_count = 4 + random_below(maximum_layers - 3);
  unsigned layer_of[WOW2_MAX_VERTICES];
  for (unsigned vertex = 0; vertex < n; vertex++) {
    layer_of[vertex] = (uint64_t)vertex * layer_count / n;
  }

  for (unsigned left = 0; left < n; left++) {
    for (unsigned right = left + 1; right < n; right++) {
      unsigned left_layer = layer_of[left];
      unsigned right_layer = layer_of[right];
      unsigned gap = left_layer > right_layer
                         ? left_layer - right_layer
                         : right_layer - left_layer;
      if (gap == 0 && random_below(100) < 75) {
        graph_add_edge(graph, left, right);
      } else if (gap == 1 && random_below(100) < 60) {
        graph_add_edge(graph, left, right);
      }
    }
  }
  for (unsigned layer = 0; layer + 1 < layer_count; layer++) {
    unsigned left = n;
    unsigned right = n;
    for (unsigned vertex = 0; vertex < n; vertex++) {
      if (layer_of[vertex] == layer && left == n) {
        left = vertex;
      }
      if (layer_of[vertex] == layer + 1 && right == n) {
        right = vertex;
      }
    }
    if (left < n && right < n &&
        !graph_has_edge(graph, left, right)) {
      graph_add_edge(graph, left, right);
    }
  }
}

static void initial_graph(Graph *graph, unsigned n, unsigned restart) {
  switch (restart % 3) {
  case 0:
    random_connected_graph(graph, n);
    break;
  case 1:
    block_chain_graph(graph, n);
    break;
  default:
    layered_path_graph(graph, n);
    break;
  }
  if (!graph_connected(graph)) {
    for (unsigned vertex = 1; vertex < n; vertex++) {
      if (!graph_has_edge(graph, vertex - 1, vertex)) {
        graph_add_edge(graph, vertex - 1, vertex);
      }
    }
  }
}

static Evaluation evaluate(const Graph *graph) {
  Evaluation evaluation = {
      .reduction = wow2_reduction61(graph),
      .residue = graph_residue(graph),
      .diameter = graph_diameter(graph),
      .edges = graph_edge_count(graph),
  };
  return evaluation;
}

static bool witness(const Evaluation *evaluation) {
  return evaluation->diameter >= 4 &&
         evaluation->reduction.margin < 0;
}

static int pressure(const Evaluation *evaluation) {
  if (evaluation->diameter < 4) {
    return -100000 + evaluation->diameter;
  }
  return -10000 * evaluation->reduction.margin -
         100 * (int)evaluation->reduction.reducible_vertices -
         evaluation->reduction.total_margin;
}

static bool better(const Evaluation *left, const Evaluation *right) {
  bool left_nonbase = left->diameter >= 4;
  bool right_nonbase = right->diameter >= 4;
  if (left_nonbase != right_nonbase) {
    return left_nonbase;
  }
  if (left->reduction.margin != right->reduction.margin) {
    return left->reduction.margin < right->reduction.margin;
  }
  if (left->reduction.reducible_vertices !=
      right->reduction.reducible_vertices) {
    return left->reduction.reducible_vertices <
           right->reduction.reducible_vertices;
  }
  if (left->reduction.total_margin !=
      right->reduction.total_margin) {
    return left->reduction.total_margin <
           right->reduction.total_margin;
  }
  if (left->diameter != right->diameter) {
    return left->diameter > right->diameter;
  }
  return left->reduction.bound > right->reduction.bound;
}

static void print_result(const char *label, const Graph *graph,
                         const Evaluation *evaluation, unsigned restart,
                         uint64_t iteration) {
  char graph6[384];
  if (!graph6_encode(graph, graph6, sizeof(graph6))) {
    die("graph6 encoding failed");
  }
  printf("%s restart=%u iteration=%" PRIu64
         " graph6=%s n=%u m=%u residue=%u diameter=%d bound=%u "
         "best_child_bound=%u margin=%d best_vertex=%u "
         "best_vertex_components=%u reducible_vertices=%u "
         "total_margin=%d pressure=%d edges=",
         label, restart, iteration, graph6, graph->n, evaluation->edges,
         evaluation->residue, evaluation->diameter,
         evaluation->reduction.bound,
         evaluation->reduction.best_child_bound,
         evaluation->reduction.margin,
         evaluation->reduction.best_vertex,
         evaluation->reduction.best_vertex_components,
         evaluation->reduction.reducible_vertices,
         evaluation->reduction.total_margin, pressure(evaluation));
  graph_print_edges(graph, stdout);
  fputc('\n', stdout);
  fflush(stdout);
}

static void graph_from_labeled_code(Graph *graph, unsigned n, uint64_t code) {
  graph_clear(graph, n);
  unsigned bit = 0;
  for (unsigned right = 1; right < n; right++) {
    for (unsigned left = 0; left < right; left++) {
      if ((code & (UINT64_C(1) << bit)) != 0) {
        graph_add_edge(graph, left, right);
      }
      bit++;
    }
  }
}

static void self_test(void) {
  uint64_t tested = 0;
  for (unsigned n = 2; n <= 6; n++) {
    unsigned edge_slots = n * (n - 1) / 2;
    uint64_t limit = UINT64_C(1) << edge_slots;
    for (uint64_t code = 0; code < limit; code++) {
      Graph graph;
      graph_from_labeled_code(&graph, n, code);
      if (!graph_connected(&graph)) {
        continue;
      }
      Evaluation evaluation = evaluate(&graph);
      if (evaluation.diameter >= 4 &&
          evaluation.reduction.margin < 0) {
        die("small-graph reduction self-test failed");
      }
      tested++;
    }
  }
  printf("self-test: PASS connected_labeled_graphs=%" PRIu64 "\n",
         tested);
}

int main(int argc, char **argv) {
  if (argc == 2 && strcmp(argv[1], "--self-test") == 0) {
    self_test();
    return EXIT_SUCCESS;
  }

  Options options = parse_options(argc, argv);
  random_state = options.seed;
  Graph global_best_graph;
  Evaluation global_best = {0};
  bool have_global_best = false;
  unsigned global_restart = 0;
  uint64_t global_iteration = 0;

  for (unsigned restart = 0; restart < options.restarts; restart++) {
    Graph current_graph;
    if (options.start_graph6 == NULL) {
      initial_graph(&current_graph, options.order, restart);
    } else {
      char error[128];
      if (!graph6_parse(options.start_graph6, &current_graph, error,
                        sizeof(error))) {
        die("internal --start parse failure");
      }
      unsigned kicks = restart == 0 ? 0 : 1 + restart % 7;
      for (unsigned kick = 0; kick < kicks; kick++) {
        unsigned left;
        unsigned right;
        random_pair(options.order, &left, &right);
        graph_toggle_edge(&current_graph, left, right);
        if (!graph_connected(&current_graph)) {
          graph_toggle_edge(&current_graph, left, right);
        }
      }
    }
    Evaluation current = evaluate(&current_graph);
    if (!have_global_best || better(&current, &global_best)) {
      have_global_best = true;
      global_best = current;
      global_best_graph = current_graph;
      global_restart = restart;
      global_iteration = 0;
      print_result("IMPROVEMENT", &global_best_graph, &global_best,
                   restart, 0);
    }
    if (witness(&current)) {
      print_result("WITNESS", &current_graph, &current, restart, 0);
      return EXIT_SUCCESS;
    }

    uint64_t stagnant = 0;
    for (uint64_t iteration = 1; iteration <= options.iterations;
         iteration++) {
      unsigned left;
      unsigned right;
      random_pair(options.order, &left, &right);
      Graph candidate_graph = current_graph;
      graph_toggle_edge(&candidate_graph, left, right);
      if (!graph_connected(&candidate_graph)) {
        stagnant++;
        continue;
      }

      Evaluation candidate = evaluate(&candidate_graph);
      int candidate_pressure = pressure(&candidate);
      int current_pressure = pressure(&current);
      bool accept = better(&candidate, &current);
      if (!accept && candidate_pressure == current_pressure) {
        accept = random_below(100) < 20;
      } else if (!accept) {
        unsigned loss =
            (unsigned)(current_pressure - candidate_pressure);
        unsigned temperature =
            1 + (unsigned)(8 * (options.iterations - iteration) /
                           options.iterations);
        accept =
            random_below(loss + temperature + 1) < temperature;
      }
      if (accept) {
        current_graph = candidate_graph;
        current = candidate;
      }

      if (!have_global_best || better(&candidate, &global_best)) {
        have_global_best = true;
        global_best = candidate;
        global_best_graph = candidate_graph;
        global_restart = restart;
        global_iteration = iteration;
        stagnant = 0;
        print_result("IMPROVEMENT", &global_best_graph, &global_best,
                     restart, iteration);
      } else {
        stagnant++;
      }
      if (witness(&candidate)) {
        print_result("WITNESS", &candidate_graph, &candidate, restart,
                     iteration);
        return EXIT_SUCCESS;
      }

      if (stagnant >= 1500) {
        current_graph = global_best_graph;
        unsigned kicks = 2 + random_below(5);
        for (unsigned kick = 0; kick < kicks; kick++) {
          random_pair(options.order, &left, &right);
          graph_toggle_edge(&current_graph, left, right);
          if (!graph_connected(&current_graph)) {
            graph_toggle_edge(&current_graph, left, right);
          }
        }
        current = evaluate(&current_graph);
        stagnant = 0;
      }
    }
  }

  print_result("BEST", &global_best_graph, &global_best, global_restart,
               global_iteration);
  return EXIT_SUCCESS;
}
