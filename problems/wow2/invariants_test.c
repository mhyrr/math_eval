/*
 * Brute-force oracle for the invariant kernel.
 *
 * Every invariant added for the WOWII registry is recomputed here by the most
 * obviously correct method available -- subset enumeration, permutation
 * enumeration, Floyd-Warshall -- and compared against the optimized routine on
 * every labeled graph up to order six. The reference implementations
 * deliberately share nothing with invariants.c beyond the graph representation.
 *
 * Two checks here are load bearing beyond simple agreement:
 *
 *   - the max-leaf spanning tree is compared against direct enumeration of
 *     spanning trees, which tests the connected-dominating-set identity the
 *     fast path relies on, including its failure at order two;
 *   - well-total-domination is compared against a full proper-subset minimality
 *     scan, which tests the upward-closure argument that reduces minimality to
 *     dropping a single vertex.
 */

#include "invariants.h"

#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

enum {
  REFERENCE_MAX_ORDER = 6,
  INFINITE_DISTANCE = 1000,
};

static unsigned popcount64(uint64_t value) {
  return (unsigned)__builtin_popcountll(value);
}

static unsigned first_vertex(uint64_t vertices) {
  return (unsigned)__builtin_ctzll(vertices);
}

static uint64_t bit(unsigned vertex) { return UINT64_C(1) << vertex; }

static uint64_t vertex_mask(unsigned n) {
  return n == 0 ? 0 : (UINT64_C(1) << n) - 1;
}

static void die(const char *message, const Graph *graph) {
  fprintf(stderr, "error: %s on ", message);
  graph_print_edges(graph, stderr);
  fputc('\n', stderr);
  exit(EXIT_FAILURE);
}

static void graph_from_labeled_code(Graph *graph, unsigned n, uint64_t code) {
  graph_clear(graph, n);
  unsigned index = 0;
  for (unsigned v = 1; v < n; v++) {
    for (unsigned u = 0; u < v; u++) {
      if ((code & (UINT64_C(1) << index)) != 0) {
        graph_add_edge(graph, u, v);
      }
      index++;
    }
  }
}

/* --------------------------------------------------------------- reference */

