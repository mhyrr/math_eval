# Pre-registration: Phase 4 validation test

**Written 2026-07-25, BEFORE any Phase 4 problem is scored.** Nothing below may be
edited after scoring begins. Amendments go in a dated appendix at the bottom with
the reason, so a reader can see what moved and when.

---

## Why this document exists

The rubric has been pointed in two directions, and **we knew the answer to both
before we started.**

| test | result | but |
|---|---|---|
| Phase 1 corpus (known successes) | 93% CANDIDATE | derived from the same cases; pure hindsight |
| Phase 3 survey (CH neighbourhood) | 0–8% CANDIDATE | everyone expected null; the brief said so outright |

An instrument that separates "already solved" from "famously hopeless" has
distinguished two things anybody could distinguish. **The untested direction is
the one where the instrument can embarrass us**, and the failure mode nobody has
ruled out is this:

> The rubric is an elaborate restatement of the base rate. Point it anywhere, get
> something in the neighbourhood of 1–3%, dressed in good citations.

Phase 3's 8% is not evidence against that. It is *close enough to the base rate*
to be exactly what a base-rate machine would emit.

## The design, and why it beats "pick a new field"

The standard objection to validating this rubric is that **no lab publishes which
problems its system attempted and failed**, so there are no negative labels
anywhere. That is still true.

But there is a different source of labels: **time.** The
`teorth/erdosproblems` database records `status.state` and `status.last_update`
per problem. 470 problems were touched after the database's 2025-08-31 seed date
and **326 of them are now resolved**. That gives real outcome labels for a large
set of problems that were open at a known past date.

So the test is a **time-sliced hold-out**, not a new-field survey:

1. Freeze a cutoff date.
2. Select problems that were open at the cutoff.
3. Score them using **only sources dated before the cutoff**.
4. Reveal the current database status.
5. Ask whether CANDIDATE problems resolved at a higher rate than RULED_OUT ones.

This is the first thing in the project that is a genuine hold-out rather than a
retrodiction, because the label is not available at scoring time.

---

## The contamination problem, stated honestly

**I am not a blind scorer and cannot be made into one.** Over this session I have
read the outcomes of roughly two dozen post-cutoff results in detail — Erdős #90,
#1196, #728, #1026, #1067, #591, #1119, the Jacobian counterexample, Cichoń's
maximum, and others. That knowledge does not go away because I want it to.

Three mitigations, none of them complete:

1. **Exclusion list.** Every problem discussed in `corpus/`, `survey/`, this
   session's transcript, or the frozen wiki's notable-cases page is excluded from
   the hold-out. The list is fixed in the appendix below before selection runs.
2. **Source discipline at scoring time.** No fetch of any page dated after the
   cutoff while scoring. Statements come from `formal-conjectures` files and
   pre-cutoff literature only. The database status field is **not read** until
   step 4.
3. **Declared, not defended.** Where contamination is suspected on a specific
   row, it gets flagged in that row rather than argued away.

**Residual risk: real and unquantifiable.** A model trained on the open internet
may recall an outcome without knowing it recalls it. This test is therefore
*better* than retrodiction and *weaker* than a true blind hold-out, and no number
it produces should be described as validation without that sentence attached.

---

## Specification

### Cutoff
**2025-08-31**, pinned to commit **`009173b2`** (`2025-08-31T23:55:32Z`), the last
commit touching `data/problems.yaml` before the boundary. Chosen because it is
the database's seed date — the one date on which status is known and uniform for
every problem — not for anything about the results after it.

### Population
Problems with `status.state == "open"` **in the historical file at `009173b2`**,
minus the exclusion list.

> **Corrected before scoring, 2026-07-25.** The first draft of this document
> defined the population as `state == "open" AND last_update == "2025-08-31"` in
> the *current* file. That is wrong and would have destroyed the test: it selects
> only problems that are *still* open, so the resolved-since count would be zero
> by construction and there would be nothing to validate against. The fix is to
> read the historical file, which GitHub serves at any commit. Recorded here
> rather than silently patched — this is exactly the class of error
> pre-registration exists to make visible.

### Field selection rule — fixed before looking at outcomes
Stratify by `tags`. Take every tag with **n ≥ 25 problems** in the database and
**≥ 3 usable problems** in the pool. Restrict to problems with a Lean statement
in `formal-conjectures`, so statements are PRIMARY-sourced.

**No tag is chosen for being interesting.** The comparison runs across all
qualifying tags at once, which removes the "which field did you pick" degree of
freedom entirely. Problems carrying several qualifying tags are assigned to their
**rarest** tag, so the small fields are not swamped.

