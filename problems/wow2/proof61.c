#include "conjectures.h"
#include "reduction61.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  ROOTED_ENUMERATION_LIMIT = 24,
};

typedef struct {
  uint64_t graphs;
  uint64_t base_graphs;
  uint64_t nonbase_graphs;
  uint64_t packing_reducible;
  uint64_t packing_critical;
  uint64_t maximum_degree_reducible;
  uint64_t any_reducible;
  uint64_t critical;
  uint64_t critical_equality;
  uint64_t cut_component_reducible;
  uint64_t induction_critical;
} ScanStats;

typedef struct {
  unsigned independence;
  unsigned forest;
  unsigned forest_containing[WOW2_MAX_VERTICES];
  unsigned forest_avoiding[WOW2_MAX_VERTICES];
} RootedProfile;

static unsigned popcount64(uint64_t value) {
  return (unsigned)__builtin_popcountll(value);
}

static unsigned first_vertex(uint64_t vertices) {
  return (unsigned)__builtin_ctzll(vertices);
}

static void die(const char *message) {
  fprintf(stderr, "error: %s\n", message);
  exit(EXIT_FAILURE);
}

static unsigned degree(const Graph *graph, unsigned vertex) {
  return popcount64(graph->adj[vertex]);
}

static unsigned maximum_degree(const Graph *graph) {
  unsigned maximum = 0;
  for (unsigned vertex = 0; vertex < graph->n; vertex++) {
    unsigned current = degree(graph, vertex);
    if (current > maximum) {
      maximum = current;
    }
  }
  return maximum;
}

static bool subset_is_independent(const Graph *graph, uint64_t vertices) {
  uint64_t scan = vertices;
  while (scan != 0) {
    unsigned vertex = first_vertex(scan);
    scan &= scan - 1;
    if ((graph->adj[vertex] & scan) != 0) {
      return false;
    }
  }
  return true;
}

static bool has_independent_set(const Graph *graph, uint64_t candidates,
                                unsigned needed) {
  if (needed == 0) {
    return true;
  }
  if (popcount64(candidates) < needed) {
    return false;
  }

  unsigned selected = first_vertex(candidates);
  unsigned selected_degree = 0;
  uint64_t scan = candidates;
  while (scan != 0) {
    unsigned vertex = first_vertex(scan);
    scan &= scan - 1;
    unsigned candidate_degree =
        popcount64(graph->adj[vertex] & candidates);
    if (candidate_degree > selected_degree) {
      selected = vertex;
      selected_degree = candidate_degree;
    }
  }

  uint64_t selected_bit = UINT64_C(1) << selected;
  uint64_t without_closed =
      candidates & ~selected_bit & ~graph->adj[selected];
  if (has_independent_set(graph, without_closed, needed - 1)) {
    return true;
  }
  return has_independent_set(graph, candidates & ~selected_bit, needed);
}

typedef struct {
  const Graph *graph;
  unsigned target;
  unsigned residue;
  uint64_t all_vertices;
  uint64_t forbidden[WOW2_MAX_VERTICES];
} PackingSearch;

static bool search_packing(const PackingSearch *search, uint64_t available,
                           uint64_t packing, unsigned size) {
  if (size == search->target) {
    return has_independent_set(
        search->graph, search->all_vertices & ~packing, search->residue);
  }
  if (popcount64(available) < search->target - size) {
    return false;
  }

  unsigned vertex = first_vertex(available);
  uint64_t bit = UINT64_C(1) << vertex;
  if (search_packing(search, available & ~search->forbidden[vertex],
                     packing | bit, size + 1)) {
    return true;
  }
  return search_packing(search, available & ~bit, packing, size);
}

static bool packing_independence_certificate(const Graph *graph) {
  int diameter = graph_diameter(graph);
  PackingSearch search = {
      .graph = graph,
      .target = (unsigned)(diameter + 2) / 3,
      .residue = graph_residue(graph),
      .all_vertices = (UINT64_C(1) << graph->n) - 1,
  };
  for (unsigned vertex = 0; vertex < graph->n; vertex++) {
    uint64_t forbidden =
        graph->adj[vertex] | (UINT64_C(1) << vertex);
    uint64_t neighbors = graph->adj[vertex];
    while (neighbors != 0) {
      unsigned neighbor = first_vertex(neighbors);
      neighbors &= neighbors - 1;
      forbidden |= graph->adj[neighbor];
    }
    search.forbidden[vertex] = forbidden;
  }
  return search_packing(&search, search.all_vertices, 0, 0);
}

