#include "conjectures.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------- recording */

static void record(Wow2Evaluation *result, const char *key, Rational value) {
  if (result->field_count >= WOW2_MAX_FIELDS) {
    abort();
  }
  result->fields[result->field_count].key = key;
  result->fields[result->field_count].value = value;
  result->field_count++;
}

static void record_int(Wow2Evaluation *result, const char *key, int64_t value) {
  record(result, key, rational_from_int(value));
}

static void record_set(Wow2Evaluation *result, const char *key,
                       uint64_t vertices) {
  if (result->set_count >= 2) {
    abort();
  }
  result->sets[result->set_count].key = key;
  result->sets[result->set_count].vertices = vertices;
  result->set_count++;
}

static void set_inequality(Wow2Evaluation *result, Rational lhs, Rational rhs) {
  result->lhs = lhs;
  result->rhs = rhs;
  result->phi = rational_sub(lhs, rhs);
  result->pressure = result->phi;
  result->equality = rational_sign(result->phi) == 0;
  result->witness = rational_sign(result->phi) > 0;
}

/*
 * Same verdict as set_inequality, but with a search gradient taken from the
 * statement BEFORE its rounding step.
 *
 * Almost every inequality here ends in a floor, a ceiling, or an integer square
 * root, so phi is integer valued and the equality surface is a plateau: a hill
 * climber reaches phi = 0 in a few hundred toggles and then sees no difference
 * between neighbours for millions more. The calibration run measured exactly
 * that -- 16 million evaluations of conjecture 109 at order 13 produced two
 * improvements and never left zero, while a counterexample of that order was
 * already known. Passing the unrounded difference restores the slope.
 *
 * The smooth value only orders candidates. Whether a graph is a counterexample
 * is always decided by the rounded statement.
 */
static void set_inequality_smooth(Wow2Evaluation *result, Rational lhs,
                                  Rational rhs, Rational smooth) {
  set_inequality(result, lhs, rhs);
  result->pressure = smooth;
}

static void set_implication(Wow2Evaluation *result, bool hypothesis,
                            bool conclusion, Rational slack) {
  result->hypothesis = hypothesis;
  result->conclusion = conclusion;
  result->witness = hypothesis && !conclusion;
  /*
   * Reward approaching the hypothesis from outside and punish satisfying the
   * conclusion, so a hill climber walks toward the filtered region and away
   * from graphs that already carry the required structure.
   */
  result->pressure =
      conclusion ? rational_sub(slack, rational_from_int(1000)) : slack;
}

static void skip(Wow2Evaluation *result, const char *reason) {
  result->evaluated = false;
  result->skip_reason = reason;
}

/* --------------------------------------------------------------- helpers */

static unsigned ceil_sqrt(unsigned value) {
  unsigned root = 0;
  while ((uint64_t)root * root < value) {
    root++;
  }
  return root;
}

/*
 * The least integer k with e^k >= x, for rational x >= 1. Conjecture 103 is the
 * one statement in the pool with a transcendental term, so its bound is decided
 * by comparing against certified rational enclosures of e^k rather than by
 * evaluating a logarithm in floating point. The enclosures are exact to twelve
 * decimal places while x is a rational whose denominator is at most the order,
 * so the ambiguous case cannot arise; it aborts rather than guessing if it
 * somehow does.
 */
static int64_t ceil_natural_log(Rational value) {
  static const int64_t kScale = 1000000000000; /* 10^12 */
  static const int64_t kLower[] = {
      1000000000000,   /* e^0 = 1 exactly */
      2718281828459,   /* e^1 = 2.718281828459045... */
      7389056098930,   /* e^2 = 7.389056098930650... */
      20085536923187,  /* e^3 = 20.085536923187667... */
      54598150033144,  /* e^4 = 54.598150033144239... */
      148413159102576, /* e^5 = 148.413159102576603... */
      403428793492735, /* e^6 = 403.428793492735122... */
  };
  enum { kMaxPower = 6 };

  if (rational_compare(value, rational_from_int(1)) < 0) {
    abort(); /* a connected graph on two or more vertices has ecc_avg >= 1 */
  }
  for (int64_t power = 0; power <= kMaxPower; power++) {
    Rational lower = rational_make(kLower[power], kScale);
    if (power == 0) {
      /* e^0 is exactly one, so it needs no enclosure. */
      if (rational_compare(value, lower) <= 0) {
        return 0;
      }
      continue;
    }
    if (rational_compare(value, lower) <= 0) {
      return power;
    }
    Rational upper = rational_make(kLower[power] + 1, kScale);
    if (rational_compare(value, upper) < 0) {
      abort(); /* x sits inside the enclosure; the bounds need more digits */
    }
  }
  abort(); /* ecc_avg is below the order, far under e^6 */
}

