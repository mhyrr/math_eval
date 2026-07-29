#include "invariants.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  READY_DISTANCE = 1u << 0,
  READY_GIRTH = 1u << 1,
  READY_SQUARE_RADIUS = 1u << 2,
  READY_LOCAL_INDEPENDENCE = 1u << 3,
  READY_COMPLEMENT = 1u << 4,
  READY_COMPLEMENT_LOCAL_MIN = 1u << 5,
  READY_INDEPENDENCE = 1u << 6,
  READY_INDUCED_TREE = 1u << 7,
  READY_INDUCED_PATH = 1u << 8,
  READY_PATH_COVER = 1u << 9,
  READY_MAX_LEAF = 1u << 10,
  READY_TRIANGLES = 1u << 11,
  READY_FOUR_CYCLE = 1u << 12,
  READY_HAMILTONIAN = 1u << 13,
  READY_DOMINATION = 1u << 14,
  READY_ZERO_STEP = 1u << 15,
  READY_PENDANT = 1u << 16,
  READY_FOREST = 1u << 17,
  READY_BIPARTITE = 1u << 18,
} ReadyFlag;

static uint64_t vertex_mask(unsigned n) {
  return n == 0 ? 0 : (UINT64_C(1) << n) - 1;
}

static unsigned popcount64(uint64_t value) {
  return (unsigned)__builtin_popcountll(value);
}

static unsigned first_vertex(uint64_t vertices) {
  return (unsigned)__builtin_ctzll(vertices);
}

static uint64_t bit(unsigned vertex) { return UINT64_C(1) << vertex; }

static bool ready(Invariants *cache, ReadyFlag flag) {
  if ((cache->computed & (uint64_t)flag) != 0) {
    return true;
  }
  cache->computed |= (uint64_t)flag;
  return false;
}

void invariants_init(Invariants *cache, const Graph *graph) {
  memset(cache, 0, sizeof(*cache));
  cache->graph = graph;
}

bool invariants_supports_subsets(const Graph *graph) {
  return graph->n <= WOW2_SUBSET_ORDER_LIMIT;
}

/* ---------------------------------------------------------------- distance */

static void compute_distances(Invariants *cache) {
  const Graph *graph = cache->graph;
  for (unsigned source = 0; source < graph->n; source++) {
    uint8_t *row = cache->distance[source];
    memset(row, WOW2_UNREACHABLE, graph->n);
    unsigned queue[WOW2_MAX_VERTICES];
    unsigned head = 0;
    unsigned tail = 0;
    row[source] = 0;
    queue[tail++] = source;
    while (head < tail) {
      unsigned u = queue[head++];
      uint64_t neighbors = graph->adj[u];
      while (neighbors != 0) {
        unsigned v = first_vertex(neighbors);
        neighbors &= neighbors - 1;
        if (row[v] != WOW2_UNREACHABLE) {
          continue;
        }
        row[v] = (uint8_t)(row[u] + 1);
        queue[tail++] = v;
      }
    }
  }

  cache->radius = UINT_MAX;
  cache->diameter = 0;
  for (unsigned u = 0; u < graph->n; u++) {
    unsigned eccentricity = 0;
    for (unsigned v = 0; v < graph->n; v++) {
      unsigned distance = cache->distance[u][v];
      if (distance == WOW2_UNREACHABLE) {
        abort(); /* callers guarantee a connected graph */
      }
      if (distance > eccentricity) {
        eccentricity = distance;
      }
    }
    cache->eccentricity[u] = eccentricity;
    if (eccentricity < cache->radius) {
      cache->radius = eccentricity;
    }
    if (eccentricity > cache->diameter) {
      cache->diameter = eccentricity;
    }
  }

  cache->center = 0;
  cache->boundary = 0;
  for (unsigned u = 0; u < graph->n; u++) {
    if (cache->eccentricity[u] == cache->radius) {
      cache->center |= bit(u);
    }
    if (cache->eccentricity[u] == cache->diameter) {
      cache->boundary |= bit(u);
    }
  }
}