static RootedProfile rooted_profile(const Graph *graph) {
  if (graph->n > ROOTED_ENUMERATION_LIMIT) {
    die("rooted profile exceeds the exact enumeration limit");
  }

  RootedProfile profile = {0};
  uint64_t limit = UINT64_C(1) << graph->n;
  for (uint64_t vertices = 0; vertices < limit; vertices++) {
    unsigned size = popcount64(vertices);
    if (size > profile.independence &&
        subset_is_independent(graph, vertices)) {
      profile.independence = size;
    }
    if (!graph_subset_is_forest(graph, vertices)) {
      continue;
    }
    if (size > profile.forest) {
      profile.forest = size;
    }
    for (unsigned vertex = 0; vertex < graph->n; vertex++) {
      unsigned *best =
          (vertices & (UINT64_C(1) << vertex)) != 0
              ? &profile.forest_containing[vertex]
              : &profile.forest_avoiding[vertex];
      if (size > *best) {
        *best = size;
      }
    }
  }
  return profile;
}

static void distance_matrix(
    const Graph *graph,
    int distances[WOW2_MAX_VERTICES][WOW2_MAX_VERTICES]) {
  for (unsigned source = 0; source < graph->n; source++) {
    unsigned queue[WOW2_MAX_VERTICES];
    unsigned head = 0;
    unsigned tail = 0;
    for (unsigned vertex = 0; vertex < graph->n; vertex++) {
      distances[source][vertex] = -1;
    }
    distances[source][source] = 0;
    queue[tail++] = source;
    while (head < tail) {
      unsigned vertex = queue[head++];
      uint64_t neighbors = graph->adj[vertex];
      while (neighbors != 0) {
        unsigned neighbor = first_vertex(neighbors);
        neighbors &= neighbors - 1;
        if (distances[source][neighbor] >= 0) {
          continue;
        }
        distances[source][neighbor] = distances[source][vertex] + 1;
        queue[tail++] = neighbor;
      }
    }
  }
}

