# Validation: the rubric discriminates between fields and does not predict outcomes

**Phase 4 memo. 2026-07-25.** 56 open Erdős problems, drawn by a pre-registered
deterministic rule, scored blind against a 2025-08-31 cutoff with the statement
source mechanically redacted, scores committed at `64dd1f3`, outcomes read after.

**The result is split, and the two halves point opposite ways.**

- **The field discrimination is real.** CANDIDATE rates run 0% to 57% across
  eight tags, the variation traces to axis calls written down before the reveal,
  and the community's own finite-certificate labels land inside the rubric's
  Route W nominations 3 for 3 (p = 0.00014). The instrument is not a base-rate
  machine emitting 1–3% wherever you point it. **F1 does not trigger.**
- **The outcome prediction is not real.** Problems scored CANDIDATE resolved at
  7% against 14% for RULED_OUT. Restricted to the four resolutions that are
  explicitly AI-assisted, the two rates are **identical, 7.1% and 7.1%**. The
  filter selected AI-resolved problems at exactly its own base rate. **F2
  triggers.**

The ruling-in half has now been tested and does not work. `RUBRIC.md` said the
instrument was asymmetric and better at ruling out; that claim survives, and the
other half of it does not.

---

## 1. Predictions, in the order they were written

### P1 — overall CANDIDATE rate, optimistic, tag-weighted: 20–45%

**24.9% tag-weighted, 25% unweighted (14 of 56). HIT.**

With a caveat that undercuts it: the rate is a function of one anchor sentence.
`RUBRIC.md` defines `B2 = 3` as "mathlib covers the ambient theory", and three
defensible readings of that phrase give **9%, 25% and 36%** on the same 56
problems with every other axis call identical. The middle reading was fixed in
`scripts/mk_survey4.py` before the reveal, and two of the three readings land
inside P1's range. A prediction interval 25 points wide, hit by a number that
moves 27 points under reinterpretation, is not much of a test.

### P2 — CANDIDATE rate varies across tags by a factor of ≥ 3

**0% to 57%. HIT.**

| tag | CANDIDATE | rate | resolved since cutoff |
|---|---|---|---|
| graph theory | 4/7 | 57% | 1 |
| unit fractions | 4/7 | 57% | 0 |
| number theory | 3/7 | 43% | 0 |
| additive basis | 1/7 | 14% | 3 |
| distances | 1/7 | 14% | 0 |
| primes | 1/7 | 14% | 0 |
| additive combinatorics | 0/7 | 0% | 1 |
| sidon sets | 0/7 | 0% | 2 |

Two tags score zero, so the highest-to-lowest ratio is undefined; against the
lowest nonzero tag it is 4.0×.

The pre-registration's success condition also required the variation be
"explicable by the axes rather than post-hoc". It is. Distances scores low
because `B2 = 1` — mathlib has essentially no incidence geometry — and its single
CANDIDATE comes through Route R, not F. Sidon sets and additive combinatorics
score zero because they combine `B2 = 2` with asymptotic statements that admit no
finite witness. Graph theory and unit fractions score high because finite
combinatorial and elementary-rational objects sit on mature mathlib apparatus.
Every one of those calls is in the scoring table, committed before the reveal.

**Now read the right-hand column.** The two tags scored 0% produced 3 of the 7
resolutions. The two scored 57% produced 1. The field discrimination is real and
it is pointing the wrong way.

### P3 — CANDIDATE problems resolve at a higher rate than RULED_OUT

**7% versus 14%. MISS, and numerically inverted.**

