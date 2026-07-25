# Findings: the CH neighbourhood is not tractable, and the reason is structural

**Phase 3 memo. 2026-07-25.**

24 open problems from partition calculus, big Ramsey degrees, cardinal
characteristics, and CH-adjacent combinatorics, scored against `rubric/RUBRIC.md`.

**Result: 0 of 24 score `CANDIDATE`. Granting every gate the survey could not
discharge, 2 of 24 — and both of those hang on a single axis call that, moved by
one point, takes the survey to 0 under both readings.**

This is the expected outcome and it is a complete result. What follows is the
axis-by-axis argument for why, which matters more than the number.

---

## 0. The headline, with its denominators

| population | CANDIDATE rate | regime |
|---|---|---|
| Phase 1 corpus, resolved cases | 13/14 = **93%** | retrodiction on cases the rubric was built from |
| Phase 3 survey, strict | 0/24 = **0%** | hand-picked, gates honestly assessed |
| Phase 3 survey, optimistic | 2/24 = **8%** | same, every unassessed gate *granted* |

The instrument separates the territory it was built on from the territory it was
pointed at by roughly an order of magnitude. That is the falsification test, and
the rubric passes it.

**Read the 8% correctly.** It is higher than the 0.7–2.5% autonomous-resolution
rate in the friendliest field, and that is not a contradiction — they are
different quantities. 0.7–2.5% is how often a *screen* resolves a problem. 8% is
how often a *filter* passes one, on 24 problems chosen by hand for this survey.
`CANDIDATE` means "in the ~2% pool worth spending on," so the base rate applies to
the two survivors, not instead of them. Selection freedom here is high — this is
the Bolzano regime, not the Gemini-screen regime — which makes 8% a *ceiling* on
what a real screen of this territory would yield.

**The optimistic column is the stronger test, not the softer one.** It hands the
territory every gate the survey could not discharge and asks whether the routes
still decline. They do, 22 times in 24. When a null result survives that, the
routes did the work, not the gates.

---

## 1. Route by route

Four routes exist. Three are barred for the entire territory, structurally, and
the fourth is barred for all but two problems.

### Route W — witness/refutation: **0 clearances. Barred by B1, universally.**

Route W needs `D ≥ 4` (the answer can be a finite object) **and** `B1 ≥ 4` (an
exact automatic checker exists for candidate objects).

**`B1 = 0` or `1` for all 24 problems.** No exceptions, and the reason is not
contingent. Every candidate object in this territory is infinite: a colouring of
`[𝔠]³`, a graph on ℵ₂ vertices, a maximal almost disjoint family, a non-principal
ultrafilter, a forcing poset. There is nothing to hand an evaluator. The Phase 1
corpus's most favourable case, `alphaevolve-matmul-4x4`, has an exact symbolic
evaluator that *is* the certificate checker; nothing here is within reach of that
mechanism, because the mechanism requires finite input.

The interesting confirmation is `ch-erdos-918`, which scores **D = 3** — the
highest witness-representability in the survey, matching the
`sawin-unit-distance-explicit` anchor, "a finite description generating an
infinite family." Erdős–Hajnal–Shelah constructions really are given by finite
recipes over ordinals. Route W blocks anyway, on `B1 = 0`: verifying a proposed
graph means checking a claim about *every* ℵ₁-sized subgraph. The best witness in
the territory paired with no way to check it is the cleanest possible vindication
of splitting B into B1 and B2 in v1.1.

**A trap worth recording.** `cc-brendle-01-ppoint` — "assume CH and add random
reals; is there a P-point?" — is `form: EXISTENTIAL`. It *looks* witnessable. The
P-point is a non-principal ultrafilter on ω living in a forcing extension; it has
no finite presentation and `D = 0`. This is the `erdos-90` error in a new costume:
scoring the certificate the statement's logical form suggests rather than the one
that exists.

### Route F — formal/mathlib: **2 clearances. Barred by a fixed-model limit.**

This is where Phase 3 did its real work, because Route F was scored near-zero from
indirect evidence and the whole territory's Route F prospects hung on it.