static void require_distance(Invariants *cache) {
  if (!ready(cache, READY_DISTANCE)) {
    compute_distances(cache);
  }
}

const uint8_t (*invariants_distance(Invariants *cache))[WOW2_MAX_VERTICES] {
  require_distance(cache);
  return cache->distance;
}

unsigned invariants_eccentricity(Invariants *cache, unsigned vertex) {
  require_distance(cache);
  return cache->eccentricity[vertex];
}

unsigned invariants_radius(Invariants *cache) {
  require_distance(cache);
  return cache->radius;
}

unsigned invariants_diameter(Invariants *cache) {
  require_distance(cache);
  return cache->diameter;
}

uint64_t invariants_center(Invariants *cache) {
  require_distance(cache);
  return cache->center;
}

uint64_t invariants_boundary(Invariants *cache) {
  require_distance(cache);
  return cache->boundary;
}

unsigned invariants_eccentricity_sum(Invariants *cache) {
  require_distance(cache);
  unsigned sum = 0;
  for (unsigned u = 0; u < cache->graph->n; u++) {
    sum += cache->eccentricity[u];
  }
  return sum;
}

Rational invariants_average_eccentricity(Invariants *cache) {
  return rational_make((int64_t)invariants_eccentricity_sum(cache),
                       (int64_t)cache->graph->n);
}

/* Lean `distToSet`: the minimum distance from `vertex` into `set`, 0 if empty. */
static unsigned distance_to_set(Invariants *cache, unsigned vertex,
                                uint64_t set) {
  require_distance(cache);
  if (set == 0) {
    return 0;
  }
  unsigned best = UINT_MAX;
  uint64_t scan = set;
  while (scan != 0) {
    unsigned target = first_vertex(scan);
    scan &= scan - 1;
    unsigned distance = cache->distance[vertex][target];
    if (distance < best) {
      best = distance;
    }
  }
  return best;
}

unsigned invariants_set_ecc(Invariants *cache, uint64_t set) {
  uint64_t outside = vertex_mask(cache->graph->n) & ~set;
  if (outside == 0) {
    return 0;
  }
  unsigned best = 0;
  while (outside != 0) {
    unsigned vertex = first_vertex(outside);
    outside &= outside - 1;
    unsigned distance = distance_to_set(cache, vertex, set);
    if (distance > best) {
      best = distance;
    }
  }
  return best;
}

unsigned invariants_set_ecc_all(Invariants *cache, uint64_t set) {
  if (set == 0) {
    return 0;
  }
  unsigned best = 0;
  uint64_t scan = vertex_mask(cache->graph->n);
  while (scan != 0) {
    unsigned vertex = first_vertex(scan);
    scan &= scan - 1;
    unsigned distance = distance_to_set(cache, vertex, set);
    if (distance > best) {
      best = distance;
    }
  }
  return best;
}

unsigned invariants_set_dist_min(Invariants *cache, uint64_t set) {
  uint64_t outside = vertex_mask(cache->graph->n) & ~set;
  if (outside == 0) {
    return 0;
  }
  unsigned best = UINT_MAX;
  while (outside != 0) {
    unsigned vertex = first_vertex(outside);
    outside &= outside - 1;
    unsigned distance = distance_to_set(cache, vertex, set);
    if (distance < best) {
      best = distance;
    }
  }
  return best;
}

/* ------------------------------------------------------------------- girth */

/*
 * Shortest cycle by breadth-first search from every vertex. For a fixed source
 * the value found is at least the girth and is exact when the source lies on a
 * shortest cycle, so the minimum over all sources is the girth. Zero means
 * acyclic, matching the Lean `girth` (its `egirth` is the one that returns the
 * top element).
 */
