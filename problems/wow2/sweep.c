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
  uint64_t skipped_count;
  uint64_t equality_count;
  uint64_t applicable_count;
  uint64_t witness_count;
  Rational best_phi;
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

static void print_evaluation(const char *label, const Graph *graph,
                             const Wow2Evaluation *evaluation) {
  char graph6[384];
  if (!graph6_encode(graph, graph6, sizeof(graph6))) {
    die("graph6 encoding failed");
  }

  printf("%s mode=%s graph6=%s n=%u m=%u", label,
         evaluation->conjecture->name, graph6, graph->n,
         graph_edge_count(graph));

  if (!evaluation->evaluated) {
    printf(" skipped=%s", evaluation->skip_reason);
  } else if (evaluation->conjecture->shape == WOW2_SHAPE_INEQUALITY) {
    char phi[64];
    char lhs[64];
    char rhs[64];
    rational_format(evaluation->phi, phi, sizeof(phi));
    rational_format(evaluation->lhs, lhs, sizeof(lhs));
    rational_format(evaluation->rhs, rhs, sizeof(rhs));
    printf(" phi=%s lhs=%s rhs=%s", phi, lhs, rhs);
  } else {
    printf(" hypothesis=%d conclusion=%d", evaluation->hypothesis ? 1 : 0,
           evaluation->conclusion ? 1 : 0);
  }

  for (unsigned i = 0; i < evaluation->field_count; i++) {
    char value[64];
    rational_format(evaluation->fields[i].value, value, sizeof(value));
    printf(" %s=%s", evaluation->fields[i].key, value);
  }
  if (evaluation->evaluated) {
    char pressure[64];
    rational_format(evaluation->pressure, pressure, sizeof(pressure));
    printf(" pressure=%s", pressure);
  }
  /*
   * A bulk RECORD stream is meant to be parsed, and the certificates behind a
   * dense graph run to hundreds of pairs. They stay on the lines a reader
   * actually looks at.
   */
  if (strcmp(label, "RECORD") != 0) {
    for (unsigned i = 0; i < evaluation->set_count; i++) {
      printf(" %s=", evaluation->sets[i].key);
      print_vertex_set(evaluation->sets[i].vertices, stdout);
    }
    fputs(" edges=", stdout);
    graph_print_edges(graph, stdout);
  }
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
        distance[u][v] = graph_has_edge(graph, u, v) ? 1 : INFINITY_DISTANCE;
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

static Rational field_value(const Wow2Evaluation *evaluation, const char *key) {
  for (unsigned i = 0; i < evaluation->field_count; i++) {
    if (strcmp(evaluation->fields[i].key, key) == 0) {
      return evaluation->fields[i].value;
    }
  }
  die("self-test asked for a field the evaluator does not record");
  return rational_from_int(0);
}

static int64_t field_int(const Wow2Evaluation *evaluation, const char *key) {
  Rational value = field_value(evaluation, key);
  if (!rational_is_integer(value)) {
    die("self-test expected an integer field");
  }
  return value.num;
}

static void self_test(void) {
  const Wow2Conjecture *conjecture_59 = wow2_lookup("59");
  const Wow2Conjecture *conjecture_61 = wow2_lookup("61");
  if (conjecture_59 == NULL || conjecture_61 == NULL) {
    die("the registry lost conjecture 59 or 61");
  }

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
      unsigned expected_bipartite = reference_largest_bipartite(&graph);
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
    Wow2Evaluation evaluation = wow2_evaluate(&complete, conjecture_61);
    if (field_int(&evaluation, "residue") != 1 ||
        field_int(&evaluation, "diameter") != 1 ||
        field_int(&evaluation, "induced_forest") != 2 ||
        rational_sign(evaluation.phi) != 0) {
      die("complete-graph family check failed");
    }

    Graph path;
    graph_clear(&path, n);
    for (unsigned u = 0; u + 1 < n; u++) {
      graph_add_edge(&path, u, u + 1);
    }
    evaluation = wow2_evaluate(&path, conjecture_61);
    if (field_int(&evaluation, "diameter") != (int)n - 1 ||
        field_int(&evaluation, "induced_forest") != n ||
        rational_sign(evaluation.phi) > 0) {
      die("path family check failed");
    }

    Graph star;
    graph_clear(&star, n);
    for (unsigned leaf = 1; leaf < n; leaf++) {
      graph_add_edge(&star, 0, leaf);
    }
    evaluation = wow2_evaluate(&star, conjecture_61);
    if (field_int(&evaluation, "residue") != n - 1 ||
        field_int(&evaluation, "diameter") != (n == 2 ? 1 : 2) ||
        field_int(&evaluation, "induced_forest") != n ||
        rational_sign(evaluation.phi) != 0) {
      die("star family check failed");
    }

    evaluation = wow2_evaluate(&star, conjecture_59);
    if (field_int(&evaluation, "induced_forest") != n) {
      die("star family check failed for conjecture 59");
    }
  }

  /*
   * The calibration conjectures are registered because their counterexamples
   * are already published. Rebuilding those witnesses here means a later
   * refactor that quietly breaks an evaluator cannot pass the suite, and it
   * pins the reported invariants against the values in the upstream Lean file.
   *
   * Conjecture 58's counterexample is not here: it has 79 vertices and this
   * representation stops at 63. Reaching it needs the parametric lane, where
   * the family K(3,3) joined to K(c) is checked as a function of c rather than
   * built one vertex at a time.
   */
  unsigned witnesses = 0;

  /* Conjecture 103, order 11: a triangle with four leaves on two of its
   * vertices. Upstream records alpha = 9, b = 10, ecc_avg = 30/11. */
  Graph leafy;
  graph_clear(&leafy, 11);
  graph_add_edge(&leafy, 0, 1);
  graph_add_edge(&leafy, 1, 2);
  graph_add_edge(&leafy, 0, 2);
  for (unsigned leaf = 0; leaf < 4; leaf++) {
    graph_add_edge(&leafy, 0, 3 + leaf);
    graph_add_edge(&leafy, 1, 7 + leaf);
  }
  Wow2Evaluation evaluation = wow2_evaluate(&leafy, wow2_lookup("103"));
  if (!evaluation.evaluated || !evaluation.witness ||
      field_int(&evaluation, "independence_number") != 9 ||
      field_int(&evaluation, "induced_bipartite") != 10 ||
      rational_compare(field_value(&evaluation, "average_eccentricity"),
                       rational_make(30, 11)) != 0) {
    print_evaluation("EVALUATION", &leafy, &evaluation);
    die("conjecture 103 did not reproduce its published counterexample");
  }
  witnesses++;

  /* Conjecture 109, order 13: an empty graph on 7 joined to two disjoint
   * triangles. Upstream records alpha = 7, residue = 2, b = 9. */
  Graph joined;
  graph_clear(&joined, 13);
  for (unsigned base = 7; base < 13; base += 3) {
    graph_add_edge(&joined, base, base + 1);
    graph_add_edge(&joined, base + 1, base + 2);
    graph_add_edge(&joined, base, base + 2);
  }
  for (unsigned free_vertex = 0; free_vertex < 7; free_vertex++) {
    for (unsigned clique = 7; clique < 13; clique++) {
      graph_add_edge(&joined, free_vertex, clique);
    }
  }
  evaluation = wow2_evaluate(&joined, wow2_lookup("109"));
  if (!evaluation.evaluated || !evaluation.witness ||
      field_int(&evaluation, "independence_number") != 7 ||
      field_int(&evaluation, "residue") != 2 ||
      field_int(&evaluation, "induced_bipartite") != 9) {
    print_evaluation("EVALUATION", &joined, &evaluation);
    die("conjecture 109 did not reproduce its published counterexample");
  }
  witnesses++;

  printf("self-test: PASS labeled_graphs=%" PRIu64
         " family_orders=2..20 conjectures=%u calibration_witnesses=%u\n",
         tested, wow2_conjecture_count(), witnesses);
}

