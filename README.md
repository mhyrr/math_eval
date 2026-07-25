# llm-math-tractability

Building an empirically grounded rubric that predicts which open math problems are
tractable for LLM-assisted resolution, then applying it to infinite combinatorics
and set theory — the neighborhood of the continuum hypothesis — to identify
candidates, or to rigorously explain why none exist.

This is problem **evaluation**, not problem solving. Nothing here attempts to prove
or disprove anything. The deliverables are a classification dataset, a scoring
rubric, and a scored survey.

## Status

| phase | deliverable | state |
|---|---|---|
| 1 — Ground truth corpus | `corpus/` (20 cases) | **complete** |
| 2 — Taxonomy + rubric | `rubric/TAXONOMY.md`, `rubric/RUBRIC.md` | **complete (v1.2)** |
| 3 — Survey of target territory | `survey/` (24 problems), `survey/RESULTS.md` | **complete** |
| — final memo | `memo/FINDINGS.md` | **complete** |

Phase 2 produced a two-dimensional taxonomy (certificate type × production
process, plus flags for frontier, Peirce inference mode, and Gowers culture) and
a gated four-route scoring instrument. Retrodiction against the corpus is in
`rubric/RETRODICTION.md`: 13/14 resolved cases score `CANDIDATE`, 4/4 negatives
correctly declined. That is a sanity check, **not** validation — see RUBRIC.md
Part 4.

## Phase 3 result

**24 open problems from partition calculus, big Ramsey degrees, cardinal
characteristics and CH-adjacent combinatorics. 0 score `CANDIDATE`. Granting
every gate the survey could not discharge, 2 — and both flip to `RULED_OUT` if a
single axis call moves by one point.**

| population | CANDIDATE rate |
|---|---|
| Phase 1 corpus, resolved cases | 13/14 = 93% |
| Phase 3 survey, strict | 0/24 = 0% |
| Phase 3 survey, optimistic | 2/24 = 8% |

Three routes close for the whole territory on independent grounds: no evaluator
exists for infinite candidate objects (Route W), the reservoir is drained where
it is reachable and needs new theory where it is rich (Route R), and prior
attention is HIGH by construction (Route N). Route F closes because **a
consistency statement is not expressible in a proof assistant's fixed model** —
which is DeepMind's own note in the Lean repo that would have to support it, and
is the resolution of the forcing trap flagged in Phase 2.

The argument is in `memo/FINDINGS.md`; the ranked table in `survey/RESULTS.md`.

Phase 3 also **corrected `corpus/NEGATIVE-SPACE.md`**, which claimed no AI result
in set theory existed. Eleven wiki entries do exist; every green one is retrieval
or formalization, and zero primary contributions reached verified status.
And it **rebuilt Axis E** (v1.2) after finding the reverse-mathematics anchors
were both a category error and out of range.

## Layout

```
corpus/     one YAML per resolved case, plus SCHEMA.md, README.md,
            DISTRIBUTION.md (generated), NEGATIVE-SPACE.md
rubric/     axes, anchors, worked examples
survey/     scored open problems
memo/       the final prose memo
scripts/    scraping and tabulation
sources/    pinned source material
```

## Regenerating the distribution table

```sh
python3 -m venv .venv && ./.venv/bin/pip install pyyaml
./.venv/bin/python scripts/tabulate.py > corpus/DISTRIBUTION.md
./.venv/bin/python scripts/tabulate.py --exclude-thin   # drop weakly-sourced rows
```

## Evidentiary standard

Every factual claim about a resolved problem carries a source link with a tier
(`PRIMARY` / `EXPERT_COMMENTARY` / `JOURNALISM` / `SOCIAL_MEDIA`). Claims that
could not be verified are listed per-case under `unverifiable_claims:` rather
than dropped or quietly asserted — 37 such claims are recorded as of Phase 1.

Preprint and social-media claims are marked as such. One case in the corpus is
peer-reviewed; the rest are not. Where the public record is ambiguous about how
much a human contributed versus a model, the case file says so instead of
guessing.

Read `corpus/NEGATIVE-SPACE.md` before drawing conclusions from `corpus/` — a
corpus of successes is a biased instrument, and the absences carry more
information than the hits for Phase 3.
