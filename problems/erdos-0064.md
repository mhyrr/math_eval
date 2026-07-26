# Erdős #64 — cycles of length a power of 2

status: DEFINED
class: W — community label `falsifiable`

| | |
|---|---|
| problem page | https://www.erdosproblems.com/64 (human-readable; 403s automation) |
| statement from | `FormalConjectures/ErdosProblems/64.lean` @ `393aa9a` — PRIMARY |
| database status | `falsifiable`, last_update 2025-08-31 (`sources/erdos-problems.yaml`, pinned 2026-07-25) |
| prize | $1000 (per database) |
| tags | graph theory, cycles |

## Statement

> Does every finite graph with minimum degree at least 3 contain a cycle of
> length 2^k for some k ≥ 2?

## The certificate

A **no** is a single finite graph with minimum degree ≥ 3 and no cycle of
length 4, 8, 16, … Checking is exact: enumerate cycle lengths (girth-style
search per power of 2 up to the circumference). A **yes** is a theorem.

## Known, from the statement source

The Lean file carries the bare question. The $1000 tag in the database is the
largest prize in the labeled pool, which signals both that the community
takes it seriously and that it has been attacked — the two readings pull in
opposite directions and the SEARCHED stage has to separate them.

## Before an attempt

- **G0 not discharged.** A prize this size almost guarantees a literature of
  partial results (density versions, girth constraints, computer searches).
  The search must find the frontier: what graph families are already
  excluded as counterexamples, and how far exhaustive enumeration has gone.
- **Feasibility:** min-degree-3 graphs are enumerable by order (`geng`-class
  tools); absence of short power-of-2 cycles prunes hard (no 4-cycles is a
  strong local condition). A staged enumeration — order by order, 4- and
  8-cycle-free first — is a well-defined machine attack, and the certificate
  self-verifies. Whether the plausible counterexample size is reachable is
  the ASSESSED question.
