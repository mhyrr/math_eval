# Incidence SAT campaign

Date opened: 2026-07-29

## Decision

The next search uses a direct incidence encoding. The first campaign searched
local block neighborhoods and several structured subfamilies; it never
represented the complete fixed-block-count problem. This campaign does.

Calibration is a hard gate:

1. pin the published 46-block witness and recover it from the SAT model;
2. find a valid 46-block covering without pinning any candidate;
3. only then spend the main budget on 45 and 44.

Failure at step 2 diagnoses the search machinery. It says nothing about 45 or
44.

## Exact variables and constraints

For a requested block count `b`, let `x[i,p]` mean point `p` occurs in block
`i`. The core formula contains:

- exactly eight true `x[i,*]` variables for every block;
- a witness selector `w[i,p,q]` for every block and unordered pair, with
  `w[i,p,q] -> x[i,p]` and `w[i,p,q] -> x[i,q]`;
- `OR_i w[i,p,q]` for each of the 1,128 pairs;
- at least seven occurrences of every point, a redundant consequence of pair
  coverage that improves propagation;
- strict descending lexicographic order on all blocks, removing their
  permutation symmetry and forbidding duplicates.

Cardinalities use an internally generated unary prefix counter. The encoder
has no dependency beyond the Python standard library. Every decoded SAT model
is passed through both existing exact verifiers; the SAT formula never
replaces the certificate predicate.

## Safe symmetry breaking

Block 1 is fixed to `{1,...,8}`. This loses no solutions: choose any block,
rename its eight points to `1,...,8`, rename the other forty points
arbitrarily, and call that block first. Among distinct weight-eight rows, this
row is automatically lexicographically greatest.

All rows are therefore strictly ordered from greatest to least. This loses no
solutions because block labels have no mathematical meaning, and it also
forbids duplicate blocks. Within the fixed block and its forty-point
complement, incidence columns are ordered nonincreasingly. This is also safe:
take the lexicographically greatest incidence matrix in the residual row and
column permutation orbit.

Column ordering is not applied in pinned-candidate checks, fixed-core
neighborhoods, or anchor-family submodels, where frozen labels or structural
groups reduce the available point symmetry.

The unrestricted model also normalizes a guaranteed repeated pair. A
block-intersection count proves that every 44-block cover has a pair occurring
in at least four blocks, while every 45- or 46-block cover has one occurring in
at least three. Name that pair `{1,2}`, choose one of its blocks as block 1,
and the strict row order places all blocks containing it first. The model
therefore fixes `{1,2}` into rows 1–4 at target 44 and rows 1–3 at targets 45
and 46. Residual column ordering respects the distinguished pair and sorts
only `{1,2}`, `{3,...,8}`, and `{9,...,48}` internally.

For completeness, let `r_x` be point replications, `m_e` pair multiplicities,
and `s_ij = |B_i intersect B_j|`. Put

```text
E = sum_e (m_e - 1) = 28b - 1128
L = sum_x binom(r_x, 2) = sum_{i<j} s_ij
P = sum_e binom(m_e, 2) = sum_{i<j} binom(s_ij, 2).
```

Since `binom(s,2) >= s-1`, `P >= L-binom(b,2)`. Convexity and `r_x >= 7`
give `L >= 1120` at `b=44` and `L >= 1176` at `b=45`. Therefore

```text
H = P-E = sum_e binom(m_e-1, 2) >= 70  when b=44
H >= 54                                  when b=45.
```

If every 44-block pair had multiplicity at most three, then `H <= E/2 =
52`, a contradiction. At 45 blocks, `H > 0` already forces multiplicity at
least three. The same calculation gives multiplicity at least three for 46
blocks. This lemma is used only in the unrestricted model; it conflicts with
the separate seven-anchor structural family.

Replication upper bounds are optional experiment partitions, never part of
the unrestricted formula. The known 46-block construction already shows why:
eight points occur nine times. An `UNSAT` result under an upper bound applies
only to that partition.

## Solver paths

The installed solver is Z3 4.16.0. It accepts DIMACS CNF and emits a model, so
it is sufficient for construction search and calibration. Runs record the
CNF SHA-256, seed, timeout, solver command, status, statistics, decoded
artifact, and exact-verifier reports.

The generated DIMACS remains compatible with a proof-producing CDCL solver.
Z3 `UNSAT` is useful engineering evidence, but this campaign will not report a
new mathematical lower bound without preserving and independently checking a
DRAT/LRAT-style proof from a proof-producing solver.

## Large exact neighborhoods

For a 45-block seed, an exact neighborhood fixes selected rows and leaves the
remaining rows free. The free rows still use the same incidence variables and
pair constraints; they are not selected from a finite candidate pool.

Neighborhood sizes run through 3, 4, 5, 6, 8, 10, and 12 free blocks. Frozen
cores are chosen by several deterministic rules:

- blocks containing many uniquely covered pairs;
- blocks containing few uniquely covered pairs;
- blocks incident to currently uncovered pairs;
- seeded random ejection.

Each core is a bounded subproblem. A completed `UNSAT` result rules out only
that frozen core.

## Overnight schedule

1. Encoder tests and pinned-witness round trip.
2. Short 46-block free runs across solver seeds and phase settings.
3. If free 46 calibration succeeds, launch 45-block unrestricted and
   overlapping 3–12-block neighborhoods.
4. Launch 44 only after the same encoder has produced a free 46 result and
   the 45 runs are stable.
5. Checkpoint every SAT construction and update `EXPERIMENTS.md`.

The stop condition is the morning budget, a valid 44/45 construction, or a
repeatable solver defect. Timeouts are recorded as timeouts.
