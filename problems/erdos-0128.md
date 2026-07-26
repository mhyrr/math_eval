# Erdős #128 — dense induced subgraphs forcing a triangle

status: DEFINED
class: W — community label `falsifiable`

| | |
|---|---|
| problem page | https://www.erdosproblems.com/128 (human-readable; 403s automation) |
| statement from | `FormalConjectures/ErdosProblems/128.lean` @ `393aa9a` — PRIMARY |
| database status | `falsifiable`, last_update 2025-10-31 (`sources/erdos-problems.yaml`, pinned 2026-07-25) |
| prize | $250 (per database) |
| tags | graph theory |

## Statement

> Let G be a graph with n vertices such that every induced subgraph on ≥ n/2
> vertices has more than n²/50 edges. Must G contain a triangle?

## The certificate

A **no** is a single triangle-free graph on n vertices in which every
induced subgraph on ≥ n/2 vertices exceeds n²/50 edges. The catch, and the
reason this file is seeded with a caution: verifying the density condition
naively means checking **all** induced subgraphs of size ≥ n/2 —
exponential in n. Triangle-freeness is cheap; the hypothesis is not. The
check is exact but only feasible for small n or for candidates with enough
symmetry that the minimum-density subgraph can be located analytically.

## Known, from the statement source

The Lean file carries the bare question. The database's `last_update` moved
2025-10-31 while staying open — maintainer attention after the Phase 4
seed date, worth a look at what changed on the page during SEARCHED.

## Before an attempt

- **G0 not discharged.** The n²/50 constant smells like the surviving corner
  of a family of results (similar problems trade the constant against the
  subgraph fraction); the literature around it must be mapped first.
- **Feasibility:** the natural candidates are blow-ups of small triangle-free
  graphs, where the density condition reduces to a small optimization. That
  makes a structured search plausible despite the exponential naive check —
  but establishing this is ASSESSED-stage work, not something to assert.
