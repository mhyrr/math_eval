# Result

Date closed: 2026-07-30

## Verdict

The expanded campaign did not improve the covering bound. It independently
constructed a valid 48-block covering, but did not independently reach 46
blocks and found no valid 45- or 44-block construction. After the clean-room
search closed, exact neighborhoods derived from the published witness produced
three valid 46-block completions. They retain 42 or 43 published blocks and do
not count as independent reproductions.

The [live Covering Repository entry](https://ljcr.dmgordon.org/cover/show_cover.php?k=8&t=2&v=48)
was checked again after the expanded attempt and still recorded

```text
44 <= C(48,8,2) <= 46.
```

The saved published 46-block reference and all three derived 46-block
calibration artifacts pass both local verifiers. None is a new record.

## What was established

### Valid construction

`structured_artifacts/structured_cyclic_48_seed480802_valid_5886a11e9550990b.txt`
is an independently generated 48-block covering. Its SHA-256 is
`5886a11e9550990b26d7c31d1547c2140918f1dc57455d8af77b614b30e56cd0`.

Both verification paths report:

- 48 blocks of eight distinct points and no duplicate blocks;
- all 48 points used;
- 0 of 1,128 pairs uncovered;
- multiplicity histogram `1:912, 2:216`;
- 216 repeated incidences and no redundant block.

The command in `README.md` regenerates the same canonical artifact from seed
480802.

### Independent verification

`verifier.py` counts all 28 pairs in every block. `independent_verify.py`
constructs a block-index incidence set for each point and tests pair coverage
by set intersection; it imports no primary-verifier code. Their tests cover
wrong block sizes, repeated points, out-of-range points, missing pairs,
duplicate blocks, complete-pair small covers, and an independently generated
affine plane of order three.

The valid 48-block artifact, the post-comparison published 46-block reference,
and the three solver-derived 46-block calibrations pass both paths. Both paths
also agree exactly on the uncovered pairs in the preserved 46-, 45-, and
44-block near misses.

### Bound improvement

None. No independently generated construction with at most 46 blocks passed
the exact predicate.

### Exact resolution

None. No 44-block construction was found.

### Bounded negative search

The clean-room phase exercised four materially different representations:

1. fixed-replication incidence swaps and free whole-block annealing;
2. finite-pool and unrestricted Z3 large-neighborhood repair;
3. cyclic, GF(8) group-divisible, and Z8-orbit constructions;
4. a truncated projective-plane `PG(2,7)` completion model.

The strongest clean-room near misses have 17 uncovered pairs at 46 blocks, 30
at 45, and 43 at 44. The exact-repair `UNSAT` records rule out only the frozen
cores, replacement counts, replication caps, or finite pools named in their
JSON files. Solver timeouts and unsuccessful stochastic searches say nothing
about nonexistence of a 45- or 44-block cover.

The expanded incidence-SAT campaign then calibrated exact fixed-core search at
46 blocks and swept:

- 83 distinct 45-block cores with three arbitrary replacement blocks;
- 59 distinct 45-block cores with four arbitrary replacement blocks;
- 78 distinct 44-block cores with three arbitrary replacement blocks;
- 20 selected 44-block cores with four arbitrary replacement blocks.

All 240 completed cores were `UNSAT`. These are exact local statements about
retaining the named 40–42 blocks from the published design. They do not cover
unrelated constructions. The continuation confirmed that Z3 can emit textual
DRAT, but no independent DRAT checker is installed. The local statements
therefore remain computational records rather than publishable certificates.

Two strengthened unrestricted 46-block formulas timed out after 600 seconds
each. Since the machinery did not solve the known-positive calibration, the
campaign did not spend or interpret unrestricted 45- or 44-block runs. A
published-compatible 45-block anchor-family model and a five-block
published-derived repair also timed out at 600 seconds.

## Post-clean-room comparison

The live entry attributes the 46-block witness to Franco Atzeni and records
the method as local search. Its point-replication histogram is
`7:24, 8:16, 9:8`. The clean main lane concentrated on replication seven and
eight, which excluded the architecture of the known result.

Deleting the best block from that witness produces
`best/published-pruned-45.txt`, a 45-block near-cover with six uncovered pairs.
Fixed-core repairs replacing one through four blocks are UNSAT within their
recorded neighborhoods; a five-block repair timed out. Jointly reducing the
full 46-block witness to 45 is UNSAT when at most two replacement blocks are
allowed and timed out with three. These are useful neighborhood facts, not
global lower bounds.

No new record construction exists to compare for novelty. The independent
48-block cover differs in block count and has no claim on the published
frontier. The strongest derived 46-block completion
`best/sat-lns-46-r6-folded-seed8001.txt` shares 42 blocks with the normalized
published witness, so it is useful as a solver calibration only.

## Six-hour continuation and next experiment

The unrestricted 46-block positive calibration using row-only symmetry and a
five-free-block 45 neighborhood both timed out after six hours. A
five-free-block 44 neighborhood finished UNSAT after 4h 11m. That result
excludes only its named 39-row frozen core; no proof was requested, and it is
not a global lower bound. The continuation found no construction and changed
no bound.

The next solver step remains a locally provisioned dedicated CDCL solver such
as CaDiCaL or Kissat, followed by an independent proof checker before treating
any UNSAT as mathematics. The direct unrestricted encoding is sound and now
has exact pair witnesses, selectable symmetry, and a complete two-branch
44-block reduction. If the unrestricted 46 calibration passes, run
unrestricted 45 first; only then spend on the two unrestricted 44 branches.

See `EXPERIMENTS.md` for parameters, runtimes, artifacts, and the boundary of
every negative claim.
