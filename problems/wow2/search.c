#include "conjectures.h"

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
  Wow2Mode mode;
  const char *start_graph6;
} Options;

static void die(const char *message) {
  fprintf(stderr, "error: %s\n", message);
  exit(EXIT_FAILURE);
}

static uint64_t random_state;

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

static Wow2Mode parse_mode(const char *text) {
  if (strcmp(text, "61") == 0) {
    return WOW2_MODE_61;
  }
  if (strcmp(text, "59") == 0) {
    return WOW2_MODE_59;
  }
  die("--mode must be 59 or 61");
  return WOW2_MODE_61;
}

static Options parse_options(int argc, char **argv) {
  Options options = {
      .order = 12,
      .iterations = 10000,
      .restarts = 20,
      .seed = UINT64_C(0x61c0ffee),
      .mode = WOW2_MODE_61,
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
    } else if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
      options.mode = parse_mode(argv[++i]);
    } else if (strcmp(argv[i], "--start") == 0 && i + 1 < argc) {
      options.start_graph6 = argv[++i];
    } else if (strcmp(argv[i], "--help") == 0) {
      puts("usage: wow2-search [--mode 61|59] [--order N]\n"
           "                   [--iterations N] [--restarts N] [--seed N]\n"
           "                   [--start GRAPH6]\n"
           "\n"
           "Stochastic edge-toggle search over connected labeled graphs.\n"
           "Each candidate is scored with the exact evaluator. Restarts\n"
           "alternate random connected, thin-neck block, and layered-path\n"
           "initial states.");
      exit(EXIT_SUCCESS);
    } else {
      die("unknown or incomplete option");
    }
  }
  if (options.order < 2 || options.order > 30) {
    die("--order must be from 2 through 30");
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
    if (start.n < 2 || start.n > 30 || !graph_connected(&start)) {
      die("--start must be a connected graph of order 2 through 30");
    }
    options.order = start.n;
  }
  return options;
}

static void random_pair(unsigned n, unsigned *u, unsigned *v) {
  *u = random_below(n);
  do {
    *v = random_below(n);
  } while (*v == *u);
  if (*u > *v) {
    unsigned swap = *u;
    *u = *v;
    *v = swap;
  }
}

static void random_connected_graph(Graph *graph, unsigned n) {
  graph_clear(graph, n);
  for (unsigned v = 1; v < n; v++) {
    graph_add_edge(graph, v, random_below(v));
  }
  unsigned density = 10 + random_below(76);
  for (unsigned u = 0; u < n; u++) {
    for (unsigned v = u + 1; v < n; v++) {
      if (!graph_has_edge(graph, u, v) && random_below(100) < density) {
        graph_add_edge(graph, u, v);
      }
    }
  }
}

static void block_chain_graph(Graph *graph, unsigned n) {
  graph_clear(graph, n);
  unsigned maximum_blocks = n / 2;
  if (maximum_blocks > 6) {
    maximum_blocks = 6;
  }
  unsigned block_count =
      maximum_blocks <= 2 ? maximum_blocks : 2 + random_below(maximum_blocks - 1);
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
    for (unsigned u = start; u < end; u++) {
      for (unsigned v = u + 1; v < end; v++) {
        if (random_below(100) < 85) {
          graph_add_edge(graph, u, v);
        }
      }
    }
    for (unsigned v = start + 1; v < end; v++) {
      if (!graph_has_edge(graph, start, v)) {
        graph_add_edge(graph, start, v);
      }
    }
    if (block > 0) {
      graph_add_edge(graph, start - 1, start);
    }
    start = end;
  }

  if (!graph_connected(graph)) {
    for (unsigned v = 1; v < n; v++) {
      if (!graph_has_edge(graph, v - 1, v)) {
        graph_add_edge(graph, v - 1, v);
      }
    }
  }
}

