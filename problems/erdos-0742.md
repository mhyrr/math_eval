# Erdős #742 — Murty–Simon, open only on a finite range

status: DEFINED
class: W — community label `decidable` (the strongest label: a finite
computation settles it either way)

| | |
|---|---|
| problem page | https://www.erdosproblems.com/742 (human-readable; 403s automation) |
| statement from | `FormalConjectures/ErdosProblems/742.lean` @ `393aa9a` — PRIMARY |
| database status | `decidable`, last_update 2025-08-31 (`sources/erdos-problems.yaml`, pinned 2026-07-25) |
| prize | — |
| tags | graph theory |

## Statement

> **Murty–Simon conjecture.** Let G be a graph on n vertices with diameter 2
> such that deleting any edge increases the diameter. Then G has at most
> ⌊n²/4⌋ edges, with equality conjectured for the balanced complete
> bipartite graph.

## Why this file exists: the unresolved part is a finite set of integers

From the statement source: Füredi [Fü92] proved the conjecture **for all
sufficiently large n**; Fan [Fa87] verified it for **n ≤ 24 and n = 26**.
What remains is every n between Fan's ceiling and Füredi's threshold — a
finite, in-principle-checkable range. That is what the community's
`decidable` label is recording, and it is the rarest and best shape in this
folder: not "find a needle," but "close a known finite gap."

Plesník [Pl75] gives the general bound |E| < 3n(n−1)/8 — the slack between
that and n²/4 is what any finite-range attack is closing.

## The certificate

For each fixed n, exhaustive verification over diameter-2-critical graphs
on n vertices — a finite computation, though "finite" and "feasible" part
company quickly as n grows. A refutation would be a single critical graph
beating ⌊n²/4⌋ edges: self-verifying in milliseconds.

## Before an attempt

- **G0 is the whole game here, and it is sharply posed:** (1) What is
  Füredi's threshold, explicitly? The Lean file says only "sufficiently
  large"; if the proof's constant is astronomical (or ineffective), the
  "finite gap" framing collapses in practice. (2) Has the verified range
  moved past Fan's 1987 computation in 38 years? Both answers exist in the
  literature and determine the verdict almost by themselves.
- **Feasibility** then reduces to: largest n reachable by modern
  enumeration of diameter-2-critical graphs vs. the effective threshold.
  If the two are within an order of magnitude, this is a serious ATTEMPT
  candidate for compute + SAT; if they are separated by hundreds of orders,
  it parks.
