#ifndef WOW2_INVARIANTS_H
#define WOW2_INVARIANTS_H

#include "graph.h"
#include "rational.h"

/*
 * Exact graph invariants used by the WOWII conjecture registry.
 *
 * Every definition here is transcribed from the pinned Lean source in
 * google-deepmind/formal-conjectures, not from the ambient graph-theory
 * convention, because the two disagree in places that decide verdicts. Three
 * cases worth naming, each recorded at its accessor below:
 *
 *   - girth is 0 for an acyclic graph, not infinity;
 *   - ecc(S) maximizes over vertices OUTSIDE S, eccSet over ALL vertices;
 *   - a total dominating set requires every vertex, including its own members,
 *     to have a NEIGHBOUR in the set.
 *
 * Values are computed lazily and cached: a conjecture that needs only the
 * distance matrix never pays for the Hamiltonian-path search. Accessors take a
 * mutable cache for that reason.
 */

enum {
  /*
   * Invariants whose exact algorithm enumerates or memoizes over vertex
   * subsets. At order 20 the Hamiltonian-path table is 8 MB; past that the
   * sweeper reports a skip rather than silently truncating coverage.
   */
  WOW2_SUBSET_ORDER_LIMIT = 20,
  /*
   * The path-cover search carries an endpoint per state and cannot use the
   * usual "start each path at the lowest uncovered vertex" canonicalization
   * (see invariants.c), so it costs a factor of n more and stops earlier.
   */
  WOW2_PATH_COVER_ORDER_LIMIT = 18,
  WOW2_UNREACHABLE = 255,
};

typedef struct {
  const Graph *graph;
  uint64_t computed;

  uint8_t distance[WOW2_MAX_VERTICES][WOW2_MAX_VERTICES];
  unsigned eccentricity[WOW2_MAX_VERTICES];
  unsigned radius;
  unsigned diameter;
  uint64_t center;
  uint64_t boundary;

  unsigned girth;
  unsigned graph_square_radius;

  unsigned local_independence[WOW2_MAX_VERTICES];
  unsigned local_independence_max;
  unsigned local_independence_min;
  unsigned local_independence_sum;

  Graph complement;
  unsigned complement_local_independence_min;

  InducedResult induced_forest;
  InducedResult induced_bipartite;
  unsigned independence_number;
  unsigned largest_induced_tree;
  unsigned largest_induced_path;
  unsigned path_cover_number;
  unsigned max_leaf_spanning_tree;

  unsigned triangles[WOW2_MAX_VERTICES];
  unsigned triangles_max;
  unsigned triangles_min;
  unsigned triangles_min_frequency;

  bool has_four_cycle;
  bool has_hamiltonian_path;
  unsigned total_domination_number;
  bool well_totally_dominated;
  unsigned havel_hakimi_zero_step;
  unsigned pendant_count;
} Invariants;

void invariants_init(Invariants *cache, const Graph *graph);

/* True when the graph is small enough for the subset-enumerating invariants. */
bool invariants_supports_subsets(const Graph *graph);

const uint8_t (*invariants_distance(Invariants *cache))[WOW2_MAX_VERTICES];
unsigned invariants_eccentricity(Invariants *cache, unsigned vertex);
unsigned invariants_radius(Invariants *cache);
unsigned invariants_diameter(Invariants *cache);

/* Vertices of minimum eccentricity. */
uint64_t invariants_center(Invariants *cache);
/* Vertices of maximum eccentricity; DeLaVina's boundary set B. */
uint64_t invariants_boundary(Invariants *cache);

/* Sum of eccentricities over all vertices, for the average-eccentricity term. */
unsigned invariants_eccentricity_sum(Invariants *cache);

/*
 * Lean `ecc G S`: max over v NOT in S of dist(v, S). Zero when S is everything.
 */
unsigned invariants_set_ecc(Invariants *cache, uint64_t set);
/*
 * Lean `eccSet G S`: max over ALL v of dist(v, S). Zero when S is empty.
 */
unsigned invariants_set_ecc_all(Invariants *cache, uint64_t set);
/*
 * Lean `distMin G S`: min over v NOT in S of dist(v, S). Zero when S is
 * everything.
 */
unsigned invariants_set_dist_min(Invariants *cache, uint64_t set);

/* Length of a shortest cycle; 0 when the graph is acyclic. */
unsigned invariants_girth(Invariants *cache);
/* Radius of the square graph, where u ~ v iff their distance is at most 2. */
unsigned invariants_graph_square_radius(Invariants *cache);

/* Independence number of the subgraph induced on the open neighbourhood. */
unsigned invariants_local_independence(Invariants *cache, unsigned vertex);
unsigned invariants_local_independence_max(Invariants *cache);
unsigned invariants_local_independence_min(Invariants *cache);
/* Average local independence, exact: the sum over the order. */
Rational invariants_local_independence_average(Invariants *cache);

const Graph *invariants_complement(Invariants *cache);
unsigned invariants_complement_local_independence_min(Invariants *cache);
Rational invariants_complement_average_degree(Invariants *cache);

/* The two optimizers already in graph.c, memoized so shared modes pay once. */
InducedResult invariants_induced_forest(Invariants *cache);
InducedResult invariants_induced_bipartite(Invariants *cache);

unsigned invariants_independence_number(Invariants *cache);
/* Largest induced subgraph that is a tree: connected and acyclic. */
unsigned invariants_largest_induced_tree(Invariants *cache);
/* Largest induced tree whose every vertex has degree at most two. */
unsigned invariants_largest_induced_path(Invariants *cache);
/* Fewest vertex-disjoint paths partitioning the vertex set. */
unsigned invariants_path_cover_number(Invariants *cache);
/* Most leaves over all spanning trees. */
unsigned invariants_max_leaf_spanning_tree(Invariants *cache);

unsigned invariants_triangles(Invariants *cache, unsigned vertex);
unsigned invariants_triangles_max(Invariants *cache);
unsigned invariants_triangles_min_frequency(Invariants *cache);
/* True when a four-cycle exists; it need not be induced. */
bool invariants_has_four_cycle(Invariants *cache);
bool invariants_triangle_free(Invariants *cache);

bool invariants_has_hamiltonian_path(Invariants *cache);
unsigned invariants_total_domination_number(Invariants *cache);
/* True when every minimal total dominating set has the same cardinality. */
bool invariants_well_totally_dominated(Invariants *cache);

/* First Havel-Hakimi step, counting from zero, at which a zero appears. */
unsigned invariants_havel_hakimi_zero_step(Invariants *cache);
unsigned invariants_pendant_count(Invariants *cache);

Rational invariants_average_eccentricity(Invariants *cache);

#endif
