# Continuation prompt — after Phase 4

Phases 1–4 are complete. Read `README.md` for the shape, `memo/VALIDATION.md`
for where the instrument stands, `CLAUDE.md` for how to work here.

**Added 2026-07-26:** `CONCLUSIONS.md` (outsider-readable summary) and
`problems/` (the target pool — 14 seeded definitions, 43-problem queue, and
the one-by-one process). Item 3 below — the never-evaluated Route N — now has
its vehicle: the SEARCHED stage of each problem file is a rigorous G0
discharge. Work the queue there instead of designing a new subsample.

**Added later the same day:** the beyond-Erdős sweep. `problems/POOLS.md`
maps every non-Erdős pool with verified yields and denominators; six new
definition files (two SAT-community targets, four warehouse finds); and a
21st corpus case, `corpus/knuth-claude-cycles.yaml` — found because our
pinned statement source still marked it open five weeks after it fell.
Partial answer to item 5 below: https://aimath.robertj1.com/ tracks
AI-resolution claims (245 entries, labels spot-checked clean). Candidate
future corpus cases surfaced by the census, not yet written: the Aristotle
Kourovka eight (arXiv:2607.17477), the Carbery/Grok counterexample, the
Aaronson–Witteveen QMA case, the AlphaProof Nexus non-Erdős results
(arXiv:2605.22763).

---

## Where Phase 4 left things

The hold-out ran: 56 open Erdős problems across 8 tags, drawn by a deterministic
pre-registered rule, scored blind against a 2025-08-31 cutoff with the statement
source mechanically redacted, scores committed at `64dd1f3` before outcomes were
read at `c7b6aef`. The commits are the experiment.

**The rubric discriminates between fields and does not predict outcomes.** Rates
run 0%–57% across tags from pre-committed axis calls, the community's own
`verifiable`/`falsifiable` labels fall inside Route W 3 for 3 (p = 0.00014), and
`CANDIDATE` resolves at 7% against `RULED_OUT`'s 14% — identical, 7.1% each, on
the AI-assisted subset.

`NEXT-SESSION.md` used to pre-commit to two branches. The result picked neither,
so the call recorded in `memo/VALIDATION.md` §8 is: **retire the ruling-in
function, keep the ruling-out function and the vocabulary.**

## What is genuinely open

**1. Rebuild or retire Axis B2.** It is the axis the survey turns on and the one
that failed, and the failure is specific: four problems that received
machine-checked Lean proofs (#42, #152, #330, #871) score `B2 = 2` under every
reading in the sensitivity band. B2 encodes the AlphaProof Nexus scope condition
— successes concentrate "where Lean's mathematics library is mature" — and the
2026 prover agents in this sample did not obey it. A rebuild needs evidence about
what *does* gate those systems. **This project does not have that evidence**, and
inventing it would be the failure mode `CLAUDE.md` exists to prevent. Getting it
means reading what the four resolving efforts actually did.

**2. `#330` is unexplained.** The database says `proved (Lean)` (2026-05-11); the
`formal-conjectures` file's main theorem still reads `category research open`
with `answer(sorry)` and cites nothing. One of the two is stale. It was reported
UNCLEAR rather than guessed. Resolving it is a small, checkable task.

**3. Route N has never been evaluated.** It needs a rigorously discharged G0 —
a literature search per problem — which `--optimistic` deliberately does not
grant. It is the corpus's most-used route and **45% of the Phase 4 sample would
clear it conditionally**. Untested, not refuted. A 10-problem subsample with real
G0 work would settle more than another 56 scored rows.

**4. Route R rests on one row.** `#212`'s `C = 4` is unsourced recall that
Erdős–Ulam follows from Bombieri–Lang, flagged in that row as the survey's
weakest call. One clearance in 56 with a soft source is not evidence.

**5. The census gap.** The community AI census froze 2026-06-30 and nothing
replaced it. `scripts/erdos.py` plus the reveal machinery is most of a
replacement. Whether to run it continuously is a product question, not a research
one.

## What not to redo

- **Phases 1–3.** Corpus, taxonomy, rubric and the CH survey are complete.
  `scripts/score.py --retrodict` still produces a byte-identical
  `rubric/RETRODICTION.md`; the Phase 3 survey still scores 2/24.
- **The Phase 4 sample.** `scripts/erdos.py --sample` is deterministic. Rescoring
  the same 56 with knowledge of the outcomes produces a story, not a test.
- **Any argument that the Phase 3 null depends on the rubric.** It does not. The
  boundary finding — consistency statements are not expressible in a proof
  assistant's fixed model, rather than merely hard — is structural, has a primary
  source, and survives the scoring layer's failure intact.

## Standing hazards

- `sources/erdos-problems.yaml` **contains outcomes**. Any future blind work uses
  `sources/erdos-cutoff.yaml` and `scripts/blind.py`.
- The `formal-conjectures` Lean files **are the answer key** — 358 of 510 carry
  `category research solved`, 436 an inline `answer(...)`. `scripts/blind.py
  --selftest` is the guard; run it before trusting a redacted statement.
- `erdosproblems.com` still returns 403. The workaround table is in `CLAUDE.md`.
