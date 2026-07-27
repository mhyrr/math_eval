#include "conjectures.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  uint64_t graph_count;
  uint64_t equality_count;
  uint64_t witness_count;
  int best_phi;
  bool have_best;
  Graph best_graph;
  Wow2Evaluation best_evaluation;
} ScanStats;

static unsigned popcount64(uint64_t value) {
  return (unsigned)__builtin_popcountll(value);
}

static void die(const char *message) {
  fprintf(stderr, "error: %s\n", message);
  exit(EXIT_FAILURE);
}

static void print_vertex_set(uint64_t vertices, FILE *stream) {
  bool first = true;
  fputc('[', stream);
  while (vertices != 0) {
    unsigned u = (unsigned)__builtin_ctzll(vertices);
    vertices &= vertices - 1;
    fprintf(stream, "%s%u", first ? "" : ",", u);
    first = false;
  }
  fputc(']', stream);
}

static void print_evaluation(const char *label, Wow2Mode mode,
                             const Graph *graph,
                             const Wow2Evaluation *evaluation) {
  char graph6[384];
  if (!graph6_encode(graph, graph6, sizeof(graph6))) {
    die("graph6 encoding failed");
  }

  printf("%s mode=%u graph6=%s n=%u m=%u residue=%u diameter=%d ",
         label, (unsigned)mode, graph6, evaluation->n, evaluation->edges,
         evaluation->residue, evaluation->diameter);
  if (mode == WOW2_MODE_61) {
    printf("ceil_diameter_over_3=%u ", evaluation->diameter_term);
  } else {
    printf("induced_bipartite=%u ", evaluation->bipartite.size);
  }
  printf("bound=%u induced_forest=%u phi=%d forest_vertices=",
         evaluation->bound, evaluation->forest.size, evaluation->phi);
  print_vertex_set(evaluation->forest.vertices, stdout);
  if (mode == WOW2_MODE_59) {
    fputs(" bipartite_vertices=", stdout);
    print_vertex_set(evaluation->bipartite.vertices, stdout);
  }
  fputs(" edges=", stdout);
  graph_print_edges(graph, stdout);
  fputc('\n', stdout);
}

static unsigned reference_residue(const Graph *graph) {
  int degrees[WOW2_MAX_VERTICES];
  unsigned length = graph->n;
  for (unsigned u = 0; u < graph->n; u++) {
    degrees[u] = (int)popcount64(graph->adj[u]);
  }

  while (length > 0) {
    for (unsigned i = 0; i < length; i++) {
      unsigned maximum = i;
      for (unsigned j = i + 1; j < length; j++) {
        if (degrees[j] > degrees[maximum]) {
          maximum = j;
        }
      }
      int swap = degrees[i];
      degrees[i] = degrees[maximum];
      degrees[maximum] = swap;
    }
    int degree = degrees[0];
    if (degree == 0) {
      return length;
    }
    if (degree < 0 || (unsigned)degree >= length) {
      die("reference residue received a nongraphical sequence");
    }
    for (int i = 1; i <= degree; i++) {
      degrees[i]--;
    }
    memmove(degrees, degrees + 1, (length - 1) * sizeof(*degrees));
    length--;
  }
  return 0;
}

static unsigned reference_largest_forest(const Graph *graph) {
  uint64_t limit = UINT64_C(1) << graph->n;
  unsigned best = 0;
  for (uint64_t vertices = 0; vertices < limit; vertices++) {
    unsigned size = popcount64(vertices);
    if (size > best && graph_subset_is_forest(graph, vertices)) {
      best = size;
    }
  }
  return best;
}

static unsigned reference_largest_bipartite(const Graph *graph) {
  uint64_t limit = UINT64_C(1) << graph->n;
  unsigned best = 0;
  for (uint64_t vertices = 0; vertices < limit; vertices++) {
    unsigned size = popcount64(vertices);
    if (size > best && graph_subset_is_bipartite(graph, vertices)) {
      best = size;
    }
  }
  return best;
}

static int reference_diameter(const Graph *graph) {
  enum {
    INFINITY_DISTANCE = 1000,
  };
  int distance[WOW2_MAX_VERTICES][WOW2_MAX_VERTICES];
  for (unsigned u = 0; u < graph->n; u++) {
    for (unsigned v = 0; v < graph->n; v++) {
      if (u == v) {
        distance[u][v] = 0;
      } else {
        distance[u][v] =
            graph_has_edge(graph, u, v) ? 1 : INFINITY_DISTANCE;
      }
    }
  }
  for (unsigned k = 0; k < graph->n; k++) {
    for (unsigned u = 0; u < graph->n; u++) {
      for (unsigned v = 0; v < graph->n; v++) {
        int through = distance[u][k] + distance[k][v];
        if (through < distance[u][v]) {
          distance[u][v] = through;
        }
      }
    }
  }

  int diameter = 0;
  for (unsigned u = 0; u < graph->n; u++) {
    for (unsigned v = 0; v < graph->n; v++) {
      if (distance[u][v] == INFINITY_DISTANCE) {
        return -1;
      }
      if (distance[u][v] > diameter) {
        diameter = distance[u][v];
      }
    }
  }
  return diameter;
}

