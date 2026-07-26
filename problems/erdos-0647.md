# Erdős #647 — another n where m + τ(m) never gets ahead

status: DEFINED
class: W — community label `verifiable`

| | |
|---|---|
| problem page | https://www.erdosproblems.com/647 (human-readable; 403s automation) |
| statement from | `FormalConjectures/ErdosProblems/647.lean` @ `393aa9a` — PRIMARY |
| database status | `verifiable`, last_update 2025-08-31 (`sources/erdos-problems.yaml`, pinned 2026-07-25) |
| prize | £25 (per database) |
| tags | number theory |

## Statement

> Let τ(n) count the divisors of n. Is there some n > 24 such that
> max_{m<n}(m + τ(m)) ≤ n + 2?

True for n = 24 (verified in the Lean file by `decide`). Erdős thought it
"extremely doubtful" that infinitely many such n exist — but conjectured
("seems certain") that a relaxed version, max over n−k < m < n only, has
infinitely many n for every k. Both variants are in the file.

## The certificate

A **yes** is a single integer n; the check computes τ(m) for all m < n. Pure
computation, exact, embarrassingly parallel — the most attempt-ready shape
in the folder. A **no** (no such n exists) is a theorem about divisor-sum
peaks and is not finitely certifiable.

## Known, from the statement source

Only the n = 24 base case and Erdős's stated expectations. Note the tension:
the headline question is `verifiable`, but Erdős's own guess is that the
answer hides at no finite height ("extremely doubtful" infinitely many —
and possibly none beyond 24).

## Before an attempt

- **G0 not discharged, and here it is cheap and decisive:** this is exactly
  the kind of statement that gets swept computationally and posted (OEIS,
  the problem page's own commentary). Establish the current search frontier
  first — someone has likely pushed n into large territory already, and the
  file does not say how far.
- **Feasibility:** sieve τ over blocks; the constraint max(m + τ(m)) ≤ n + 2
  is a running-maximum condition that fails fast almost everywhere (any
  recent m with large τ kills a long stretch of n). Cost scales linearly
  with the frontier; the question is only whether the frontier is already
  far enough that marginal sweeping is noise. That, plus the heuristic
  density of qualifying n, is the ASSESSED content.
