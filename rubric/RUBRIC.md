# Tractability rubric v1

Derived from the 20-case Phase 1 corpus. Companion to `TAXONOMY.md`, which
defines the classification vocabulary this rubric scores against.

## The instrument is asymmetric, on purpose

It answers **"can this be ruled out?"** far better than **"will this work?"**

That is not a design compromise, it is what the evidence supports. The base
rate for autonomous resolution of a listed open problem is **0.7%–2.5%** in the
friendliest available field (`gemini-erdos-survey-700`,
`alphaproof-nexus-erdos-nine`). Any instrument claiming to identify problems
that *will* fall is claiming to beat a 1-in-40 prior on 20 observations. It
cannot.

So the outputs are:

| verdict | meaning | confidence |
|---|---|---|
| `RULED_OUT` | a necessary condition fails; no known mechanism applies | **high** — this is what the corpus supports |
| `CANDIDATE` | clears at least one route; base rate still applies | **low** — a filter, not a prediction |
| `UNDETERMINED` | a gate or axis could not be assessed from available evidence | — record what's missing |

A `CANDIDATE` verdict means *"this is in the ~2% pool worth spending on,"* never
*"this will be solved."* Any use of this rubric that reports candidates without
the prior attached is misusing it.

---

## Structure: three gates, then three routes

Not a weighted sum. The corpus shows the axes are **not substitutable** — a rich
adjacent reservoir cannot compensate for having no way to verify the answer.
Summing would let strength on one axis mask a fatal zero on another, which is
precisely the error that produces the "AI will crack CH" genre of prediction.

Instead: kill on gates, then ask whether **any** of three concrete mechanisms
applies. Each route is a real pathway observed in the corpus, with its own
necessary conditions.

---

# Part 1 — Gates

A gate failure is disqualifying regardless of every other score.

## G0 — Is it open to the *field*, or only to the *list*?

The single most common way a problem gets "solved" is that it was not open.

- **8 of 13** hits in the Gemini 700-problem screen were identification of
  existing published solutions, not discovery.
- The teorth wiki logs ~70 instances under literature search.
- The October 2025 GPT-5 episode was this failure in public.

Thomas Bloom's clarification is the operative definition: a problem listed
"open" means roughly that *at least one professional mathematician looked and
did not find a published solution*. That is a statement about the list's
coverage, not humanity's knowledge.

| verdict | condition |
|---|---|
| PASS | a specialist in the subfield has been asked, or a literature search targeting the specific statement has been run |
| FAIL | status inherited from a list without independent check |

**This gate must be discharged by work, not by scoring.** It is the one place
the rubric demands an action rather than a judgment.

## G1 — Does resolution require extensive new theory?

The corpus's only unanimous finding. **Zero of 20 cases** involved building new
framework. The evidence is unusually strong because it comes from all three
directions at once:

- Gowers, co-signing the Erdős #90 digest: *"many of the ideas needed for the
  proof were present in the literature already"*
- Tao on Erdős #1026: the proof *"turned out to not be particularly novel"*
- DeepMind, on its own results: *"we do not claim any Level 3 ('Major Advance')
  and Level 4 ('Landmark Breakthrough') results"*
- AlphaProof Nexus authors: *"Even most Erdős problems remain out of reach, let
  alone problems that require extensive new theory."*

| verdict | condition |
|---|---|
| PASS | plausibly resolvable with techniques that exist somewhere in the literature today |
| FAIL | the field's own experts describe it as awaiting a new framework |

## G2 — Does a verification standard exist?

If no one can say what would settle it, nothing can settle it.

| verdict | condition |
|---|---|
| PASS | there is an agreed form the answer would take, and a community or checker able to adjudicate |
| FAIL | the question is not yet sharp enough to have a wrong answer |

---

# Part 2 — Axes

Five axes, 0–5. Anchors are corpus cases, so the scale is calibrated against
observed events rather than intuition.

## Axis A — Certificate cost

*What must the skeptic actually do to be convinced?* Inverted so high = cheap.

| score | anchor |
|---|---|
| 5 | `jacobian-conjecture-c3` — one 3×3 symbolic determinant and three point evaluations. Minutes. |
| 4 | `erdos-728-factorial-divisibility` — Lean-checked, plus a short human writeup readable in an afternoon. |
| 3 | `gpt5pro-convex-optimization-bound` — self-contained argument in a mature area; Bubeck verified in **25 minutes**. |
| 2 | `erdos-90-unit-distance` — nine specialists produced a digest in days; requires class field theory to read. |
| 1 | `erdos-1196-primitive-sets` — 35 pages, 9 figures, new machinery to absorb, seven authors. Weeks to months. |
| 0 | no agreed certificate form |

