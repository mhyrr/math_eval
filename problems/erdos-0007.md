# Erdős #7 — a covering system with all moduli odd

status: DEFINED
class: W — community label `verifiable`

| | |
|---|---|
| problem page | https://www.erdosproblems.com/7 (human-readable; 403s automation) |
| statement from | `FormalConjectures/ErdosProblems/7.lean` @ `393aa9a` — PRIMARY |
| database status | `verifiable`, last_update 2025-08-31 (`sources/erdos-problems.yaml`, pinned 2026-07-25) |
| prize | — |
| tags | number theory, covering systems |

## Statement

> Is there a covering system all of whose moduli are odd (and greater than 1)?

A covering system is a finite set of congruences a_i (mod m_i) such that every
integer satisfies at least one of them.

## The certificate

A **yes** is a finite list of congruences. Checking it is exact CRT
arithmetic: verify every residue class modulo lcm(m_i) is hit. Cheap,
mechanical, self-certifying — the cleanest Route W shape in the Phase 4
sample (`survey4/e7-number-theory.yaml`). A **no** is a theorem, not a
certificate; only the affirmative direction is finitely checkable, which is
what the community's `verifiable` label means.

## Known, from the statement source

The Lean file carries the bare question. Anything beyond it belongs to the
SEARCHED stage.

## Prior scoring

Phase 4 scored this blind: CANDIDATE via Routes W and F. That verdict carries
no predictive weight (`memo/VALIDATION.md`); the problem is here because of
the certificate shape and the community's independent label. After the
cutoff the database relabeled it `open` → `verifiable` with `last_update`
unchanged — vocabulary backfill, not an event.

## Before an attempt

- **G0 not discharged.** This is a well-known covering-systems question;
  expect substantial literature and possibly large computational sweeps
  already on record. The literature search must establish what moduli bounds
  and search regions are already excluded.
- **Feasibility unassessed.** The search space (choices of odd moduli and
  residues) is unbounded; known exclusions will determine whether any
  reachable region remains.
