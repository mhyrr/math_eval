# Experiment ledger

Date: 2026-07-29

Every artifact uses sorted points and lexicographically sorted blocks. Search
scores are diagnostics; only `verifier.py` and `independent_verify.py` decide
validity. `UNSAT` below applies only to the named finite neighborhood and
constraints. `unknown` means the configured timeout elapsed.

## Clean-room phase

| Lane | Target | Seed / budget | Outcome | Artifact |
|---|---:|---|---|---|
| Cyclic construction | 48 | seed 480802; 961 evaluations; 0.004 s | valid, 0 uncovered | `structured_artifacts/structured_cyclic_48_seed480802_valid_5886a11e9550990b.txt` |
| Fixed-replication swaps | 46 | seeds 1001–1002 | best 50 uncovered | `best/local-46-seed1002.txt` |
| Free block replacement | 46 | seed 2001; iteration 150,924 | 46 uncovered | `best/block-46-seed2001.txt` |
| Free repair from cyclic seed | 46 | seeds 3001–3005 | best 17 uncovered at seed 3005, iteration 81,416 | `best/cyclic-repair-46-seed3005.txt` |
| Free block replacement | 45 | seed 4101; iteration 204,268 | 30 uncovered | `best/block-repair-45-seed4101.txt` |
| Free block replacement | 44 | seed 4201; iteration 60,390 | 43 uncovered | `best/block-repair-44-seed4201.txt` |
| Cyclic pruning | 46 / 45 / 44 | seed 480802; 961 evaluations each | 38 / 57 / 76 uncovered | `structured_artifacts/structured_cyclic_*_seed480802_best.txt` and `best/structured_cyclic_44_seed480802_best.txt` |
| Six-group GF(8) construction | 46 / 45 | seed 480802; 4 restarts × 20,000 steps | 95 / 109 uncovered | `structured_artifacts/structured_gdd_*_seed480802_best.txt` |
| Z8 orbit construction | 46 | seed 8101; 2 × 2,000,000 steps; 6.55 s | 64 uncovered | `best/orbit8-nearmiss-seed8101.txt` |
| Finite-pool Z3 repair | 46 | seeds 3101–3202; replace 3, 4, or 10; 2,500–8,000 candidates | replace-3 UNSAT; larger cases UNSAT or unknown | `best/exact-repair-46-seed*.json` |
| Unrestricted fixed-core Z3 repair | 46 | seeds 3401–3701; replace 3–5 | all named cores UNSAT; 0.26–68.49 solver s | `best/direct-repair-46-seed*.json` |
| Joint retention + unrestricted Z3 | 46 | valid cyclic 48 seed; retain 40 + add 6; 300 s | unknown | `best/joint-cyclic-reduce-pb-46.json` |
| Truncated PG(2,7) | 46 | conic + external point deleted; forced 18 full traces; replication 7–8; 180 s | unknown | reproducible command in `pg27_search.py` |

The 46-, 45-, and 44-block near misses were checked by both verification
paths; their uncovered counts agree exactly.

## Post-comparison phase

The published witness was first inspected after the clean-room lanes above had
closed.

| Experiment | Budget | Outcome | Artifact |
|---|---|---|---|
| Verify live published 46-block witness | exact | valid by both paths; 0 uncovered | `published-reference-46.txt` |
| Greedy one-block deletion | exhaustive over 46 single deletions | 45 blocks, 6 uncovered | `best/published-pruned-45.txt` |
| Fixed-core repair of the 45 near-cover | replace 1–5; up to 120 s | replace 1–4 UNSAT; replace 5 unknown | `best/postpublished-direct-45-seed510*.json` |
| Joint repair of the 45 near-cover | retain 43 + 2, 42 + 3, 41 + 4, or 40 + 5 | 2-block neighborhood UNSAT; larger neighborhoods unknown at 120–300 s | `best/postpublished-joint-*.json` |
| Joint reduction of published 46 to 45 | retain 44 + 1, 43 + 2, or 42 + 3 | 1- and 2-block neighborhoods UNSAT; 3-block neighborhood unknown at 180 s | `best/published46-to45-r*.json` |

The six uncovered pairs in the strongest 45-block near-cover are
`(16,18)`, `(16,44)`, `(16,46)`, `(33,46)`, `(45,46)`, and `(46,47)`.

## Incidence-SAT campaign

