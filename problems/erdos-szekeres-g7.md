# ES(7) = 33? — the first open Erdős–Szekeres case

status: DEFINED
class: W — finite certificate via UNSAT exhaustion; lower-bound witness already known

| | |
|---|---|
| precedent computation | Heule–Scheucher, *Happy Ending: An Empty Hexagon in Every Set of 30 Points*, TACAS 2024 — https://arxiv.org/abs/2403.00737 — PRIMARY |
| formal verification of precedent | Lean, ITP 2024 — https://drops.dagstuhl.de/entities/document/10.4230/LIPIcs.ITP.2024.35 — PRIMARY |
| current attempt on record | Dumitru, *Notes on the 33-point Erdős–Szekeres problem*, 2025-12-30 — https://arxiv.org/abs/2512.24061 — PRIMARY |
| tags | discrete geometry, ramsey theory |

## Statement

> Does every set of 33 points in general position in the plane contain 7
> points in convex position?

The Erdős–Szekeres conjecture says ES(n) = 2^(n−2) + 1. It is proved for
n ≤ 6 (the n = 6 case is Szekeres–Peters 2006, by computer). n = 7 is the
first open case: the classical construction gives 32 points with no convex
heptagon, so ES(7) ≥ 33, and the conjecture says exactly 33.

## The certificate

- **Yes (ES(7) = 33)** is an exhaustion over order types: no 33-point
  configuration avoids a convex 7-gon. The modern artifact is a SAT UNSAT
  certificate over triple-orientation variables — precisely the machinery
  that settled the empty-hexagon problem in 2024 (17,300 CPU hours,
  O(n⁴)-clause encoding, subsequently formally verified in Lean).
- **No** would be a set of 33 explicit points — checkable instantly — and
  would refute the Erdős–Szekeres conjecture itself. Nobody expects this.

## Known, from the record

- The empty-hexagon computation is the direct methodological ancestor and
  its authors' encoding is public (PRIMARY above).
- A December 2025 arXiv note (Dumitru, PRIMARY above) built a SAT encoding
  for the 33-point case with convex-layer anchoring, produced UNSAT
  certificates for anchored subfamilies, and reports that "heavy-tailed
  behavior currently dominates the computational effort" — i.e. someone
  competent is already on it and the naive encoding does not close it.

## Why it is in this folder

The strongest *shape* in the geometry pool: both directions finite, the
exact machinery exists, the precedent problem fell two years ago, and the
question is a named conjecture with real standing (the original 1935 "happy
ending" lineage). Attention is moderate — one active arXiv attempt on
record, versus the crowd on Ramsey numbers.

## Before an attempt

- **G0 partially discharged:** the Dumitru note is the current frontier as
  of 2025-12-30. A SEARCHED pass must check whether Scheucher, Heule, or
  the Dumitru line has posted anything since, and what the symmetry-breaking
  state of the art is for order-type exhaustion at n = 33.
- **Feasibility:** the empty hexagon needed 17,300 CPU hours at 30 points;
  33 points over full order-type space is substantially larger, and the one
  attempt on record stalled on heavy tails. The ASSESSED stage should
  extract the branching statistics from the Dumitru paper rather than
  guessing.