### Sample size and allocation
**60 problems**, **roughly equal per tag** across the top 8 qualifying tags
(7–8 each) — *not* proportional. Proportional allocation would hand ~41 of 60 to
number theory and leave every other field too small to compare, which would gut
P2, the primary prediction. Equal allocation maximises power for the cross-field
contrast at the cost of a less representative overall rate; P1 is therefore
reported as a tag-weighted figure with the unweighted one alongside.

If sourcing forces a smaller sample, the achieved n is reported and the power
statement recomputed, not quietly dropped.

### Feasibility — verified 2026-07-25, before scoring

| quantity | count |
|---|---|
| problems in the cutoff snapshot | 992 |
| **open at cutoff** | **637** |
| of those, resolved by 2026-07-25 | **113 (17.7%)** |
| after exclusion list (61 numbers) | 612 |
| **and carrying a Lean statement — the usable pool** | **313** |
| of the usable pool, resolved since | 49 (15.7%) |
| qualifying tags | 16 |

The design runs. n = 60 out of a usable pool of 313, against a 15.7% label rate.

---

### What the outcome label actually means — the design's main weakness

**"Resolved since 2025-08-31" is not "an AI solved it."** The 17.7% mixes at
least three things:

1. genuinely new results, human or machine;
2. **database catch-up** — someone found the problem was already answered in the
   literature (the frozen wiki logs ~84 such rows under literature search);
3. reclassification, e.g. a problem moved to `independent` or `falsifiable`.

The rubric predicts *AI-tractability*. The label measures *status change*. Those
are different targets and the gap is real.

It is not fatal, for one reason: **the rubric already has a route for category 2.**
Route N is defined as "open because nobody tried, not because it is hard," and the
Gemini screen's own diagnosis of why problems sat open was "obscurity rather than
difficulty." So the test is still informative — but P3 must be read as *"does the
filter predict status change"*, which is weaker than *"does the filter predict
AI-solvability"*, and it will be reported in those words.

The cutoff snapshot's status vocabulary is also coarser than today's: only
`open` and `solved` existed in August 2025. The richer labels (`independent`,
`falsifiable`, `not provable`) came later. So the cutoff side of the comparison
carries less information than the reveal side.

**If the run shows a way to separate category 1 from category 2 post hoc**, that
split becomes the headline and P3 gets reported both ways. I do not currently
know how to do it cleanly, and I am recording that I do not, rather than
discovering it conveniently later.

## Predictions, recorded now

I commit to these before scoring. **Getting them wrong is informative and will be
reported as prominently as getting them right.**

| # | prediction | my confidence |
|---|---|---|
| P1 | Overall CANDIDATE rate across the 60, optimistic reading, tag-weighted: **20–45%** | moderate |
| P2 | CANDIDATE rate varies across tags by a factor of **≥ 3** between highest and lowest qualifying tag | moderate |
| P3 | Problems scored CANDIDATE resolve at a **higher** rate than RULED_OUT ones | low–moderate |
| P4 | Route N (neglect) is the most-used route, as in the corpus | moderate |
| P5 | Set-theory-tagged problems in this sample score at or near the Phase 3 rate | high |

**P1's range is wide on purpose.** A narrow prediction here would be false
precision — I genuinely do not know, which is the reason for running this.

**A note on the base rate, because it will be misread otherwise.** The hold-out's
label rate is **17.7%**, not the 1–3% this project calibrates against everywhere
else. Those are different quantities and both are correct: 1–3% is *autonomous AI
resolution of a listed problem*; 17.7% is *any status change over eleven months,
by anyone, including catch-up on decades-old literature*. Quoting one where the
other belongs would be exactly the Rule 4 error this project keeps flagging in
other people's work.

**Power, stated now.** At n = 60 with a 15.7% label rate, roughly 9 problems in
the sample will have resolved. **P3 is badly underpowered** — distinguishing a
real filter effect from noise on 9 positives is not something 60 problems can do
except for a very large effect. This is a known limitation of the design, not a
discovery to be made after the fact. P2 is the prediction this test can actually
settle; P3 is a bonus reported with its confidence interval.

## What counts as failure — written before the data

The instrument **fails** this test if any of the following holds:

- **F1 — base-rate machine.** Every qualifying tag lands within a few points of
  every other, i.e. P2 is false and the spread is flat. Then the rubric's field
  discrimination is not real and the honest report is that it restates the prior.
- **F2 — no outcome signal.** CANDIDATE and RULED_OUT problems resolve at
  indistinguishable rates (P3 false, with the difference inside noise for the
  achieved n). The filter would be sorting on something that does not predict.
- **F3 — inversion.** RULED_OUT problems resolve at a *higher* rate. This would
  mean the axes are anti-correlated with tractability and the instrument is worse
  than a coin.
