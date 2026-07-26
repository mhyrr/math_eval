# Erdős #287 — gaps in Egyptian fraction representations of 1

status: DEFINED
class: W — community label `falsifiable`

| | |
|---|---|
| problem page | https://www.erdosproblems.com/287 (human-readable; 403s automation) |
| statement from | `FormalConjectures/ErdosProblems/287.lean` @ `393aa9a` — PRIMARY |
| database status | `falsifiable`, last_update 2025-12-05 (`sources/erdos-problems.yaml`, pinned 2026-07-25) |
| prize | — |
| tags | number theory, unit fractions |

## Statement

> Let k ≥ 2. Is it true that for any distinct integers 1 < n₁ < ⋯ < n_k with
> Σ 1/n_i = 1, we must have max(n_{i+1} − n_i) ≥ 3?

## The certificate

A **no** is a finite list of integers: distinct, reciprocals summing to 1,
all consecutive gaps ≤ 2. Checking is exact rational arithmetic in
microseconds. This is as clean as a refutation certificate gets.

## Known, from the statement source

- Gap ≥ 2 is equivalent to "1 is not a sum of reciprocals of consecutive
  integers," proved by Erdős [Er32].
- 1 = 1/2 + 1/3 + 1/6 has max gap 3, so 3 would be best possible.
- The file records a route to a **proof**: if for all large N there is a
  prime p ∈ [N, 2N] with (p+1)/2 also prime — itself open — then the
  conjecture holds with at most finitely many exceptions. A conditional
  bridge from an unrelated-looking prime conjecture; whoever works this
  problem should read that variant first.

## Before an attempt

- **G0 not discharged.** Unit fractions is an active area (this is the tag
  where Phase 4's rubric scored highest); the 2025-12-05 database touch
  needs explaining.
- **Feasibility:** refutation search = enumerate near-consecutive sequences
  with reciprocal sum 1 — a bounded search for each k, and the structure
  (gaps ≤ 2 means the sequence is pinned between consecutive-integer sums)
  cuts it down hard. Erdős's own [Er32] result closes gap-1 sequences, so
  any counterexample mixes gap-1 and gap-2 steps. A serious computational
  sweep may already exist; find it before running one.