/* ------------------------------------------------------- open conjectures */

/* f(G) >= ceil(sqrt(residue(G) * b(G))) */
static void evaluate_59(Invariants *cache, Wow2Evaluation *result) {
  unsigned residue = graph_residue(cache->graph);
  InducedResult forest = invariants_induced_forest(cache);
  InducedResult bipartite = invariants_induced_bipartite(cache);
  record_int(result, "residue", residue);
  record_int(result, "induced_bipartite", bipartite.size);
  record_int(result, "induced_forest", forest.size);
  record_set(result, "forest_vertices", forest.vertices);
  record_set(result, "bipartite_vertices", bipartite.vertices);
  set_inequality_smooth(
      result, rational_from_int(ceil_sqrt(residue * bipartite.size)),
      rational_from_int(forest.size),
      /* sqrt(r b) > f exactly when r b > f^2, so the squared form has the same
       * sign and needs no irrational term. */
      rational_from_int((int64_t)residue * (int64_t)bipartite.size -
                        (int64_t)forest.size * (int64_t)forest.size));
}

/* f(G) >= residue(G) + ceil(diameter(G) / 3) */
static void evaluate_61(Invariants *cache, Wow2Evaluation *result) {
  unsigned residue = graph_residue(cache->graph);
  unsigned diameter = invariants_diameter(cache);
  unsigned term = (diameter + 2) / 3;
  InducedResult forest = invariants_induced_forest(cache);
  record_int(result, "residue", residue);
  record_int(result, "diameter", diameter);
  record_int(result, "ceil_diameter_over_3", term);
  record_int(result, "induced_forest", forest.size);
  record_set(result, "forest_vertices", forest.vertices);
  set_inequality(result, rational_from_int(residue + term),
                 rational_from_int(forest.size));
  /*
   * The documented smoother score. Phi only moves when the diameter crosses a
   * multiple of three, so the search would otherwise face a flat surface across
   * most single-edge changes.
   */
  result->pressure = rational_from_int(
      3 * ((int64_t)residue - (int64_t)forest.size) + (int64_t)diameter);
}

/* Ls(G) >= 2 (l_avg(G) - 1) */
static void evaluate_2(Invariants *cache, Wow2Evaluation *result) {
  Rational average = invariants_local_independence_average(cache);
  unsigned leaves = invariants_max_leaf_spanning_tree(cache);
  record(result, "local_independence_average", average);
  record_int(result, "max_leaf_spanning_tree", leaves);
  set_inequality(result,
                 rational_mul(rational_from_int(2),
                              rational_sub(average, rational_from_int(1))),
                 rational_from_int(leaves));
}

/* f(G) >= ceil((p(G) + b(G) + 1) / 2) */
static void evaluate_40(Invariants *cache, Wow2Evaluation *result) {
  unsigned cover = invariants_path_cover_number(cache);
  InducedResult forest = invariants_induced_forest(cache);
  InducedResult bipartite = invariants_induced_bipartite(cache);
  Rational inner =
      rational_make((int64_t)cover + (int64_t)bipartite.size + 1, 2);
  record_int(result, "path_cover_number", cover);
  record_int(result, "induced_bipartite", bipartite.size);
  record_int(result, "induced_forest", forest.size);
  record_set(result, "forest_vertices", forest.vertices);
  set_inequality_smooth(result, rational_from_int(rational_ceil(inner)),
                        rational_from_int(forest.size),
                        rational_sub(inner, rational_from_int(forest.size)));
}