static void compute_girth(Invariants *cache) {
  const Graph *graph = cache->graph;
  unsigned best = UINT_MAX;
  for (unsigned source = 0; source < graph->n; source++) {
    unsigned distance[WOW2_MAX_VERTICES];
    unsigned parent[WOW2_MAX_VERTICES];
    unsigned queue[WOW2_MAX_VERTICES];
    unsigned head = 0;
    unsigned tail = 0;
    for (unsigned i = 0; i < graph->n; i++) {
      distance[i] = UINT_MAX;
      parent[i] = UINT_MAX;
    }
    distance[source] = 0;
    queue[tail++] = source;
    while (head < tail) {
      unsigned u = queue[head++];
      uint64_t neighbors = graph->adj[u];
      while (neighbors != 0) {
        unsigned v = first_vertex(neighbors);
        neighbors &= neighbors - 1;
        if (distance[v] == UINT_MAX) {
          distance[v] = distance[u] + 1;
          parent[v] = u;
          queue[tail++] = v;
          continue;
        }
        if (parent[u] == v || parent[v] == u) {
          continue;
        }
        unsigned candidate = distance[u] + distance[v] + 1;
        if (candidate < best) {
          best = candidate;
        }
      }
    }
  }
  cache->girth = best == UINT_MAX ? 0 : best;
}

unsigned invariants_girth(Invariants *cache) {
  if (!ready(cache, READY_GIRTH)) {
    compute_girth(cache);
  }
  return cache->girth;
}

unsigned invariants_graph_square_radius(Invariants *cache) {
  if (ready(cache, READY_SQUARE_RADIUS)) {
    return cache->graph_square_radius;
  }
  require_distance(cache);
  const Graph *graph = cache->graph;
  Graph square;
  graph_clear(&square, graph->n);
  for (unsigned u = 0; u < graph->n; u++) {
    for (unsigned v = u + 1; v < graph->n; v++) {
      if (cache->distance[u][v] <= 2) {
        graph_add_edge(&square, u, v);
      }
    }
  }

  unsigned radius = UINT_MAX;
  for (unsigned source = 0; source < square.n; source++) {
    unsigned distance[WOW2_MAX_VERTICES];
    unsigned queue[WOW2_MAX_VERTICES];
    unsigned head = 0;
    unsigned tail = 0;
    for (unsigned i = 0; i < square.n; i++) {
      distance[i] = UINT_MAX;
    }
    distance[source] = 0;
    queue[tail++] = source;
    unsigned eccentricity = 0;
    while (head < tail) {
      unsigned u = queue[head++];
      uint64_t neighbors = square.adj[u];
      while (neighbors != 0) {
        unsigned v = first_vertex(neighbors);
        neighbors &= neighbors - 1;
        if (distance[v] != UINT_MAX) {
          continue;
        }
        distance[v] = distance[u] + 1;
        if (distance[v] > eccentricity) {
          eccentricity = distance[v];
        }
        queue[tail++] = v;
      }
    }
    if (tail != square.n) {
      abort(); /* the square of a connected graph is connected */
    }
    if (eccentricity < radius) {
      radius = eccentricity;
    }
  }
  cache->graph_square_radius = radius == UINT_MAX ? 0 : radius;
  return cache->graph_square_radius;
}

/* ------------------------------------------------------- independent sets */

/*
 * Maximum independent set inside `allowed`, by branching on a maximum-degree
 * vertex. Isolated vertices are taken outright and a degree-one vertex is
 * always safe to take, which is what keeps the sparse graphs cheap.
 */
static unsigned max_independent_set(const Graph *graph, uint64_t allowed) {
  unsigned taken = 0;
  for (;;) {
    if (allowed == 0) {
      return taken;
    }
    unsigned pivot = WOW2_MAX_VERTICES;
    unsigned pivot_degree = 0;
    bool reduced = false;
    uint64_t scan = allowed;
    while (scan != 0) {
      unsigned v = first_vertex(scan);
      scan &= scan - 1;
      uint64_t neighbors = graph->adj[v] & allowed;
      unsigned degree = popcount64(neighbors);
      if (degree == 0) {
        taken++;
        allowed &= ~bit(v);
        reduced = true;
        break;
      }
      if (degree == 1) {
        taken++;
        allowed &= ~(bit(v) | neighbors);
        reduced = true;
        break;
      }
      if (degree > pivot_degree) {
        pivot_degree = degree;
        pivot = v;
      }
    }
    if (reduced) {
      continue;
    }

    uint64_t without_pivot = allowed & ~bit(pivot);
    unsigned excluding = max_independent_set(graph, without_pivot);
    unsigned including =
        1 + max_independent_set(graph, without_pivot & ~graph->adj[pivot]);
    return taken + (including > excluding ? including : excluding);
  }
}

