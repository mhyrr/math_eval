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
| 4 — Time-sliced hold-out | `survey4/` (56 problems), `memo/VALIDATION.md` | **complete** |

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

## Phase 4 result — the hold-out, and the half that failed

**56 open Erdős problems across 8 tags, scored blind against a 2025-08-31 cutoff,
scores committed before outcomes were read.** The first genuine hold-out in the
project: Phases 1 and 3 both pointed the instrument where the answer was known.

The result splits.

| question | answer |
|---|---|
| Does it discriminate between fields? | **Yes.** 0%–57% across eight tags, traceable to axis calls committed before the reveal. |
| Do its axes mean what they say? | **Yes.** The community's own `verifiable`/`falsifiable` labels land inside Route W's nominations 3 for 3, p = 0.00014. |
| Does `CANDIDATE` predict resolution? | **No.** 1/14 = 7% vs 6/42 = 14% for `RULED_OUT`; on AI-assisted resolutions only, 7.1% vs 7.1% — identical. |

7 of the 56 resolved in eleven months, **4 of them AI-assisted** (GPT-5.5 Pro, a
DeepMind prover agent, GPT-5.5 with Codex, Claude Opus 4.5). The filter captured
AI-resolved problems at exactly its own base rate.

The failing axis is **B2, formal library coverage**, and not at its threshold —
four problems that received machine-checked Lean proofs score `B2 = 2` under
*every* reading in the sensitivity band. B2 encodes the 2023–2025 constraint that
successes concentrate where mathlib is mature; the 2026 prover agents in this
sample did not obey it.

**Consequence: the ruling-in half of the scoring layer is retired.** `CANDIDATE`
should not be reported as a shortlist. `RULED_OUT` and the axis-by-axis argument
behind it — what Phase 3 actually delivered — survive, as does the vocabulary.
The full accounting, including every prediction that missed, is in
`memo/VALIDATION.md`; the pre-registration it is scored against is
`memo/PREREGISTRATION.md`.

One number worth carrying: **~7% AI-assisted resolution over eleven months on a
stratified sample of the open Erdős database.** That is a third regime, distinct
from the 1–3% autonomous-screen rate and from any filter-pass rate, and it should
never be quoted as either (Rule 4).

## Layout

```
corpus/     one YAML per resolved case, plus SCHEMA.md, README.md,
            DISTRIBUTION.md (generated), NEGATIVE-SPACE.md
rubric/     axes, anchors, worked examples
survey/     scored open problems (Phase 3, CH neighbourhood)
survey4/    the Phase 4 hold-out: SAMPLE.md, 56 generated rows,
            RESULTS.md (stage 2), OUTCOMES.md (stage 3)
memo/       FINDINGS.md (Phase 3), PREREGISTRATION.md and VALIDATION.md (Phase 4)
scripts/    scraping, tabulation, blinding, scoring, reveal
sources/    pinned source material — erdos-cutoff.yaml is the scoring-time
            view, erdos-problems.yaml is the reveal snapshot
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
