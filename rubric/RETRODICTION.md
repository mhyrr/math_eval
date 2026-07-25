# Retrodiction: rubric v1 against the Phase 1 corpus

**Hindsight warning.** Scored by someone who knows every outcome. This catches gross errors; it is not validation. See RUBRIC.md Part 4.

## Resolved cases (n=14) -- should score CANDIDATE

```
+ alphaevolve-math-constructions: CANDIDATE -- clears W (witness/refutation)
    blocked F (formal/mathlib): formal_library_coverage 1 < 3 (no mature formal library for the ambient theory)
    blocked R (reservoir/transfer): reservoir_proximity 1 < 4
    blocked N (neglect/low-attention): prior_attention HIGH != LOW
+ alphaevolve-matmul-4x4: CANDIDATE -- clears W (witness/refutation)
    blocked F (formal/mathlib): formal_library_coverage 1 < 3 (no mature formal library for the ambient theory)
    blocked R (reservoir/transfer): reservoir_proximity 1 < 4
    blocked N (neglect/low-attention): prior_attention HIGH != LOW
+ alphaproof-nexus-erdos-nine: CANDIDATE -- clears F (formal/mathlib), N (neglect/low-attention)
    blocked W (witness/refutation): witness_representability 2 < 4; evaluator_availability 2 < 4
    blocked R (reservoir/transfer): reservoir_proximity 1 < 4
+ bolzano-eight-problems: CANDIDATE -- clears N (neglect/low-attention)
    blocked W (witness/refutation): statement form admits no finite witness; witness_representability 2 < 4; evaluator_availability 2 < 4
    blocked F (formal/mathlib): formal_library_coverage 2 < 3 (no mature formal library for the ambient theory)
    blocked R (reservoir/transfer): reservoir_proximity 3 < 4
? erdos-1026-monotone-subsequence: UNDETERMINED -- gate not assessed: open_to_field
+ erdos-1196-primitive-sets: CANDIDATE -- clears R (reservoir/transfer)
    blocked W (witness/refutation): statement form admits no finite witness; witness_representability 1 < 4; evaluator_availability 1 < 4
    blocked F (formal/mathlib): formal_library_coverage 1 < 3 (no mature formal library for the ambient theory)
    blocked N (neglect/low-attention): prior_attention HIGH != LOW; certificate_cost 2 < 3
+ erdos-397-formalization: CANDIDATE -- clears F (formal/mathlib), N (neglect/low-attention)
    blocked W (witness/refutation): witness_representability 1 < 4; evaluator_availability 2 < 4
    blocked R (reservoir/transfer): reservoir_proximity 1 < 4
+ erdos-52-sum-product-human-followup: CANDIDATE -- clears R (reservoir/transfer)
    blocked W (witness/refutation): evaluator_availability 1 < 4
    blocked F (formal/mathlib): formal_library_coverage 1 < 3 (no mature formal library for the ambient theory)
    blocked N (neglect/low-attention): prior_attention HIGH != LOW
+ erdos-728-factorial-divisibility: CANDIDATE -- clears F (formal/mathlib), N (neglect/low-attention)
    blocked W (witness/refutation): witness_representability 1 < 4; evaluator_availability 2 < 4
    blocked R (reservoir/transfer): reservoir_proximity 1 < 4
+ erdos-90-unit-distance: CANDIDATE -- clears R (reservoir/transfer)
    blocked W (witness/refutation): witness_representability 1 < 4; evaluator_availability 1 < 4
    blocked F (formal/mathlib): formal_library_coverage 1 < 3 (no mature formal library for the ambient theory)
    blocked N (neglect/low-attention): prior_attention HIGH != LOW; certificate_cost 2 < 3
+ funsearch-cap-set: CANDIDATE -- clears W (witness/refutation)
    blocked F (formal/mathlib): formal_library_coverage 1 < 3 (no mature formal library for the ambient theory)
    blocked R (reservoir/transfer): reservoir_proximity 1 < 4
    blocked N (neglect/low-attention): prior_attention HIGH != LOW
+ gemini-aletheia-autonomous-erdos: CANDIDATE -- clears N (neglect/low-attention)
    blocked W (witness/refutation): statement form admits no finite witness; witness_representability 2 < 4; evaluator_availability 1 < 4
    blocked F (formal/mathlib): formal_library_coverage 1 < 3 (no mature formal library for the ambient theory)
    blocked R (reservoir/transfer): reservoir_proximity 3 < 4
+ jacobian-conjecture-c3: CANDIDATE -- clears W (witness/refutation)
    blocked F (formal/mathlib): formal_library_coverage 2 < 3 (no mature formal library for the ambient theory)
    blocked R (reservoir/transfer): reservoir_proximity 1 < 4
    blocked N (neglect/low-attention): prior_attention HIGH != LOW
+ sawin-unit-distance-explicit: CANDIDATE -- clears R (reservoir/transfer)
    blocked W (witness/refutation): witness_representability 3 < 4; evaluator_availability 1 < 4
    blocked F (formal/mathlib): formal_library_coverage 1 < 3 (no mature formal library for the ambient theory)
    blocked N (neglect/low-attention): prior_attention HIGH != LOW
```