InducedResult invariants_induced_forest(Invariants *cache) {
  if (!ready(cache, READY_FOREST)) {
    cache->induced_forest = graph_largest_induced_forest(cache->graph);
  }
  return cache->induced_forest;
}

InducedResult invariants_induced_bipartite(Invariants *cache) {
  if (!ready(cache, READY_BIPARTITE)) {
    cache->induced_bipartite = graph_largest_induced_bipartite(cache->graph);
  }
  return cache->induced_bipartite;
}

unsigned invariants_independence_number(Invariants *cache) {
  if (!ready(cache, READY_INDEPENDENCE)) {
    cache->independence_number =
        max_independent_set(cache->graph, vertex_mask(cache->graph->n));
  }
  return cache->independence_number;
}

static void compute_local_independence(Invariants *cache) {
  const Graph *graph = cache->graph;
  cache->local_independence_max = 0;
  cache->local_independence_min = UINT_MAX;
  cache->local_independence_sum = 0;
  for (unsigned v = 0; v < graph->n; v++) {
    unsigned value = max_independent_set(graph, graph->adj[v]);
    cache->local_independence[v] = value;
    cache->local_independence_sum += value;
    if (value > cache->local_independence_max) {
      cache->local_independence_max = value;
    }
    if (value < cache->local_independence_min) {
      cache->local_independence_min = value;
    }
  }
  if (graph->n == 0) {
    cache->local_independence_min = 0;
  }
}

static void require_local_independence(Invariants *cache) {
  if (!ready(cache, READY_LOCAL_INDEPENDENCE)) {
    compute_local_independence(cache);
  }
}

unsigned invariants_local_independence(Invariants *cache, unsigned vertex) {
  require_local_independence(cache);
  return cache->local_independence[vertex];
}

unsigned invariants_local_independence_max(Invariants *cache) {
  require_local_independence(cache);
  return cache->local_independence_max;
}

unsigned invariants_local_independence_min(Invariants *cache) {
  require_local_independence(cache);
  return cache->local_independence_min;
}

Rational invariants_local_independence_average(Invariants *cache) {
  require_local_independence(cache);
  return rational_make((int64_t)cache->local_independence_sum,
                       (int64_t)cache->graph->n);
}

/* -------------------------------------------------------------- complement */

const Graph *invariants_complement(Invariants *cache) {
  if (!ready(cache, READY_COMPLEMENT)) {
    const Graph *graph = cache->graph;
    graph_clear(&cache->complement, graph->n);
    for (unsigned u = 0; u < graph->n; u++) {
      for (unsigned v = u + 1; v < graph->n; v++) {
        if (!graph_has_edge(graph, u, v)) {
          graph_add_edge(&cache->complement, u, v);
        }
      }
    }
  }
  return &cache->complement;
}

unsigned invariants_complement_local_independence_min(Invariants *cache) {
  if (!ready(cache, READY_COMPLEMENT_LOCAL_MIN)) {
    const Graph *complement = invariants_complement(cache);
    unsigned best = UINT_MAX;
    for (unsigned v = 0; v < complement->n; v++) {
      unsigned value = max_independent_set(complement, complement->adj[v]);
      if (value < best) {
        best = value;
      }
    }
    cache->complement_local_independence_min = best == UINT_MAX ? 0 : best;
  }
  return cache->complement_local_independence_min;
}

Rational invariants_complement_average_degree(Invariants *cache) {
  const Graph *complement = invariants_complement(cache);
  return rational_make((int64_t)(2 * graph_edge_count(complement)),
                       (int64_t)complement->n);
}

/* ------------------------------------------------- induced trees and paths */