static void print_profile(const Graph *graph) {
  char graph6[384];
  if (!graph6_encode(graph, graph6, sizeof(graph6))) {
    die("graph6 encoding failed");
  }

  unsigned residue = graph_residue(graph);
  int diameter = graph_diameter(graph);
  unsigned diameter_term = (unsigned)(diameter + 2) / 3;
  unsigned bound = residue + diameter_term;
  unsigned max_degree = maximum_degree(graph);
  RootedProfile roots = rooted_profile(graph);
  InducedResult optimized = graph_largest_induced_forest(graph);
  if (roots.forest != optimized.size) {
    die("rooted enumeration disagrees with induced-forest optimizer");
  }

  int distances[WOW2_MAX_VERTICES][WOW2_MAX_VERTICES];
  distance_matrix(graph, distances);
  unsigned endpoint_pairs[WOW2_MAX_VERTICES] = {0};
  for (unsigned left = 0; left < graph->n; left++) {
    for (unsigned right = left + 1; right < graph->n; right++) {
      if (distances[left][right] == diameter) {
        endpoint_pairs[left]++;
        endpoint_pairs[right]++;
      }
    }
  }

  Wow2Reduction61 reductions = wow2_reduction61(graph);
  printf("PROFILE graph6=%s n=%u m=%u residue=%u alpha=%u "
         "alpha_minus_residue=%d diameter=%d diameter_term=%u bound=%u "
         "forest=%u forest_minus_alpha=%d phi=%d max_degree=%u "
         "maximum_degree_reducible=%u any_reducible=%u "
         "cut_component_reducible=%u induction_reducible=%u\n",
         graph6, graph->n, graph_edge_count(graph), residue,
         roots.independence, (int)roots.independence - (int)residue,
         diameter, diameter_term, bound, roots.forest,
         (int)roots.forest - (int)roots.independence,
         (int)bound - (int)roots.forest, max_degree,
         reductions.maximum_degree_connected_reducible,
         reductions.connected_reducible, reductions.cut_reducible,
         reductions.margin >= 0);

  for (unsigned vertex = 0; vertex < graph->n; vertex++) {
    Graph smaller;
    graph_delete_vertex(graph, vertex, &smaller);
    unsigned components = 0;
    unsigned component_bound =
        wow2_component_bound61(&smaller, &components);
    bool connected = components == 1;
    int eccentricity = 0;
    for (unsigned other = 0; other < graph->n; other++) {
      if (distances[vertex][other] > eccentricity) {
        eccentricity = distances[vertex][other];
      }
    }
    unsigned smaller_residue = graph_residue(&smaller);
    int smaller_diameter = connected ? graph_diameter(&smaller) : -1;
    int bound_delta = INT_MIN;
    int component_bound_delta = INT_MIN;
    if (connected) {
      bound_delta =
          (int)wow2_bound61(&smaller) - (int)bound;
    } else {
      component_bound_delta = (int)component_bound - (int)bound;
    }
    printf("VERTEX v=%u degree=%u maximum_degree=%u components_after=%u "
           "cut=%u eccentricity=%d diametral_endpoint_pairs=%u "
           "residue_after=%u residue_delta=%d diameter_after=%d "
           "bound_delta=%d component_bound_delta=%d "
           "forest_containing=%u forest_avoiding=%u\n",
           vertex, degree(graph, vertex),
           degree(graph, vertex) == max_degree, components,
           components > 1, eccentricity,
           endpoint_pairs[vertex], smaller_residue,
           (int)smaller_residue - (int)residue, smaller_diameter,
           bound_delta, component_bound_delta,
           roots.forest_containing[vertex],
           roots.forest_avoiding[vertex]);
  }
  fputs("EDGES ", stdout);
  graph_print_edges(graph, stdout);
  fputc('\n', stdout);
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
  uint64_t connected_graphs = 0;
  uint64_t maximum_degree_deletions = 0;
  for (unsigned n = 2; n <= 6; n++) {
    unsigned edge_slots = n * (n - 1) / 2;
    uint64_t limit = UINT64_C(1) << edge_slots;
    for (uint64_t code = 0; code < limit; code++) {
      Graph graph;
      graph_from_labeled_code(&graph, n, code);
      if (!graph_connected(&graph)) {
        continue;
      }
      connected_graphs++;
      RootedProfile roots = rooted_profile(&graph);
      InducedResult optimized = graph_largest_induced_forest(&graph);
      if (roots.forest != optimized.size) {
        die("rooted forest self-test failed");
      }

      unsigned residue = graph_residue(&graph);
      if (graph_diameter(&graph) >= 4 &&
          !packing_independence_certificate(&graph)) {
        die("small-graph packing reduction failed");
      }
      unsigned max_degree = maximum_degree(&graph);
      for (unsigned vertex = 0; vertex < graph.n; vertex++) {
        if (degree(&graph, vertex) != max_degree) {
          continue;
        }
        Graph smaller;
        graph_delete_vertex(&graph, vertex, &smaller);
        if (graph_residue(&smaller) < residue) {
          die("maximum-degree residue monotonicity failed");
        }
        maximum_degree_deletions++;
      }
    }
  }

  Graph critical;
  char error[128];
  if (!graph6_parse("H????~e", &critical, error, sizeof(error)) ||
      wow2_reduction61(&critical).connected_reducible ||
      !wow2_reduction61(&critical).cut_reducible) {
    die("known critical graph check failed");
  }
  RootedProfile roots = rooted_profile(&critical);
  if (graph_residue(&critical) != 6 || graph_diameter(&critical) != 4 ||
      roots.independence != 7 || roots.forest != 8) {
    die("known critical graph invariant check failed");
  }

  printf("self-test: PASS connected_labeled_graphs=%" PRIu64
         " maximum_degree_deletions=%" PRIu64 "\n",
         connected_graphs, maximum_degree_deletions);
}

