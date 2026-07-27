#ifndef WOW2_GRAPH_H
#define WOW2_GRAPH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

enum {
  WOW2_MAX_VERTICES = 63,
};

typedef struct {
  uint8_t n;
  uint64_t adj[WOW2_MAX_VERTICES];
} Graph;

typedef struct {
  unsigned size;
  uint64_t vertices;
} InducedResult;

void graph_clear(Graph *graph, unsigned n);
void graph_add_edge(Graph *graph, unsigned u, unsigned v);
void graph_remove_edge(Graph *graph, unsigned u, unsigned v);
void graph_toggle_edge(Graph *graph, unsigned u, unsigned v);
void graph_delete_vertex(const Graph *graph, unsigned vertex, Graph *result);
bool graph_has_edge(const Graph *graph, unsigned u, unsigned v);
unsigned graph_edge_count(const Graph *graph);
bool graph_connected(const Graph *graph);
int graph_diameter(const Graph *graph);
unsigned graph_residue(const Graph *graph);

bool graph_subset_is_forest(const Graph *graph, uint64_t vertices);
bool graph_subset_is_bipartite(const Graph *graph, uint64_t vertices);
InducedResult graph_largest_induced_forest(const Graph *graph);
InducedResult graph_largest_induced_bipartite(const Graph *graph);

bool graph6_parse(const char *text, Graph *graph, char *error,
                  size_t error_size);
bool graph6_encode(const Graph *graph, char *output, size_t output_size);
void graph_print_edges(const Graph *graph, FILE *stream);

#endif