**Worked example 1** — Erdős #90 scores 2, not 5, despite being a
*counterexample*. The model's exponent was inexplicit; there was no finite
object until Sawin supplied one. Score the certificate that exists.

**Worked example 2** — `axiom-fel-conjecture-disputed` scores 4 on this axis
and is still worthless, because Axis A measures cost-to-check, not value. The
frontier flag in `TAXONOMY.md` catches it; Axis A does not and should not.

## Axis B1 — Evaluator availability

*Is there an exact automatic checker for candidate **objects**?* Drives Route W.

| score | anchor |
|---|---|
| 5 | `alphaevolve-matmul-4x4` — exact symbolic evaluator that *is* the certificate checker; partial progress measurable |
| 4 | `funsearch-cap-set` — cheap evaluator over explicit finite objects |
| 3 | numeric evaluator with tolerance, or expensive exact check |
| 2 | `bolzano-eight-problems` — LLM verifier only; materially weaker guarantee |
| 1 | `erdos-1196-primitive-sets` — expert refereeing only |
| 0 | nothing |

## Axis B2 — Formal library coverage

*Does a proof assistant's library cover the **ambient theory**?* Drives Route F.

| score | anchor |
|---|---|
| 5 | ambient theory fully formalized, heavily used |
| 3 | `alphaproof-nexus-erdos-nine`, `erdos-728` — mathlib covers the ambient theory (Kummer, p-adic valuations); tasks decompose into subgoals |
| 2 | `jacobian-conjecture-c3` — commutative algebra present, this corner unformalized |
| 1 | `alphaevolve-math-constructions` — nothing formalized |
| 0 | the theory has no formal treatment at all |

**B1 and B2 were one axis in v1.0, and that was a bug.** AlphaEvolve has a
perfect evaluator (B1=5) and no formal coverage whatsoever (B2=1). The merged
axis scored it high and it wrongly cleared Route F — a route it never used.
Retrodiction caught this; see `RETRODICTION.md`.

The AlphaProof Nexus authors state the B2 scope condition directly: successes
concentrate *"where Lean's mathematics library is mature and tasks often
decompose into tractable subgoals."*

**Worked example** — the same problem scores differently by era. Cap sets scored
B1=4 in 2023 because someone wrote an evaluator. The mechanism did not change
between FunSearch (2023) and AlphaEvolve (2025); the set of problems with a
cheap evaluator did.

## Axis C — Reservoir proximity

*Is there unexploited machinery in an adjacent field?* The Gowers axis, and the
only one that predicts **frontier impact** rather than mere activity.

| score | anchor |
|---|---|
| 5 | `erdos-90-unit-distance` — 80 years of combinatorial attack; solution came from class field towers. Gowers: the ideas "were present in the literature already." |
| 4 | `erdos-1196-primitive-sets` — probabilistic frame "overlooked by the prior literature since Erdős's seminal 1935 paper" |
| 3 | `erdos-1026-monotone-subsequence` — square-packing results existed and applied, but a human had to find them |
| 1 | `erdos-728-factorial-divisibility` — Kummer's theorem (1852) is native to the field; no import |
| 0 | the field's own techniques are the only candidates and are exhausted |