- **F4 — no discrimination against Phase 3.** Overall rate comes in at or below
  the Phase 3 optimistic figure of 8%. Then Phase 3's null result was the
  instrument declining everything, not the territory being distinctive.

**F1 and F4 are the ones I actually expect to be live.** F2 is likely to be
underpowered rather than decisive at n = 60, and that limitation is stated here
rather than discovered later.

## What counts as success

**Not** "it nominated the right problems." Success is:

- the rate varies meaningfully across fields (P2), **and**
- the variation is explicable by the axes rather than post-hoc (each field's rate
  traces to specific axis distributions, written down before the outcome reveal),
  **and**
- the Phase 3 territory remains an outlier at the bottom (P5).

P3 holding would be a bonus and would be reported with heavy caveats about
contamination and power. **P3 failing does not by itself sink the instrument**,
because the rubric is explicitly a diagnostic that rules out rather than a
predictor that rules in — but it must then be reported that the ruling-in half
has been tested and does not work.

---

## Procedure, in order

1. Snapshot the database to `sources/erdos-problems.yaml`. **SHAs pinned
   2026-07-25:**

   | source | commit | date |
   |---|---|---|
   | `teorth/erdosproblems` cutoff snapshot | `009173b2` | 2025-08-31T23:55:32Z |
   | `teorth/erdosproblems` reveal snapshot | `35ba233a` | 2026-07-25T14:09:39Z |
   | `google-deepmind/formal-conjectures` | `5a60e068` | 2026-07-25T14:08:30Z |

   The reveal snapshot is pinned **now**, before scoring, so the outcome set
   cannot drift while the scoring runs. The database is edited most days.
2. Apply the exclusion list, then the selection rule. Write the sample to
   `survey4/SAMPLE.md` **before scoring**.
3. Score all 60 from pre-cutoff sources only. Status field not consulted.
4. Freeze scores. Commit.
5. Reveal outcomes. Compute rates.
6. Report against P1–P5 and F1–F4, in that order, whatever they say.

Step 4 is the one that makes this a hold-out rather than a story. If the scores
are not committed before the reveal, the test did not happen.

---

## Appendix A — exclusion list (fixed 2026-07-25, pre-selection)

Every Erdős problem number appearing anywhere in `corpus/`, `survey/`, or this
session's research. Excluded from the Phase 4 population because outcome
knowledge is confirmed or likely:

```
52  70  75  90  164  397  401  501  590  591  592  593  594  595  596  597  598
601  602  623  728  729  740  846  857  918  949  965  1026  1028  1034  1036
1037  1039  1044  1048  1067  1068  1071  1080  1095  1098  1102  1119  1123
1127  1128  1154  1167  1168  1169  1170  1171  1172  1173  1174  1175  1176
1177  1196  1217
```

Also excluded: every problem listed on the frozen wiki's *Notable cases* page,
and every problem appearing in any table in the frozen wiki, since those outcomes
were read during Phase 3.

**This is a large exclusion and it shrinks the population.** That is the correct
trade. A hold-out contaminated at the selection step is worth nothing.

## Appendix C — the selection rule is code, not prose

`scripts/erdos.py --sample <formal-conjectures>/FormalConjectures/ErdosProblems`

Deterministic (seed `20260725`), reads only the cutoff snapshot, applies the
exclusion list and the stratification rule mechanically, and prints the sample.
Re-running it reproduces the identical 56 problems.

A pre-registered rule that lives only in prose is a promise. One that lives in a
deterministic function is a constraint. **If the sample the next session scores
is not the one this command prints, the test did not happen.**

Achieved sample, drawn 2026-07-25 before scoring:

| tag | pool | drawn |
|---|---|---|
| additive basis | 14 | 7 |
| additive combinatorics | 15 | 7 |
| distances | 15 | 7 |
| graph theory | 15 | 7 |
| number theory | 138 | 7 |
| primes | 24 | 7 |
| sidon sets | 15 | 7 |
| unit fractions | 17 | 7 |

**Achieved n = 56**, not the 60 targeted — 8 strata × 7 rather than 60 split
unevenly. Recorded rather than adjusted; the power statement above is unchanged
in substance (expected ~9 resolved becomes ~8).

## Appendix B — amendments

**2026-07-25, pre-scoring.** Population definition corrected from a
current-file query to a historical-snapshot query. See "Population" above; the
original would have yielded zero outcome labels.

**2026-07-25, pre-scoring.** Target n stated as 60; achieved 56 under equal
allocation across 8 strata. See Appendix C.

*(any change after scoring begins is recorded here with a date and a reason)*