static void reference_distances(const Graph *graph,
                                int distance[WOW2_MAX_VERTICES]
                                            [WOW2_MAX_VERTICES]) {
  for (unsigned u = 0; u < graph->n; u++) {
    for (unsigned v = 0; v < graph->n; v++) {
      if (u == v) {
        distance[u][v] = 0;
      } else {
        distance[u][v] = graph_has_edge(graph, u, v) ? 1 : INFINITE_DISTANCE;
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
}

static unsigned reference_subset_edges(const Graph *graph, uint64_t vertices) {
  unsigned twice = 0;
  uint64_t scan = vertices;
  while (scan != 0) {
    unsigned u = first_vertex(scan);
    scan &= scan - 1;
    twice += popcount64(graph->adj[u] & vertices);
  }
  return twice / 2;
}

static bool reference_subset_connected(const Graph *graph, uint64_t vertices) {
  if (vertices == 0) {
    return false;
  }
  uint64_t reached = bit(first_vertex(vertices));
  uint64_t frontier = reached;
  while (frontier != 0) {
    unsigned u = first_vertex(frontier);
    frontier &= frontier - 1;
    uint64_t fresh = graph->adj[u] & vertices & ~reached;
    reached |= fresh;
    frontier |= fresh;
  }
  return reached == vertices;
}

static bool reference_subset_independent(const Graph *graph,
                                         uint64_t vertices) {
  uint64_t scan = vertices;
  while (scan != 0) {
    unsigned u = first_vertex(scan);
    scan &= scan - 1;
    if ((graph->adj[u] & vertices) != 0) {
      return false;
    }
  }
  return true;
}

static unsigned reference_independence(const Graph *graph, uint64_t allowed) {
  unsigned best = 0;
  for (uint64_t subset = allowed;; subset = (subset - 1) & allowed) {
    if (reference_subset_independent(graph, subset)) {
      unsigned size = popcount64(subset);
      if (size > best) {
        best = size;
      }
    }
    if (subset == 0) {
      break;
    }
  }
  return best;
}

static unsigned reference_largest_induced_tree(const Graph *graph) {
  unsigned best = 0;
  uint64_t full = vertex_mask(graph->n);
  for (uint64_t subset = 0; subset <= full; subset++) {
    if (subset == 0) {
      continue;
    }
    unsigned size = popcount64(subset);
    if (size <= best) {
      continue;
    }
    if (reference_subset_connected(graph, subset) &&
        reference_subset_edges(graph, subset) == size - 1) {
      best = size;
    }
  }
  return best;
}

static unsigned reference_largest_induced_path(const Graph *graph) {
  unsigned best = 0;
  uint64_t full = vertex_mask(graph->n);
  for (uint64_t subset = 0; subset <= full; subset++) {
    if (subset == 0) {
      continue;
    }
    unsigned size = popcount64(subset);
    if (size <= best) {
      continue;
    }
    if (!reference_subset_connected(graph, subset) ||
        reference_subset_edges(graph, subset) != size - 1) {
      continue;
    }
    bool thin = true;
    uint64_t scan = subset;
    while (scan != 0) {
      unsigned u = first_vertex(scan);
      scan &= scan - 1;
      if (popcount64(graph->adj[u] & subset) > 2) {
        thin = false;
        break;
      }
    }
    if (thin) {
      best = size;
    }
  }
  return best;
}

/* The shortest cycle has no chord, so it appears as an induced 2-regular set. */
static unsigned reference_girth(const Graph *graph) {
  unsigned best = UINT_MAX;
  uint64_t full = vertex_mask(graph->n);
  for (uint64_t subset = 0; subset <= full; subset++) {
    unsigned size = popcount64(subset);
    if (size < 3 || size >= best) {
      continue;
    }
    bool regular = true;
    uint64_t scan = subset;
    while (scan != 0) {
      unsigned u = first_vertex(scan);
      scan &= scan - 1;
      if (popcount64(graph->adj[u] & subset) != 2) {
        regular = false;
        break;
      }
    }
    if (regular && reference_subset_connected(graph, subset)) {
      best = size;
    }
  }
  return best == UINT_MAX ? 0 : best;
}

static bool reference_hamiltonian_path(const Graph *graph) {
  unsigned order[WOW2_MAX_VERTICES];
  for (unsigned i = 0; i < graph->n; i++) {
    order[i] = i;
  }
  /* Heap's algorithm over every vertex ordering. */
  unsigned counters[WOW2_MAX_VERTICES] = {0};
  for (;;) {
    bool ok = true;
    for (unsigned i = 0; i + 1 < graph->n; i++) {
      if (!graph_has_edge(graph, order[i], order[i + 1])) {
        ok = false;
        break;
      }
    }
    if (ok) {
      return true;
    }
    unsigned i = 0;
    while (i < graph->n) {
      if (counters[i] < i) {
        unsigned swap_with = (i % 2 == 0) ? 0 : counters[i];
        unsigned hold = order[swap_with];
        order[swap_with] = order[i];
        order[i] = hold;
        counters[i]++;
        break;
      }
      counters[i] = 0;
      i++;
    }
    if (i == graph->n) {
      return false;
    }
  }
}

/* Enumerate spanning trees directly rather than trusting the CDS identity. */
static unsigned reference_max_leaf_spanning_tree(const Graph *graph) {
  if (!graph_connected(graph)) {
    return 0;
  }
  unsigned edge_left[64];
  unsigned edge_right[64];
  unsigned edge_count = 0;
  for (unsigned u = 0; u < graph->n; u++) {
    for (unsigned v = u + 1; v < graph->n; v++) {
      if (graph_has_edge(graph, u, v)) {
        edge_left[edge_count] = u;
        edge_right[edge_count] = v;
        edge_count++;
      }
    }
  }

  unsigned best = 0;
  uint64_t limit = UINT64_C(1) << edge_count;
  for (uint64_t choice = 0; choice < limit; choice++) {
    if (popcount64(choice) != graph->n - 1u) {
      continue;
    }
    Graph tree;
    graph_clear(&tree, graph->n);
    uint64_t scan = choice;
    while (scan != 0) {
      unsigned index = first_vertex(scan);
      scan &= scan - 1;
      graph_add_edge(&tree, edge_left[index], edge_right[index]);
    }
    if (!graph_connected(&tree)) {
      continue;
    }
    unsigned leaves = 0;
    for (unsigned v = 0; v < tree.n; v++) {
      if (popcount64(tree.adj[v]) == 1) {
        leaves++;
      }
    }
    if (leaves > best) {
      best = leaves;
    }
  }
  return best;
}

/* Minimum path cover = order minus the most edges in a spanning linear forest. */
static unsigned reference_path_cover(const Graph *graph) {
  unsigned edge_left[64];
  unsigned edge_right[64];
  unsigned edge_count = 0;
  for (unsigned u = 0; u < graph->n; u++) {
    for (unsigned v = u + 1; v < graph->n; v++) {
      if (graph_has_edge(graph, u, v)) {
        edge_left[edge_count] = u;
        edge_right[edge_count] = v;
        edge_count++;
      }
    }
  }

  unsigned best_edges = 0;
  uint64_t limit = UINT64_C(1) << edge_count;
  for (uint64_t choice = 0; choice < limit; choice++) {
    unsigned size = popcount64(choice);
    if (size <= best_edges) {
      continue;
    }
    Graph forest;
    graph_clear(&forest, graph->n);
    uint64_t scan = choice;
    bool ok = true;
    while (scan != 0) {
      unsigned index = first_vertex(scan);
      scan &= scan - 1;
      graph_add_edge(&forest, edge_left[index], edge_right[index]);
    }
    for (unsigned v = 0; v < forest.n && ok; v++) {
      if (popcount64(forest.adj[v]) > 2) {
        ok = false;
      }
    }
    if (ok && graph_subset_is_forest(&forest, vertex_mask(forest.n))) {
      best_edges = size;
    }
  }
  return graph->n - best_edges;
}

static bool reference_is_total_dominating(const Graph *graph, uint64_t set) {
  for (unsigned v = 0; v < graph->n; v++) {
    if ((graph->adj[v] & set) == 0) {
      return false;
    }
  }
  return true;
}

/* Minimality by scanning every proper subset, not just single removals. */
static bool reference_well_totally_dominated(const Graph *graph,
                                             unsigned *number) {
  uint64_t full = vertex_mask(graph->n);
  unsigned smallest = UINT_MAX;
  unsigned minimal_low = UINT_MAX;
  unsigned minimal_high = 0;
  for (uint64_t set = 1; set <= full; set++) {
    if (!reference_is_total_dominating(graph, set)) {
      continue;
    }
    unsigned size = popcount64(set);
    if (size < smallest) {
      smallest = size;
    }
    bool minimal = true;
    for (uint64_t subset = (set - 1) & set;; subset = (subset - 1) & set) {
      if (reference_is_total_dominating(graph, subset)) {
        minimal = false;
        break;
      }
      if (subset == 0) {
        break;
      }
    }
    if (!minimal) {
      continue;
    }
    if (size < minimal_low) {
      minimal_low = size;
    }
    if (size > minimal_high) {
      minimal_high = size;
    }
  }
  *number = smallest == UINT_MAX ? 0 : smallest;
  return smallest == UINT_MAX ? true : minimal_low == minimal_high;
}

/* -------------------------------------------------------------- the checks */

static void check_graph(const Graph *graph, bool connected) {
  Invariants cache;
  invariants_init(&cache, graph);

  if (invariants_independence_number(&cache) !=
      reference_independence(graph, vertex_mask(graph->n))) {
    die("independence number", graph);
  }
  for (unsigned v = 0; v < graph->n; v++) {
    if (invariants_local_independence(&cache, v) !=
        reference_independence(graph, graph->adj[v])) {
      die("local independence", graph);
    }
  }
  if (invariants_largest_induced_tree(&cache) !=
      reference_largest_induced_tree(graph)) {
    die("largest induced tree", graph);
  }
  if (invariants_largest_induced_path(&cache) !=
      reference_largest_induced_path(graph)) {
    die("largest induced path", graph);
  }
  if (invariants_girth(&cache) != reference_girth(graph)) {
    die("girth", graph);
  }
  if (invariants_has_hamiltonian_path(&cache) !=
      reference_hamiltonian_path(graph)) {
    die("hamiltonian path", graph);
  }
  if (invariants_max_leaf_spanning_tree(&cache) !=
      reference_max_leaf_spanning_tree(graph)) {
    die("max-leaf spanning tree", graph);
  }
  if (invariants_path_cover_number(&cache) != reference_path_cover(graph)) {
    die("path cover number", graph);
  }

  unsigned expected_domination = 0;
  bool expected_well =
      reference_well_totally_dominated(graph, &expected_domination);
  if (invariants_total_domination_number(&cache) != expected_domination) {
    die("total domination number", graph);
  }
  if (invariants_well_totally_dominated(&cache) != expected_well) {
    die("well totally dominated", graph);
  }

  unsigned pendants = 0;
  for (unsigned v = 0; v < graph->n; v++) {
    if (popcount64(graph->adj[v]) == 1) {
      pendants++;
    }
  }
  if (invariants_pendant_count(&cache) != pendants) {
    die("pendant count", graph);
  }

  bool expected_four_cycle = false;
  for (unsigned u = 0; u < graph->n && !expected_four_cycle; u++) {
    for (unsigned v = u + 1; v < graph->n && !expected_four_cycle; v++) {
      for (unsigned a = 0; a < graph->n && !expected_four_cycle; a++) {
        for (unsigned b = a + 1; b < graph->n; b++) {
          if (a == u || a == v || b == u || b == v) {
            continue;
          }
          if (graph_has_edge(graph, u, a) && graph_has_edge(graph, a, v) &&
              graph_has_edge(graph, v, b) && graph_has_edge(graph, b, u)) {
            expected_four_cycle = true;
            break;
          }
        }
      }
    }
  }
  if (invariants_has_four_cycle(&cache) != expected_four_cycle) {
    die("four cycle", graph);
  }

  for (unsigned v = 0; v < graph->n; v++) {
    unsigned expected = 0;
    uint64_t neighbors = graph->adj[v];
    for (unsigned a = 0; a < graph->n; a++) {
      for (unsigned b = a + 1; b < graph->n; b++) {
        if ((neighbors & bit(a)) != 0 && (neighbors & bit(b)) != 0 &&
            graph_has_edge(graph, a, b)) {
          expected++;
        }
      }
    }
    if (invariants_triangles(&cache, v) != expected) {
      die("triangles at vertex", graph);
    }
  }

  if (!connected) {
    return;
  }

  int distance[WOW2_MAX_VERTICES][WOW2_MAX_VERTICES];
  reference_distances(graph, distance);
  const uint8_t (*fast)[WOW2_MAX_VERTICES] = invariants_distance(&cache);
  for (unsigned u = 0; u < graph->n; u++) {
    for (unsigned v = 0; v < graph->n; v++) {
      if ((int)fast[u][v] != distance[u][v]) {
        die("distance matrix", graph);
      }
    }
  }

  unsigned expected_radius = UINT_MAX;
  for (unsigned u = 0; u < graph->n; u++) {
    unsigned eccentricity = 0;
    for (unsigned v = 0; v < graph->n; v++) {
      if ((unsigned)distance[u][v] > eccentricity) {
        eccentricity = (unsigned)distance[u][v];
      }
    }
    if (invariants_eccentricity(&cache, u) != eccentricity) {
      die("eccentricity", graph);
    }
    if (eccentricity < expected_radius) {
      expected_radius = eccentricity;
    }
  }
  if (invariants_radius(&cache) != expected_radius) {
    die("radius", graph);
  }
  if (invariants_diameter(&cache) != (unsigned)graph_diameter(graph)) {
    die("diameter", graph);
  }

  /* ecc excludes the set, eccSet does not; distMin is the outer minimum. */
  uint64_t full = vertex_mask(graph->n);
  for (uint64_t set = 0; set <= full; set++) {
    unsigned expected_ecc = 0;
    unsigned expected_ecc_all = 0;
    unsigned expected_dist_min = UINT_MAX;
    bool any_outside = false;
    for (unsigned v = 0; v < graph->n; v++) {
      unsigned to_set = 0;
      if (set != 0) {
        to_set = UINT_MAX;
        for (unsigned s = 0; s < graph->n; s++) {
          if ((set & bit(s)) != 0 && (unsigned)distance[v][s] < to_set) {
            to_set = (unsigned)distance[v][s];
          }
        }
      }
      if (to_set > expected_ecc_all) {
        expected_ecc_all = to_set;
      }
      if ((set & bit(v)) == 0) {
        any_outside = true;
        if (to_set > expected_ecc) {
          expected_ecc = to_set;
        }
        if (to_set < expected_dist_min) {
          expected_dist_min = to_set;
        }
      }
    }
    if (!any_outside) {
      expected_ecc = 0;
      expected_dist_min = 0;
    }
    if (set == 0) {
      expected_ecc_all = 0;
    }
    if (invariants_set_ecc(&cache, set) != expected_ecc) {
      die("set eccentricity", graph);
    }
    if (invariants_set_ecc_all(&cache, set) != expected_ecc_all) {
      die("set eccentricity over all vertices", graph);
    }
    if (invariants_set_dist_min(&cache, set) != expected_dist_min) {
      die("set distance minimum", graph);
    }
  }

  unsigned expected_square_radius = UINT_MAX;
  for (unsigned u = 0; u < graph->n; u++) {
    unsigned eccentricity = 0;
    for (unsigned v = 0; v < graph->n; v++) {
      unsigned steps = (distance[u][v] + 1) / 2;
      if (steps > eccentricity) {
        eccentricity = steps;
      }
    }
    if (eccentricity < expected_square_radius) {
      expected_square_radius = eccentricity;
    }
  }
  if (invariants_graph_square_radius(&cache) != expected_square_radius) {
    die("graph square radius", graph);
  }
}

int main(void) {
  uint64_t all_graphs = 0;
  uint64_t connected_graphs = 0;
  for (unsigned n = 1; n <= REFERENCE_MAX_ORDER; n++) {
    unsigned slots = n * (n - 1) / 2;
    uint64_t limit = UINT64_C(1) << slots;
    for (uint64_t code = 0; code < limit; code++) {
      Graph graph;
      graph_from_labeled_code(&graph, n, code);
      bool connected = graph_connected(&graph);
      check_graph(&graph, connected);
      all_graphs++;
      connected_graphs += connected ? 1 : 0;
    }
  }
  printf("invariant self-test: PASS labeled_graphs=%" PRIu64
         " connected=%" PRIu64 " orders=1..%d\n",
         all_graphs, connected_graphs, REFERENCE_MAX_ORDER);
  return EXIT_SUCCESS;
}
