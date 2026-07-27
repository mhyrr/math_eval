/*
 * Structural checks for the endgame of Written on the Wall II, Conjecture 61.
 *
 * Reads graph6 records on stdin and tabulates, over connected graphs:
 *
 *   1. Lemma 7 stress test: for every maximum-degree vertex v, the sum of
 *      component residues of G-v must be at least residue(G).
 *   2. Two-connected census: a 2-connected (DL)-counterexample needs every
 *      maximum-degree vertex inside every diametral pair, and d = 1 (mod 3).
 *      Graphs escaping that trap are verified to satisfy (DL) directly;
 *      survivors are printed as the residual class.
 *   3. Deletion census cross-check against the published PROOF-061 numbers.
 *   4. Free-attachment repair test: at every maximum-degree vertex v with
 *      component bound exactly b(G)-1, look for per-component induced
 *      forests meeting the quota in which no tree holds two neighbors of v.
 *      Success means the union plus v is a b(G)-forest.
 */

#include "graph.h"
#include "reduction61.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

static void die(const char *message) {
  fprintf(stderr, "error: %s\n", message);
  exit(EXIT_FAILURE);
}

static unsigned popcount64(uint64_t value) {
  return (unsigned)__builtin_popcountll(value);
}

static unsigned first_vertex(uint64_t vertices) {
  return (unsigned)__builtin_ctzll(vertices);
}