/* f(G) >= distMin(minimum-degree set) + ceil(distMin(maximum-degree set) / 3) */
static void evaluate_65(Invariants *cache, Wow2Evaluation *result) {
  const Graph *graph = cache->graph;
  unsigned minimum = UINT_MAX;
  unsigned maximum = 0;
  for (unsigned v = 0; v < graph->n; v++) {
    unsigned degree = (unsigned)__builtin_popcountll(graph->adj[v]);
    if (degree < minimum) {
      minimum = degree;
    }
    if (degree > maximum) {
      maximum = degree;
    }
  }
  uint64_t low = 0;
  uint64_t high = 0;
  for (unsigned v = 0; v < graph->n; v++) {
    unsigned degree = (unsigned)__builtin_popcountll(graph->adj[v]);
    if (degree == minimum) {
      low |= UINT64_C(1) << v;
    }
    if (degree == maximum) {
      high |= UINT64_C(1) << v;
    }
  }

  unsigned near_low = invariants_set_dist_min(cache, low);
  unsigned near_high = invariants_set_dist_min(cache, high);
  InducedResult forest = invariants_induced_forest(cache);
  record_int(result, "dist_min_min_degree", near_low);
  record_int(result, "dist_min_max_degree", near_high);
  record_int(result, "induced_forest", forest.size);
  record_set(result, "forest_vertices", forest.vertices);
  set_inequality_smooth(
      result,
      rational_from_int((int64_t)near_low + (int64_t)((near_high + 2) / 3)),
      rational_from_int(forest.size),
      rational_sub(rational_add(rational_from_int(near_low),
                                rational_make(near_high, 3)),
                   rational_from_int(forest.size)));
}

/*
 * alpha(G) <= ceil((max_v l(v) + (1/2) * degreeL2Norm(complement)) / 2)
 *
 * DeLaVina's length(H) is the degree L2 norm, the square root of the sum of the
 * squared degrees -- not the diameter. The module docstring upstream still
 * describes an older revision that used the complement's diameter, and reading
 * it instead of the theorem produced witnesses at order nine that are artefacts
 * of the misreading, not counterexamples.
 *
 * The bound rearranges to ceil((2m + sqrt(s)) / 4), and the least integer k
 * with 4k >= 2m + sqrt(s) is the least k with 4k - 2m >= 0 and s <= (4k - 2m)^2.
 * That is one integer comparison, so the square root never has to be taken.
 */
static void evaluate_100(Invariants *cache, Wow2Evaluation *result) {
  const Graph *complement = invariants_complement(cache);
  if (!graph_connected(complement)) {
    /*
     * The Lean statement assumes a connected complement. A graph failing that
     * lies outside the conjecture; it is not a survivor of it.
     */
    skip(result, "complement_disconnected");
    return;
  }

  int64_t squares = 0;
  for (unsigned v = 0; v < complement->n; v++) {
    int64_t degree = (int64_t)__builtin_popcountll(complement->adj[v]);
    squares += degree * degree;
  }
  int64_t local_max = (int64_t)invariants_local_independence_max(cache);
  int64_t bound = 0;
  while (true) {
    int64_t slack = 4 * bound - 2 * local_max;
    if (slack >= 0 && squares <= slack * slack) {
      break;
    }
    bound++;
  }

  unsigned independence = invariants_independence_number(cache);
  record_int(result, "independence_number", independence);
  record_int(result, "local_independence_max", local_max);
  record_int(result, "complement_degree_square_sum", squares);
  int64_t root = 0;
  while ((root + 1) * (root + 1) <= squares) {
    root++;
  }
  set_inequality_smooth(
      result, rational_from_int(independence), rational_from_int(bound),
      rational_sub(rational_from_int(independence),
                   rational_make(2 * local_max + root, 4)));
}

/* path(G) >= radius(G) + floor(l_avg(G)) ^ [G is C(4)-free] */
static void evaluate_133(Invariants *cache, Wow2Evaluation *result) {
  unsigned radius = invariants_radius(cache);
  Rational average = invariants_local_independence_average(cache);
  bool four_cycle_free = !invariants_has_four_cycle(cache);
  unsigned path = invariants_largest_induced_path(cache);
  /* The exponent is one when C(4)-free and zero otherwise, and x^0 is one. */
  int64_t term = four_cycle_free ? rational_floor(average) : 1;
  record_int(result, "radius", radius);
  record(result, "local_independence_average", average);
  record_int(result, "four_cycle_free", four_cycle_free ? 1 : 0);
  record_int(result, "largest_induced_path", path);
  set_inequality_smooth(
      result, rational_from_int((int64_t)radius + term),
      rational_from_int(path),
      rational_sub(rational_add(rational_from_int(radius),
                                four_cycle_free ? average
                                                : rational_from_int(1)),
                   rational_from_int(path)));
}

