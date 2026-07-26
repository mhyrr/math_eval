# Erdős #647 — another n where m + τ(m) never gets ahead

status: VERDICT — PARK (the finite search frontier is already ~6.16·10^17;
one local-to-global attempt is closed below)
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

## Search and feasibility, checked 2026-07-26

G0 changed the verdict.

- [OEIS A087280](https://oeis.org/A087280) records no further solution through
  \(10^{12}\), citing Patrik Idén's segmented-sieve report —
  EXPERT_COMMENTARY backed by a linked computational preprint.
- Scott Hughes's
  [proof-chain package](https://github.com/scottdhughes/erdos647-proof-chain)
  reduces candidates \(n>84\) to \(n=2520N\), with \(N\) in 41 residue
  classes modulo 46189, and packages a finite-range certificate excluding
  \(24<n\le615736321200000000\) — PRIMARY artifact, author-supplied and not
  independently reproduced in this repository.
- Its
  [Stage-1 boundary](https://github.com/scottdhughes/erdos647-proof-chain/blob/main/docs/stage1_boundary.md)
  retires fixed-depth positive-footprint congruence trees: a CRT all-avoid
  branch survives every finite extra-prime pool of that form — PRIMARY
  artifact.
- The problem's
  [discussion thread](https://www.erdosproblems.com/forum/thread/647) lists
  several people currently working on it and records the reduction's
  development — SOCIAL_MEDIA; useful for provenance, not peer review.

A larger undirected sieve is noise. A negative proof needs a mechanism outside
the retired bounded congruence-tree family; a positive search should begin
beyond the reduced frontier.

## Attempt record

Greg explicitly opened one solving attempt inside this otherwise evaluative
repository. [`erdos647/`](erdos647/) tests whether Erdős's relaxed fixed-window
conjecture offers a soft bridge to the full problem. The exact run through
\(10^9\) found locally quiet windows, but every one remained covered by an
older divisor peak; the strongest local example passed nine shifts, failed the
tenth, and also had a separate historical blocker.

**Verdict: PARK.** Revisit only with an idea that couples the forced prime
chain to expiry of the global running-max envelope. More local-window search
and larger fixed congruence trees have already said what they know.
