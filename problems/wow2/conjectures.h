#ifndef WOW2_CONJECTURES_H
#define WOW2_CONJECTURES_H

#include "graph.h"

typedef enum {
  WOW2_MODE_59 = 59,
  WOW2_MODE_61 = 61,
} Wow2Mode;

typedef struct {
  unsigned n;
  unsigned edges;
  unsigned residue;
  int diameter;
  InducedResult forest;
  InducedResult bipartite;
  unsigned diameter_term;
  unsigned bound;
  int phi;
} Wow2Evaluation;

/*
 * This is the conjecture plug-in boundary. Graph generation, graph6 I/O,
 * exact invariants, and search do not know the shape of an inequality.
 * Add a mode here and one deterministic evaluation branch in conjectures.c.
 */
Wow2Evaluation wow2_evaluate(const Graph *graph, Wow2Mode mode);
const char *wow2_mode_name(Wow2Mode mode);

#endif