/* tree(G) >= floor(girth(G) / 2) - 1 + max_v l(v) */
static void evaluate_141(Invariants *cache, Wow2Evaluation *result) {
  unsigned girth = invariants_girth(cache);
  unsigned local_max = invariants_local_independence_max(cache);
  unsigned tree = invariants_largest_induced_tree(cache);
  record_int(result, "girth", girth);
  record_int(result, "local_independence_max", local_max);
  record_int(result, "largest_induced_tree", tree);
  set_inequality_smooth(
      result,
      rational_from_int((int64_t)(girth / 2) - 1 + (int64_t)local_max),
      rational_from_int(tree),
      rational_sub(rational_add(rational_make(girth, 2),
                                rational_from_int((int64_t)local_max - 1)),
                   rational_from_int(tree)));
}

/* tree(G) >= (2/3) girth(G) + eccSet(boundary) */
static void evaluate_142(Invariants *cache, Wow2Evaluation *result) {
  unsigned girth = invariants_girth(cache);
  unsigned boundary = invariants_set_ecc_all(cache, invariants_boundary(cache));
  unsigned tree = invariants_largest_induced_tree(cache);
  record_int(result, "girth", girth);
  record_int(result, "boundary_eccentricity", boundary);
  record_int(result, "largest_induced_tree", tree);
  set_inequality(
      result,
      rational_add(rational_mul(rational_make(2, 3), rational_from_int(girth)),
                   rational_from_int(boundary)),
      rational_from_int(tree));
}

/* tree(G) >= girth(G) - 1 + ecc(center); ecc here excludes the centre itself */
static void evaluate_144(Invariants *cache, Wow2Evaluation *result) {
  unsigned girth = invariants_girth(cache);
  unsigned centre = invariants_set_ecc(cache, invariants_center(cache));
  unsigned tree = invariants_largest_induced_tree(cache);
  record_int(result, "girth", girth);
  record_int(result, "center_eccentricity", centre);
  record_int(result, "largest_induced_tree", tree);
  set_inequality(result,
                 rational_from_int((int64_t)girth - 1 + (int64_t)centre),
                 rational_from_int(tree));
}

/* 2 eccSet(boundary) <= tree(G) * lMin(complement) */
static void evaluate_145(Invariants *cache, Wow2Evaluation *result) {
  unsigned local_min = invariants_complement_local_independence_min(cache);
  if (local_min == 0) {
    skip(result, "complement_local_independence_min_zero");
    return;
  }
  unsigned boundary = invariants_set_ecc_all(cache, invariants_boundary(cache));
  unsigned tree = invariants_largest_induced_tree(cache);
  record_int(result, "boundary_eccentricity", boundary);
  record_int(result, "complement_local_independence_min", local_min);
  record_int(result, "largest_induced_tree", tree);
  set_inequality(result, rational_from_int(2 * (int64_t)boundary),
                 rational_from_int((int64_t)tree * (int64_t)local_min));
}

/* 2 eccSet(boundary) <= tree(G) * radius(G squared) */
static void evaluate_146(Invariants *cache, Wow2Evaluation *result) {
  unsigned square_radius = invariants_graph_square_radius(cache);
  if (square_radius == 0) {
    skip(result, "graph_square_radius_zero");
    return;
  }
  unsigned boundary = invariants_set_ecc_all(cache, invariants_boundary(cache));
  unsigned tree = invariants_largest_induced_tree(cache);
  record_int(result, "boundary_eccentricity", boundary);
  record_int(result, "graph_square_radius", square_radius);
  record_int(result, "largest_induced_tree", tree);
  set_inequality(result, rational_from_int(2 * (int64_t)boundary),
                 rational_from_int((int64_t)tree * (int64_t)square_radius));
}

