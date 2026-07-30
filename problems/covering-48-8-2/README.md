# Covering design C(48,8,2)

status: CLOSED — NO BOUND IMPROVEMENT
class: B — exact discrete construction

## Problem

Find as few 8-element subsets ("blocks") of

```text
{1, 2, ..., 48}
```

as possible so that every unordered pair of distinct points occurs in at
least one block. The recorded starting bounds are

```text
44 <= C(48,8,2) <= 46.
```

This campaign treats those bounds as given. Under its clean-room rule, the
published 46-block construction is not inspected until the independent main
search has ended.

The outcomes have different force:

- 46 blocks: independently reproduces the known upper bound.
- 45 blocks: improves the upper bound by one.
- 44 blocks: meets the recorded lower bound and proves C(48,8,2) = 44.
- no construction: a bounded negative search result, not a nonexistence proof.

## Certificate format

Candidate files are UTF-8 text. Blank lines and lines beginning with `#` are
ignored. Every other line is one block, written as eight whitespace-separated
decimal point labels. A canonical candidate has:

- points within each block in increasing order;
- blocks in lexicographic order;
- no duplicate blocks.

The verifier accepts noncanonical ordering, but never duplicate points or
duplicate blocks.

## Exact validity predicate

A target candidate is valid exactly when:

1. its used point set is `{1, ..., 48}`;
2. every block contains exactly eight distinct in-range points;
3. no two blocks are the same set;
4. all `binom(48,2) = 1,128` unordered pairs occur.

The primary verifier enumerates pairs inside blocks and counts their
multiplicities. `independent_verify.py` instead represents each point by the
set of block indices containing it; a pair is covered when those two incidence
sets intersect. The second path deliberately imports no verifier code.

## Outcome

The clean-room attempt found and independently verified a 48-block covering.
It did not independently reproduce the known 46-block upper bound and found
no valid 45- or 44-block covering. The best clean-room near misses leave:

- 17 pairs uncovered with 46 blocks;
- 30 pairs uncovered with 45 blocks;
- 43 pairs uncovered with 44 blocks.

After the clean-room phase closed, the live published 46-block construction
was fetched and passed both verifiers. Removing its best single block gives a
45-block near-cover with six uncovered pairs. A second incidence-SAT campaign
found several valid 46-block completions while retaining 42 or 43 published
blocks, but no valid 45- or 44-block repair.

The calibrated exact sweep ruled out 142 distinct published-derived frozen
cores at target 45 and 98 at target 44. Those are local computational
statements, not a lower bound: every model retains 40–42 blocks from the known
construction, and Z3 supplied no independently checked UNSAT proof. Two free
46-block calibration runs timed out, so unrestricted 45/44 timeouts were not
treated as meaningful.

The recorded bound therefore remains `44 <= C(48,8,2) <= 46`.

## Reproduction

All generated executables and solver instances go under `/tmp`; durable
candidates and experiment records stay in this directory.

```sh
cd problems/covering-48-8-2
make test

# Rebuild the independently generated valid 48-block covering.
python3 structured_search.py --mode cyclic --blocks 48 --seed 480802 \
  --cyclic-evaluations 20000 --output-dir /tmp/c48-reproduction --json
python3 verifier.py \
  /tmp/c48-reproduction/structured_cyclic_48_seed480802_valid_5886a11e9550990b.txt
python3 independent_verify.py \
  /tmp/c48-reproduction/structured_cyclic_48_seed480802_valid_5886a11e9550990b.txt

# Recheck the preserved artifact.
make verify-best

# Reproduce exact three-block neighborhoods around all single deletions.
python3 lns_sweep.py published-reference-46.txt \
  --target-blocks 45 --free-count 3 --modes weak,deficit \
  --timeout 120 --seed-base 7100

# Recheck the solver-derived positive 46-block calibration.
python3 verifier.py best/sat-lns-46-r6-folded-seed8001.txt
python3 independent_verify.py best/sat-lns-46-r6-folded-seed8001.txt
```

Search programs expose their recorded parameters through `--help`. See
[PLAN.md](PLAN.md), [SAT-CAMPAIGN.md](SAT-CAMPAIGN.md),
[EXPERIMENTS.md](EXPERIMENTS.md), and [RESULT.md](RESULT.md) for the design,
run ledger, and conclusion.