typedef struct {
  const Graph *graph;
  unsigned best;
  unsigned cap;
  bool paths_only;
} GrowthSearch;

/* Vertices reachable from `set` without passing through `banned`. */
static uint64_t reachable_from(const Graph *graph, uint64_t set,
                               uint64_t banned) {
  uint64_t reached = set;
  uint64_t frontier = set;
  while (frontier != 0) {
    unsigned u = first_vertex(frontier);
    frontier &= frontier - 1;
    uint64_t fresh = graph->adj[u] & ~reached & ~banned;
    reached |= fresh;
    frontier |= fresh;
  }
  return reached;
}

/*
 * Grow every induced tree containing `set` exactly once. A vertex may join the
 * current tree only if it has exactly one neighbour inside it: zero would
 * disconnect, two or more would close a cycle. Each candidate is tried as the
 * next addition and then banned, which is what makes the enumeration
 * duplicate-free. For the path variant the unique neighbour must also still
 * have room, keeping every degree at most two.
 */
static void grow_induced(GrowthSearch *search, uint64_t set, uint64_t banned) {
  unsigned size = popcount64(set);
  if (size > search->best) {
    search->best = size;
  }
  if (search->best >= search->cap) {
    return;
  }
  if (popcount64(reachable_from(search->graph, set, banned)) <= search->best) {
    return;
  }

  uint64_t frontier = 0;
  uint64_t scan = set;
  while (scan != 0) {
    unsigned u = first_vertex(scan);
    scan &= scan - 1;
    frontier |= search->graph->adj[u];
  }
  frontier &= ~set & ~banned;

  while (frontier != 0) {
    unsigned v = first_vertex(frontier);
    frontier &= frontier - 1;
    uint64_t inside = search->graph->adj[v] & set;
    if (popcount64(inside) != 1) {
      banned |= bit(v);
      continue;
    }
    if (search->paths_only) {
      unsigned anchor = first_vertex(inside);
      if (popcount64(search->graph->adj[anchor] & set) > 1) {
        banned |= bit(v);
        continue;
      }
    }
    grow_induced(search, set | bit(v), banned);
    banned |= bit(v);
    if (search->best >= search->cap) {
      return;
    }
  }
}

static unsigned largest_induced_growth(const Graph *graph, unsigned cap,
                                       bool paths_only) {
  GrowthSearch search = {
      .graph = graph,
      .best = graph->n == 0 ? 0 : 1,
      .cap = cap,
      .paths_only = paths_only,
  };
  uint64_t banned = 0;
  for (unsigned v = 0; v < graph->n; v++) {
    if (search.best >= search.cap) {
      break;
    }
    grow_induced(&search, bit(v), banned);
    banned |= bit(v);
  }
  return search.best;
}

unsigned invariants_largest_induced_tree(Invariants *cache) {
  if (!ready(cache, READY_INDUCED_TREE)) {
    /* An induced tree is an induced forest, so the forest size caps it. */
    unsigned cap = invariants_induced_forest(cache).size;
    cache->largest_induced_tree =
        largest_induced_growth(cache->graph, cap, false);
  }
  return cache->largest_induced_tree;
}

unsigned invariants_largest_induced_path(Invariants *cache) {
  if (!ready(cache, READY_INDUCED_PATH)) {
    unsigned cap = invariants_largest_induced_tree(cache);
    cache->largest_induced_path =
        largest_induced_growth(cache->graph, cap, true);
  }
  return cache->largest_induced_path;
}

/* --------------------------------------------- Hamiltonian and path covers */