/* Ls(G) >= max_v l(v) + max_v T(v) [G is C(4)-free] */
static void evaluate_160(Invariants *cache, Wow2Evaluation *result) {
  unsigned local_max = invariants_local_independence_max(cache);
  unsigned triangles = invariants_triangles_max(cache);
  bool four_cycle_free = !invariants_has_four_cycle(cache);
  unsigned leaves = invariants_max_leaf_spanning_tree(cache);
  record_int(result, "local_independence_max", local_max);
  record_int(result, "max_triangles_at_vertex", triangles);
  record_int(result, "four_cycle_free", four_cycle_free ? 1 : 0);
  record_int(result, "max_leaf_spanning_tree", leaves);
  set_inequality(result,
                 rational_from_int((int64_t)local_max +
                                   (four_cycle_free ? (int64_t)triangles : 0)),
                 rational_from_int(leaves));
}

/* alpha(G) <= 1 + l_avg(G) implies a Hamiltonian path */
static void evaluate_194(Invariants *cache, Wow2Evaluation *result) {
  unsigned independence = invariants_independence_number(cache);
  Rational average = invariants_local_independence_average(cache);
  Rational slack = rational_sub(rational_add(rational_from_int(1), average),
                                rational_from_int(independence));
  bool conclusion = invariants_has_hamiltonian_path(cache);
  record_int(result, "independence_number", independence);
  record(result, "local_independence_average", average);
  record_int(result, "hamiltonian_path", conclusion ? 1 : 0);
  set_implication(result, rational_sign(slack) >= 0, conclusion, slack);
}

/* b(G) <= 2 + ecc_avg(G) implies a Hamiltonian path */
static void evaluate_198a(Invariants *cache, Wow2Evaluation *result) {
  InducedResult bipartite = invariants_induced_bipartite(cache);
  Rational average = invariants_average_eccentricity(cache);
  Rational slack = rational_sub(rational_add(rational_from_int(2), average),
                                rational_from_int(bipartite.size));
  bool conclusion = invariants_has_hamiltonian_path(cache);
  record_int(result, "induced_bipartite", bipartite.size);
  record(result, "average_eccentricity", average);
  record_int(result, "hamiltonian_path", conclusion ? 1 : 0);
  set_implication(result, rational_sign(slack) >= 0, conclusion, slack);
}

/* tree(G) = ceil(1 + l_avg(G)) implies a Hamiltonian path */
static void evaluate_200(Invariants *cache, Wow2Evaluation *result) {
  unsigned tree = invariants_largest_induced_tree(cache);
  Rational average = invariants_local_independence_average(cache);
  int64_t target = rational_ceil(rational_add(rational_from_int(1), average));
  bool conclusion = invariants_has_hamiltonian_path(cache);
  int64_t gap = (int64_t)tree - target;
  record_int(result, "largest_induced_tree", tree);
  record(result, "local_independence_average", average);
  record_int(result, "hypothesis_target", target);
  record_int(result, "hamiltonian_path", conclusion ? 1 : 0);
  /* An equality hypothesis has no slack, so distance to it is the signal. */
  set_implication(result, gap == 0, conclusion,
                  rational_from_int(gap < 0 ? gap : -gap));
}

/* Ls(G) <= 4 [residue(G) = 2] + 2 implies a Hamiltonian path */
static void evaluate_217(Invariants *cache, Wow2Evaluation *result) {
  unsigned leaves = invariants_max_leaf_spanning_tree(cache);
  unsigned residue = graph_residue(cache->graph);
  int64_t bound = 4 * (residue == 2 ? 1 : 0) + 2;
  Rational slack = rational_from_int(bound - (int64_t)leaves);
  bool conclusion = invariants_has_hamiltonian_path(cache);
  record_int(result, "max_leaf_spanning_tree", leaves);
  record_int(result, "residue", residue);
  record_int(result, "hamiltonian_path", conclusion ? 1 : 0);
  set_implication(result, rational_sign(slack) >= 0, conclusion, slack);
}