static void scan_stream(bool emit_critical,
                        bool emit_maximum_degree_critical,
                        bool emit_induction_critical,
                        bool emit_packing_critical,
                        bool analyze_packing,
                        uint64_t expected_critical, bool have_expectation,
                        uint64_t expected_induction_critical,
                        bool have_induction_expectation) {
  char line[4096];
  ScanStats stats = {0};
  while (fgets(line, sizeof(line), stdin) != NULL) {
    if (strncmp(line, ">>graph6<<", 10) == 0 ||
        strspn(line, " \t\r\n") == strlen(line)) {
      continue;
    }
    Graph graph;
    char error[128];
    if (!graph6_parse(line, &graph, error, sizeof(error))) {
      fprintf(stderr, "error: graph6 record %" PRIu64 ": %s\n",
              stats.graphs + 1, error);
      exit(EXIT_FAILURE);
    }
    if (graph.n < 2 || !graph_connected(&graph)) {
      die("input contains a trivial or disconnected graph");
    }

    stats.graphs++;
    int diameter = graph_diameter(&graph);
    if (diameter <= 3) {
      stats.base_graphs++;
      continue;
    }
    stats.nonbase_graphs++;
    bool packing_reducible = false;
    if (analyze_packing) {
      if (graph.n > ROOTED_ENUMERATION_LIMIT) {
        die("packing scan exceeds the exact enumeration limit");
      }
      packing_reducible = packing_independence_certificate(&graph);
      if (packing_reducible) {
        stats.packing_reducible++;
      } else {
        stats.packing_critical++;
      }
    }
    Wow2Reduction61 reductions = wow2_reduction61(&graph);
    if (reductions.maximum_degree_connected_reducible) {
      stats.maximum_degree_reducible++;
    }
    if (reductions.connected_reducible) {
      stats.any_reducible++;
    } else {
      stats.critical++;
      unsigned bound = wow2_bound61(&graph);
      if (graph_largest_induced_forest(&graph).size == bound) {
        stats.critical_equality++;
      }
    }
    if (reductions.cut_reducible) {
      stats.cut_component_reducible++;
    }
    if (reductions.margin < 0) {
      stats.induction_critical++;
    }

    bool emit = (emit_critical && !reductions.connected_reducible) ||
                (emit_maximum_degree_critical &&
                 !reductions.maximum_degree_connected_reducible) ||
                (emit_induction_critical && reductions.margin < 0) ||
                (emit_packing_critical && !packing_reducible);
    if (emit) {
      char graph6[384];
      if (!graph6_encode(&graph, graph6, sizeof(graph6))) {
        die("graph6 encoding failed");
      }
      puts(graph6);
    }
  }
  if (ferror(stdin)) {
    die("failed while reading graph6 input");
  }
  if (stats.graphs == 0) {
    die("no graph6 records were read");
  }

  bool emitting = emit_critical || emit_maximum_degree_critical ||
                  emit_induction_critical || emit_packing_critical;
  FILE *stream = emitting ? stderr : stdout;
  fprintf(stream,
          "SUMMARY graphs=%" PRIu64 " base=%" PRIu64
          " nonbase=%" PRIu64 " packing_analyzed=%u"
          " packing_reducible=%" PRIu64
          " packing_critical=%" PRIu64
          " maximum_degree_reducible=%" PRIu64
          " any_reducible=%" PRIu64 " critical=%" PRIu64
          " critical_equality=%" PRIu64
          " cut_component_reducible=%" PRIu64
          " induction_critical=%" PRIu64 "\n",
          stats.graphs, stats.base_graphs, stats.nonbase_graphs,
          analyze_packing,
          stats.packing_reducible, stats.packing_critical,
          stats.maximum_degree_reducible, stats.any_reducible,
          stats.critical, stats.critical_equality,
          stats.cut_component_reducible, stats.induction_critical);

  if (have_expectation && stats.critical != expected_critical) {
    fprintf(stderr, "error: expected %" PRIu64
                    " critical graphs, observed %" PRIu64 "\n",
            expected_critical, stats.critical);
    exit(EXIT_FAILURE);
  }
  if (have_induction_expectation &&
      stats.induction_critical != expected_induction_critical) {
    fprintf(stderr, "error: expected %" PRIu64
                    " induction-critical graphs, observed %" PRIu64 "\n",
            expected_induction_critical, stats.induction_critical);
    exit(EXIT_FAILURE);
  }
}

