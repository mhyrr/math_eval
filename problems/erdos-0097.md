# Erdős #97 — a convex polygon vertex with few equidistant vertices

status: DEFINED
class: W — community label `falsifiable`

| | |
|---|---|
| problem page | https://www.erdosproblems.com/97 (human-readable; 403s automation) |
| statement from | `FormalConjectures/ErdosProblems/97.lean` @ `393aa9a` — PRIMARY |
| database status | `falsifiable`, last_update 2025-08-31 (`sources/erdos-problems.yaml`, pinned 2026-07-25) |
| prize | $100 (per database) |
| tags | geometry, distances, convex |

## Statement

> Does every convex polygon have a vertex with no other 4 vertices
> equidistant from it?

## The certificate

A **no** is a single convex polygon in which every vertex has 4 other
vertices equidistant from it. Finite point set; the check is arithmetic on
coordinates. One honesty note for the ASSESSED stage: a counterexample's
coordinates may be algebraic rather than rational, in which case "exact
check" means symbolic computation, not floating point. Still finite, still
mechanical.

## Known, from the statement source

- Erdős originally conjectured the same with **3** equidistant vertices
  [Er46b]. **Danzer refuted that version**: a convex polygon on 9 points in
  which every vertex has three vertices equidistant from it (the distance
  varying by vertex). Construction explained in [Er87b].
- So the pattern "this family of conjectures fails to a small explicit
  polygon" has a precedent one parameter down — the strongest argument that
  this file belongs in the pool. The 4-vertex version asks whether Danzer's
  phenomenon extends.

## Before an attempt

- **G0 not discharged.** Danzer's construction and its aftermath need
  reading; someone may have pushed to 4 already.
- **Feasibility:** search over convex configurations with prescribed
  equidistance patterns is a small-n geometric constraint problem — SMT/
  numerical-then-algebraic pipelines apply. Plausible counterexample sizes
  (Danzer's was 9 points) look reachable.
