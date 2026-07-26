# Erdős #848 — sets where ab + 1 is never squarefree, open only for small N

status: DEFINED
class: W — community label `decidable` (a finite computation settles it
either way)

| | |
|---|---|
| problem page | https://www.erdosproblems.com/848 (human-readable; 403s automation) |
| statement from | `FormalConjectures/ErdosProblems/848.lean` @ `393aa9a` — PRIMARY |
| database status | `decidable`, last_update 2025-10-19 (`sources/erdos-problems.yaml`, pinned 2026-07-25) |
| prize | — |
| tags | number theory |

## Statement

> Is the maximum size of a set A ⊆ {1,…,N} such that ab + 1 is never
> squarefree (for all a, b ∈ A, including a = b) achieved by taking those
> n ≡ 7 (mod 25)?

(For n ≡ 7 mod 25, ab + 1 ≡ 50 ≡ 0 mod 25, hence never squarefree.)

## Why this file exists: the unresolved part is a finite range

From the statement source: **Sawhney proved the conjecture for all
sufficiently large N**, with a stability refinement — any A of size at least
(1/25 − c)N with N large is contained in {n ≡ 7 mod 25} or {n ≡ 18 mod 25}.
Like #742, what remains is N below Sawhney's threshold: a finite gap, which
is what the `decidable` label records. Same caveat as #742: "sufficiently
large" must be made effective before the gap is real.

## The certificate

For fixed N, the maximum is an exact combinatorial computation: build the
graph on {1,…,N} joining a,b when ab + 1 is not squarefree... inverted —
qualifying sets are cliques in the compatibility relation "ab + 1 not
squarefree" (with loops: a²+1 must also be non-squarefree). Max-clique is
expensive in general but N here is concrete, the relation is highly
structured (congruence classes dominate), and any exceeding set found is a
self-verifying refutation.

## Before an attempt

- **G0 sharply posed, as with #742:** (1) Is Sawhney's threshold effective,
  and what is it? (2) What N-range has been verified exactly? The file's
  reference block ("this note") needs chasing to the actual document. The
  2025-10-19 database touch may already reflect movement.
- **Feasibility** follows directly from those two numbers: if exact
  computation can meet the effective threshold, the whole problem closes by
  machine — the cleanest possible win for this folder's thesis. If the
  threshold is ineffective or astronomical, PARK with that fact recorded.