unsigned invariants_path_cover_number(Invariants *cache) {
  if (ready(cache, READY_PATH_COVER)) {
    return cache->path_cover_number;
  }
  const Graph *graph = cache->graph;
  if (graph->n > WOW2_PATH_COVER_ORDER_LIMIT) {
    abort(); /* callers check invariants_supports_subsets first */
  }

  /*
   * State (covered set, endpoint of the path currently being extended). The
   * tempting canonicalization -- always begin the next path at the lowest
   * uncovered vertex -- is WRONG: that vertex need not be an endpoint of its
   * path in the optimal cover. So a new path may open at any uncovered vertex,
   * which costs the extra factor of n and is why this stops at a lower order
   * than the other subset algorithms.
   */
  size_t states = ((size_t)1 << graph->n) * graph->n;
  uint8_t *cost = malloc(states);
  if (cost == NULL) {
    abort();
  }
  memset(cost, 0xff, states);

  for (unsigned v = 0; v < graph->n; v++) {
    cost[((size_t)bit(v) * graph->n) + v] = 1;
  }

  uint64_t full = vertex_mask(graph->n);
  for (uint64_t covered = 1; covered <= full; covered++) {
    for (unsigned v = 0; v < graph->n; v++) {
      if ((covered & bit(v)) == 0) {
        continue;
      }
      uint8_t here = cost[((size_t)covered * graph->n) + v];
      if (here == 0xff) {
        continue;
      }
      uint64_t remaining = full & ~covered;
      uint64_t extend = graph->adj[v] & remaining;
      while (extend != 0) {
        unsigned u = first_vertex(extend);
        extend &= extend - 1;
        size_t index = ((size_t)(covered | bit(u)) * graph->n) + u;
        if (here < cost[index]) {
          cost[index] = here;
        }
      }
      uint64_t restart = remaining;
      while (restart != 0) {
        unsigned u = first_vertex(restart);
        restart &= restart - 1;
        size_t index = ((size_t)(covered | bit(u)) * graph->n) + u;
        if (here + 1 < cost[index]) {
          cost[index] = (uint8_t)(here + 1);
        }
      }
    }
  }

  unsigned best = UINT_MAX;
  for (unsigned v = 0; v < graph->n; v++) {
    uint8_t value = cost[((size_t)full * graph->n) + v];
    if (value != 0xff && value < best) {
      best = value;
    }
  }
  free(cost);
  if (best == UINT_MAX) {
    abort(); /* every graph has the all-singletons cover */
  }
  cache->path_cover_number = best;
  return best;
}

bool invariants_has_hamiltonian_path(Invariants *cache) {
  if (ready(cache, READY_HAMILTONIAN)) {
    return cache->has_hamiltonian_path;
  }
  const Graph *graph = cache->graph;
  if (graph->n > WOW2_SUBSET_ORDER_LIMIT) {
    abort();
  }
  if (graph->n == 1) {
    cache->has_hamiltonian_path = true;
    return true;
  }

  /* ends[covered] is the set of vertices at which such a path can finish. */
  size_t count = (size_t)1 << graph->n;
  uint64_t *ends = calloc(count, sizeof(*ends));
  if (ends == NULL) {
    abort();
  }
  for (unsigned v = 0; v < graph->n; v++) {
    ends[bit(v)] = bit(v);
  }
  uint64_t full = vertex_mask(graph->n);
  for (uint64_t covered = 1; covered <= full; covered++) {
    uint64_t reachable_ends = ends[covered];
    if (reachable_ends == 0) {
      continue;
    }
    uint64_t remaining = full & ~covered;
    while (remaining != 0) {
      unsigned u = first_vertex(remaining);
      remaining &= remaining - 1;
      if ((graph->adj[u] & reachable_ends) != 0) {
        ends[covered | bit(u)] |= bit(u);
      }
    }
  }
  cache->has_hamiltonian_path = ends[full] != 0;
  free(ends);
  return cache->has_hamiltonian_path;
}

/* ------------------------------------------------- max-leaf spanning trees */