Calibration from the corpus: `MIXED`-culture problems (stated in the
problem-solving culture, closed with another field's machinery) advance the
frontier **5 times out of 6**; pure problem-solving cases **7 of 13**.

**Worked example** — score the *reservoir*, not the *effort spent*. Erdős #90
had enormous prior expert attention and still scores 5, because the attention
was all from inside one field. Prior attention is weak evidence of
intractability; an empty reservoir is strong evidence.

## Axis D — Witness representability

*Can the answer be a finite object?* Gates Route W entirely.

| score | anchor |
|---|---|
| 5 | `alphaevolve-matmul-4x4`, `funsearch-cap-set`, `jacobian-conjecture-c3` — the answer *is* a finite object: a decomposition, a set in F₃ⁿ, three polynomials |
| 4 | `erdos-52-sum-product` — finite objects, but a family of them with a parameter |
| 3 | `sawin-unit-distance-explicit` — a finite description generating an infinite family |
| 1 | `erdos-90-unit-distance`, `erdos-1196` — the answer is an asymptotic statement about infinite families |
| 0 | the answer is a statement about all models of a theory |

**D measures witness finiteness ONLY, not search difficulty.** In v1.0 cap sets
scored 3 on the grounds that the space is "large and unstructured" — but that is
a fact about finding the witness, not about whether one exists in finite form. A
cap set is an explicit finite object; it scores 5. Conflating the two made the
axis unable to do its one job, which is gating Route W.

**Worked example — the press-coverage trap, scored.** Erdős #90 was a
*counterexample* and scores **1**, because the counterexample is an infinite
family and the model's exponent was inexplicit. Route W is correctly blocked;
Route R carries the case. Meanwhile the Jacobian counterexample scores 5. Both
are "counterexamples to an 80-year-old conjecture"; they are not remotely the
same object.

## Axis E — Decision status  *(rebuilt in v1.2; see below for what broke)*

*Is the answer a theorem of the governing axioms, or a statement about which
models exist?* Higher = safer for the routes.

| score | condition | survey occupant |
|---|---|---|
| 5 | ZFC theorem about countable objects; inside reverse-math range | `brd-dobrinen-6-1` |
| 4 | ZFC theorem about countable objects; ACA₀–ATR₀ region | `pc-erdos-592` |
| 3 | ZFC theorem expected, but objects are uncountable — reverse math cannot reach it | `pc-erdos-70` |
| 2 | ZFC status genuinely unknown; specialists split | `ch-erdos-501` |
| 1 | known independent, or known to need large cardinals; the live question is now consistency **strength** | `brd-uncountable` |
| 0 | the question **is** a consistency question | `cc-roitman` |

### What was wrong with v1.1, and why it matters

The v1.1 anchors graded proof-theoretic strength at 5–3 and *our epistemic
state* at 2–0. Those are not one axis. It is the same defect `TAXONOMY.md`
diagnosed in the flat four-mechanism scheme — a scale whose values are not
commensurable — appearing a second time in a different place.

It also had a **range** error. The Big Five live in second-order arithmetic,
which reaches only "structures that are either themselves countable, or which
can be represented by countable 'codes'"; even "the set ℝ of all real numbers is
a third-order object which is not directly represented in second-order
arithmetic" ([SEP, *Reverse Mathematics*](https://plato.stanford.edu/entries/reverse-mathematics/)).
Π¹₁-CA₀ is the top of the ladder and sits far below ZFC. Grading CH-neighbourhood
questions on it is a ruler with the right units and the wrong range.

**The rebuild was not a demotion.** Reverse mathematics turns out to be exactly
the right instrument for *part* of the target territory and structurally
inapplicable to the rest, and the axis now says which is which:

- Big Ramsey degrees of **countable** structures and partition calculus at
  **countable** ordinals are statements about countable objects. Anglès d'Auriac,
  Cholak, Dzhafarov, Monin and Patey computed reverse-mathematical strengths for
  Milliken, Halpern–Läuchli, Devlin and the Rado graph ([arXiv 2007.09739](https://arxiv.org/abs/2007.09739)).
- Cardinal characteristics, ℵ₁⁺ partition relations, and every consistency
  question are out of range entirely. **Consistency strength** grades those, and
  it moves: separating ten Cichoń values needed four strongly compact cardinals
  in 2017 and needed nothing beyond ZFC by 2022 ([Goldstern–Kellner–Mejía–Shelah,
  JEMS](https://ems.press/journals/jems/articles/3375044)). Under the v1.1
  anchors that five-year movement is invisible, because the whole episode happens
  above the top of the Big Five.

The corpus is unaffected — every Phase 1 case scores 4–5 under both readings, and
`scripts/score.py --retrodict` produces a byte-identical `RETRODICTION.md`. What
changed is that the sub-3 anchors are now grounded on observed set-theoretic
facts instead of constructed by analogy.

### What E is actually for

Its only use in this instrument is gating Route F. Phase 3 established what that
gate turns on, from the primary side — DeepMind's own Lean repo, in
`FormalConjectures/ErdosProblems/1175.lean`:

> Shelah's result is a *consistency* statement — it asserts the existence of a
> model of ZFC, not a ZFC theorem. Lean operates inside a single (fixed) model of
> its set theory, so we cannot directly express "consistent with ZFC" without
> leaving ZFC.

A proof assistant works inside one fixed model. A ZFC theorem is expressible; a
consistency statement is not. `E = 0` is therefore a structural bar, not a
difficulty rating — which is also the resolution of the forcing trap flagged in
`TAXONOMY.md`. See `memo/FINDINGS.md` §3.

---

# Part 3 — Routes

A problem is a `CANDIDATE` if it clears **any** route. Each is a mechanism
actually observed, with necessary conditions read off the cases that used it.

## Route W — Witness / refutation

> Exhibit a finite object that settles the question.

**Necessary:** `D ≥ 4` and `B ≥ 4` and polarity admits a finite witness.

**Corpus:** `jacobian-conjecture-c3`, `alphaevolve-math-constructions`,
`alphaevolve-matmul-4x4`, `funsearch-cap-set`

**The polarity constraint is absolute.** Proof by counterexample refutes a
universal by proving the negated existential, ¬∀x P(x) ≡ ∃x ¬P(x), and an
existential is proved by exhibiting one. You can refute a universal by example;
you can **never confirm one that way**. So this route — the cheapest in the
rubric — is structurally unavailable to any problem whose expected answer is
"the conjecture holds."

This explains why the 2026 frontier results are disproportionately destructive:
Jacobian (false), unit distance (false), sum-product (false). Certificate
economics, not model preference.

## Route F — Formal / mathlib

> Generate against a proof assistant until it compiles.

**Necessary:** `B ≥ 3` and `E ≥ 3` and the statement is formalizable without
ambiguity.

**Corpus:** `erdos-728-factorial-divisibility`, `alphaproof-nexus-erdos-nine`,
`erdos-397-formalization`, `alphaproof-imo-2024`

**Two residual risks Lean does not cover**, both live in the corpus:
statement-formalization error (silent, cascading), and provenance — see
`axiom-fel-conjecture-disputed`, where the proof machine-checked and the
mathematics was supplied by humans. Route F clearance means *checkable*, not
*valuable*.

## Route R — Reservoir / transfer

> Import machinery from a field that has not been asked.

**Necessary:** `C ≥ 4` and `A ≥ 2` and an expert community able to verify in
days-to-weeks.

**Corpus:** `erdos-90-unit-distance`, `erdos-1196-primitive-sets`,
`gemini-aletheia-autonomous-erdos`, `bolzano-eight-problems`

The highest-value route and the least mechanizable. Its output is often not a
finished result but an **opened route** — see `sawin-unit-distance-explicit`
(human supplies the explicit exponent within days) and
`erdos-52-sum-product-human-followup` (humans disprove a second major conjecture
with the same machinery, no AI involved).

**Consequence for how the rubric is read:** Route R clearance predicts *"an
AI-assisted community closes this faster than it otherwise would,"* not *"an AI
solves it."* On the corpus evidence that is the more common and more valuable
outcome, and it is a lower bar.

## Route N — Neglect / low prior attention

> The problem is open because nobody tried, not because it is hard.

**Necessary:** `prior_attention == LOW` and `A ≥ 3` and **G0 discharged by
actual literature work**, not merely asserted.

**Corpus:** `gemini-aletheia-autonomous-erdos`, `bolzano-eight-problems`,
`erdos-728-factorial-divisibility`, `alphaproof-nexus-erdos-nine`,
`erdos-397-formalization`

**This route was missing from v1.0 and retrodiction found it.** Three resolved
cases scored `RULED_OUT` under the original three routes: they had no reservoir
(C=3), no formal coverage (B2=1), and no finite witness. Yet they resolved —
because nobody had seriously attacked them. The Gemini screen's own diagnosis of
why problems sat open was *"obscurity rather than difficulty."*

It is now the **most-used route in the corpus** (5 of 14 resolved cases), which
is uncomfortable and probably correct: the modal AI mathematical success is on a
problem that was never really defended.

**Route N is one literature search away from being a G0 failure**, which is why
it demands a rigorous G0 rather than a nominal one. The distinction between
"nobody tried" and "somebody tried and published" is the entire difference
between `gemini-aletheia-autonomous-erdos` and `gpt5-erdos-claim-october-2025`.

**Phase 3 consequence, and it is a large one.** Every problem in the CH
neighbourhood has HIGH prior attention by construction — partition calculus and
cardinal characteristics have been worked continuously since the 1960s by people
who wanted these answers. Route N is closed for the entire target territory
before any other axis is scored.

---

# Part 4 — Validation status

**Position after Phase 4: the ruling-out half holds, the ruling-in half is
tested and does not work.** Full accounting in `memo/VALIDATION.md`.

A time-sliced hold-out — 56 open Erdős problems across 8 tags, scored blind
against a 2025-08-31 cutoff, scores committed before outcomes were read — found:

| claim | verdict |
|---|---|
| The rubric discriminates between fields rather than restating the prior | **holds** — 0%–57% across tags, from axis calls committed before the reveal |
| Axes D and B1 measure something stable | **holds** — the database's independent `verifiable`/`falsifiable` labels fall inside Route W 3 for 3, p = 0.00014 |
| `CANDIDATE` marks problems more likely to be resolved | **fails** — 7% vs 14% for `RULED_OUT`; on AI-assisted resolutions, 7.1% vs 7.1% |

**Read `CANDIDATE` accordingly.** It was already defined as a filter rather than
a prediction; Phase 4 removes the option of reading it as anything else. Report
`RULED_OUT` verdicts and the axis-by-axis argument. Do not report a
`CANDIDATE` list as a shortlist.

**Axis B2 is the diagnosed failure and its calibration is suspect.** Four
problems that then received machine-checked Lean proofs score `B2 = 2` under
every reading tested. B2 encodes the AlphaProof Nexus scope condition — successes
concentrate "where Lean's mathematics library is mature" — and the 2026 prover
agents that resolved those four were evidently not gated on it. The axis is also
the one the whole survey turns on: three defensible readings of its `B2 = 3`
anchor give 9%, 25% and 36% CANDIDATE on the same 56 problems.

Everything below was written before that test and is kept as the record of what
was known then.

**Honest position (pre-Phase 4): this rubric is not yet validated.**

| test | available? | why |
|---|---|---|
| Retrodiction on corpus | weak | derived from the same 20 cases; hindsight bias is unavoidable since I know each outcome |
| Discrimination on negatives | partial | 3 negative cases exist (`gpt5-erdos-claim-october-2025`, `axiom-fel-conjecture-disputed`, `gpt5pro-convex-optimization-bound`) and the rubric should decline all three for *different* reasons |
| Hold-out on failures | **impossible** | AlphaProof Nexus does not publish which 344 of 353 problems failed; the Gemini screen does not publish the 687. No labeled negatives exist anywhere in the literature. |

**The real test is Phase 3, and it is a falsification test.** If this rubric
scores a large fraction of CH-adjacent problems as `CANDIDATE`, the rubric is
wrong — because nothing in set theory has fallen to these methods, and a
correctly calibrated instrument must reproduce that. A rubric that nominates
half the survey has failed, regardless of how good its reasoning looks.

## Phase 3 result — the test was run

| population | CANDIDATE rate |
|---|---|
| Phase 1 corpus, resolved cases (retrodiction) | **13/14 = 93%** |
| Phase 3 survey, 24 CH-adjacent problems, strict | **0/24 = 0%** |
| Phase 3 survey, optimistic (every unassessed gate *granted*) | **2/24 = 8%** |

The instrument discriminates by roughly an order of magnitude between the
territory it was built on and the territory it was pointed at. It passes.

Three caveats that belong next to that number:

1. **8% is above the 0.7–2.5% base rate, and that is not a contradiction.** Those
   are different quantities in different regimes (Rule 4). 0.7–2.5% is a
   *resolution* rate under a near-exhaustive screen. 8% is a *filter-pass* rate
   on 24 hand-picked problems. `CANDIDATE` means "in the ~2% pool," so the base
   rate applies *to the survivors*, not instead of them.
2. **The optimistic column is the stronger test, not the friendlier one.** It
   hands the territory every gate it could not discharge and asks whether the
   *routes* still decline. They do, 22 times out of 24.
3. **Both survivors hang on a single axis call.** Each clears Route F alone, each
   via `B2 = 3`, and each drops to `RULED_OUT` at `B2 = 2`. Capping B2 at 2
   across the survey yields 0/24 under both readings.

Run `scripts/score.py --retrodict` for the corpus pass,
`scripts/score.py --survey survey/` for the strict survey, and
`--survey survey/ --optimistic` for the granted-gate form.
