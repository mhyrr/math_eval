# Covering design C(48,8,2)

status: IMPLEMENTING
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

## Reproduction

Commands will be fixed here as implementations land. All generated executables
and scratch state go under `/tmp`; durable candidates and concise experiment
records stay in this directory.

```sh
cd problems/covering-48-8-2
python3 -m unittest -v test_verifiers.py
```

See [PLAN.md](PLAN.md) for the campaign design and `RESULT.md` for the final
run record.