/* total domination number <= Havel-Hakimi zero step + frequency of min T(v) */
static void evaluate_291(Invariants *cache, Wow2Evaluation *result) {
  unsigned domination = invariants_total_domination_number(cache);
  unsigned step = invariants_havel_hakimi_zero_step(cache);
  unsigned frequency = invariants_triangles_min_frequency(cache);
  record_int(result, "total_domination_number", domination);
  record_int(result, "havel_hakimi_zero_step", step);
  record_int(result, "min_triangle_frequency", frequency);
  set_inequality(result, rational_from_int(domination),
                 rational_from_int((int64_t)step + (int64_t)frequency));
}

/* triangle-free and path(G) <= 4 implies well totally dominated */
static void evaluate_314(Invariants *cache, Wow2Evaluation *result) {
  bool triangle_free = invariants_triangle_free(cache);
  unsigned path = invariants_largest_induced_path(cache);
  bool conclusion = invariants_well_totally_dominated(cache);
  record_int(result, "triangle_free", triangle_free ? 1 : 0);
  record_int(result, "largest_induced_path", path);
  record_int(result, "well_totally_dominated", conclusion ? 1 : 0);
  set_implication(result, triangle_free && path <= 4, conclusion,
                  rational_from_int(triangle_free ? 4 - (int64_t)path : -100));
}

/* average degree of the complement <= pendant count implies well dominated */
static void evaluate_316(Invariants *cache, Wow2Evaluation *result) {
  Rational average = invariants_complement_average_degree(cache);
  unsigned pendants = invariants_pendant_count(cache);
  Rational slack = rational_sub(rational_from_int(pendants), average);
  bool conclusion = invariants_well_totally_dominated(cache);
  record(result, "complement_average_degree", average);
  record_int(result, "pendant_count", pendants);
  record_int(result, "well_totally_dominated", conclusion ? 1 : 0);
  set_implication(result, rational_sign(slack) >= 0, conclusion, slack);
}

/* max_v l(v) <= 1 implies well totally dominated */
static void evaluate_322(Invariants *cache, Wow2Evaluation *result) {
  unsigned local_max = invariants_local_independence_max(cache);
  bool conclusion = invariants_well_totally_dominated(cache);
  record_int(result, "local_independence_max", local_max);
  record_int(result, "well_totally_dominated", conclusion ? 1 : 0);
  set_implication(result, local_max <= 1, conclusion,
                  rational_from_int(1 - (int64_t)local_max));
}

/* ------------------------------------------------ calibration conjectures */

/* f(G) >= ceil(b(G) / l_avg(G)); refuted at order 79 */
static void evaluate_58(Invariants *cache, Wow2Evaluation *result) {
  Rational average = invariants_local_independence_average(cache);
  InducedResult forest = invariants_induced_forest(cache);
  InducedResult bipartite = invariants_induced_bipartite(cache);
  /* Lean division by zero yields zero; a connected graph never reaches it. */
  int64_t bound =
      rational_sign(average) == 0
          ? 0
          : rational_ceil(
                rational_div(rational_from_int(bipartite.size), average));
  record(result, "local_independence_average", average);
  record_int(result, "induced_bipartite", bipartite.size);
  record_int(result, "induced_forest", forest.size);
  record_set(result, "forest_vertices", forest.vertices);
  set_inequality_smooth(
      result, rational_from_int(bound), rational_from_int(forest.size),
      /*
       * The unrounded ratio is what actually climbs in the refuting family:
       * K(3,3) with one vertex coned over K(c) gives b / l_avg = 7(6+c)/(19+c),
       * which passes 6 only at c = 73. The ceiling hides every step of that
       * approach, so the search needs the ratio itself.
       */
      rational_sign(average) == 0
          ? rational_from_int(-1000)
          : rational_sub(rational_div(rational_from_int(bipartite.size),
                                      average),
                         rational_from_int(forest.size)));
}