**The answer is not "mathlib doesn't have forcing yet." It is that consistency
statements cannot be expressed in a proof assistant at all.** From DeepMind's own
Lean repository, in `FormalConjectures/ErdosProblems/1175.lean`:

> Shelah's result is a *consistency* statement — it asserts the existence of a
> model of ZFC, not a ZFC theorem. Lean operates inside a single (fixed) model of
> its set theory, so we cannot directly express "consistent with ZFC" without
> leaving ZFC.

The same repo, in `1119.lean`, records the Kumar–Shelah and Schilhan–Weinert model
constructions in **prose, outside any theorem**, noting that "consistency
statements about models of ZFC are not directly formalizable inside Lean's fixed
model."

That is the people who would have to build Route F, saying it cannot be built for
this class of question. `E = 0` is a structural bar, not a difficulty rating.

**What does exist, precisely.** The Phase 3 investigation found more than expected,
and the shape of it is the finding:

| system | forcing / independence | partition calculus |
|---|---|---|
| **mathlib4** (Lean) | **none.** `Mathlib/SetTheory/` = `Cardinal`, `Descriptive`, `Lists`, `Ordinal`, `ZFC`. Code search: 0 hits for forcing/poset/generic-filter, 0 for `ContinuumHypothesis`; the sole "Cohen forcing" hit is a line in `docs/references.bib` | none in mathlib; `cardinalPartitionRel` and `chromaticCardinal` were defined ad hoc by formal-conjectures |
| **Flypitch** (Lean, standalone) | full CH independence, both directions ([ITP 2019](https://arxiv.org/abs/1904.10570), [CPP 2020](https://arxiv.org/abs/2102.02901)). **Never merged into mathlib.** Ported to Lean 4 [May 2026](https://klatz.co/blog/flypitch/) | — |
| **Isabelle/ZF** | [`Independence_CH`](https://www.isa-afp.org/entries/Independence_CH.html), 37 theory files, ctm approach | — |
| **Isabelle/HOL** (`ZFC_in_HOL`) | — | [`Ordinal_Partitions`](https://www.isa-afp.org/entries/Ordinal_Partitions.html): Larson, Specker, Erdős–Milner; plus Nash-Williams and Wetzel's problem |

**The Isabelle/ZF forcing development does not generalise, and its authors say
so.** From [arXiv 2210.15609](https://arxiv.org/abs/2210.15609) §8–9: the Δ-system
lemma was proved "only for ℵ₁-sized families; thus limiting us to the case of the
ℵ₁-chain condition"; "the present infrastructure does not allow class forcing out
of the box"; general Cohen posets `Fn_κ(I,J)` are listed under *future work*.
Every new independence result would need preservation theorems that are exactly
what is missing.

Three numbers on cost, all from the same source:

- The mathematics of the ZFC+¬CH model "was already in place by the end of
  November 2020, it was only **9 months** later that we were able to finish the
  formalization of that result. The missing pieces were essentially bureaucracy."
- `forces(·0 ∈ 1·)` prints at **nearly 20k symbols**. `forces(·¬·¬·0 ∈ 1···)`
  cannot be printed at all.
- "no current technology allows us to write a reasonably complex (and correct)
  theorem statement in a computer and expect to obtain a proof after hitting
  'Enter'."

Nine months of bureaucracy on top of already-finished mathematics, for a theorem
from 1963, with objects that blow up past printability. Generate-against-a-checker
does not survive contact with this.

**The one place Route F is alive.** `ch-erdos-1119-CONTROL` shows the line inside a
single file. Erdős's 1964 theorems — "if 𝔠 > ℵ₁ then every such family is
countable," "under CH there is an uncountable one" — are **ZFC theorems about a CH
hypothesis**, and Paulson formalized exactly that class of statement in
Isabelle/HOL as Wetzel's problem. The Kumar–Shelah and Schilhan–Weinert model
constructions are consistency statements and live in a comment.

**CH-equivalences are formalizable. CH-independence is not.** That distinction —
not the presence or absence of a forcing library — is what B2 measures in this
territory, and it is the only reason two problems survive.

### Route R — reservoir/transfer: **0 clearances. Barred by A, and by G1 where C is highest.**

Route R needs `C ≥ 4` **and** `A ≥ 2`. It is the highest-value route in the corpus
and the one a reader would expect set theory to clear, because the field
constantly imports machinery.

It fails, and the failure is instructive rather than incidental.

**Where C is low, the reason is that the reservoir is already drained.** Cardinal
characteristics scores `C = 1–2` across the board. The machinery reached for —
templates, matrix iterations, Boolean ultrapowers, creature forcing, ultrapowers of
partial orders, large cardinals — is native to the subfield and already deployed by
the people posing the problems. That is close to the `C = 0` anchor: "the field's
own techniques are the only candidates and are exhausted."

**Where C is highest, G1 kills it.** `brd-dobrinen-6-2` scores **C = 5**, the only
5 in the survey, and it is the `erdos-90` shape stated outright: a body of results
proved by forcing, where the request is to reprove them combinatorially, and where
the transfer has already succeeded once (Hubička, triangle-free case). Then
Dobrinen's own next sentence: *"new methods will be needed for k-clique-free
homogeneous graphs for k ≥ 4."*

That is the field's leading expert saying the problem awaits a new framework —
the G1 disqualifying condition, stated by the person best positioned to know.
**The most reservoir-rich problem in the territory is disqualified by the axis the
Phase 1 corpus found unanimous.** No case in 20 built new theory; this one requires
it.

The same pattern recurs in `cc-brendle-12`, where Brendle sketches the approach —
a three-dimensional matrix iteration — and names the obstacle: "we do not know how
to show the completeness of all the embeddings in this three-dimensional setting."
The machinery being reached for is not another field's. It is a hypothetical
extension of the subfield's own tool, one dimension up. **That is theory-building,
not transfer, and Route R is not what it needs.**

### Route N — neglect: **0 clearances. Closed before the survey started.**

`RUBRIC.md` already anticipated this: every problem in the CH neighbourhood has
HIGH prior attention by construction. Partition calculus and cardinal
characteristics have been worked continuously since the 1960s by Erdős, Hajnal,
Rado, Shelah, Todorcevic, Brendle, Mejía and others who wanted these answers.

The survey confirms it empirically rather than assuming it. The four unverified
candidate results on the teorth wiki (#501, #593, #598, #623) are all from
January–June 2026, all human-plus-model collaborations with named humans. These
problems are not neglected. They are attacked and not closed.

---

## 2. Axis A is the quiet killer, and it has receipts

`certificate_cost` appears as a blocking condition on **all 24 problems** — more
than any other. Route R needs `A ≥ 2`, Route N needs `A ≥ 3`, and the cardinal
characteristics slice sits at `A = 1` throughout.

`A = 1` is normally a soft judgement. Here it is measured, three times, on experts:

1. **Mildenberger, "A solution to Roitman's problem"** ([arXiv 1404.7343](https://arxiv.org/abs/1404.7343)).
   Posted 2014-04-29. **Withdrawn 2015-02-20**, comment: "Lemma 3.2 is flawed." A
   claimed solution by a specialist stood publicly for roughly ten months.
2. **Goldstern–Kellner–Mejía–Shelah on Cichoń's maximum with the evasion number.**
   Per Yamazoe's footnote ([arXiv 2401.14600](https://arxiv.org/html/2401.14600v1),
   p.8): "[GKMS21] stated that they proved the same separation result, but later
   found a gap in their proof." Four authors, one of them Shelah. The theorem was
   then proved by someone else with a different method.
3. **Cohen's P-point claim** in the standard random model — per Brendle, "his
   argument is flawed (as pointed out by Guzmán and Hrušák)."

Three independent expert-level errors surviving publication in one subfield. `A = 1`
is not pessimism; it is the observed rate.

**This is also why the four AI candidate results sat unverified.** Two of them
(#593, #623) are logged as "candidate full solution." Nobody checked them in the
weeks before the census froze — not because nobody cared, but because in this
subfield checking is a research project. A model that can generate a plausible
forcing argument in this territory has produced something the community cannot
cheaply refute *or* confirm, which is the worst of both.

---

## 3. The forcing trap: resolved, and worse than flagged

`TAXONOMY.md` flagged consistency questions as "witness-*shaped* but verifying like
an informal proof — cheap to exhibit, expensive to check," and asked Phase 3 to
determine whether that reading is right and whether a fifth route is needed.

**The reading is wrong in the direction that simplifies the analysis.** It is not
that the certificate is cheap to exhibit and expensive to check. It is that **the
certificate is not expressible in the checker at all** — see the DeepMind quote in
§1. A forcing notion has no representation inside a proof assistant's fixed model,
so there is nothing to exhibit *to a checker*, cheaply or otherwise.

**No fifth route is needed.** A fifth route would have been the wrong repair: it
would have modelled "expensive verification," which the existing axes already
handle through A. The actual situation is a hard bar on Route F for a
syntactically identifiable class of question, which the rebuilt Axis E now
encodes as `E = 0`.

This single move explains the cardinal-characteristics slice completely. Seven
problems, `E = 0` on all seven, Route F barred on all seven, `B1 = 0` bars Route W,
`C ≤ 2` bars Route R, HIGH attention bars Route N. Nothing left.

---

## 4. The two survivors, and why they are not recommendations

Under the optimistic reading, two problems clear — both on Route F alone, both via
`B2 = 3`, both dropping to `RULED_OUT` at `B2 = 2`.

**`pc-erdos-592`** — which countable ordinals β satisfy ω^β → (ω^β, 3)²?
The ambient theory is genuinely formalized: Larson, Specker and Erdős–Milner are
all in Isabelle/HOL's `Ordinal_Partitions`. Countable ordinals are countable
objects, so `E = 4` and reverse mathematics actually applies. **The caveat is
load-bearing: this is B2 = 3 in Isabelle/HOL, not Lean.** Every AI system in the
Phase 1 corpus that used Route F used Lean. The one library covering this ambient
theory is not the one the machines are pointed at.

**`ch-erdos-1068`** — does every graph of chromatic number ℵ₁ contain a countable
infinitely-connected subgraph? `B2 = 3` here rests on a demonstration rather than
an inference: the immediately neighbouring problem **#1067** — same
`chromaticCardinal` machinery, same `InfinitelyConnected` predicate — was actually
formalized in Lean, by Alexeev using Aristotle and Aleph Prover, in January 2026.
And #1067's resolution moved from a consistency result (Komjáth 2013) to a ZFC
construction (Soukup 2015) to "a simpler elementary example" (Bowler–Pitz 2024) —
a trajectory toward a short checkable certificate.

Both facts are about **#1067**, not #1068. The #1067 formalization is a
`TRANSLATION` of a known human proof, logged in wiki section 2(b) with prior
literature green. Nothing in the record shows an AI system *finding* an argument
in this area.

**Neither of these is a prediction, and reporting them as a top-3 would misuse the
instrument.** `CANDIDATE` means "not ruled out by any of four mechanisms" against
a 1–3% prior. Two problems survived a filter; that is all. There is no third, and
manufacturing one to fill the slot would be exactly the failure mode `CLAUDE.md`
warns about.

---

## 5. What this says about the rubric itself

**It works, with one axis doing suspiciously much.** Route W, Route R and Route N
close for the whole territory on independent grounds — no evaluator, drained
reservoir, high attention — and those closures are robust to any single axis
call. Route F carries the entire remaining signal, and within Route F, B2 carries
it alone. The survey's whole outcome moves on two judgements I made about how well
Isabelle/HOL and Lean cover two specific corners.

That is a fragility worth naming rather than smoothing. It is also the natural
place to attack this memo.

**Axis E needed rebuilding and now discriminates.** The v1.1 version graded
proof-theoretic strength at 5–3 and epistemic state at 2–0 — two incommensurable
things on one scale, the same defect `TAXONOMY.md` diagnosed once already. It also
had a range error: the Big Five live in second-order arithmetic and top out at
Π¹₁-CA₀, far below ZFC.

The rebuilt axis spreads across the survey from 5 to 0, and the spread is
meaningful: `brd-dobrinen-6-1` (big Ramsey degrees of *countable* structures)
scores 5 and sits inside reverse-math range, where Anglès d'Auriac–Cholak–
Dzhafarov–Monin–Patey have actually computed strengths; `brd-uncountable` — the
same subject at uncountable cardinals — scores 1, because Shelah's Halpern–Läuchli
at κ assumes a measurable. **The same subject, five bands apart, split by the
cardinality of its objects.** An axis that produces that spread is measuring
something.

**What did not get tested.** `EVIDENCE` as a certificate type is still unoccupied.
`ABDUCTION` is still untested — and this territory, unlike the Erdős corpus, is
where Zahavy's thesis could in principle be tested, since new axioms are exactly
what some of these questions want. Nothing in the survey settles it.

---

## 6. The reframing question: reservoirs, not problems

Phase 2 left open whether the valuable question is *"which problem, if closed,
would reveal a reservoir for its neighbourhood"* rather than *"which problem can a
model close"* — motivated by Erdős #90 opening the class-field-tower reservoir that
humans then drained to disprove the sum-product conjecture weeks later, with no AI
involved.

**The survey has a clean answer for this territory, and it is the same answer.**

The reservoir question is `brd-dobrinen-6-2`: replace forcing proofs with purely
combinatorial ones. It scores `C = 5`. It has a precedent — Hubička's parameter-space
proof for the triangle-free graph. If it fell for k ≥ 4, it would do exactly what
the reframing hopes for: convert a body of independence-flavoured results into
combinatorics, and in doing so move a whole class of problems from `E = 0` and
`B2 = 0` into territory where a proof assistant could reach them. **It is the
single highest-leverage question in the survey and it is not close.**

It is also the one Dobrinen says needs new methods. `G1` fails, the rubric declines
it, and I think the rubric is right to — but this is the row where a reader should
push hardest, because a rubric calibrated on "no case built new theory" will always
decline the problem whose value is that it would build some.

**The honest summary of the reframing:** it identifies the right target and does
not change the verdict. A lower bar is still a bar, and G1 is above it.

---

## 7. What would change this conclusion

Stated concretely, so a reader can check whether it has happened:

1. **A proof assistant gains a general forcing framework** in which a *new*
   consistency result — not a re-formalization of Cohen or Solovay — is proved.
   The Isabelle/ZF authors' own future-work list (general `Fn_κ(I,J)`, class
   forcing, minimal Separation for forcing) is the roadmap. This would move `E = 0`
   from a structural bar to a coverage gap.
2. **`Ordinal_Partitions` gets a Lean equivalent inside mathlib**, putting the one
   well-covered ambient theory in this territory where the systems actually work.
3. **Dobrinen 6.2 falls for some k ≥ 4** — combinatorial replacements for forcing
   proofs — which would raise B2 and lower E across the big Ramsey slice at once.
4. ~~**Any of the four unverified 2026 wiki candidates gets verified.**~~
   **CHECKED 2026-07-25 — none did.** See §8.

---

## 8. Follow-up, 2026-07-25: the four claims, and an authoritative source

The memo's own falsification test — §7 item 4 — was run the same day. **All four
unverified 2026 AI candidate results in this territory are still open, and none
of them moved anything.**

The check became possible because the `erdosproblems.com` 403 has a workaround
that Phase 1 and the first Phase 3 pass both missed. The site is closed to
automated fetch; the **database behind it is a public YAML file** in
[`teorth/erdosproblems`](https://github.com/teorth/erdosproblems), 1217 problems,
actively maintained, carrying per-problem `status.state`, `status.last_update`,
`tags` and `formalized.state`. Pull it with `scripts/erdos.py --fetch`.

| problem | claim on the wiki | database status | `last_update` |
|---|---|---|---|
| #501 | Sungchul Lee / GPT-5.5 Pro, 29 May–1 Jun 2026, conditional partial | **open** | 2025-08-31 |
| #593 | Eric Li / GPT, 23 Jun 2026, candidate **full solution** | **open** | 2025-08-31 |
| #598 | Chojecki / Aristotle + GPT-5.4 Pro, 22 Apr 2026, partial | **open** | 2025-08-31 |
| #623 | Sungchul Lee / GPT-5.5 Pro, 4 Jun 2026, candidate **full solution** | **open** | 2025-08-31 |

**The `last_update` column is the finding, not the status column.** All four
still carry the database's seed date. The maintainers are demonstrably active —
they updated #591 to `proved` on 2026-01-17, #1067 to `disproved (Lean)` on
2026-02-02, #1119 to `independent` on 2025-12-30, #1167 and #1175 on 2026-01-23,
and touched 470 problems since the seed. They just never touched these four.
Two claimed *full solutions*, one to a $500 problem, produced no edit at all.

That is the difference between "unverified" and "ignored," and it is the
strongest single piece of evidence in this memo for Axis A. A community that
updates its database weekly did not update it for two claimed full solutions,
because checking them is a research project and nobody has done it.

### Independent corroboration of two axis calls

The database also carries community-assigned labels that map onto rubric axes,
assigned by mathematicians with no knowledge of this rubric. Run
`scripts/erdos.py --corroborate`.

**Axis E.** Ten problems in 1217 carry an independence label (`independent`,
`not provable`, `not disprovable`). **Eight of the ten** are tagged set theory,
ramsey theory, or chromatic number — in or immediately beside the surveyed
territory. Axis E is measuring a property that really is concentrated in one
place, and the concentration was recorded by someone else.

**Route W, weakly.** The database labels problems `decidable`, `falsifiable` or
`verifiable` when a finite computation would settle them — which is precisely
Route W's requirement. Set theory carries **0 of 35**. But the share is low
everywhere (graph theory 6.5% is the maximum among fields with n ≥ 25), so the
expected count for 35 problems is about one. **This is consistent with the Route
W closure and does not independently establish it**; sidon sets, divisors,
arithmetic progressions and additive basis are also at zero. Cite it as
corroboration, never as evidence on its own.

### What this does not fix

The database gives status, dates and tags. **It does not give problem
statements.** Those still come from DeepMind's Lean files or the literature, so
the sourcing chain for what each problem *says* is unchanged. And "open" in this
database is still the *list* status Bloom described, not a statement about
humanity's knowledge — so G0 remains discharged nominally, not rigorously, on
every row.

---

## Sources and their weaknesses

Every survey row carries source links with tiers and a per-row
`unverifiable_claims` block. The recurring weaknesses, stated once:

- **`erdosproblems.com` still 403s.** All Erdős statements are taken from
  DeepMind's `formal-conjectures` Lean files (repo HEAD `7587c62`, 2026-07-25),
  which carry statement-level reference blocks. Better than Phase 1's paraphrase
  from commentary, still not the database.
- **G0 is discharged nominally, not rigorously, everywhere.**
  `g0_checked_rigorously` is `false` on all 24 rows. No specialist was asked. For
  the 11 problems marked `open_to_field: true`, that status rests on a curated
  repo's `research open` marker plus its reference block.
- **The Brendle problem list is from 2017 and the Dobrinen list from 2021.** Both
  are demonstrably stale — Brendle's Problem 13 was overtaken by Cichoń's maximum,
  and Balko et al. (2025) closed much of Dobrinen 6.1. Those seven rows carry
  `open_to_field: null` for that reason, and the survey reports them as
  UNDETERMINED rather than guessing.
- **17 of 24 rows have `no_new_theory_required: null`.** Whether a partition
  relation open since 1971 needs new theory is not something the public record
  answers. The gate returning UNDETERMINED is the gate working, not a gap in the
  survey.
- **`brd-uncountable` scores an area, not a problem.** Dobrinen characterises the
  field rather than posing a numbered question there. It is the weakest row and
  should be read as such.
- **Nothing here is peer-reviewed except the sources.** The survey is a scoring
  exercise; its axis calls are judgements by one analyst against written anchors,
  and the two survivors turn on two of them.