static void graph_from_labeled_code(Graph *graph, unsigned n, uint64_t code) {
  graph_clear(graph, n);
  unsigned bit = 0;
  for (unsigned v = 1; v < n; v++) {
    for (unsigned u = 0; u < v; u++) {
      if ((code & (UINT64_C(1) << bit)) != 0) {
        graph_add_edge(graph, u, v);
      }
      bit++;
    }
  }
}

static void self_test(void) {
  uint64_t tested = 0;
  for (unsigned n = 1; n <= 6; n++) {
    unsigned edge_slots = n * (n - 1) / 2;
    uint64_t graph_limit = UINT64_C(1) << edge_slots;
    for (uint64_t code = 0; code < graph_limit; code++) {
      Graph graph;
      graph_from_labeled_code(&graph, n, code);
      unsigned expected_residue = reference_residue(&graph);
      unsigned actual_residue = graph_residue(&graph);
      if (actual_residue != expected_residue) {
        fprintf(stderr,
                "residue mismatch n=%u labeled_code=%" PRIu64
                " expected=%u actual=%u edges=",
                n, code, expected_residue, actual_residue);
        graph_print_edges(&graph, stderr);
        fputc('\n', stderr);
        die("residue self-test failed");
      }

      InducedResult forest = graph_largest_induced_forest(&graph);
      unsigned expected_forest = reference_largest_forest(&graph);
      if (forest.size != expected_forest ||
          !graph_subset_is_forest(&graph, forest.vertices)) {
        die("induced-forest self-test failed");
      }

      InducedResult bipartite = graph_largest_induced_bipartite(&graph);
      unsigned expected_bipartite =
          reference_largest_bipartite(&graph);
      if (bipartite.size != expected_bipartite ||
          !graph_subset_is_bipartite(&graph, bipartite.vertices)) {
        die("induced-bipartite self-test failed");
      }

      int expected_diameter = reference_diameter(&graph);
      int actual_diameter = graph_diameter(&graph);
      if (actual_diameter != expected_diameter) {
        die("diameter self-test failed");
      }

      char graph6[384];
      char error[128];
      Graph round_trip;
      if (!graph6_encode(&graph, graph6, sizeof(graph6)) ||
          !graph6_parse(graph6, &round_trip, error, sizeof(error)) ||
          round_trip.n != graph.n) {
        die("graph6 self-test failed");
      }
      for (unsigned u = 0; u < graph.n; u++) {
        if (round_trip.adj[u] != graph.adj[u]) {
          die("graph6 round-trip self-test failed");
        }
      }
      tested++;
    }
  }

  for (unsigned n = 2; n <= 20; n++) {
    Graph complete;
    graph_clear(&complete, n);
    for (unsigned u = 0; u < n; u++) {
      for (unsigned v = u + 1; v < n; v++) {
        graph_add_edge(&complete, u, v);
      }
    }
    Wow2Evaluation evaluation = wow2_evaluate(&complete, WOW2_MODE_61);
    if (evaluation.residue != 1 || evaluation.diameter != 1 ||
        evaluation.forest.size != 2 || evaluation.phi != 0) {
      die("complete-graph family check failed");
    }

    Graph path;
    graph_clear(&path, n);
    for (unsigned u = 0; u + 1 < n; u++) {
      graph_add_edge(&path, u, u + 1);
    }
    evaluation = wow2_evaluate(&path, WOW2_MODE_61);
    if (evaluation.diameter != (int)n - 1 ||
        evaluation.forest.size != n || evaluation.phi > 0) {
      die("path family check failed");
    }

    Graph star;
    graph_clear(&star, n);
    for (unsigned leaf = 1; leaf < n; leaf++) {
      graph_add_edge(&star, 0, leaf);
    }
    evaluation = wow2_evaluate(&star, WOW2_MODE_61);
    if (evaluation.residue != n - 1 ||
        evaluation.diameter != (n == 2 ? 1 : 2) ||
        evaluation.forest.size != n || evaluation.phi != 0) {
      die("star family check failed");
    }
  }
  printf("self-test: PASS labeled_graphs=%" PRIu64
         " family_orders=2..20\n",
         tested);
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

static void scan_stream(Wow2Mode mode, bool emit_filter,
                        int emit_phi_at_least) {
  char line[4096];
  ScanStats stats = {
      .best_phi = INT_MIN,
  };
  while (fgets(line, sizeof(line), stdin) != NULL) {
    if (strncmp(line, ">>graph6<<", 10) == 0 ||
        strspn(line, " \t\r\n") == strlen(line)) {
      continue;
    }
    Graph graph;
    char error[128];
    if (!graph6_parse(line, &graph, error, sizeof(error))) {
      fprintf(stderr, "error: graph6 record %" PRIu64 ": %s\n",
              stats.graph_count + 1, error);
      exit(EXIT_FAILURE);
    }
    if (graph.n < 2 || !graph_connected(&graph)) {
      die("input contains a trivial or disconnected graph");
    }

    Wow2Evaluation evaluation = wow2_evaluate(&graph, mode);
    stats.graph_count++;
    if (evaluation.phi == 0) {
      stats.equality_count++;
    }
    if (evaluation.phi > 0) {
      stats.witness_count++;
      if (!emit_filter) {
        print_evaluation("WITNESS", mode, &graph, &evaluation);
      }
    }
    if (emit_filter && evaluation.phi >= emit_phi_at_least) {
      char graph6[384];
      if (!graph6_encode(&graph, graph6, sizeof(graph6))) {
        die("graph6 encoding failed");
      }
      puts(graph6);
    }
    if (!stats.have_best || evaluation.phi > stats.best_phi) {
      stats.have_best = true;
      stats.best_phi = evaluation.phi;
      stats.best_graph = graph;
      stats.best_evaluation = evaluation;
    }
  }
  if (ferror(stdin)) {
    die("failed while reading graph6 input");
  }
  if (!stats.have_best) {
    die("no graph6 records were read");
  }

  FILE *summary_stream = emit_filter ? stderr : stdout;
  fprintf(summary_stream,
          "SUMMARY mode=%u graphs=%" PRIu64 " equality=%" PRIu64
          " witnesses=%" PRIu64 " max_phi=%d\n",
          (unsigned)mode, stats.graph_count, stats.equality_count,
          stats.witness_count, stats.best_phi);
  if (!emit_filter) {
    print_evaluation("BEST", mode, &stats.best_graph,
                     &stats.best_evaluation);
  }
}

static void usage(void) {
  puts("usage: wow2-sweep [--mode 61|59] [--scan]\n"
       "       wow2-sweep [--mode 61|59] --emit-phi-at-least N\n"
       "       wow2-sweep [--mode 61|59] --eval GRAPH6\n"
       "       wow2-sweep --self-test\n"
       "\n"
       "With --scan (the default), read connected graph6 records from stdin.\n"
       "--emit-phi-at-least filters graph6 input to qualifying records and\n"
       "writes the scan summary to stderr, so it composes in pipelines.\n"
       "Mode 61 maximizes residue + ceil(diameter/3) - induced_forest.\n"
       "Mode 59 maximizes ceil(sqrt(residue*induced_bipartite)) -\n"
       "induced_forest. All reported induced maxima are exact.");
}

int main(int argc, char **argv) {
  Wow2Mode mode = WOW2_MODE_61;
  const char *single_graph = NULL;
  bool run_self_test = false;
  bool emit_filter = false;
  int emit_phi_at_least = 0;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
      mode = parse_mode(argv[++i]);
    } else if (strcmp(argv[i], "--eval") == 0 && i + 1 < argc) {
      single_graph = argv[++i];
    } else if (strcmp(argv[i], "--scan") == 0) {
      continue;
    } else if (strcmp(argv[i], "--emit-phi-at-least") == 0 &&
               i + 1 < argc) {
      char *end = NULL;
      errno = 0;
      long value = strtol(argv[++i], &end, 10);
      if (errno != 0 || end == argv[i] || *end != '\0' ||
          value < INT_MIN || value > INT_MAX) {
        die("invalid --emit-phi-at-least value");
      }
      emit_filter = true;
      emit_phi_at_least = (int)value;
    } else if (strcmp(argv[i], "--self-test") == 0) {
      run_self_test = true;
    } else if (strcmp(argv[i], "--help") == 0) {
      usage();
      return EXIT_SUCCESS;
    } else {
      usage();
      return EXIT_FAILURE;
    }
  }

  if (run_self_test) {
    self_test();
    return EXIT_SUCCESS;
  }
  if (single_graph != NULL) {
    Graph graph;
    char error[128];
    if (!graph6_parse(single_graph, &graph, error, sizeof(error))) {
      fprintf(stderr, "error: %s\n", error);
      return EXIT_FAILURE;
    }
    if (graph.n < 2 || !graph_connected(&graph)) {
      die("the conjectures require a nontrivial connected graph");
    }
    Wow2Evaluation evaluation = wow2_evaluate(&graph, mode);
    print_evaluation(evaluation.phi > 0 ? "WITNESS" : "EVALUATION",
                     mode, &graph, &evaluation);
    return EXIT_SUCCESS;
  }

  scan_stream(mode, emit_filter, emit_phi_at_least);
  return EXIT_SUCCESS;
}