**13/14 resolved cases score CANDIDATE.**

## Negative / declined cases (n=4) -- should NOT score CANDIDATE

```
- alphaproof-imo-2024: RULED_OUT -- gate failure: open_to_field
- axiom-fel-conjecture-disputed: RULED_OUT -- gate failure: open_to_field
- gpt5-erdos-claim-october-2025: RULED_OUT -- gate failure: open_to_field
- gpt5pro-convex-optimization-bound: RULED_OUT -- gate failure: open_to_field
```

**4/4 negatives correctly declined.**

## Excluded

- `gemini-erdos-survey-700` -- aggregate study, not a problem
- `teorth-wiki-aggregate` -- aggregate record, not a problem

## Per-case notes

- **alphaevolve-math-constructions** -- Clears W only. Bf=1 correctly blocks F -- these were never formalized, and the pre-split axis was wrongly clearing it.
- **alphaevolve-matmul-4x4** -- Clears W. Most favourable instance in the corpus.
- **alphaproof-imo-2024** -- Competition problems are not open. Rubric declines on G0, which is the correct treatment of a benchmark.
- **alphaproof-nexus-erdos-nine** -- Clears F and N. The 2.5% base rate lives here.
- **axiom-fel-conjecture-disputed** -- NEGATIVE CASE. Proof was supplied in the input. Declines on G0.
- **bolzano-eight-problems** -- Same shape as above. Clears N only.
- **erdos-1026-monotone-subsequence** -- Closed by a 2024 paper a human found. G0 correctly returns UNDETERMINED -- the rubric says 'go search the literature first', which is exactly what would have been right.
- **erdos-1196-primitive-sets** -- A is the fragile score: blind you estimate 'a paper' (2); post-hoc you know it is 35 pages (1). At A=1 Route R fails and this case flips to RULED_OUT. Sensitivity flagged.
- **erdos-397-formalization** -- THIN sourcing; scores inherit that.
- **erdos-52-sum-product-human-followup** -- CONTROL (human). Clears R. Be=1 correctly blocks W: no automatic checker exists for sum-product constructions.
- **erdos-728-factorial-divisibility** -- Clears F and N. mathlib covers Kummer / p-adic valuations.
- **erdos-90-unit-distance** -- A counterexample here is an infinite family, so D=1 blocks Route W despite the result being called a 'counterexample'. Route R carries it. This is the press-coverage trap, scored.
- **funsearch-cap-set** -- D corrected to 5 after the split: a cap set IS a finite explicit object. The old D=3 was scoring search difficulty, not witness representability.
- **gemini-aletheia-autonomous-erdos** -- Motivated adding Route N. C=3 blocks R, Bf=1 blocks F, yet the problems resolved -- because they were neglected, not because a reservoir was tapped. DeepMind declines to call them major.
- **gpt5-erdos-claim-october-2025** -- NEGATIVE CASE. Rubric declines on G0. Correct.
- **gpt5pro-convex-optimization-bound** -- NEGATIVE CASE. The 1.75/L bound was already published, so the problem was not open. Rubric declines on G0. Correct, and for the right reason.
- **jacobian-conjecture-c3** -- Widely believed TRUE pre-resolution. Route W is available on statement FORM, not on expected answer -- a first pass keyed on expectation and wrongly ruled out the biggest result of 2026.
- **sawin-unit-distance-explicit** -- CONTROL (human). Clears R.

## Route usage across resolved cases

- Route W (witness/refutation): 4
- Route F (formal/mathlib): 3
- Route R (reservoir/transfer): 4
- Route N (neglect/low-attention): 5
