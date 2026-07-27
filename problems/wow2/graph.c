#include "graph.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  uint64_t *keys;
  size_t capacity;
  size_t count;
} MaskSet;

typedef struct {
  const Graph *graph;
  unsigned best_deleted;
  uint64_t best_keep;
  MaskSet seen;
  bool odd_cycles;
} DeletionSearch;

static uint64_t vertex_mask(unsigned n) {
  return n == 0 ? 0 : (UINT64_C(1) << n) - 1;
}

static unsigned popcount64(uint64_t value) {
  return (unsigned)__builtin_popcountll(value);
}

static unsigned first_vertex(uint64_t vertices) {
  return (unsigned)__builtin_ctzll(vertices);
}

void graph_clear(Graph *graph, unsigned n) {
  if (n > WOW2_MAX_VERTICES) {
    abort();
  }
  graph->n = (uint8_t)n;
  memset(graph->adj, 0, sizeof(graph->adj));
}

void graph_add_edge(Graph *graph, unsigned u, unsigned v) {
  if (u >= graph->n || v >= graph->n || u == v) {
    abort();
  }
  graph->adj[u] |= UINT64_C(1) << v;
  graph->adj[v] |= UINT64_C(1) << u;
}

void graph_remove_edge(Graph *graph, unsigned u, unsigned v) {
  if (u >= graph->n || v >= graph->n || u == v) {
    abort();
  }
  graph->adj[u] &= ~(UINT64_C(1) << v);
  graph->adj[v] &= ~(UINT64_C(1) << u);
}

void graph_toggle_edge(Graph *graph, unsigned u, unsigned v) {
  if (graph_has_edge(graph, u, v)) {
    graph_remove_edge(graph, u, v);
  } else {
    graph_add_edge(graph, u, v);
  }
}

void graph_delete_vertex(const Graph *graph, unsigned vertex, Graph *result) {
  if (vertex >= graph->n || graph->n == 0 || result == graph) {
    abort();
  }

  graph_clear(result, graph->n - 1);
  for (unsigned u = 0; u < graph->n; u++) {
    if (u == vertex) {
      continue;
    }
    for (unsigned v = u + 1; v < graph->n; v++) {
      if (v == vertex || !graph_has_edge(graph, u, v)) {
        continue;
      }
      unsigned mapped_u = u - (u > vertex);
      unsigned mapped_v = v - (v > vertex);
      graph_add_edge(result, mapped_u, mapped_v);
    }
  }
}

bool graph_has_edge(const Graph *graph, unsigned u, unsigned v) {
  return u < graph->n && v < graph->n &&
         (graph->adj[u] & (UINT64_C(1) << v)) != 0;
}

unsigned graph_edge_count(const Graph *graph) {
  unsigned twice_edges = 0;
  for (unsigned u = 0; u < graph->n; u++) {
    twice_edges += popcount64(graph->adj[u]);
  }
  return twice_edges / 2;
}

bool graph_connected(const Graph *graph) {
  if (graph->n == 0) {
    return false;
  }

  uint64_t reached = UINT64_C(1);
  uint64_t frontier = reached;
  while (frontier != 0) {
    unsigned u = first_vertex(frontier);
    frontier &= frontier - 1;
    uint64_t fresh = graph->adj[u] & ~reached;
    reached |= fresh;
    frontier |= fresh;
  }
  return reached == vertex_mask(graph->n);
}

int graph_diameter(const Graph *graph) {
  if (graph->n == 0) {
    return -1;
  }

  int diameter = 0;
  for (unsigned source = 0; source < graph->n; source++) {
    int distance[WOW2_MAX_VERTICES];
    unsigned queue[WOW2_MAX_VERTICES];
    unsigned head = 0;
    unsigned tail = 0;
    for (unsigned i = 0; i < graph->n; i++) {
      distance[i] = -1;
    }
    distance[source] = 0;
    queue[tail++] = source;

    while (head < tail) {
      unsigned u = queue[head++];
      uint64_t neighbors = graph->adj[u];
      while (neighbors != 0) {
        unsigned v = first_vertex(neighbors);
        neighbors &= neighbors - 1;
        if (distance[v] >= 0) {
          continue;
        }
        distance[v] = distance[u] + 1;
        if (distance[v] > diameter) {
          diameter = distance[v];
        }
        queue[tail++] = v;
      }
    }

    if (tail != graph->n) {
      return -1;
    }
  }
  return diameter;
}

static int compare_int_desc(const void *left, const void *right) {
  int a = *(const int *)left;
  int b = *(const int *)right;
  return (b > a) - (b < a);
}

