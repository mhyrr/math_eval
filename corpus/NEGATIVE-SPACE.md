# Negative space: what the corpus does NOT contain

A corpus of successes is a biased instrument. This file records the searched-for
and not-found, because for Phase 3 the absences carry more information than the
hits.

## 1. No AI result in set theory, forcing, or cardinal characteristics

Searched (2026-07-24), no result found:

- `"set theory" OR "cardinal characteristics" OR "cardinal invariants" AI LLM proof assistant open problem solved 2026`
- `AI large language model result set theory forcing infinite combinatorics 2026`

Nothing surfaced. Not a partial result, not a formalization of a known theorem
presented as progress, not a disputed claim. **The category is empty.**

This is a soft negative — absence of evidence from two searches is not proof —
but it is corroborated from the positive side by explicit scope statements in
the primary sources:

> Successes are concentrated in areas such as combinatorics, convex
> optimization, and number theory, where Lean's mathematics library is mature
> and tasks often decompose into tractable subgoals.
> — AlphaProof Nexus, [arXiv 2605.22763](https://arxiv.org/abs/2605.22763)

Set theory is named in neither the successes nor the near-misses of any lab
report retrieved. Every one of the ~20 corpus cases sits in number theory,
combinatorics, discrete geometry, optimization, or algebra.

**Before Phase 3 concludes anything, this negative should be hardened**: search
the frozen teorth wiki tables directly for set-theoretic problem numbers, and
check whether mathlib has meaningful forcing/independence infrastructure at all.
Both are cheap and neither was done in this pass.

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
