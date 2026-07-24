# Phase 1 corpus

20 cases of AI-assisted mathematical results, 2023-12 through 2026-07.
Schema in `SCHEMA.md`. Distribution in `DISTRIBUTION.md` (regenerate with
`./.venv/bin/python scripts/tabulate.py > corpus/DISTRIBUTION.md`).
What is deliberately *absent* is in `NEGATIVE-SPACE.md` — read that before Phase 3.

## Counting caveat

20 case files, but they are not 20 equal units. Four are **aggregates** covering
hundreds of underlying events:

| aggregate case | covers |
|---|---|
| `teorth-wiki-aggregate` | ~600 logged contributions, frozen 2026-06-30 |
| `gemini-erdos-survey-700` | 700 problems screened, 13 addressed |
| `alphaproof-nexus-erdos-nine` | 353 attempted, 9 + 44 OEIS solved |
| `bolzano-eight-problems` | 8 attempted, 6 publishable |

Where one technique closed several problems at once (#1196/#1217/#164;
#728/#729/#401), that is one case with a `companion_problems` list. Splitting
them would triple-count a single mechanism.

## Seven findings that should shape the rubric

**1. `SEARCH_PLUS_VERIFIER` dominates (40% primary, 50% appearing anywhere)
and it is the mechanism least portable to set theory.** Every instance requires
a cheap automated checker — Lean, a CAS, an evaluator function. The AlphaProof
Nexus authors say so directly: successes concentrate "where Lean's mathematics
library is mature and tasks often decompose into tractable subgoals."

**2. The four-mechanism vocabulary is missing its largest category.**
Formalization — taking a known human proof into Lean — is 2(b) on the teorth
wiki with **>200 of ~600 entries**, the single biggest bucket. It is not
recall, not construction, not novel argument, and only technically
search-plus-verifier. `axiom-fel-conjecture-disputed` is the case that breaks
on this. **Recommend adding `FORMALIZATION` as a fifth primary in Phase 2.**

**3. `LITERATURE_RECALL` at 20% is an undercount, and the corpus knows it.**
Sampling headline cases biases against retrieval, because retrieval does not
make headlines except when it is mistaken for discovery. The aggregate row
implies ~45% of all logged AI contributions (formalization + literature search)
involve no new mathematics at all. Weight the aggregate, not the case count.

**4. Certificate size is overwhelmingly SMALL (65%), and that is selection,
not capability.** Only one LARGE-certificate case exists (`erdos-1196`), and it
took seven mathematicians and 35 pages. Results survive into public view when
they are cheap to check.

**5. Zero cases of substantial new theory-building. Unanimous.** Gowers on
Erdős #90: "many of the ideas needed for the proof were present in the
literature already." Tao on #1026: "not particularly novel." DeepMind
explicitly declines to claim any Level 3 or Level 4 result. Greg's draft
disqualifying axis is the best-supported thing in the corpus before Phase 2
begins.

**6. Cross-field transfer looks like 25%, but it is really one event.** Three
of five transfer cases (`erdos-90`, `sawin-unit-distance-explicit`,
`erdos-52-sum-product`) draw on the *same* Golod-Shafarevich / class-field-tower
reservoir opened in May 2026. One discovery, then humans draining it fast — see
`erdos-52`, which has no AI involvement at all and disproved the sum-product
conjecture weeks later. **The unit of analysis may be the reservoir, not the
problem.**

**7. Success rates span two orders of magnitude and the driver is selection
freedom, not model quality.**

| study | rate | regime |
|---|---|---|
| Bolzano | 6/8 = 75% | problems chosen by researchers |
| AlphaProof Nexus | 9/353 = 2.5% | curated open Erdős list |
| Gemini screen | 5/700 = 0.7% | near-exhaustive over open database |

Phase 3 must declare which regime a score is in. Hand-picking for tractability
is legitimate — it is what the rubric is *for* — but the resulting number
cannot be compared to a screen.

## Known weaknesses of this corpus

- **`erdosproblems.com` returns 403 to automated fetches.** Every problem
  statement here is paraphrased from papers and commentary, not read from the
  database. Largest sourcing weakness in Phase 1.
- **1 of 20 cases is peer-reviewed** (FunSearch, *Nature* 2023). The rest are
  preprints, blogs, lab announcements, or social media. `jacobian-conjecture-c3`
  is four days old.
- **37 flagged unverifiable claims** across the corpus. They are recorded per
  case rather than dropped.
- **3 cases rest on no PRIMARY source** (both AlphaEvolve cases; the Axiom
  dispute). The Axiom case rests on a single adversarial source and its factual
  claims are marked ALLEGED.
- **1 THIN case** (`erdos-397-formalization`), retained rather than dropped to
  avoid survivorship bias; `--exclude-thin` recomputes without it.
- **The `field` taxonomy is messy** — 15 distinct values across 20 cases, with
  "Mixed" variants that should be normalized before any field-level statistics
  are trusted. Cosmetic, but do it before Phase 2 cross-tabs.

## The classification decision worth arguing about

Cases are classified by **mechanism that closed the problem**, not by **agent
that closed it**. `erdos-728` is `SEARCH_PLUS_VERIFIER` (the process) rather
than `NOVEL_ARGUMENT` (the product), even though the output is a genuinely new
short proof. `erdos-1026` is `LITERATURE_RECALL` even though four AI tools
touched it, because a human finding a 2024 paper is what actually closed it.

7 of 20 are flagged `contested: true`. If Greg prefers classification by product
rather than process, the distribution shifts materially — `NOVEL_ARGUMENT` rises
and `SEARCH_PLUS_VERIFIER` falls. That is a Phase 2 decision, and it is the
single largest fork in the schema.
