#include "reduction61.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

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

static unsigned component_masks(
    const Graph *graph, uint64_t masks[WOW2_MAX_VERTICES]) {
  uint64_t unseen = vertex_mask(graph->n);
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

unsigned wow2_bound61(const Graph *connected_graph) {
  int diameter = graph_diameter(connected_graph);
  if (connected_graph->n == 0 || diameter < 0) {
    abort();
  }
  return graph_residue(connected_graph) +
         (unsigned)(diameter + 2) / 3;
}

unsigned wow2_component_bound61(const Graph *graph,
                                unsigned *component_count) {
  if (graph->n == 0) {
    if (component_count != NULL) {
      *component_count = 0;
    }
    return 0;
  }

  uint64_t masks[WOW2_MAX_VERTICES];
  unsigned count = component_masks(graph, masks);
  unsigned bound = 0;
  for (unsigned index = 0; index < count; index++) {
    Graph component;
    induced_subgraph(graph, masks[index], &component);
    bound += wow2_bound61(&component);
  }
  if (component_count != NULL) {
    *component_count = count;
  }
  return bound;
}

Wow2Reduction61 wow2_reduction61(const Graph *connected_graph) {
  if (connected_graph->n < 2 || !graph_connected(connected_graph)) {
    abort();
  }

  Wow2Reduction61 reduction = {
      .bound = wow2_bound61(connected_graph),
      .best_vertex = UINT_MAX,
      .total_margin = 0,
  };
  unsigned max_degree = maximum_degree(connected_graph);
  for (unsigned vertex = 0; vertex < connected_graph->n; vertex++) {
    Graph child;
    graph_delete_vertex(connected_graph, vertex, &child);
    unsigned components = 0;
    unsigned child_bound =
        wow2_component_bound61(&child, &components);
    int child_margin = (int)child_bound - (int)reduction.bound;
    reduction.total_margin += child_margin;
    if (child_bound >= reduction.bound) {
      reduction.reducible_vertices++;
      if (components == 1) {
        reduction.connected_reducible = true;
        if (degree(connected_graph, vertex) == max_degree) {
          reduction.maximum_degree_connected_reducible = true;
        }
      } else {
        reduction.cut_reducible = true;
      }
    }
    if (reduction.best_vertex == UINT_MAX ||
        child_bound > reduction.best_child_bound) {
      reduction.best_child_bound = child_bound;
      reduction.best_vertex = vertex;
      reduction.best_vertex_components = components;
    }
  }
  reduction.margin =
      (int)reduction.best_child_bound - (int)reduction.bound;
  return reduction;
}
