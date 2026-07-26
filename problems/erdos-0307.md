# Erdős #307 — two prime sets whose reciprocal sums multiply to 1

status: ASSESSED (a machine-checked barrier already exists; see below)
class: W — community label `verifiable`

| | |
|---|---|
| problem page | https://www.erdosproblems.com/307 (human-readable; 403s automation) |
| statement from | `FormalConjectures/ErdosProblems/307.lean` @ `393aa9a` — PRIMARY |
| database status | `verifiable`, last_update 2025-09-09 (`sources/erdos-problems.yaml`, pinned 2026-07-25) |
| prize | — |
| tags | number theory, unit fractions |

## Statement

> Are there two finite sets of primes P and Q such that
> 1 = (Σ_{p∈P} 1/p)(Σ_{q∈Q} 1/q)?

Asked by Barbeau [Ba76]. A weakened version (elements only required pairwise
coprime) has known solutions, e.g. 1 = (1 + 1/5)(1/2 + 1/3) — Cambie — but
none if 1 is excluded from the sets.

## The certificate

A **yes** is two finite sets of primes; the check is one exact rational
multiplication. Perfect Route W shape — which is exactly why it is the
folder's cautionary example.

## Why it is already ASSESSED

The Lean file records a **machine-checked barrier** (Bonfioli, 2026,
sorry-free, `native_decide` over the first 59 primes): any solution with Q
nonempty uses at least 59 primes in total, and ∏_{p∈P} p ≥ 2·10⁵⁶ (same for
Q by symmetry). Brute-force search below that product bound is dead — the
certificate is finite but the reachable search space provably contains no
witness. An attempt on this problem is a constructive-number-theory project,
not a computation.

Kept in the folder rather than parked outright for two reasons: it is the
concrete demonstration that **a finite certificate does not mean a feasible
search** (stage 3 of the process exists because of cases like this), and the
barrier itself is 2026 AI-adjacent activity on the problem — someone is
already working the negative side with proof assistants.

## Before an attempt

- **G0 partially discharged by the file itself** (Barbeau origin, Cambie
  examples, Bonfioli barrier — all recorded with references). A real search
  should still check for work outside the file's reference block.
- **Verdict lean:** PARK for search-based attempts; revisit only with an
  idea that constructs rather than enumerates.