unsigned graph_residue(const Graph *graph) {
  int degrees[WOW2_MAX_VERTICES];
  unsigned length = graph->n;
  for (unsigned u = 0; u < graph->n; u++) {
    degrees[u] = (int)popcount64(graph->adj[u]);
  }

  while (length > 0) {
    qsort(degrees, length, sizeof(*degrees), compare_int_desc);
    int degree = degrees[0];
    if (degree == 0) {
      return length;
    }
    if (degree < 0 || (unsigned)degree >= length) {
      abort();
    }
    for (int i = 1; i <= degree; i++) {
      degrees[i]--;
      if (degrees[i] < 0) {
        abort();
      }
    }
    memmove(degrees, degrees + 1, (length - 1) * sizeof(*degrees));
    length--;
  }
  return 0;
}

bool graph_subset_is_forest(const Graph *graph, uint64_t vertices) {
  uint64_t unseen = vertices & vertex_mask(graph->n);
  unsigned component_count = 0;
  unsigned twice_edges = 0;

  uint64_t scan = unseen;
  while (scan != 0) {
    unsigned u = first_vertex(scan);
    scan &= scan - 1;
    twice_edges += popcount64(graph->adj[u] & vertices);
  }

  while (unseen != 0) {
    component_count++;
    uint64_t frontier = UINT64_C(1) << first_vertex(unseen);
    unseen &= ~frontier;
    while (frontier != 0) {
      unsigned u = first_vertex(frontier);
      frontier &= frontier - 1;
      uint64_t fresh = graph->adj[u] & unseen;
      unseen &= ~fresh;
      frontier |= fresh;
    }
  }

  unsigned vertex_count = popcount64(vertices);
  unsigned edge_count = twice_edges / 2;
  return edge_count + component_count == vertex_count;
}

bool graph_subset_is_bipartite(const Graph *graph, uint64_t vertices) {
  int8_t color[WOW2_MAX_VERTICES] = {0};
  unsigned queue[WOW2_MAX_VERTICES];
  uint64_t remaining = vertices & vertex_mask(graph->n);

  while (remaining != 0) {
    unsigned source = first_vertex(remaining);
    unsigned head = 0;
    unsigned tail = 0;
    color[source] = 1;
    remaining &= ~(UINT64_C(1) << source);
    queue[tail++] = source;

    while (head < tail) {
      unsigned u = queue[head++];
      uint64_t neighbors = graph->adj[u] & vertices;
      while (neighbors != 0) {
        unsigned v = first_vertex(neighbors);
        neighbors &= neighbors - 1;
        if (color[v] == color[u]) {
          return false;
        }
        if (color[v] != 0) {
          continue;
        }
        color[v] = (int8_t)-color[u];
        remaining &= ~(UINT64_C(1) << v);
        queue[tail++] = v;
      }
    }
  }
  return true;
}

