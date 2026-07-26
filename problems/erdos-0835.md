# Erdős #835 — colouring the k-subsets of a 2k-set

status: DEFINED
class: W — community label `verifiable`

| | |
|---|---|
| problem page | https://www.erdosproblems.com/835 (human-readable; 403s automation) |
| statement from | `FormalConjectures/ErdosProblems/835.lean` @ `393aa9a` — PRIMARY |
| database status | `verifiable`, last_update 2025-08-31 (`sources/erdos-problems.yaml`, pinned 2026-07-25) |
| prize | — |
| tags | graph theory, hypergraphs |

## Statement

> Does there exist a k > 2 such that the k-sized subsets of {1,…,2k} can be
> coloured with k+1 colours such that for every A ⊂ {1,…,2k} with |A| = k+1,
> all k+1 colours appear among the k-sized subsets of A?

Equivalently (also in the file): is there a k > 2 with chromatic number of
the Johnson graph J(2k, k) equal to k+1?

## The certificate

A **yes** is a colouring of C(2k, k) subsets for one concrete k — finite,
and checking every (k+1)-subset is exact enumeration. Each **negative**
k-case is also a finite computation (a lower-bound certificate for χ), but
"no for all k > 2" is a theorem and not finitely certifiable — the
`verifiable` label covers only the affirmative direction.

## Known, from the statement source

- χ(J(2k, k)) > k+1 for all 3 ≤ k ≤ 8 (via the Johnson-graph spectra page,
  aeb.win.tue.nl, cited in the file).
- The file then records the **k = 9 case as also closed**: χ(J(18, 9)) ≥ 11
  > 10. So the first open case is **k = 10**: does J(20, 10), on
  C(20,10) = 184,756 vertices, have chromatic number 11?
- Nine consecutive negative cases is a strong empirical trend toward "no
  such k" — in which case this question never produces a finite
  certificate, only an unbounded ladder of negative cases. That trend is
  the main argument for PARK at the verdict stage.

## Prior scoring

Phase 4 scored this blind: CANDIDATE via Route W. No predictive weight
(`memo/VALIDATION.md`). The `open` → `verifiable` relabel was backfill.

## Before an attempt

- **G0 not discharged.** The k = 9 result is recent enough to be
  documentation-thin in the file; find who did it and how (the method
  determines whether k = 10 is incremental or a wall).
- **Feasibility:** k = 10 means colouring 184,756 vertices with 11 colours
  under Johnson-graph adjacency — large but structured; fractional/
  independent-set bounds may close it negatively without search. An
  affirmative certificate here would contradict a nine-case trend; weigh
  the spend accordingly.
