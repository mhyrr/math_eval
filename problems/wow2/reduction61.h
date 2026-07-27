#ifndef WOW2_REDUCTION61_H
#define WOW2_REDUCTION61_H

#include "graph.h"

#include <stdbool.h>

typedef struct {
  unsigned bound;
  unsigned best_child_bound;
  unsigned best_vertex;
  unsigned best_vertex_components;
  unsigned reducible_vertices;
  int margin;
  int total_margin;
  bool connected_reducible;
  bool maximum_degree_connected_reducible;
  bool cut_reducible;
} Wow2Reduction61;

unsigned wow2_bound61(const Graph *connected_graph);
unsigned wow2_component_bound61(const Graph *graph,
                                unsigned *component_count);
Wow2Reduction61 wow2_reduction61(const Graph *connected_graph);

#endif