unsigned invariants_max_leaf_spanning_tree(Invariants *cache) {
  if (ready(cache, READY_MAX_LEAF)) {
    return cache->max_leaf_spanning_tree;
  }
  const Graph *graph = cache->graph;
  if (graph->n > WOW2_SUBSET_ORDER_LIMIT) {
    abort();
  }
  if (!graph_connected(graph)) {
    cache->max_leaf_spanning_tree = 0; /* the Lean value for disconnected G */
    return 0;
  }
  if (graph->n <= 2) {
    /*
     * The internal vertices of a spanning tree form a connected dominating set
     * and the identity below inverts that. It breaks at the two smallest
     * orders, which are settled directly: the lone vertex of K(1) is a
     * spanning tree with no degree-one vertex at all, and the single edge of
     * K(2) is a spanning tree whose two endpoints are both leaves.
     */
    cache->max_leaf_spanning_tree = graph->n == 1 ? 0 : 2;
    return cache->max_leaf_spanning_tree;
  }

  uint64_t full = vertex_mask(graph->n);
  unsigned smallest = UINT_MAX;
  for (uint64_t candidate = 1; candidate <= full; candidate++) {
    unsigned size = popcount64(candidate);
    if (size >= smallest) {
      continue;
    }
    uint64_t dominated = candidate;
    uint64_t scan = candidate;
    while (scan != 0) {
      unsigned u = first_vertex(scan);
      scan &= scan - 1;
      dominated |= graph->adj[u];
    }
    if (dominated != full) {
      continue;
    }
    uint64_t reached = reachable_from(graph, bit(first_vertex(candidate)),
                                      full & ~candidate);
    if ((reached & candidate) != candidate) {
      continue;
    }
    smallest = size;
  }
  if (smallest == UINT_MAX) {
    abort(); /* the whole vertex set is connected and dominating */
  }
  cache->max_leaf_spanning_tree = graph->n - smallest;
  return cache->max_leaf_spanning_tree;
}

/* ------------------------------------------------------ triangles and C(4) */

static void compute_triangles(Invariants *cache) {
  const Graph *graph = cache->graph;
  cache->triangles_max = 0;
  cache->triangles_min = UINT_MAX;
  for (unsigned v = 0; v < graph->n; v++) {
    unsigned twice = 0;
    uint64_t neighbors = graph->adj[v];
    uint64_t scan = neighbors;
    while (scan != 0) {
      unsigned u = first_vertex(scan);
      scan &= scan - 1;
      twice += popcount64(graph->adj[u] & neighbors);
    }
    unsigned value = twice / 2;
    cache->triangles[v] = value;
    if (value > cache->triangles_max) {
      cache->triangles_max = value;
    }
    if (value < cache->triangles_min) {
      cache->triangles_min = value;
    }
  }
  if (graph->n == 0) {
    cache->triangles_min = 0;
  }
  cache->triangles_min_frequency = 0;
  for (unsigned v = 0; v < graph->n; v++) {
    if (cache->triangles[v] == cache->triangles_min) {
      cache->triangles_min_frequency++;
    }
  }
}

static void require_triangles(Invariants *cache) {
  if (!ready(cache, READY_TRIANGLES)) {
    compute_triangles(cache);
  }
}

unsigned invariants_triangles(Invariants *cache, unsigned vertex) {
  require_triangles(cache);
  return cache->triangles[vertex];
}

unsigned invariants_triangles_max(Invariants *cache) {
  require_triangles(cache);
  return cache->triangles_max;
}

unsigned invariants_triangles_min_frequency(Invariants *cache) {
  require_triangles(cache);
  return cache->triangles_min_frequency;
}

bool invariants_triangle_free(Invariants *cache) {
  require_triangles(cache);
  return cache->triangles_max == 0;
}

bool invariants_has_four_cycle(Invariants *cache) {
  if (!ready(cache, READY_FOUR_CYCLE)) {
    /*
     * A four-cycle a-b-c-d exists exactly when some pair of distinct vertices
     * has two common neighbours. The cycle need not be induced, which is what
     * DeLaVina's C(4)-free characteristic means.
     */
    const Graph *graph = cache->graph;
    cache->has_four_cycle = false;
    for (unsigned u = 0; u < graph->n && !cache->has_four_cycle; u++) {
      for (unsigned v = u + 1; v < graph->n; v++) {
        if (popcount64(graph->adj[u] & graph->adj[v]) >= 2) {
          cache->has_four_cycle = true;
          break;
        }
      }
    }
  }
  return cache->has_four_cycle;
}

/* --------------------------------------------------------- total domination */

