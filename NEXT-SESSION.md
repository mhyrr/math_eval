# Continuation prompt — Phase 4

Paste the block below into a clean session started in this repo.

---

## The plan, in four stages

Phase 4 is the **validation test**. The rubric has been pointed at known
successes (93% CANDIDATE) and at a territory everyone expected to be hopeless
(0–8%). Both directions had known answers. This tests the direction where it can
fail: a stratified sample of ordinary open problems, scored blind against a past
cutoff, then checked against what actually happened.

| stage | output | gate before moving on |
|---|---|---|
| 1 — Draw | `survey4/SAMPLE.md` | **commit** before any scoring |
| 2 — Score blind | `survey4/*.yaml` | **commit** before any reveal |
| 3 — Split labels, reveal | `survey4/OUTCOMES.md` | — |
| 4 — Report | `memo/VALIDATION.md` | — |

**The commits are the experiment.** If scores are not committed before the reveal,
the test did not happen and the write-up is a story about numbers.

### Why the sequencing is strict

The session that wrote the pre-registration is contaminated — it read the
outcomes of two dozen post-cutoff results while building Phase 3. A fresh session
is not. So everything requiring blindness happens first, and everything that
reveals outcomes happens after the scores are frozen.

That includes the label-splitting method: reading the resolving citation tells
you the outcome, so it cannot happen before stage 2 is committed.

---

```
We're continuing the llm-math-tractability project. Phases 1-3 are complete.
You're executing Phase 4, the validation test. Do not redo earlier phases.

ORIENT FIRST — read in this order:
  1. CLAUDE.md                 — operating rules, traps, mechanics, the
                                 erdosproblems.com workaround table
  2. memo/PREREGISTRATION.md   — THE CONTROLLING DOCUMENT for this phase.
                                 Predictions P1-P5 and failure conditions
                                 F1-F4 are committed. You may not edit them.
  3. memo/FINDINGS.md          — the Phase 3 result you are testing
  4. rubric/RUBRIC.md          — 3 gates, 4 routes, axis anchors (Axis E is v1.2)
  5. rubric/TAXONOMY.md        — certificate type x production process
  6. corpus/NEGATIVE-SPACE.md  — what is absent and how the last search failed

WHAT PHASE 4 IS
Everything so far tested the rubric where the answer was known in advance. This
tests it where it can embarrass us. The failure mode nobody has ruled out: the
rubric is an elaborate restatement of the base rate, producing ~1-3%-ish numbers
with good citations wherever you point it.

BLINDNESS IS THE WHOLE EXPERIMENT
sources/erdos-problems.yaml is the REVEAL snapshot and CONTAINS OUTCOMES.
Do not read it, grep it, or load it until stage 3. Use sources/erdos-cutoff.yaml,
which is the database as it stood at the 2025-08-31 cutoff.

While scoring, do not fetch any page dated after 2025-08-31. Problem statements
come from DeepMind's formal-conjectures Lean files and pre-cutoff literature. If
you catch yourself recalling an outcome, write that in the row's
unverifiable_claims and score it anyway — declared contamination is recoverable,
undeclared is not.

STAGE 1 — DRAW THE SAMPLE
  git clone --depth 1 --filter=blob:none https://github.com/google-deepmind/formal-conjectures
  ./.venv/bin/python scripts/erdos.py --sample <that>/FormalConjectures/ErdosProblems

  This is deterministic (seed 20260725) and implements the pre-registered rule.
  Do not hand-adjust the output. If the sample looks wrong, say so and stop —
  do not fix it by choosing different problems.

  Write it to survey4/SAMPLE.md. COMMIT. 56 problems across 8 tags.

STAGE 2 — SCORE ALL 56, BLIND
  One YAML per problem, same schema as survey/ (see any file there, or
  scripts/score.py --template). Every row needs sources with tiers and an
  unverifiable_claims block. Axis calls need a one-paragraph rationale where
  they are not obvious.

  Statements from the Lean files. Gates per RUBRIC.md — G0 is "open to the
  field, not just the list", and inheriting `open` from the cutoff database is
  exactly the list status Bloom described, so g0_checked_rigorously stays false
  unless you actually search.

  Then: ./.venv/bin/python scripts/score.py --survey survey4/
        ./.venv/bin/python scripts/score.py --survey survey4/ --optimistic
  Report BOTH. COMMIT the scores before going near stage 3.

STAGE 3 — SPLIT THE LABELS, THEN REVEAL
  The known weakness of this design: "resolved since the cutoff" mixes new
  results with the database catching up on old literature. The rubric predicts
  AI-tractability; the raw label measures status change.

  Split them by the YEAR OF THE RESOLVING CITATION in each problem's
  formal-conjectures reference block:
    - resolving reference published <= 2020  -> CATCH-UP (was already solved)
    - resolving reference published >= 2024  -> NEW WORK
    - in between, or no reference            -> UNCLEAR, reported separately
  If a problem's file gives no usable citation, say so; do not guess.

  Then read sources/erdos-problems.yaml and compute the outcomes.

STAGE 4 — REPORT AGAINST THE PRE-REGISTRATION
  memo/VALIDATION.md, in this order:
    1. P1-P5, each stated, then the number, then hit or miss. No reordering
       to put the wins first.
    2. F1-F4, each explicitly checked. F1 (flat across fields) and F4 (rate at
       or below Phase 3's 8%) are the live ones.
    3. The label split, with P3 reported both ways (raw and new-work-only).
    4. What this does and does not validate.

  P3 is UNDERPOWERED at n=56 — roughly 8 problems will have resolved. Report the
  interval, do not report a null as meaningful.

IF THE RUBRIC FAILS
Say so plainly and diagnose which axis. A failed validation is a complete and
publishable result and is more valuable than a passed one that was rigged. Do not
adjust axis calls after seeing outcomes; if you are tempted, that is the finding.

CONSTRAINTS
- Every claim gets a source link with a tier. Unverifiable claims go in
  unverifiable_claims:, never dropped.
- Say "ambiguous" rather than guessing.
- Never attempt to prove or disprove anything. This is evaluation.
- Quote no rate without its regime. The hold-out base rate is ~16-18% (any
  status change, by anyone, 11 months) and is NOT the 1-3% autonomous-AI figure.
  Conflating them is the Rule 4 error.
- Use the .venv (PEP 668 — see CLAUDE.md mechanics).
- erdosproblems.com returns 403; CLAUDE.md has the workaround table.

Start by reading the files and confirming you understand what must not be read
before stage 3. Then draw the sample and show it to me before scoring.
```

---

## After Phase 4

Two branches, decided by the result — do not pre-commit to either.

**If it discriminates.** The instrument is real, and the open questions become
publication and whether to run it continuously (the community census froze
2026-06-30 and nothing replaced it). Phase 3's null result gains a lot of weight
from a Phase 4 that shows the same instrument firing elsewhere.

**If it does not.** The honest product shrinks to the vocabulary and the corpus —
still worth something, since the two-dimensional taxonomy and the
certificate/evaluator/expressibility distinctions did real work in Phase 3
independent of any score. The scoring layer gets retired or rebuilt. Say which.

The one thing that stays true either way: the boundary finding from Phase 3 —
that consistency statements are not expressible in a proof assistant's fixed
model, rather than merely hard — does not depend on the rubric working.