/* alpha(G) <= floor(b(G) - ln(ecc_avg(G))); refuted at order 11 */
static void evaluate_103(Invariants *cache, Wow2Evaluation *result) {
  unsigned independence = invariants_independence_number(cache);
  InducedResult bipartite = invariants_induced_bipartite(cache);
  Rational average = invariants_average_eccentricity(cache);
  /*
   * floor(b - ln x) = b - ceil(ln x) because b is an integer, which turns the
   * only transcendental term in the pool into one exact integer comparison.
   */
  int64_t logarithm = ceil_natural_log(average);
  record_int(result, "independence_number", independence);
  record_int(result, "induced_bipartite", bipartite.size);
  record(result, "average_eccentricity", average);
  record_int(result, "ceil_log_average_eccentricity", logarithm);
  set_inequality_smooth(
      result, rational_from_int(independence),
      rational_from_int((int64_t)bipartite.size - logarithm),
      /* ceil(ln x) rises with x, so average eccentricity itself orders the
       * candidates inside one integer level. A heuristic, not the statement. */
      rational_add(rational_from_int((int64_t)independence -
                                     (int64_t)bipartite.size),
                   average));
}

/* alpha(G) <= floor((residue(G) + 2 b(G)) / 3); refuted at order 13 */
static void evaluate_109(Invariants *cache, Wow2Evaluation *result) {
  unsigned independence = invariants_independence_number(cache);
  unsigned residue = graph_residue(cache->graph);
  InducedResult bipartite = invariants_induced_bipartite(cache);
  Rational inner =
      rational_make((int64_t)residue + 2 * (int64_t)bipartite.size, 3);
  record_int(result, "independence_number", independence);
  record_int(result, "residue", residue);
  record_int(result, "induced_bipartite", bipartite.size);
  set_inequality_smooth(result, rational_from_int(independence),
                        rational_from_int(rational_floor(inner)),
                        rational_sub(rational_from_int(independence), inner));
}

/* ------------------------------------------------------------- the table */

/*
 * One reviewable screen. `min_order` is the statement's own hypothesis on the
 * order, not a search convenience: a graph below it lies outside the conjecture
 * and is reported as skipped rather than counted as a survivor.
 */
