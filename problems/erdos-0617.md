# Erdős #617 — Erdős–Gyárfás: no balanced r-colouring of K_{r²+1}

status: DEFINED
class: W — community label `falsifiable`

| | |
|---|---|
| problem page | https://www.erdosproblems.com/617 (human-readable; 403s automation) |
| statement from | `FormalConjectures/ErdosProblems/617.lean` @ `393aa9a` — PRIMARY |
| database status | `falsifiable`, last_update 2025-08-31 (`sources/erdos-problems.yaml`, pinned 2026-07-25) |
| prize | — |
| tags | graph theory |

## Statement

> Let r ≥ 3. If the edges of K_{r²+1} are r-coloured then there exist r+1
> vertices with at least one colour missing on the edges of the induced
> K_{r+1}.

Equivalently: there is no "balanced" colouring. Erdős–Gyárfás [ErGy99].

## The certificate

A **no** is a single r-colouring of K_{r²+1} for one concrete r, in which
every (r+1)-subset sees all r colours. Finite object; the check enumerates
C(r²+1, r+1) subsets — exact and fast for the first open r.

## Known, from the statement source

- Proved for r = 3 and r = 4 by Erdős–Gyárfás themselves [ErGy99]. **The
  first open case is r = 5**: a 5-colouring of K₂₆.
- The same paper shows the property **fails** for infinitely many r if
  r²+1 is replaced by r² — the conjecture sits exactly on the boundary,
  which is evidence the statement is tuned right and a counterexample at
  r²+1 would be genuinely surprising.

## Prior scoring

Phase 4 scored this blind: CANDIDATE via Routes W and F. No predictive
weight (`memo/VALIDATION.md`); it is here for the certificate shape. The
post-cutoff `open` → `falsifiable` relabel was vocabulary backfill
(`last_update` unchanged).

## Before an attempt

- **G0 not discharged.** [ErGy99] aftermath needs mapping — 25+ years is
  room for partial results on r = 5.
- **Feasibility:** r = 5 means 5-colouring the 325 edges of K₂₆ so that all
  230,230 6-subsets see all 5 colours. Raw space 5³²⁵, but this is textbook
  SAT/symmetry-breaking territory, and *either* outcome of a complete SAT
  run is progress: UNSAT for K₂₆ under useful symmetry restrictions maps
  excluded structure; SAT is a counterexample that self-verifies. The
  ASSESSED stage should estimate whether unrestricted UNSAT is within
  current solver reach (it may not be).