The second campaign added `incidence_sat.py`, a direct exact incidence-CNF
model, and `lns_sweep.py`, a deterministic fixed-core sweep driver. Frozen
rows and the pairs they already cover are constant-folded. A representative
five-free-block model fell from 85,334 variables and 234,614 clauses to 5,295
variables and 13,713 clauses without changing its solution set.

The unrestricted model uses only global symmetry and necessary conditions:
one block is `{1,...,8}`, rows are strictly ordered, residual point columns
are ordered, every point occurs at least seven times, and a block-intersection
lemma normalizes a pair of guaranteed multiplicity at least four for 44 blocks
and at least three for 45 or 46. `SAT-CAMPAIGN.md` gives the proof.

| Experiment | Exact scope | Budget | Outcome |
|---|---|---:|---|
| Pinned 46 round trip | all published rows fixed | 30 s | SAT; valid by both verifiers |
| Folded 46 calibration | 40 rows fixed, 6 arbitrary | 60 s | SAT in 33.68 solver s; valid by both verifiers |
| Free 46, strengthened unrestricted model | no candidate rows fixed; seeds 6601–6602 | 2 × 600 s | both unknown |
| Grouped-anchor 46 calibration | anchor counts `7,7,6,6,6,6,6` | 300 s | unknown |
| Published-derived 45, 3 free rows | 83 distinct 42-row frozen cores across all 46 net deletions | 10.74 total solver s | all UNSAT |
| Published-derived 45, 4 free rows | 59 distinct 41-row frozen cores; 22 weak + 38 deficit reports with one overlap | 3,274.42 total solver s | all UNSAT |
| Best published-derived 45, 5 free rows | two original and one constant-folded core | 3 × 600 s | all unknown |
| Whole-block stochastic repair at 45 | seed 6701 from six-pair near-cover | 600 s | no improvement; best remains 6 uncovered |
| Published-compatible anchor family at 45 | counts `7,6,6,6,6,6,6` | 600 s | unknown |
| Published-derived 44, 3 free rows | 78 distinct 41-row frozen cores, block 7 plus every second deletion | 12.40 total solver s | all UNSAT |
| Published-derived 44, 4 free rows | 20 distinct 40-row frozen cores from the ten best double deletions | 959.92 total solver s | all UNSAT |

The valid folded 46 calibration artifact is
`best/sat-lns-46-r6-folded-seed8001.txt`. It shares 42 blocks with the
normalized published witness and is therefore a derived calibration result,
not an independent construction. Its two verifier reports agree on zero
uncovered pairs and multiplicities
`1:1008, 2:104, 3:10, 6:2, 7:3, 9:1`.

The r=3 sweep summaries are:

- `best/overnight/summary-45-r3-weak.json`
- `best/overnight/summary-45-r3-deficit.json`
- `best/overnight/summary-44-r3-weak.json`
- `best/overnight/summary-44-r3-deficit.json`

The completed selected 44-block r=4 summaries are
`best/overnight/summary-44-r4-weak.json` and
`best/overnight/summary-44-r4-deficit.json`. The 45-block r=4 weak queue was
deliberately stopped after 22 reports; its individual JSON records and the
completed 38-report deficit summary remain under `best/overnight/`.

Representative reproduction commands:

```sh
# All net single deletions, two deterministic three-block neighborhoods.
python3 lns_sweep.py published-reference-46.txt \
  --target-blocks 45 --free-count 3 --modes weak,deficit \
  --timeout 120 --seed-base 7100

# All double deletions containing published block 7, three free blocks.
python3 lns_sweep.py published-reference-46.txt \
  --target-blocks 44 --must-delete 7 --free-count 3 \
  --modes weak,deficit --timeout 300 --seed-base 7600

# Reproduce the folded six-block positive calibration.
python3 incidence_sat.py --blocks 46 \
  --candidate published-reference-46.txt --free-count 6 --free-mode weak \
  --timeout 60 --seed 8001 --threads 1 \
  --cnf-output /tmp/c48-r6.cnf --metadata-output /tmp/c48-r6.meta.json \
  --report-output /tmp/c48-r6.json --output /tmp/c48-r6.txt
```

Every local `UNSAT` means only that the named frozen core cannot be completed
with the named number of arbitrary blocks. Z3 produced no independently
checkable proof certificate, so these records are engineering-grade bounded
negatives, not publishable lower bounds.
