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
| 1 — Ground truth corpus | `corpus/` | **complete, awaiting classification review** |
| 2 — Rubric extraction | `rubric/RUBRIC.md` | not started (gated on Phase 1 review) |
| 3 — Survey of target territory | `survey/` | not started |
| — final memo | `memo/FINDINGS.md` | not started |

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