static uint64_t vertex_mask(unsigned n) {
  return n == 0 ? 0 : (UINT64_C(1) << n) - 1;
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

static unsigned components_excluding(
    const Graph *graph, unsigned removed, uint64_t masks[WOW2_MAX_VERTICES]) {
  uint64_t unseen = vertex_mask(graph->n) & ~(UINT64_C(1) << removed);
  unsigned count = 0;
  while (unseen != 0) {
    uint64_t component = 0;
    uint64_t frontier = UINT64_C(1) << first_vertex(unseen);
    unseen &= ~frontier;
    while (frontier != 0) {
      unsigned vertex = first_vertex(frontier);
      frontier &= frontier - 1;
      component |= UINT64_C(1) << vertex;
      uint64_t fresh = graph->adj[vertex] & unseen;
      unseen &= ~fresh;
      frontier |= fresh;
    }
    masks[count++] = component;
  }
  return count;
}

static void induced_subgraph(const Graph *graph, uint64_t vertices,
                             Graph *result) {
  unsigned mapping[WOW2_MAX_VERTICES] = {0};
  unsigned order = 0;
  uint64_t scan = vertices;
  while (scan != 0) {
    unsigned vertex = first_vertex(scan);
    scan &= scan - 1;
    mapping[vertex] = order++;
  }

  graph_clear(result, order);
  scan = vertices;
  while (scan != 0) {
    unsigned left = first_vertex(scan);
    scan &= scan - 1;
    uint64_t neighbors = graph->adj[left] & vertices;
    while (neighbors != 0) {
      unsigned right = first_vertex(neighbors);
      neighbors &= neighbors - 1;
      if (left < right) {
        graph_add_edge(result, mapping[left], mapping[right]);
      }
    }
  }
}

static void all_pairs_distances(
    const Graph *graph, uint8_t distance[][WOW2_MAX_VERTICES]) {
  for (unsigned source = 0; source < graph->n; source++) {
    for (unsigned target = 0; target < graph->n; target++) {
      distance[source][target] = UINT8_MAX;
    }
    distance[source][source] = 0;
    uint64_t frontier = UINT64_C(1) << source;
    uint64_t seen = frontier;
    uint8_t depth = 0;
    while (frontier != 0) {
      depth++;
      uint64_t next = 0;
      uint64_t scan = frontier;
      while (scan != 0) {
        unsigned vertex = first_vertex(scan);
        scan &= scan - 1;
        next |= graph->adj[vertex];
      }
      next &= ~seen;
      seen |= next;
      uint64_t record = next;
      while (record != 0) {
        unsigned vertex = first_vertex(record);
        record &= record - 1;
        distance[source][vertex] = depth;
      }
      frontier = next;
    }
  }
}

/* Every tree of the induced forest on `subset` may hold at most one vertex
 * of `attach`. */
static bool trees_accept_attachment(const Graph *graph, uint64_t subset,
                                    uint64_t attach) {
  uint64_t unseen = subset;
  while (unseen != 0) {
    uint64_t tree = 0;
    uint64_t frontier = UINT64_C(1) << first_vertex(unseen);
    unseen &= ~frontier;
    while (frontier != 0) {
      unsigned vertex = first_vertex(frontier);
      frontier &= frontier - 1;
      tree |= UINT64_C(1) << vertex;
      uint64_t fresh = graph->adj[vertex] & unseen;
      unseen &= ~fresh;
      frontier |= fresh;
    }
    if (popcount64(tree & attach) >= 2) {
      return false;
    }
  }
  return true;
}

static bool component_repairable(const Graph *graph, uint64_t component,
                                 uint64_t attach, unsigned quota) {
  if (quota == 0) {
    return true;
  }
  uint64_t subset = component;
  for (;;) {
    if (popcount64(subset) >= quota &&
        graph_subset_is_forest(graph, subset) &&
        trees_accept_attachment(graph, subset, attach)) {
      return true;
    }
    if (subset == 0) {
      return false;
    }
    subset = (subset - 1) & component;
  }
}

typedef struct {
  uint64_t graphs;
  uint64_t base_graphs;
  uint64_t nonbase_graphs;

  uint64_t lemma7_checks;
  uint64_t lemma7_violations;

  uint64_t dl_graphs;
  uint64_t dl_connected;
  uint64_t dl_maximum_degree_connected;
  uint64_t dl_cut_only;
  uint64_t dl_missing;

  uint64_t two_connected;
  uint64_t escape;
  uint64_t escape_violations;
  uint64_t trapped;
  uint64_t trapped_easy;
  uint64_t trapped_easy_violations;
  uint64_t residual;
  uint64_t residual_still_dl;

  uint64_t tight_configs;
  uint64_t repaired_configs;
  uint64_t failed_configs;
  uint64_t skipped_configs;
  uint64_t tight_graphs;
  uint64_t fully_repaired_graphs;
} Stats;

static void report_graph(const char *label, const Graph *graph,
                         const char *detail) {
  char text[128];
  if (!graph6_encode(graph, text, sizeof(text))) {
    die("graph6 encoding failed");
  }
  printf("%s: %s %s\n", label, text, detail);
}

static void analyze(const Graph *graph, Stats *stats,
                    unsigned max_repair_order) {
  unsigned n = graph->n;
  unsigned residue = graph_residue(graph);
  int diameter = graph_diameter(graph);
  if (diameter < 0) {
    die("disconnected graph reached analysis");
  }
  unsigned bound = wow2_bound61(graph);
  unsigned max_degree = maximum_degree(graph);
  bool nonbase = diameter >= 4;

  stats->graphs++;
  if (nonbase) {
    stats->nonbase_graphs++;
  } else {
    stats->base_graphs++;
  }

  unsigned child_bound[WOW2_MAX_VERTICES];
  unsigned child_components[WOW2_MAX_VERTICES];
  bool two_connected = n >= 3;
  bool dl_any = false;
  bool dl_connected = false;
  bool dl_maximum_degree_connected = false;
  bool graph_has_tight = false;
  bool graph_repair_failed = false;
  bool graph_repair_skipped = false;

  for (unsigned vertex = 0; vertex < n; vertex++) {
    uint64_t masks[WOW2_MAX_VERTICES];
    unsigned count = components_excluding(graph, vertex, masks);
    if (count > 1) {
      two_connected = false;
    }
    unsigned sum_residue = 0;
    unsigned sum_bound = 0;
    unsigned quotas[WOW2_MAX_VERTICES];
    unsigned widest = 0;
    for (unsigned index = 0; index < count; index++) {
      Graph component;
      induced_subgraph(graph, masks[index], &component);
      sum_residue += graph_residue(&component);
      quotas[index] = wow2_bound61(&component);
      sum_bound += quotas[index];
      if (component.n > widest) {
        widest = component.n;
      }
    }
    child_bound[vertex] = sum_bound;
    child_components[vertex] = count;

    bool at_maximum = degree(graph, vertex) == max_degree;
    if (at_maximum && n >= 2) {
      stats->lemma7_checks++;
      if (sum_residue < residue) {
        stats->lemma7_violations++;
        report_graph("lemma7-violation", graph, "");
      }
    }

    if (!nonbase) {
      continue;
    }

    if (sum_bound >= bound) {
      dl_any = true;
      if (count == 1) {
        dl_connected = true;
        if (at_maximum) {
          dl_maximum_degree_connected = true;
        }
      }
    }

    if (at_maximum && sum_bound + 1 == bound) {
      stats->tight_configs++;
      graph_has_tight = true;
      if (widest > max_repair_order) {
        stats->skipped_configs++;
        graph_repair_skipped = true;
        continue;
      }
      bool repaired = true;
      for (unsigned index = 0; index < count; index++) {
        uint64_t attach = graph->adj[vertex] & masks[index];
        if (!component_repairable(graph, masks[index], attach,
                                  quotas[index])) {
          repaired = false;
          break;
        }
      }
      if (repaired) {
        stats->repaired_configs++;
      } else {
        stats->failed_configs++;
        graph_repair_failed = true;
        char detail[64];
        snprintf(detail, sizeof(detail), "vertex=%u", vertex);
        report_graph("repair-failure", graph, detail);
      }
    }
  }

  if (!nonbase) {
    return;
  }

  if (dl_any) {
    stats->dl_graphs++;
    if (dl_connected) {
      stats->dl_connected++;
      if (dl_maximum_degree_connected) {
        stats->dl_maximum_degree_connected++;
      }
    } else {
      stats->dl_cut_only++;
    }
  } else {
    stats->dl_missing++;
    report_graph("dl-missing", graph, "");
  }

  if (graph_has_tight) {
    stats->tight_graphs++;
    if (!graph_repair_failed && !graph_repair_skipped) {
      stats->fully_repaired_graphs++;
    }
  }

  if (!two_connected) {
    return;
  }
  stats->two_connected++;

  static uint8_t distance[WOW2_MAX_VERTICES][WOW2_MAX_VERTICES];
  all_pairs_distances(graph, distance);

  /* A maximum-degree vertex escapes when some diametral pair avoids it. */
  bool escaped = false;
  for (unsigned vertex = 0; vertex < n && !escaped; vertex++) {
    if (degree(graph, vertex) != max_degree) {
      continue;
    }
    for (unsigned left = 0; left < n && !escaped; left++) {
      if (left == vertex) {
        continue;
      }
      for (unsigned right = left + 1; right < n; right++) {
        if (right == vertex || distance[left][right] != diameter) {
          continue;
        }
        escaped = true;
        if (child_bound[vertex] < bound ||
            child_components[vertex] != 1) {
          stats->escape_violations++;
          report_graph("escape-violation", graph, "");
        }
        break;
      }
    }
  }

  if (escaped) {
    stats->escape++;
    return;
  }

  stats->trapped++;
  if (diameter % 3 != 1) {
    stats->trapped_easy++;
    for (unsigned vertex = 0; vertex < n; vertex++) {
      if (degree(graph, vertex) == max_degree &&
          child_bound[vertex] < bound) {
        stats->trapped_easy_violations++;
        report_graph("trapped-easy-violation", graph, "");
      }
    }
    return;
  }

  stats->residual++;
  bool still_dl = false;
  for (unsigned vertex = 0; vertex < n; vertex++) {
    if (degree(graph, vertex) == max_degree &&
        child_bound[vertex] >= bound) {
      still_dl = true;
    }
  }
  if (still_dl) {
    stats->residual_still_dl++;
  }
  char detail[128];
  snprintf(detail, sizeof(detail),
           "n=%u d=%d r=%u maxdeg=%u still_dl=%d", n, diameter, residue,
           max_degree, still_dl ? 1 : 0);
  report_graph("residual", graph, detail);
}

static void print_stats(const Stats *stats) {
  printf("graphs                                  %" PRIu64 "\n",
         stats->graphs);
  printf("  diameter <= 3 (base)                  %" PRIu64 "\n",
         stats->base_graphs);
  printf("  diameter >= 4                         %" PRIu64 "\n",
         stats->nonbase_graphs);
  printf("lemma 7 maximum-degree checks           %" PRIu64 "\n",
         stats->lemma7_checks);
  printf("  violations                            %" PRIu64 "\n",
         stats->lemma7_violations);
  printf("deletion census over diameter >= 4\n");
  printf("  some (DL) vertex                      %" PRIu64 "\n",
         stats->dl_graphs);
  printf("  some connected (DL) vertex            %" PRIu64 "\n",
         stats->dl_connected);
  printf("  maximum-degree connected (DL) vertex  %" PRIu64 "\n",
         stats->dl_maximum_degree_connected);
  printf("  cut-vertex rescue only                %" PRIu64 "\n",
         stats->dl_cut_only);
  printf("  no (DL) vertex                        %" PRIu64 "\n",
         stats->dl_missing);
  printf("two-connected census over diameter >= 4\n");
  printf("  two-connected graphs                  %" PRIu64 "\n",
         stats->two_connected);
  printf("  escape (pair avoids a max-degree v)   %" PRIu64 "\n",
         stats->escape);
  printf("    argument violations                 %" PRIu64 "\n",
         stats->escape_violations);
  printf("  trapped                               %" PRIu64 "\n",
         stats->trapped);
  printf("    d != 1 (mod 3), endpoint closes     %" PRIu64 "\n",
         stats->trapped_easy);
  printf("      argument violations               %" PRIu64 "\n",
         stats->trapped_easy_violations);
  printf("    residual class (d = 1 mod 3)        %" PRIu64 "\n",
         stats->residual);
  printf("      still (DL) at a max-degree v      %" PRIu64 "\n",
         stats->residual_still_dl);
  printf("free-attachment repair at tight maximum-degree deletions\n");
  printf("  tight configurations                  %" PRIu64 "\n",
         stats->tight_configs);
  printf("  repaired                              %" PRIu64 "\n",
         stats->repaired_configs);
  printf("  failed                                %" PRIu64 "\n",
         stats->failed_configs);
  printf("  skipped (component too large)         %" PRIu64 "\n",
         stats->skipped_configs);
  printf("  graphs with a tight configuration     %" PRIu64 "\n",
         stats->tight_graphs);
  printf("  graphs with every tight repair done   %" PRIu64 "\n",
         stats->fully_repaired_graphs);
}

static void build_path(Graph *graph, unsigned n) {
  graph_clear(graph, n);
  for (unsigned vertex = 0; vertex + 1 < n; vertex++) {
    graph_add_edge(graph, vertex, vertex + 1);
  }
}

static void build_cycle(Graph *graph, unsigned n) {
  build_path(graph, n);
  graph_add_edge(graph, n - 1, 0);
}

static void self_test(void) {
  Graph graph;

  build_path(&graph, 7);
  if (graph_diameter(&graph) != 6 || graph_residue(&graph) != 3 ||
      wow2_bound61(&graph) != 5) {
    die("self-test: P7 invariants");
  }
  Stats stats = {0};
  analyze(&graph, &stats, WOW2_MAX_VERTICES);
  if (stats.lemma7_violations != 0 || stats.dl_graphs != 1 ||
      stats.two_connected != 0 || stats.failed_configs != 0) {
    die("self-test: P7 analysis");
  }

  build_cycle(&graph, 9);
  if (graph_diameter(&graph) != 4 || wow2_bound61(&graph) != 5) {
    die("self-test: C9 invariants");
  }
  memset(&stats, 0, sizeof(stats));
  analyze(&graph, &stats, WOW2_MAX_VERTICES);
  if (stats.two_connected != 1 || stats.escape != 1 ||
      stats.escape_violations != 0 || stats.trapped != 0) {
    die("self-test: C9 analysis");
  }

  /* Star K_{1,4}: any single-tree forest holding two leaves rejects an
   * outside vertex adjacent to both, so attachment filtering must bite. */
  graph_clear(&graph, 5);
  for (unsigned leaf = 1; leaf < 5; leaf++) {
    graph_add_edge(&graph, 0, leaf);
  }
  uint64_t all = vertex_mask(5);
  if (!component_repairable(&graph, all, UINT64_C(0x06), 4)) {
    die("self-test: star repair should succeed");
  }
  if (component_repairable(&graph, UINT64_C(0x03), UINT64_C(0x03), 2)) {
    die("self-test: edge with both ends attached must fail");
  }

  puts("self-test: ok");
}

static void usage(void) {
  puts("usage: wow2-finish61 [--max-repair-order N]\n"
       "       wow2-finish61 --self-test\n"
       "Reads graph6 records on stdin; tabulates Lemma 7 checks, the\n"
       "two-connected (DL) census, and free-attachment repairs.");
}

int main(int argc, char **argv) {
  unsigned max_repair_order = WOW2_MAX_VERTICES;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--self-test") == 0) {
      self_test();
      return EXIT_SUCCESS;
    } else if (strcmp(argv[i], "--max-repair-order") == 0 && i + 1 < argc) {
      max_repair_order = (unsigned)strtoul(argv[++i], NULL, 10);
    } else if (strcmp(argv[i], "--help") == 0) {
      usage();
      return EXIT_SUCCESS;
    } else {
      usage();
      return EXIT_FAILURE;
    }
  }

  char line[4096];
  Stats stats = {0};
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
      return EXIT_FAILURE;
    }
    if (graph.n < 2 || !graph_connected(&graph)) {
      die("input contains a trivial or disconnected graph");
    }
    analyze(&graph, &stats, max_repair_order);
  }
  print_stats(&stats);
  return EXIT_SUCCESS;
}