static const Wow2Conjecture *parse_mode(const char *text) {
  const Wow2Conjecture *conjecture = wow2_lookup(text);
  if (conjecture == NULL) {
    fprintf(stderr, "error: unknown mode '%s'; try --list\n", text);
    exit(EXIT_FAILURE);
  }
  return conjecture;
}

static void list_modes(void) {
  printf("%-6s %-11s %-12s %s\n", "mode", "shape", "status", "statement");
  for (unsigned i = 0; i < wow2_conjecture_count(); i++) {
    const Wow2Conjecture *conjecture = wow2_conjecture_at(i);
    printf("%-6s %-11s %-12s %s\n", conjecture->name,
           conjecture->shape == WOW2_SHAPE_INEQUALITY ? "inequality"
                                                      : "implication",
           conjecture->status == WOW2_STATUS_OPEN ? "open" : "calibration",
           conjecture->summary);
  }
}

static void scan_stream(const Wow2Conjecture *conjecture, bool emit_filter,
                        int emit_phi_at_least, bool report_every) {
  char line[4096];
  ScanStats stats = {
      .best_phi = rational_from_int(0),
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

    Wow2Evaluation evaluation = wow2_evaluate(&graph, conjecture);
    stats.graph_count++;
    if (report_every) {
      print_evaluation("RECORD", &graph, &evaluation);
    }
    if (!evaluation.evaluated) {
      stats.skipped_count++;
      continue;
    }
    if (conjecture->shape == WOW2_SHAPE_INEQUALITY) {
      if (evaluation.equality) {
        stats.equality_count++;
      }
    } else if (evaluation.hypothesis) {
      stats.applicable_count++;
    }
    if (evaluation.witness) {
      stats.witness_count++;
      if (!emit_filter && !report_every) {
        print_evaluation("WITNESS", &graph, &evaluation);
      }
    }
    if (emit_filter &&
        rational_compare(evaluation.phi,
                         rational_from_int(emit_phi_at_least)) >= 0) {
      char graph6[384];
      if (!graph6_encode(&graph, graph6, sizeof(graph6))) {
        die("graph6 encoding failed");
      }
      puts(graph6);
    }
    if (!stats.have_best ||
        rational_compare(evaluation.phi, stats.best_phi) > 0) {
      stats.have_best = true;
      stats.best_phi = evaluation.phi;
      stats.best_graph = graph;
      stats.best_evaluation = evaluation;
    }
  }
  if (ferror(stdin)) {
    die("failed while reading graph6 input");
  }
  if (stats.graph_count == 0) {
    die("no graph6 records were read");
  }

  FILE *summary_stream = emit_filter ? stderr : stdout;
  if (conjecture->shape == WOW2_SHAPE_INEQUALITY) {
    char phi[64];
    rational_format(stats.best_phi, phi, sizeof(phi));
    fprintf(summary_stream,
            "SUMMARY mode=%s graphs=%" PRIu64 " equality=%" PRIu64
            " witnesses=%" PRIu64 " max_phi=%s",
            conjecture->name, stats.graph_count, stats.equality_count,
            stats.witness_count, stats.have_best ? phi : "none");
  } else {
    fprintf(summary_stream,
            "SUMMARY mode=%s graphs=%" PRIu64 " applicable=%" PRIu64
            " witnesses=%" PRIu64,
            conjecture->name, stats.graph_count, stats.applicable_count,
            stats.witness_count);
  }
  /* Coverage that was not achieved is always reported, never absorbed. */
  if (stats.skipped_count > 0) {
    fprintf(summary_stream, " skipped=%" PRIu64, stats.skipped_count);
  }
  fputc('\n', summary_stream);

  if (!emit_filter && !report_every && stats.have_best) {
    print_evaluation("BEST", &stats.best_graph, &stats.best_evaluation);
  }
}