static bool is_total_dominating(const Graph *graph, uint64_t set) {
  for (unsigned v = 0; v < graph->n; v++) {
    if ((graph->adj[v] & set) == 0) {
      return false;
    }
  }
  return true;
}

static void compute_domination(Invariants *cache) {
  const Graph *graph = cache->graph;
  if (graph->n > WOW2_SUBSET_ORDER_LIMIT) {
    abort();
  }

  uint64_t full = vertex_mask(graph->n);
  unsigned smallest = UINT_MAX;
  unsigned minimal_low = UINT_MAX;
  unsigned minimal_high = 0;
  for (uint64_t candidate = 1; candidate <= full; candidate++) {
    if (!is_total_dominating(graph, candidate)) {
      continue;
    }
    unsigned size = popcount64(candidate);
    if (size < smallest) {
      smallest = size;
    }
    /*
     * Total domination is upward closed, so a set has a total dominating proper
     * subset exactly when dropping one element leaves one. Dropping x breaks
     * total domination exactly when some vertex v has x as its only neighbour
     * inside the set -- a private neighbour for x. So the set is minimal when
     * every one of its members has one, and a single pass over the vertices
     * collecting the singleton intersections settles that in linear time
     * instead of re-testing the set once per member.
     */
    uint64_t privately_needed = 0;
    for (unsigned v = 0; v < graph->n; v++) {
      uint64_t inside = graph->adj[v] & candidate;
      if ((inside & (inside - 1)) == 0) {
        privately_needed |= inside;
      }
    }
    if (privately_needed != candidate) {
      continue;
    }
    if (size < minimal_low) {
      minimal_low = size;
    }
    if (size > minimal_high) {
      minimal_high = size;
    }
  }

  if (smallest == UINT_MAX) {
    /* No total dominating set exists; an isolated vertex is the only cause. */
    cache->total_domination_number = 0;
    cache->well_totally_dominated = true;
    return;
  }
  cache->total_domination_number = smallest;
  cache->well_totally_dominated = minimal_low == minimal_high;
}

static void require_domination(Invariants *cache) {
  if (!ready(cache, READY_DOMINATION)) {
    compute_domination(cache);
  }
}

unsigned invariants_total_domination_number(Invariants *cache) {
  require_domination(cache);
  return cache->total_domination_number;
}

bool invariants_well_totally_dominated(Invariants *cache) {
  require_domination(cache);
  return cache->well_totally_dominated;
}

/* ------------------------------------------------------------ degree facts */

static int compare_int_desc(const void *left, const void *right) {
  int a = *(const int *)left;
  int b = *(const int *)right;
  return (b > a) - (b < a);
}

unsigned invariants_havel_hakimi_zero_step(Invariants *cache) {
  if (ready(cache, READY_ZERO_STEP)) {
    return cache->havel_hakimi_zero_step;
  }
  const Graph *graph = cache->graph;
  int degrees[WOW2_MAX_VERTICES];
  unsigned length = graph->n;
  for (unsigned u = 0; u < graph->n; u++) {
    degrees[u] = (int)popcount64(graph->adj[u]);
  }

  for (unsigned step = 0;; step++) {
    qsort(degrees, length, sizeof(*degrees), compare_int_desc);
    if (length == 0 || degrees[length - 1] == 0) {
      cache->havel_hakimi_zero_step = step;
      return step;
    }
    int degree = degrees[0];
    if (degree < 0 || (unsigned)degree >= length) {
      abort();
    }
    for (int i = 1; i <= degree; i++) {
      degrees[i]--;
    }
    memmove(degrees, degrees + 1, (length - 1) * sizeof(*degrees));
    length--;
  }
}

unsigned invariants_pendant_count(Invariants *cache) {
  if (!ready(cache, READY_PENDANT)) {
    const Graph *graph = cache->graph;
    unsigned count = 0;
    for (unsigned v = 0; v < graph->n; v++) {
      if (popcount64(graph->adj[v]) == 1) {
        count++;
      }
    }
    cache->pendant_count = count;
  }
  return cache->pendant_count;
}