static void layered_path_graph(Graph *graph, unsigned n) {
  graph_clear(graph, n);
  unsigned maximum_layers = n < 10 ? n / 2 : 7;
  if (maximum_layers < 2) {
    maximum_layers = 2;
  }
  unsigned layer_count = 2 + random_below(maximum_layers - 1);
  unsigned layer_of[WOW2_MAX_VERTICES];
  for (unsigned v = 0; v < n; v++) {
    layer_of[v] = (uint64_t)v * layer_count / n;
  }

  for (unsigned u = 0; u < n; u++) {
    for (unsigned v = u + 1; v < n; v++) {
      unsigned left = layer_of[u];
      unsigned right = layer_of[v];
      unsigned gap = left > right ? left - right : right - left;
      if (gap == 0 && random_below(100) < 75) {
        graph_add_edge(graph, u, v);
      } else if (gap == 1 && random_below(100) < 70) {
        graph_add_edge(graph, u, v);
      }
    }
  }

  for (unsigned layer = 0; layer + 1 < layer_count; layer++) {
    unsigned left = n;
    unsigned right = n;
    for (unsigned v = 0; v < n; v++) {
      if (layer_of[v] == layer && left == n) {
        left = v;
      }
      if (layer_of[v] == layer + 1 && right == n) {
        right = v;
      }
    }
    if (left < n && right < n && !graph_has_edge(graph, left, right)) {
      graph_add_edge(graph, left, right);
    }
  }
  for (unsigned layer = 0; layer < layer_count; layer++) {
    unsigned representative = n;
    for (unsigned v = 0; v < n; v++) {
      if (layer_of[v] != layer) {
        continue;
      }
      if (representative == n) {
        representative = v;
      } else if (!graph_has_edge(graph, representative, v)) {
        graph_add_edge(graph, representative, v);
      }
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
    die("internal initial-state generator produced a disconnected graph");
  }
}

static int pressure(const Wow2Evaluation *evaluation, Wow2Mode mode) {
  if (mode == WOW2_MODE_61) {
    return 3 * ((int)evaluation->residue -
                (int)evaluation->forest.size) +
           evaluation->diameter;
  }
  return evaluation->phi;
}

static bool better(const Wow2Evaluation *left,
                   const Wow2Evaluation *right, Wow2Mode mode) {
  int left_pressure = pressure(left, mode);
  int right_pressure = pressure(right, mode);
  if (left_pressure != right_pressure) {
    return left_pressure > right_pressure;
  }
  if (left->phi != right->phi) {
    return left->phi > right->phi;
  }
  if (left->diameter != right->diameter) {
    return left->diameter > right->diameter;
  }
  return left->residue > right->residue;
}

static void print_result(const char *label, Wow2Mode mode, const Graph *graph,
                         const Wow2Evaluation *evaluation, unsigned restart,
                         uint64_t iteration) {
  char graph6[384];
  if (!graph6_encode(graph, graph6, sizeof(graph6))) {
    die("graph6 encoding failed");
  }
  printf("%s mode=%s restart=%u iteration=%" PRIu64
         " graph6=%s n=%u m=%u residue=%u diameter=%d ",
         label, wow2_mode_name(mode), restart, iteration, graph6,
         evaluation->n, evaluation->edges, evaluation->residue,
         evaluation->diameter);
  if (mode == WOW2_MODE_61) {
    printf("ceil_diameter_over_3=%u ", evaluation->diameter_term);
  } else {
    printf("induced_bipartite=%u ", evaluation->bipartite.size);
  }
  printf("bound=%u induced_forest=%u phi=%d pressure=%d edges=",
         evaluation->bound, evaluation->forest.size, evaluation->phi,
         pressure(evaluation, mode));
  graph_print_edges(graph, stdout);
  fputc('\n', stdout);
  fflush(stdout);
}

int main(int argc, char **argv) {
  Options options = parse_options(argc, argv);
  random_state = options.seed;

  Graph global_best_graph;
  Wow2Evaluation global_best = {0};
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
        unsigned u;
        unsigned v;
        random_pair(options.order, &u, &v);
        graph_toggle_edge(&current_graph, u, v);
        if (!graph_connected(&current_graph)) {
          graph_toggle_edge(&current_graph, u, v);
        }
      }
    }
    Wow2Evaluation current = wow2_evaluate(&current_graph, options.mode);

    if (!have_global_best || better(&current, &global_best, options.mode)) {
      have_global_best = true;
      global_best = current;
      global_best_graph = current_graph;
      global_restart = restart;
      global_iteration = 0;
      print_result("IMPROVEMENT", options.mode, &global_best_graph,
                   &global_best, restart, 0);
    }
    if (current.phi > 0) {
      print_result("WITNESS", options.mode, &current_graph, &current,
                   restart, 0);
      return EXIT_SUCCESS;
    }

    uint64_t stagnant = 0;
    for (uint64_t iteration = 1; iteration <= options.iterations;
         iteration++) {
      unsigned u;
      unsigned v;
      random_pair(options.order, &u, &v);
      Graph candidate_graph = current_graph;
      graph_toggle_edge(&candidate_graph, u, v);
      if (!graph_connected(&candidate_graph)) {
        stagnant++;
        continue;
      }

      Wow2Evaluation candidate =
          wow2_evaluate(&candidate_graph, options.mode);
      int candidate_pressure = pressure(&candidate, options.mode);
      int current_pressure = pressure(&current, options.mode);
      bool accept = better(&candidate, &current, options.mode);
      if (!accept && candidate_pressure == current_pressure) {
        accept = random_below(100) < 20;
      } else if (!accept) {
        unsigned loss = (unsigned)(current_pressure - candidate_pressure);
        unsigned temperature =
            1 + (unsigned)(6 * (options.iterations - iteration) /
                           options.iterations);
        accept = random_below(loss + temperature + 1) < temperature;
      }
      if (accept) {
        current_graph = candidate_graph;
        current = candidate;
      }

      if (!have_global_best ||
          better(&candidate, &global_best, options.mode)) {
        have_global_best = true;
        global_best = candidate;
        global_best_graph = candidate_graph;
        global_restart = restart;
        global_iteration = iteration;
        stagnant = 0;
        print_result("IMPROVEMENT", options.mode, &global_best_graph,
                     &global_best, restart, iteration);
      } else {
        stagnant++;
      }

      if (candidate.phi > 0) {
        print_result("WITNESS", options.mode, &candidate_graph, &candidate,
                     restart, iteration);
        return EXIT_SUCCESS;
      }

      if (stagnant >= 1000) {
        current_graph = global_best_graph;
        unsigned kicks = 2 + random_below(4);
        for (unsigned kick = 0; kick < kicks; kick++) {
          random_pair(options.order, &u, &v);
          graph_toggle_edge(&current_graph, u, v);
          if (!graph_connected(&current_graph)) {
            graph_toggle_edge(&current_graph, u, v);
          }
        }
        current = wow2_evaluate(&current_graph, options.mode);
        stagnant = 0;
      }
    }
  }

  print_result("BEST", options.mode, &global_best_graph, &global_best,
               global_restart, global_iteration);
  return EXIT_SUCCESS;
}
