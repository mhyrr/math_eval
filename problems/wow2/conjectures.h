#ifndef WOW2_CONJECTURES_H
#define WOW2_CONJECTURES_H

#include "graph.h"
#include "invariants.h"
#include "rational.h"

/*
 * The conjecture registry.
 *
 * Graph generation, graph6 I/O, the exact invariants, and the search do not
 * know the shape of a statement. A new WOWII conjecture is one descriptor in
 * the table at the bottom of conjectures.c plus one evaluator function, and a
 * matching brute-force formula in verify.py.
 *
 * Two shapes cover every statement in the pool:
 *
 *   INEQUALITY   asserts lhs <= rhs. A counterexample has lhs > rhs, so the
 *                slack phi = lhs - rhs is positive exactly on a witness and
 *                zero on the equality surface.
 *   IMPLICATION  asserts hypothesis -> conclusion. A counterexample satisfies
 *                the hypothesis and fails the conclusion. phi is meaningless
 *                here; the hypothesis is a filter, which is what makes these
 *                cheap to sweep deeply.
 *
 * Both sides are exact rationals. Nothing here uses floating point.
 */

typedef enum {
  WOW2_SHAPE_INEQUALITY,
  WOW2_SHAPE_IMPLICATION,
} Wow2Shape;

typedef enum {
  /* Statements DeLaVina's list and the pinned warehouse both still call open. */
  WOW2_STATUS_OPEN,
  /*
   * Statements already refuted upstream. They are registered so the pipeline
   * can be pointed at problems whose answers are known: whether it rediscovers
   * their counterexamples measures the search, not the pool.
   */
  WOW2_STATUS_CALIBRATION,
} Wow2Status;

enum {
  WOW2_MAX_FIELDS = 12,
};

typedef struct {
  const char *key;
  Rational value;
} Wow2Field;

typedef struct Wow2Conjecture Wow2Conjecture;

typedef struct {
  const Wow2Conjecture *conjecture;

  /* False when an exact invariant was out of reach; skip_reason says which. */
  bool evaluated;
  const char *skip_reason;

  /* INEQUALITY */
  Rational lhs;
  Rational rhs;
  Rational phi;
  bool equality;

  /* IMPLICATION */
  bool hypothesis;
  bool conclusion;

  bool witness;
  /*
   * Search heuristic only: larger means closer to a counterexample, and it is
   * deliberately finer grained than phi so a hill climber is not stranded on a
   * plateau of equal integers. Nothing decides a verdict from it -- that is
   * what the witness flag is for.
   */
  Rational pressure;

  unsigned field_count;
  Wow2Field fields[WOW2_MAX_FIELDS];

  /* Optional certificates, e.g. the vertices of an extremal induced forest. */
  unsigned set_count;
  struct {
    const char *key;
    uint64_t vertices;
  } sets[2];
} Wow2Evaluation;

typedef void (*Wow2Evaluator)(Invariants *cache, Wow2Evaluation *result);

struct Wow2Conjecture {
  const char *name;
  Wow2Shape shape;
  Wow2Status status;
  /* Smallest order the statement's own hypotheses admit. */
  unsigned min_order;
  /* Needs an invariant that enumerates or memoizes over vertex subsets. */
  bool needs_subsets;
  /* Needs the path cover number, which stops at a lower order still. */
  bool needs_path_cover;
  Wow2Evaluator evaluate;
  const char *summary;
};

unsigned wow2_conjecture_count(void);
const Wow2Conjecture *wow2_conjecture_at(unsigned index);
/* Returns NULL when no conjecture carries that name. */
const Wow2Conjecture *wow2_lookup(const char *name);

Wow2Evaluation wow2_evaluate(const Graph *graph,
                             const Wow2Conjecture *conjecture);

#endif