static uint64_t parse_u64(const char *text, const char *option) {
  char *end = NULL;
  errno = 0;
  unsigned long long value = strtoull(text, &end, 10);
  if (errno != 0 || end == text || *end != '\0') {
    fprintf(stderr, "error: invalid %s value\n", option);
    exit(EXIT_FAILURE);
  }
  return (uint64_t)value;
}

static void usage(void) {
  puts("usage: wow2-proof61 [--scan] [--expect-critical N]\n"
       "       wow2-proof61 --scan-packing\n"
       "       wow2-proof61 --emit-critical [--expect-critical N]\n"
       "       wow2-proof61 --emit-maximum-degree-critical\n"
       "       wow2-proof61 --emit-induction-critical\n"
       "       wow2-proof61 --emit-packing-critical\n"
       "       wow2-proof61 --profile GRAPH6\n"
       "       wow2-proof61 --self-test\n"
       "\n"
       "The scan reads connected graph6 records from stdin. A graph is\n"
       "one-vertex critical when diameter > 3 and no connected deletion\n"
       "H=G-v has residue(H)+ceil(diameter(H)/3) at least the bound for G.\n"
       "An induction-critical graph also has no cut vertex whose component\n"
       "bounds sum to the bound for G.\n"
       "A packing-critical graph has no distance-three packing S of size\n"
       "ceil(diameter/3) for which alpha(G-S) >= residue(G).\n"
       "Emit modes write graph6 records to stdout and the summary to stderr.");
}

int main(int argc, char **argv) {
  bool emit_critical = false;
  bool emit_maximum_degree_critical = false;
  bool emit_induction_critical = false;
  bool emit_packing_critical = false;
  bool analyze_packing = false;
  bool run_self_test = false;
  const char *single_graph = NULL;
  uint64_t expected_critical = 0;
  bool have_expectation = false;
  uint64_t expected_induction_critical = 0;
  bool have_induction_expectation = false;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--scan") == 0) {
      continue;
    } else if (strcmp(argv[i], "--scan-packing") == 0) {
      analyze_packing = true;
    } else if (strcmp(argv[i], "--emit-critical") == 0) {
      emit_critical = true;
    } else if (strcmp(argv[i], "--emit-maximum-degree-critical") == 0) {
      emit_maximum_degree_critical = true;
    } else if (strcmp(argv[i], "--emit-induction-critical") == 0) {
      emit_induction_critical = true;
    } else if (strcmp(argv[i], "--emit-packing-critical") == 0) {
      emit_packing_critical = true;
      analyze_packing = true;
    } else if (strcmp(argv[i], "--profile") == 0 && i + 1 < argc) {
      single_graph = argv[++i];
    } else if (strcmp(argv[i], "--expect-critical") == 0 &&
               i + 1 < argc) {
      expected_critical = parse_u64(argv[++i], "--expect-critical");
      have_expectation = true;
    } else if (strcmp(argv[i], "--expect-induction-critical") == 0 &&
               i + 1 < argc) {
      expected_induction_critical =
          parse_u64(argv[++i], "--expect-induction-critical");
      have_induction_expectation = true;
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

  unsigned emit_modes = (unsigned)emit_critical +
                        (unsigned)emit_maximum_degree_critical +
                        (unsigned)emit_induction_critical +
                        (unsigned)emit_packing_critical;
  if (emit_modes > 1) {
    die("choose one emit mode");
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
      die("the conjecture requires a nontrivial connected graph");
    }
    print_profile(&graph);
    return EXIT_SUCCESS;
  }

  scan_stream(emit_critical, emit_maximum_degree_critical,
              emit_induction_critical, emit_packing_critical,
              analyze_packing, expected_critical,
              have_expectation, expected_induction_critical,
              have_induction_expectation);
  return EXIT_SUCCESS;
}
