# Negative space: what the corpus does NOT contain

A corpus of successes is a biased instrument. This file records the searched-for
and not-found, because for Phase 3 the absences carry more information than the
hits.

## 1. AI in set theory: retrieval and formalization, no discovery

**REWRITTEN 2026-07-25 (Phase 3). The Phase 1 version of this section claimed
"the category is empty." That was wrong, and the way it was wrong is worth
keeping.**

Phase 1 searched for the *words* — "set theory", "forcing", "cardinal
characteristics" — across the AI-mathematics literature and the teorth wiki, found
nothing, and concluded the category was empty. The words genuinely do not appear.
But the wiki indexes problems **by number**, not by name. Cross-referencing
set-theoretic Erdős problem numbers against the same frozen file finds eleven
entries that the keyword search walked straight past.

**Method (Phase 3, reproducible).** `git clone https://github.com/teorth/erdosproblems.wiki.git`
— last commit is literally `freeze`, dated 2026-06-30. Parse the tables in
`AI-contributions-to-Erdős-problems.md`: **538 rows, 405 distinct problems.** Then
match against the set-theoretic and infinitary-combinatorics problem numbers,
which were obtained from DeepMind's
[formal-conjectures](https://github.com/google-deepmind/formal-conjectures) repo
(AMS class 03, plus files using `ℵ_ 1` / `ω_ 1` / `chromaticCardinal` /
`cardinalPartitionRel`) because `erdosproblems.com` still returns 403.

### What is actually there

| section | problems | outcome |
|---|---|---|
| 1(a) standalone, 1(c) building on literature, 1(d) with humans | #75 ×2, #501, #593, #598, #623 | 2 🟡 partial, 4 ⚪ **unverified**, **0 🟢** |
| 2(a) literature search | #591 ×2, #602, #965 | 3 🟢, 1 🟡 |
| 2(b) formalization | #1067 | 🟢 — of Bowler–Pitz (2024), by Aleph Prover + Aristotle |

**Every green is retrieval or translation. Not one primary contribution in this
territory reached verified status.** For comparison, primary contributions
wiki-wide run **90 green / 256 rows = 35%**.

Two further AI-touching results in the territory, neither on the wiki:

- Erdős #1067's Lean formalization, attributed in DeepMind's repo to "Alexeev
  using Aristotle and Aleph Prover" — a translation of a known human proof.
- A Lean 4 port of **Flypitch** (CH independence), by Ian Klatzco,
  [2026-05-11](https://klatz.co/blog/flypitch/), "over the course of approximately
  a week, mostly unattended Claude." A port of a 2019 formalization of a 1963
  theorem.

### How much weight this carries

**0/6 against a 35% base rate is p ≈ 0.075 on a one-sided binomial. That is
suggestive and not significant**, and it should never be quoted alone. Six rows
cannot establish a field-level claim.

What it does do is *corroborate* a structural argument that stands on its own
(`memo/FINDINGS.md`): consistency statements are not expressible in any proof
assistant's fixed model, so Route F is structurally barred; no evaluator exists
for uncountable objects, so Route W is barred; and prior attention is HIGH by
construction, so Route N is barred. The wiki data is consistent with that and
would have been the first place a counterexample showed up.

The corroboration from the positive side still holds:

> Successes are concentrated in areas such as combinatorics, convex
> optimization, and number theory, where Lean's mathematics library is mature
> and tasks often decompose into tractable subgoals.
> — AlphaProof Nexus, [arXiv 2605.22763](https://arxiv.org/abs/2605.22763)

### The replacement claim

Not *"nobody has tried."* Named humans pointed frontier models at these problems
repeatedly between January and June 2026 — Sungchul Lee at #501 and #623, Eric Li
at #593, Przemek Chojecki at #598 — and the four candidate results they produced
were **all still unverified when the census froze**. The correct statement is:

> AI has been applied to this territory, and has produced literature retrieval,
> formalization of known proofs, and unverified claims. It has produced no
> checkable new result.

That the candidates sat unverified is itself evidence for Axis A. Nobody could
cheaply check them.

### Followed up 2026-07-25: none of the four were ever accepted

Checked against the live [`teorth/erdosproblems`](https://github.com/teorth/erdosproblems)
database (`scripts/erdos.py --status 501 593 598 623`). **All four still `open`,
all four still carrying the database's seed `last_update` of 2025-08-31** — while
the maintainers touched 470 other problems in the same period, including
promoting #591 to `proved`, #1067 to `disproved (Lean)`, and #1119 to
`independent`.

Two of the four were logged as candidate *full solutions*. Neither produced a
single edit. The correct reading is not "unverified pending review" but
**"nobody checked, because checking is a research project."** Which is Axis A,
measured.

### Method lesson worth reusing

A keyword search over a corpus indexed by identifier will return a false
negative and look like a clean one. Search by the corpus's own index, not by the
vocabulary of the thing you're looking for.

## 2. No case where the AI built substantial new theory

Every positive case resolves by one of: an explicit witness, a short argument in
existing technique, retrieval, or search against a checker. None involved
inventing a framework.

Gowers, on the strongest case in the corpus (Erdős #90):

> many of the ideas needed for the proof were present in the literature already

Tao, on Erdős #1026, said the proof "turned out to not be particularly novel."
DeepMind explicitly declined to claim any Level 3 or Level 4 result. The
labs' own calibrated statements and the outside experts agree on this point,
which is unusual in this literature and therefore worth weighting heavily.

Greg's draft axis "dependence on large novel theory-building (disqualifying)"
is **confirmed by unanimous evidence** — not a single counterexample in the
corpus. It is the best-supported axis before Phase 2 even starts.

## 3. No published failure denominators from the labs

Only three sources in the corpus state what was attempted rather than what
succeeded: AlphaProof Nexus (9/353, 44/492), the Gemini screen (13/700), and
Bolzano (6/8). Every other case is a success reported without a denominator.

The teorth wiki is the only record with an explicit "incorrect" category.
It was frozen 2026-06-30 and no community census now runs.

## 4. Not retrieved in this pass — known gaps, not claimed absences

- The OpenAI primary announcement page for the Erdős #90 result (only the
  CDN-hosted PDF of the nine-author remarks paper was located).
- `erdosproblems.com` problem pages — the site returns **403** to automated
  fetches, so every problem statement in this corpus is paraphrased from
  papers and commentary rather than read from the database. This is the
  single largest sourcing weakness in Phase 1.
  **Partly fixed in Phase 3:** DeepMind's
  [formal-conjectures](https://github.com/google-deepmind/formal-conjectures)
  repo carries Lean formalizations of ~850 conjectures including most of the
  set-theoretic Erdős problems, each with a statement-level reference block and
  an explicit `research open` / `research solved` marker. It is a PRIMARY-tier
  route around the 403 and every Phase 3 survey statement is sourced from it.
  It does not fix Phase 1's corpus, which was written before this was found.
- The Hacker News Jacobian thread (**HTTP 429**).
- arXiv 2606.03419, "Optimizing Explicit Unit-Distance Lower-Bound Certificates."
- The AlphaEvolve paper itself (results taken from IEEE Spectrum and R&D World).
- Axiom AI's response to the Wegner critique, if one exists.
- The 2024 AlphaProof IMO primary announcement.

## 5. Structural gap: nothing between "days old" and "refereed"

Exactly one case in the corpus is peer-reviewed: FunSearch (Nature, 2023).
Everything from the 2025–26 wave is preprint, blog post, lab announcement, or
social media. The Jacobian result is four days old at corpus time.

The corpus therefore cannot distinguish "verified" from "not yet falsified."
Where it says RESOLVED, read: *no expert has publicly objected, and the people
best positioned to object are frequently co-authors.*
