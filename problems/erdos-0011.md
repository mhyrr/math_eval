# Erdős #11 — odd numbers as squarefree + power of 2

status: DEFINED
class: W — rubric call only (no community label; see below)

| | |
|---|---|
| problem page | https://www.erdosproblems.com/11 (human-readable; 403s automation) |
| statement from | `FormalConjectures/ErdosProblems/11.lean` @ `393aa9a` — PRIMARY |
| database status | `open`, last_update 2026-03-14 (`sources/erdos-problems.yaml`, pinned 2026-07-25) |
| prize | — |
| tags | additive basis |

## Statement

> Is every odd n > 1 the sum of a squarefree number and a power of 2?

Erdős often asked it under the weaker assumption that n is not divisible
by 4. A companion question in the same file: every odd n > 1 as a squarefree
number plus **two** powers of 2.

## The certificate

A **no** is a single odd integer n: check n − 2^k for squarefreeness for
every 2^k < n. Finite, exact, fast. A **yes** is a theorem. Only the
refutation is finitely checkable.

## Known, from the statement source

- Verified for all odd 1 < n < 2^50 (Lean file, `variants`). Any
  counterexample exceeds 2^50 — the cheap region is gone.
- Granville–Soundararajan [GrSo98], Theorem 1: if the conjecture holds, the
  set of primes p with 2^p ≡ 2 (mod p²) is infinite. The conjecture has
  consequences experts cared about, which cuts against neglect.

## Prior scoring

One of Route W's four blind nominations in Phase 4 — the only one of the
four that does **not** carry a community finite-certificate label. The other
three (#7, #617, #835) all do; on the construct-validity result
(`memo/VALIDATION.md` §5) that makes this the rubric's one out-on-a-limb
pick, which is exactly why it is seeded: if the community's labeling is
merely incomplete, this is what an unlabeled member of the class looks like.
The database bumped `last_update` 2026-03-14 without a status change —
maintainer attention, nothing resolved.

## Before an attempt

- **G0 not discharged.** [GrSo98] proves the literature is real; the search
  must establish the current verification frontier (2^50 is the file's
  number, not necessarily the record) and any structural results since.
- **Feasibility:** refutation search beyond 2^50 is a large but
  well-structured computation (sieve squarefree + subtract powers of 2). The
  ASSESSED stage should estimate cost per decade of range and whether
  heuristics (density of squarefree numbers) make a counterexample plausible
  at all — the naive heuristic says no.