static void usage(void) {
  puts("usage: wow2-sweep [--mode NAME] [--scan]\n"
       "       wow2-sweep [--mode NAME] --emit-phi-at-least N\n"
       "       wow2-sweep [--mode NAME] --eval GRAPH6\n"
       "       wow2-sweep [--mode NAME] --scan --report\n"
       "       wow2-sweep --list\n"
       "       wow2-sweep --self-test\n"
       "\n"
       "With --scan (the default), read connected graph6 records from stdin.\n"
       "--emit-phi-at-least filters graph6 input to qualifying records and\n"
       "writes the scan summary to stderr, so it composes in pipelines.\n"
       "--report prints one RECORD line per input graph, including skipped\n"
       "ones, so a caller that knows which graph is which can group results.\n"
       "--list prints every registered WOWII conjecture. Inequality modes\n"
       "maximize phi = lhs - rhs, positive exactly on a counterexample;\n"
       "implication modes report the hypothesis and the conclusion, and a\n"
       "counterexample satisfies the first and fails the second. All\n"
       "arithmetic is exact: integers and rationals, never floating point.");
}

int main(int argc, char **argv) {
  const Wow2Conjecture *conjecture = wow2_lookup("61");
  const char *single_graph = NULL;
  bool run_self_test = false;
  bool emit_filter = false;
  bool report_every = false;
  int emit_phi_at_least = 0;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
      conjecture = parse_mode(argv[++i]);
    } else if (strcmp(argv[i], "--eval") == 0 && i + 1 < argc) {
      single_graph = argv[++i];
    } else if (strcmp(argv[i], "--scan") == 0) {
      continue;
    } else if (strcmp(argv[i], "--emit-phi-at-least") == 0 && i + 1 < argc) {
      char *end = NULL;
      errno = 0;
      long value = strtol(argv[++i], &end, 10);
      if (errno != 0 || end == argv[i] || *end != '\0' || value < INT_MIN ||
          value > INT_MAX) {
        die("invalid --emit-phi-at-least value");
      }
      emit_filter = true;
      emit_phi_at_least = (int)value;
    } else if (strcmp(argv[i], "--report") == 0) {
      report_every = true;
    } else if (strcmp(argv[i], "--list") == 0) {
      list_modes();
      return EXIT_SUCCESS;
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
    Wow2Evaluation evaluation = wow2_evaluate(&graph, conjecture);
    print_evaluation(evaluation.witness ? "WITNESS" : "EVALUATION", &graph,
                     &evaluation);
    return EXIT_SUCCESS;
  }

  scan_stream(conjecture, emit_filter, emit_phi_at_least, report_every);
  return EXIT_SUCCESS;
}
