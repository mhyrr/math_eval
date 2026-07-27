#include "graph.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Exact checker for the candidate MAX-packing lemma used in the #61 proof
 * attack. A state is the set of vertices still present. Each transition
 * deletes a current maximum-degree vertex; after n-residue(G) transitions,
 * Griggs--Kleitman guarantees that the remaining residue(G) vertices are
 * independent. The checker asks whether some such deletion set contains the
 * required distance-three packing, and separately whether every tie-breaking
 * sequence does.
 */
enum {
  EXACT_LIMIT = 20,
};

typedef struct {
  const Graph *graph;
  uint64_t all_vertices;
  uint64_t forbidden[WOW2_MAX_VERTICES];
  unsigned residue;
  unsigned target;
  uint8_t *failed;
  uint8_t *all_memo;
} Search;

static unsigned popcount64(uint64_t value) {
  return (unsigned)__builtin_popcountll(value);
}

static unsigned first_vertex(uint64_t vertices) {
  return (unsigned)__builtin_ctzll(vertices);
}

static void die(const char *message) {
  fprintf(stderr, "error: %s\n", message);
  exit(EXIT_FAILURE);
}

static bool is_independent(const Graph *graph, uint64_t vertices) {
  uint64_t scan = vertices;
  while (scan != 0) {
    unsigned vertex = first_vertex(scan);
    scan &= scan - 1;
    if ((graph->adj[vertex] & scan) != 0) {
      return false;
    }
  }
  return true;
}

static bool contains_packing(const Search *search, uint64_t available,
                             unsigned size) {
  if (size == search->target) {
    return true;
  }
  if (popcount64(available) < search->target - size) {
    return false;
  }

  unsigned vertex = first_vertex(available);
  uint64_t bit = UINT64_C(1) << vertex;
  if (contains_packing(search,
                       available & ~search->forbidden[vertex],
                       size + 1)) {
    return true;
  }
  return contains_packing(search, available & ~bit, size);
}

static bool maxine_search(Search *search, uint64_t remaining) {
  if (search->failed[remaining] != 0) {
    return false;
  }
  if (popcount64(remaining) == search->residue) {
    if (!is_independent(search->graph, remaining)) {
      die("MAX reached residue vertices before becoming independent");
    }
    return contains_packing(
        search, search->all_vertices & ~remaining, 0);
  }

  unsigned maximum = 0;
  uint64_t choices = 0;
  uint64_t scan = remaining;
  while (scan != 0) {
    unsigned vertex = first_vertex(scan);
    scan &= scan - 1;
    unsigned degree =
        popcount64(search->graph->adj[vertex] & remaining);
    if (degree > maximum) {
      maximum = degree;
      choices = UINT64_C(1) << vertex;
    } else if (degree == maximum) {
      choices |= UINT64_C(1) << vertex;
    }
  }

  while (choices != 0) {
    unsigned vertex = first_vertex(choices);
    choices &= choices - 1;
    if (maxine_search(search,
                      remaining & ~(UINT64_C(1) << vertex))) {
      return true;
    }
  }
  search->failed[remaining] = 1;
  return false;
}

static bool every_maxine_search(Search *search, uint64_t remaining) {
  if (search->all_memo[remaining] != 0) {
    return search->all_memo[remaining] == 1;
  }
  if (popcount64(remaining) == search->residue) {
    if (!is_independent(search->graph, remaining)) {
      die("MAX reached residue vertices before becoming independent");
    }
    bool result = contains_packing(
        search, search->all_vertices & ~remaining, 0);
    search->all_memo[remaining] = result ? 1 : 2;
    return result;
  }

  unsigned maximum = 0;
  uint64_t choices = 0;
  uint64_t scan = remaining;
  while (scan != 0) {
    unsigned vertex = first_vertex(scan);
    scan &= scan - 1;
    unsigned degree =
        popcount64(search->graph->adj[vertex] & remaining);
    if (degree > maximum) {
      maximum = degree;
      choices = UINT64_C(1) << vertex;
    } else if (degree == maximum) {
      choices |= UINT64_C(1) << vertex;
    }
  }

  while (choices != 0) {
    unsigned vertex = first_vertex(choices);
    choices &= choices - 1;
    if (!every_maxine_search(
            search, remaining & ~(UINT64_C(1) << vertex))) {
      search->all_memo[remaining] = 2;
      return false;
    }
  }
  search->all_memo[remaining] = 1;
  return true;
}

static bool maxine_packing_status(const Graph *graph,
                                  bool *every_sequence) {
  if (graph->n > EXACT_LIMIT) {
    die("exact MAX search exceeds order 20");
  }
  int diameter = graph_diameter(graph);
  Search search = {
      .graph = graph,
      .all_vertices = (UINT64_C(1) << graph->n) - 1,
      .residue = graph_residue(graph),
      .target = (unsigned)(diameter + 2) / 3,
  };
  for (unsigned vertex = 0; vertex < graph->n; vertex++) {
    uint64_t forbidden =
        graph->adj[vertex] | (UINT64_C(1) << vertex);
    uint64_t neighbors = graph->adj[vertex];
    while (neighbors != 0) {
      unsigned neighbor = first_vertex(neighbors);
      neighbors &= neighbors - 1;
      forbidden |= graph->adj[neighbor];
    }
    search.forbidden[vertex] = forbidden;
  }

  size_t states = (size_t)UINT64_C(1) << graph->n;
  search.failed = calloc(states, sizeof(*search.failed));
  search.all_memo = calloc(states, sizeof(*search.all_memo));
  if (search.failed == NULL || search.all_memo == NULL) {
    die("failed to allocate MAX search memo");
  }
  bool found = maxine_search(&search, search.all_vertices);
  *every_sequence =
      every_maxine_search(&search, search.all_vertices);
  free(search.failed);
  free(search.all_memo);
  return found;
}

int main(void) {
  char line[4096];
  uint64_t graphs = 0;
  uint64_t base = 0;
  uint64_t certified = 0;
  uint64_t universal = 0;
  uint64_t failures = 0;
  while (fgets(line, sizeof(line), stdin) != NULL) {
    if (strncmp(line, ">>graph6<<", 10) == 0 ||
        strspn(line, " \t\r\n") == strlen(line)) {
      continue;
    }
    Graph graph;
    char error[128];
    if (!graph6_parse(line, &graph, error, sizeof(error))) {
      fprintf(stderr, "error: graph6 record %" PRIu64 ": %s\n",
              graphs + 1, error);
      return EXIT_FAILURE;
    }
    if (graph.n < 2 || !graph_connected(&graph)) {
      die("input contains a trivial or disconnected graph");
    }

    graphs++;
    if (graph_diameter(&graph) <= 3) {
      base++;
      continue;
    }
    bool every = false;
    if (maxine_packing_status(&graph, &every)) {
      certified++;
      if (every) {
        universal++;
      }
    } else {
      char graph6[384];
      if (!graph6_encode(&graph, graph6, sizeof(graph6))) {
        die("graph6 encoding failed");
      }
      failures++;
      puts(graph6);
    }
  }
  if (ferror(stdin)) {
    die("failed while reading graph6 input");
  }
  if (graphs == 0) {
    die("no graph6 records were read");
  }

  fprintf(stderr,
          "SUMMARY graphs=%" PRIu64 " base=%" PRIu64
          " certified=%" PRIu64 " universal=%" PRIu64
          " failures=%" PRIu64 "\n",
          graphs, base, certified, universal, failures);
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
