#include "conjectures.h"

#include <stdint.h>
#include <stdlib.h>

static unsigned ceil_sqrt(unsigned value) {
  unsigned root = 0;
  while ((uint64_t)root * root < value) {
    root++;
  }
  return root;
}

Wow2Evaluation wow2_evaluate(const Graph *graph, Wow2Mode mode) {
  Wow2Evaluation evaluation = {
      .n = graph->n,
      .edges = graph_edge_count(graph),
      .residue = graph_residue(graph),
      .diameter = graph_diameter(graph),
      .forest = graph_largest_induced_forest(graph),
  };
  if (evaluation.diameter < 0) {
    abort();
  }

  switch (mode) {
  case WOW2_MODE_61:
    evaluation.diameter_term = (unsigned)(evaluation.diameter + 2) / 3;
    evaluation.bound = evaluation.residue + evaluation.diameter_term;
    break;
  case WOW2_MODE_59:
    evaluation.bipartite = graph_largest_induced_bipartite(graph);
    evaluation.bound =
        ceil_sqrt(evaluation.residue * evaluation.bipartite.size);
    break;
  default:
    abort();
  }
  evaluation.phi = (int)evaluation.bound - (int)evaluation.forest.size;
  return evaluation;
}

const char *wow2_mode_name(Wow2Mode mode) {
  switch (mode) {
  case WOW2_MODE_59:
    return "59";
  case WOW2_MODE_61:
    return "61";
  default:
    return "unknown";
  }
}