static uint64_t mix64(uint64_t value) {
  value += UINT64_C(0x9e3779b97f4a7c15);
  value = (value ^ (value >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
  value = (value ^ (value >> 27)) * UINT64_C(0x94d049bb133111eb);
  return value ^ (value >> 31);
}

static void mask_set_init(MaskSet *set, size_t capacity) {
  set->capacity = 1;
  while (set->capacity < capacity) {
    set->capacity *= 2;
  }
  set->count = 0;
  set->keys = malloc(set->capacity * sizeof(*set->keys));
  if (set->keys == NULL) {
    abort();
  }
  for (size_t i = 0; i < set->capacity; i++) {
    set->keys[i] = UINT64_MAX;
  }
}

static void mask_set_destroy(MaskSet *set) {
  free(set->keys);
  set->keys = NULL;
  set->capacity = 0;
  set->count = 0;
}

static void mask_set_rehash(MaskSet *set) {
  MaskSet larger;
  mask_set_init(&larger, set->capacity * 2);
  for (size_t i = 0; i < set->capacity; i++) {
    uint64_t key = set->keys[i];
    if (key == UINT64_MAX) {
      continue;
    }
    size_t index = (size_t)mix64(key) & (larger.capacity - 1);
    while (larger.keys[index] != UINT64_MAX) {
      index = (index + 1) & (larger.capacity - 1);
    }
    larger.keys[index] = key;
    larger.count++;
  }
  mask_set_destroy(set);
  *set = larger;
}

static bool mask_set_insert(MaskSet *set, uint64_t key) {
  if ((set->count + 1) * 10 >= set->capacity * 7) {
    mask_set_rehash(set);
  }
  size_t index = (size_t)mix64(key) & (set->capacity - 1);
  while (set->keys[index] != UINT64_MAX) {
    if (set->keys[index] == key) {
      return false;
    }
    index = (index + 1) & (set->capacity - 1);
  }
  set->keys[index] = key;
  set->count++;
  return true;
}

static bool cycle_dfs(const Graph *graph, uint64_t keep, unsigned u,
                      unsigned parent, uint8_t color[WOW2_MAX_VERTICES],
                      unsigned parents[WOW2_MAX_VERTICES],
                      uint64_t *cycle) {
  color[u] = 1;
  uint64_t neighbors = graph->adj[u] & keep;
  while (neighbors != 0) {
    unsigned v = first_vertex(neighbors);
    neighbors &= neighbors - 1;
    if (v == parent) {
      continue;
    }
    if (color[v] == 0) {
      parents[v] = u;
      if (cycle_dfs(graph, keep, v, u, color, parents, cycle)) {
        return true;
      }
    } else if (color[v] == 1) {
      uint64_t found = (UINT64_C(1) << u) | (UINT64_C(1) << v);
      unsigned cursor = u;
      while (cursor != v) {
        cursor = parents[cursor];
        found |= UINT64_C(1) << cursor;
      }
      *cycle = found;
      return true;
    }
  }
  color[u] = 2;
  return false;
}

static bool find_cycle(const Graph *graph, uint64_t keep, uint64_t *cycle) {
  uint8_t color[WOW2_MAX_VERTICES] = {0};
  unsigned parents[WOW2_MAX_VERTICES] = {0};
  uint64_t remaining = keep;
  while (remaining != 0) {
    unsigned source = first_vertex(remaining);
    if (color[source] == 0 &&
        cycle_dfs(graph, keep, source, UINT_MAX, color, parents, cycle)) {
      return true;
    }
    remaining &= ~(UINT64_C(1) << source);
    for (unsigned u = 0; u < graph->n; u++) {
      if (color[u] != 0) {
        remaining &= ~(UINT64_C(1) << u);
      }
    }
  }
  return false;
}

static bool find_odd_cycle(const Graph *graph, uint64_t keep,
                           uint64_t *cycle) {
  int8_t color[WOW2_MAX_VERTICES] = {0};
  unsigned parent[WOW2_MAX_VERTICES] = {0};
  unsigned depth[WOW2_MAX_VERTICES] = {0};
  unsigned queue[WOW2_MAX_VERTICES];
  uint64_t remaining = keep;

  while (remaining != 0) {
    unsigned source = first_vertex(remaining);
    unsigned head = 0;
    unsigned tail = 0;
    color[source] = 1;
    parent[source] = source;
    depth[source] = 0;
    remaining &= ~(UINT64_C(1) << source);
    queue[tail++] = source;

    while (head < tail) {
      unsigned u = queue[head++];
      uint64_t neighbors = graph->adj[u] & keep;
      while (neighbors != 0) {
        unsigned v = first_vertex(neighbors);
        neighbors &= neighbors - 1;
        if (color[v] == 0) {
          color[v] = (int8_t)-color[u];
          parent[v] = u;
          depth[v] = depth[u] + 1;
          remaining &= ~(UINT64_C(1) << v);
          queue[tail++] = v;
          continue;
        }
        if (color[v] != color[u]) {
          continue;
        }

        unsigned a = u;
        unsigned b = v;
        uint64_t found = 0;
        while (depth[a] > depth[b]) {
          found |= UINT64_C(1) << a;
          a = parent[a];
        }
        while (depth[b] > depth[a]) {
          found |= UINT64_C(1) << b;
          b = parent[b];
        }
        while (a != b) {
          found |= (UINT64_C(1) << a) | (UINT64_C(1) << b);
          a = parent[a];
          b = parent[b];
        }
        found |= UINT64_C(1) << a;
        *cycle = found;
        return true;
      }
    }
  }
  return false;
}

static bool obstruction(const DeletionSearch *search, uint64_t keep,
                        uint64_t *vertices) {
  if (search->odd_cycles) {
    return find_odd_cycle(search->graph, keep, vertices);
  }
  return find_cycle(search->graph, keep, vertices);
}

static void greedy_upper_bound(DeletionSearch *search, uint64_t full) {
  uint64_t keep = full;
  uint64_t blocked = 0;
  while (obstruction(search, keep, &blocked)) {
    unsigned chosen = first_vertex(blocked);
    unsigned chosen_degree =
        popcount64(search->graph->adj[chosen] & keep);
    uint64_t choices = blocked & ~(UINT64_C(1) << chosen);
    while (choices != 0) {
      unsigned candidate = first_vertex(choices);
      choices &= choices - 1;
      unsigned degree =
          popcount64(search->graph->adj[candidate] & keep);
      if (degree > chosen_degree) {
        chosen = candidate;
        chosen_degree = degree;
      }
    }
    keep &= ~(UINT64_C(1) << chosen);
  }
  search->best_keep = keep;
  search->best_deleted = search->graph->n - popcount64(keep);
}

static void deletion_search(DeletionSearch *search, uint64_t keep) {
  unsigned deleted = search->graph->n - popcount64(keep);
  if (deleted >= search->best_deleted ||
      !mask_set_insert(&search->seen, keep)) {
    return;
  }

  uint64_t blocked = 0;
  if (!obstruction(search, keep, &blocked)) {
    search->best_deleted = deleted;
    search->best_keep = keep;
    return;
  }

  unsigned ordered[WOW2_MAX_VERTICES];
  unsigned order_count = 0;
  while (blocked != 0) {
    unsigned u = first_vertex(blocked);
    blocked &= blocked - 1;
    unsigned at = order_count;
    unsigned degree = popcount64(search->graph->adj[u] & keep);
    while (at > 0) {
      unsigned previous = ordered[at - 1];
      unsigned previous_degree =
          popcount64(search->graph->adj[previous] & keep);
      if (previous_degree >= degree) {
        break;
      }
      ordered[at] = previous;
      at--;
    }
    ordered[at] = u;
    order_count++;
  }

  for (unsigned i = 0; i < order_count; i++) {
    deletion_search(search, keep & ~(UINT64_C(1) << ordered[i]));
  }
}

static InducedResult largest_avoiding_obstruction(const Graph *graph,
                                                  bool odd_cycles) {
  DeletionSearch search = {
      .graph = graph,
      .best_deleted = graph->n,
      .best_keep = 0,
      .odd_cycles = odd_cycles,
  };
  mask_set_init(&search.seen, 1024);
  uint64_t full = vertex_mask(graph->n);
  greedy_upper_bound(&search, full);
  if (search.best_deleted > 0) {
    deletion_search(&search, full);
  }
  mask_set_destroy(&search.seen);

  InducedResult result = {
      .size = popcount64(search.best_keep),
      .vertices = search.best_keep,
  };
  return result;
}

InducedResult graph_largest_induced_forest(const Graph *graph) {
  return largest_avoiding_obstruction(graph, false);
}

InducedResult graph_largest_induced_bipartite(const Graph *graph) {
  return largest_avoiding_obstruction(graph, true);
}

static void set_error(char *error, size_t error_size, const char *message) {
  if (error != NULL && error_size > 0) {
    snprintf(error, error_size, "%s", message);
  }
}

bool graph6_parse(const char *text, Graph *graph, char *error,
                  size_t error_size) {
  while (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n') {
    text++;
  }
  size_t length = strcspn(text, "\r\n \t");
  if (length == 0) {
    set_error(error, error_size, "empty graph6 record");
    return false;
  }
  unsigned first = (unsigned char)text[0];
  if (first < 63 || first > 125) {
    set_error(error, error_size, "only short graph6 records are supported");
    return false;
  }
  unsigned n = first - 63;
  if (n > WOW2_MAX_VERTICES) {
    set_error(error, error_size, "graph exceeds WOW2_MAX_VERTICES");
    return false;
  }
  size_t bit_count = (size_t)n * (n - 1) / 2;
  size_t character_count = (bit_count + 5) / 6;
  if (length != character_count + 1) {
    set_error(error, error_size, "wrong graph6 record length");
    return false;
  }

  graph_clear(graph, n);
  size_t bit_index = 0;
  for (unsigned j = 1; j < n; j++) {
    for (unsigned i = 0; i < j; i++) {
      unsigned encoded = (unsigned char)text[1 + bit_index / 6];
      if (encoded < 63 || encoded > 126) {
        set_error(error, error_size, "invalid graph6 character");
        return false;
      }
      unsigned bit = 5 - (unsigned)(bit_index % 6);
      if (((encoded - 63) & (1U << bit)) != 0) {
        graph_add_edge(graph, i, j);
      }
      bit_index++;
    }
  }
  return true;
}

bool graph6_encode(const Graph *graph, char *output, size_t output_size) {
  size_t bit_count = (size_t)graph->n * (graph->n - 1) / 2;
  size_t character_count = (bit_count + 5) / 6;
  if (graph->n > 62 || output_size < character_count + 2) {
    return false;
  }
  output[0] = (char)(graph->n + 63);
  memset(output + 1, 63, character_count);

  size_t bit_index = 0;
  for (unsigned j = 1; j < graph->n; j++) {
    for (unsigned i = 0; i < j; i++) {
      if (graph_has_edge(graph, i, j)) {
        unsigned bit = 5 - (unsigned)(bit_index % 6);
        output[1 + bit_index / 6] =
            (char)(output[1 + bit_index / 6] + (1U << bit));
      }
      bit_index++;
    }
  }
  output[1 + character_count] = '\0';
  return true;
}

void graph_print_edges(const Graph *graph, FILE *stream) {
  bool first = true;
  fputc('[', stream);
  for (unsigned u = 0; u < graph->n; u++) {
    for (unsigned v = u + 1; v < graph->n; v++) {
      if (!graph_has_edge(graph, u, v)) {
        continue;
      }
      fprintf(stream, "%s[%u,%u]", first ? "" : ",", u, v);
      first = false;
    }
  }
  fputc(']', stream);
}
