# Implementation and search plan

Date opened: 2026-07-29

## A. Verifier before search

Build a standard-library Python verifier that parses the certificate format and
checks the exact target predicate. Its report must include:

- block count;
- used and missing points;
- uncovered pairs;
- the full pair-multiplicity histogram;
- repeated pair incidences beyond the first;
- blocks whose removal preserves coverage.

Build a second verifier from point-to-block incidence intersections. It must
share neither the pair-counting routine nor validity logic with the first.
Cross-check them on malformed target inputs and independently generated small
coverings, including complete pair coverings and finite affine-plane lines.

## B. Search invariants

The validity predicate remains binary. Search may use the following exact
integer signals without changing it:

1. uncovered pair count;
2. weighted uncovered deficit, giving persistent pairs extra weight;
3. squared pair-multiplicity deviation;
4. repeated pair incidences;
5. removable blocks and single-block replacement potential.

Every run records its seed, block target, time/evaluation budget, lane, best
uncovered count, and saved artifact. Candidates are canonicalized before
hashing or saving. Every valid candidate at the smallest block count seen is
preserved.

## C. Three search lanes

### Lane 1 — fixed-degree incidence local search

Represent a construction as a 0/1 point-by-block incidence matrix with block
sum eight. Point replication is fixed near its necessary range (normally seven
or eight). A move swaps two point occurrences between two blocks, preserving
both row and column totals. Pair multiplicities and the uncovered-pair score
are updated incrementally. Use tabu tenure, simulated annealing, and periodic
perturbation to cross plateaus.

This lane searches the whole balanced incidence space and is the main route to
46 and 45.

### Lane 2 — exact large-neighborhood repair

Take a near-cover, remove a selected set of weak blocks, and generate a finite
pool of replacement blocks biased toward its uncovered pairs. Freeze the
remaining core. Encode exact pair coverage and a fixed replacement count as
Boolean constraints for the installed Z3 binary. A satisfying assignment is a
certificate; `unknown` or `unsat` applies only to that finite repair pool.

This lane makes coordinated multi-block changes that pair swaps cannot express
in one step.

### Lane 3 — structured generation and deterministic pruning

Generate cyclic and group-divisible seeds from arithmetic point labellings:
translation orbits in `Z_48`, short subgroup orbits, and six groups of eight
with controlled within- and cross-group coverage. Verify the generated
coverings exactly, then prune or replace blocks deterministically, using the
same exact-repair engine only after the structure has produced the candidate
pool.

This lane tests whether the target wants a global algebraic pattern rather than
a locally optimized incidence matrix.

## D. Order of attack and stop rule

1. Pass both verifier paths and tests.
2. Find a clean-room valid covering, initially allowing 48 blocks if needed,
   then establish the required 46-block baseline.
3. Run all three lanes at 45 blocks with recorded bounded budgets.
4. Run promising lanes at 44 blocks only after 45 has useful signal; a valid
   44-block candidate ends the search.
5. Preserve the strongest near-miss and write the bounded outcome plainly.

The main attempt closes after all three lanes have completed at least one
recorded run and further repetitions show no new best score within their fixed
budgets. Only then may the live Covering Repository status and published
constructions be inspected for comparison.

## E. Completion checks

A saved 46-, 45-, or 44-block candidate is reported as valid only after both
verifiers pass it in the same turn. Any 45- or 44-block result additionally
gets a fresh-context verification that knows only the definition and candidate
path. `RESULT.md` separates:

- construction validity;
- independent verification;
- bound improvement;
- exact resolution;
- bounded negative search.