| population | CANDIDATE | RULED_OUT | Fisher one-sided |
|---|---|---|---|
| all 7 resolutions | 1/14 = 7.1% | 6/42 = 14.3% | p = 0.884 |
| new work only (drop #847, #330) | 1/14 = 7.1% | 4/42 = 9.5% | p = 0.777 |
| **AI-assisted only** | **1/14 = 7.1%** | **3/42 = 7.1%** | p = 0.695 |

95% Clopper-Pearson intervals: CANDIDATE 0–34%, RULED_OUT 5–29%. They overlap
across almost their whole range.

**Do not read the inversion as an inversion.** With 7 resolutions in 56 problems,
a uniform 12.5% rate predicts 1.75 CANDIDATE resolutions and 5.25 RULED_OUT
against 1 and 6 observed. That is the null almost exactly, not evidence that the
axes run backwards. The honest statement is *no signal*, which is F2, not F3.

P3 was pre-registered as badly underpowered at this n and it was. That is the one
part of this test that discovered nothing, because it was written down in advance.

### P4 — Route N is the most-used route, as in the corpus

**0 clearances. MISS — by construction, not by evidence.**

`route_n` requires `g0_checked_rigorously`, `--optimistic` does not grant it
because it is not in `GATES`, and G0 cannot be discharged for 56 problems without
a per-problem literature search. So Route N scores zero whatever the territory
looks like. This was flagged and the reporting decided **before scoring**
(`PREREGISTRATION.md`, Appendix B, 2026-07-25): report a secondary count of rows
that would clear Route N if G0 were discharged.

**That conditional count is 25 of 56 (45%)** — which would make Route N the
most-used route by a wide margin over F (12), W (4) and R (1), and P4 a hit. Both
numbers stand: the primary is a miss, the conditional is a hit, and the gap
between them is a fact about the instrument's operating cost rather than about
mathematics. Route N is the corpus's most-used route and it is the one route this
design cannot evaluate.

### P5 — set-theory-tagged problems score at or near the Phase 3 rate

**NOT TESTABLE. No set-theory-tagged problem is in the sample.**

The selection rule takes the eight largest qualifying tags; set theory is not among
them. P5 was written without checking that the sample it governs would contain a
single instance of its subject. That is a defect in the pre-registration, caught
by the reveal rather than by the author, and it is reported as a miss of process
rather than a null result.

---

## 2. Failure conditions, each checked

### F1 — base-rate machine, flat across fields — **NOT TRIGGERED**

0% to 57%, with the spread traceable to specific pre-committed axis calls. The
central worry going in was that the rubric restates the prior wherever you point
it. It does not. Whatever else is wrong here, the instrument distinguishes
territories.

### F2 — no outcome signal — **TRIGGERED**

CANDIDATE and RULED_OUT resolve indistinguishably; on the AI-assisted subset,
identically. Pre-registered as "likely to be underpowered rather than decisive",
and that is what it is: this test cannot separate a weak real effect from none.
What it can say is that no *large* effect exists, and a filter that nominates 25%
of a sample and captures 25% of its AI resolutions has demonstrated nothing.

### F3 — inversion — **NOT TRIGGERED**

Nominally RULED_OUT resolved more often. Statistically the observed counts sit on
the null. Claiming an inversion from 1 versus 6 would be the same error in the
opposite direction.

### F4 — rate at or below Phase 3's optimistic 8% — **NOT TRIGGERED, narrowly**

25% against Phase 3's 8%. But under the technique reading of B2 the rate is 9%,
one point clear of the threshold. Whether F4 fires is decided by the same
sentence that decides P1. Phase 3's territory still looks distinctive, and the
margin is thinner than the headline suggests.

---

## 3. The label split, and what the reveal forced

The pre-registration's stated weakness was that "resolved since the cutoff" mixes
new results, database catch-up, and reclassification. The reveal made a fourth
distinction necessary and supplied a mechanical way to draw it.

**3 of the 10 status changes are vocabulary backfill.** #7, #617 and #835 moved
from `open` to `verifiable`/`falsifiable` with `last_update` **still on the
2025-08-31 seed date**. The maintainers added the finite-certificate vocabulary
after the seed and applied it retroactively. Nothing happened to those problems;
they are still open. Any P3 computed on raw status change is measuring the
maintainers' vocabulary — and it matters, because **3 of the 4 CANDIDATE rows
that "moved" are backfill**. The naive reading gives CANDIDATE 29% versus 14% and
looks like a win. It is an artifact.

Three more problems (#11, #85, #855) had `last_update` bumped while staying open:
maintainer attention without resolution.

That leaves **7 real resolutions, 12.5% of the sample**, against the
pre-registration's projected ~8. Applying the pre-registered citation-year rule:

| bucket | n | which |
|---|---|---|
| NEW WORK (resolving citation ≥ 2024) | 5 | #42, #152, #619, #868, #871 |
| CATCH-UP (≤ 2020) | 0 | — |
| UNCLEAR | 2 | #847, #330 |

- **#847** is bucketed NEW WORK by the rule and is substantively CATCH-UP: it
  resolves on Reiher–Rödl–Sales, *J. London Math. Soc.* (2024), published more
  than a year before the cutoff. The pre-registered `≥ 2024 → NEW WORK` boundary
  is simply wrong for it. The boundary is not being moved after seeing the data.
- **#330** carries `proved (Lean)` in the database while its Lean file's main
  theorem still reads `category research open` with `answer(sorry)` and cites
  nothing. No usable citation, so it is reported unclear rather than guessed.

P3 both ways is in §1: 7.1% vs 14.3% raw, 7.1% vs 9.5% new-work-only. Neither
separates.

**The one genuinely good news about the label.** 4 of the 7 resolutions are
explicitly AI-assisted — #42 by GPT-5.5 Pro, #152 by a DeepMind prover agent,
#619 by GPT-5.5 with Codex, #871 by Claude Opus 4.5, three of them carrying
machine-checked proof links. The pre-registration worried that the label would
measure status change rather than AI-tractability. On this sample they largely
coincide, which makes the target closer to the rubric's actual subject than
feared — and makes the null harder to explain away.

---

## 4. Which axis failed, and how

**B2, `formal_library_coverage`, and not at its threshold — at its premise.**

Route F carries 12 of the 14 CANDIDATE nominations, so the survey is mostly a B2
survey. Four problems received machine-checked Lean proofs after being scored
`B2 = 2`, i.e. Route F blocked: **#42, #152, #330, #871**. Two more resolutions
(#847, #868) were also outside Route F. Route F rows resolved at 1/12 = 8.3%
against 6/44 = 13.6% for everything else.

The misses are not near the line. None of #42, #152, #330 or #871 reaches
`B2 = 3` under *any* of the three readings in the sensitivity band, including the
broadest. And they are not scattered: they sit in sidon sets and additive basis,
the two tags B2 scored lowest, while Route F's clearances sat in graph theory,
unit fractions and elementary number theory. The prediction and the outcome are
close to orthogonal.

The premise is the problem. B2 asks whether mathlib is mature in the problem's
area, on the strength of the AlphaProof Nexus scope condition — successes
concentrate "where Lean's mathematics library is mature and tasks often decompose
into tractable subgoals." That described the 2023–2025 systems the Phase 1 corpus
was built from. The systems that resolved these four in 2026 — a DeepMind prover
agent, GPT-5.5 with Codex, Claude Opus 4.5 — were evidently not gated on ambient
mathlib coverage in the problem's field. **B2 is calibrated on a constraint that
appears to have relaxed underneath it.**

Note what this is not. It is not a case of the rubric being too strict: the same
axis nominated 12 problems, 11 of which did not resolve. It selected the wrong
12.

---

## 5. The one clean hit, and it is not a prediction

The database labels a problem `decidable`, `falsifiable` or `verifiable` when a
finite computation would settle it. That is Route W's requirement — `D ≥ 4`, the
answer can be a finite object, and `B1 ≥ 4`, an exact automatic checker exists —
written in the maintainers' vocabulary by people with no knowledge of this rubric.

- Route W cleared, blind: **#7, #11, #617, #835**
- Community finite-certificate labels: **#7, #617, #835**
- **3 of 3 inside Route W.** No RULED_OUT row carries the label.

Under random assignment of 3 labels among 56 problems, the chance all land inside
a nominated set of 4 is **0.00014**.

This is construct validity, not predictive validity. Route W's conditions and the
community's label mean nearly the same thing, so the agreement tests whether the
axes were *applied* consistently across 56 blind judgements — not whether they
forecast anything. Those three problems remain open. But it is the only place in
this project where an independent party, using its own vocabulary and its own
judgement, picked out the same problems the instrument did, and it is direct
evidence that Axes D and B1 measure something stable.

---

## 6. What this validates, and what it does not

**Validated.**

1. **The instrument is not a base-rate machine.** F1 was the live worry and it
   does not trigger. Rates vary 0–57% across fields and the variation is
   explicable from pre-committed axis calls.
2. **Axes D and B1 are applied consistently and mean what they say** — the
   finite-certificate agreement, p = 0.00014.
3. **The vocabulary does work the score does not.** Separating certificate type
   from production process, evaluator from library coverage, and witness
   finiteness from search difficulty is what made rows like #170 (excellent
   evaluator, unwitnessable question) and #213 (exact checker, but the answer is
   a scheme) scoreable at all. That machinery survives the scoring layer's
   failure, as `NEXT-SESSION.md` anticipated it might.

**Not validated.**

1. **The ruling-in half.** CANDIDATE does not predict resolution, including
   AI-assisted resolution. This was the untested direction and it has now been
   tested.
2. **The headline rate.** Any single number from this instrument is an analyst's
   reading of one sentence about mathlib. 9%, 25%, 36% — same problems, same
   everything else.
3. **Route N.** Never evaluated: it needs a rigorously discharged G0, which costs
   a literature search per problem. It is the corpus's most-used route and 45% of
   this sample would clear it conditionally. Untested, not refuted.
4. **Route R.** One clearance in 56, and #212's `C = 4` rests on unsourced recall
   that Erdős–Ulam follows from Bombieri–Lang — flagged in that row as the
   least-sourced call in the survey. A route resting on one row with a soft
   source is not evidence of anything.
5. **Anything about set theory.** P5 had no subjects. Phase 3's null result gains
   nothing from this test and loses nothing.

**Still true independent of all of it:** the Phase 3 boundary finding — that
consistency statements are not expressible in a proof assistant's fixed model,
rather than merely hard — is a structural argument with a primary source and does
not depend on the rubric working.

---

## 7. Threats to this result, in order of how much they worry me

1. **Blindness is imperfect and the leak runs the flattering way.** 40 of 56
   statements come from a post-cutoff file version. Redaction is a blacklist;
   `scripts/blind.py --selftest` verifies over 453 non-sampled files that no
   status marker, answer payload or proof link survives, but statement-form prose
   ("it is not known whether X") is deliberately kept because it is the
   statement, and in a post-cutoff file it weakly signals "still open". A scorer
   who senses a problem is still open can score it RULED_OUT and manufacture
   agreement with P3. **P3 failed anyway**, which is the strongest thing that can
   be said about this leak: it could only have helped, and did not.
2. **One analyst, no second scorer.** Every axis call is a single judgement
   against written anchors. The B2 band exists because I could not settle a
   reading with myself; a second scorer would produce a different survey.
3. **n = 56, 7 events.** P3 could not have succeeded convincingly either. A
   positive result at this n would have deserved the same paragraph.
4. **Route W's 3-for-3 could be tautological.** If `verifiable` and `D ≥ 4, B1 ≥
   4` are definitionally the same test, the agreement measures care rather than
   validity. I think care is worth measuring across 56 blind rows, but a reader
   who disagrees loses the only clean positive here.
5. **`#330` is unexplained.** The database says `proved (Lean)`; the Lean file
   says open with a `sorry`. One of the two is stale. I did not resolve it and
   did not let it into the NEW WORK bucket.

---

## 8. What follows

`NEXT-SESSION.md` pre-committed to two branches. The result picks neither
cleanly, so: **the scoring layer's ruling-in function is retired; its ruling-out
function and the vocabulary are kept.**

Concretely, and each of these is checkable:

1. **Stop reporting CANDIDATE as a shortlist.** It does not predict, on the only
   test that could have shown it doing so. Report RULED_OUT verdicts and the
   axis-by-axis argument behind them, which is what Phase 3 actually delivered
   and what survived here.
2. **Rebuild or retire B2.** It is the axis the survey turns on and the axis that
   failed, and the failure has a specific shape: it encodes a 2023–2025
   constraint about mathlib maturity that the 2026 prover agents in this very
   sample did not obey. Any rebuild needs evidence about what *does* gate them,
   which this project does not have.
3. **The one number worth watching.** Of 56 ordinary open Erdős problems, 7
   resolved in 11 months and 4 of those were AI-assisted. That is roughly **7%
   AI-assisted resolution over eleven months on a stratified sample of the open
   database** — a regime figure with an honest denominator, and the first one in
   this project that is neither the 1–3% autonomous-screen rate nor a filter-pass
   rate. It is not comparable to either. It should be quoted with this sentence
   attached.