static const Wow2Conjecture kConjectures[] = {
    {"2", WOW2_SHAPE_INEQUALITY, WOW2_STATUS_OPEN, 2, true, false, evaluate_2,
     "Ls >= 2 (l_avg - 1)"},
    {"40", WOW2_SHAPE_INEQUALITY, WOW2_STATUS_OPEN, 2, false, true, evaluate_40,
     "f >= ceil((path cover + b + 1) / 2)"},
    {"59", WOW2_SHAPE_INEQUALITY, WOW2_STATUS_OPEN, 2, false, false,
     evaluate_59, "f >= ceil(sqrt(residue * b))"},
    {"61", WOW2_SHAPE_INEQUALITY, WOW2_STATUS_OPEN, 2, false, false,
     evaluate_61, "f >= residue + ceil(diameter / 3)"},
    {"65", WOW2_SHAPE_INEQUALITY, WOW2_STATUS_OPEN, 2, false, false,
     evaluate_65, "f >= distMin(min degree) + ceil(distMin(max degree) / 3)"},
    {"100", WOW2_SHAPE_INEQUALITY, WOW2_STATUS_OPEN, 2, false, false,
     evaluate_100, "alpha <= ceil((max l(v) + diam(complement) / 2) / 2)"},
    {"133", WOW2_SHAPE_INEQUALITY, WOW2_STATUS_OPEN, 2, false, false,
     evaluate_133, "path >= radius + floor(l_avg) ^ [C4-free]"},
    {"141", WOW2_SHAPE_INEQUALITY, WOW2_STATUS_OPEN, 2, false, false,
     evaluate_141, "tree >= floor(girth / 2) - 1 + max l(v)"},
    {"142", WOW2_SHAPE_INEQUALITY, WOW2_STATUS_OPEN, 2, false, false,
     evaluate_142, "tree >= (2/3) girth + eccSet(boundary)"},
    {"144", WOW2_SHAPE_INEQUALITY, WOW2_STATUS_OPEN, 2, false, false,
     evaluate_144, "tree >= girth - 1 + ecc(center)"},
    {"145", WOW2_SHAPE_INEQUALITY, WOW2_STATUS_OPEN, 2, false, false,
     evaluate_145, "2 eccSet(boundary) <= tree * lMin(complement)"},
    {"146", WOW2_SHAPE_INEQUALITY, WOW2_STATUS_OPEN, 2, false, false,
     evaluate_146, "2 eccSet(boundary) <= tree * radius(G^2)"},
    {"160", WOW2_SHAPE_INEQUALITY, WOW2_STATUS_OPEN, 2, true, false,
     evaluate_160, "Ls >= max l(v) + max T(v) [C4-free]"},
    {"194", WOW2_SHAPE_IMPLICATION, WOW2_STATUS_OPEN, 2, true, false,
     evaluate_194, "alpha <= 1 + l_avg implies a Hamiltonian path"},
    {"198a", WOW2_SHAPE_IMPLICATION, WOW2_STATUS_OPEN, 2, true, false,
     evaluate_198a, "b <= 2 + ecc_avg implies a Hamiltonian path"},
    {"200", WOW2_SHAPE_IMPLICATION, WOW2_STATUS_OPEN, 2, true, false,
     evaluate_200, "tree = ceil(1 + l_avg) implies a Hamiltonian path"},
    {"217", WOW2_SHAPE_IMPLICATION, WOW2_STATUS_OPEN, 2, true, false,
     evaluate_217, "Ls <= 4 [residue = 2] + 2 implies a Hamiltonian path"},
    {"291", WOW2_SHAPE_INEQUALITY, WOW2_STATUS_OPEN, 3, true, false,
     evaluate_291, "total domination <= Havel-Hakimi zero step + freq min T"},
    {"314", WOW2_SHAPE_IMPLICATION, WOW2_STATUS_OPEN, 2, true, false,
     evaluate_314,
     "triangle-free and path <= 4 implies well totally dominated"},
    {"316", WOW2_SHAPE_IMPLICATION, WOW2_STATUS_OPEN, 2, true, false,
     evaluate_316, "avg degree(complement) <= pendants implies WTD"},
    {"322", WOW2_SHAPE_IMPLICATION, WOW2_STATUS_OPEN, 5, true, false,
     evaluate_322, "max l(v) <= 1 implies well totally dominated"},

    {"58", WOW2_SHAPE_INEQUALITY, WOW2_STATUS_CALIBRATION, 2, false, false,
     evaluate_58, "f >= ceil(b / l_avg); refuted at order 79"},
    {"103", WOW2_SHAPE_INEQUALITY, WOW2_STATUS_CALIBRATION, 2, false, false,
     evaluate_103, "alpha <= floor(b - ln ecc_avg); refuted at order 11"},
    {"109", WOW2_SHAPE_INEQUALITY, WOW2_STATUS_CALIBRATION, 2, false, false,
     evaluate_109, "alpha <= floor((residue + 2b) / 3); refuted at order 13"},
};

unsigned wow2_conjecture_count(void) {
  return (unsigned)(sizeof(kConjectures) / sizeof(*kConjectures));
}

const Wow2Conjecture *wow2_conjecture_at(unsigned index) {
  if (index >= wow2_conjecture_count()) {
    abort();
  }
  return &kConjectures[index];
}

const Wow2Conjecture *wow2_lookup(const char *name) {
  for (unsigned i = 0; i < wow2_conjecture_count(); i++) {
    if (strcmp(kConjectures[i].name, name) == 0) {
      return &kConjectures[i];
    }
  }
  return NULL;
}

Wow2Evaluation wow2_evaluate(const Graph *graph,
                             const Wow2Conjecture *conjecture) {
  Wow2Evaluation result;
  memset(&result, 0, sizeof(result));
  result.conjecture = conjecture;
  result.evaluated = true;
  result.lhs = rational_from_int(0);
  result.rhs = rational_from_int(0);
  result.phi = rational_from_int(0);
  result.pressure = rational_from_int(0);

  if (graph->n < conjecture->min_order) {
    skip(&result, "below_statement_min_order");
    return result;
  }
  if (conjecture->needs_subsets && !invariants_supports_subsets(graph)) {
    skip(&result, "order_above_subset_limit");
    return result;
  }
  if (conjecture->needs_path_cover && graph->n > WOW2_PATH_COVER_ORDER_LIMIT) {
    skip(&result, "order_above_path_cover_limit");
    return result;
  }

  Invariants cache;
  invariants_init(&cache, graph);
  conjecture->evaluate(&cache, &result);
  return result;
}
