# Conclusions

**For the reader who has not lived in this repository.** This page states what
the project found, in plain terms, and where the work goes next. Every claim
here is argued in detail in a memo; the reading map at the bottom says which.

---

## What this project did

Between 2023 and 2026, AI systems started resolving open mathematics problems —
some famous, most obscure. The reporting on these events is unreliable in both
directions: press releases overstate, corrections arrive quietly, and nobody
keeps score. This project kept score, then tried to turn the score into a
predictive instrument.

Concretely, in four phases:

1. Documented **20 cases** of AI systems attacking real problems, every claim
   sourced and tiered, every unverifiable claim listed as unverifiable
   (`corpus/`).
2. Built a **classification and scoring rubric** from those cases: what kind of
   certificate did the skeptic get, how was it produced, what had to be true
   about the problem for the attack to work (`rubric/`).
3. Applied it to **set theory and infinite combinatorics** — the neighborhood
   of the continuum hypothesis — asking whether any open problem there is a
   realistic target (`survey/`, `memo/FINDINGS.md`).
4. Ran the honest test: scored **56 open Erdős problems blind** against an
   eleven-month-old snapshot, froze the scores in git, then looked up what had
   actually happened to those problems since (`survey4/`,
   `memo/VALIDATION.md`).

## The four findings

**1. In set theory, there are no targets, and the reason is structural.**
Zero of 24 surveyed problems near the continuum hypothesis are realistic
candidates, and the argument does not rest on "AI isn't good enough yet." The
central obstacle: most questions in this territory are *consistency*
statements — "this cannot be decided from the standard axioms" — and a proof
assistant, which works inside one fixed model of those axioms, cannot even
state such a claim, let alone check it. That is DeepMind's own engineering
note, not our speculation. Add that candidate objects are infinite (nothing to
hand a checker), the field's own machinery is already exhausted by the people
who built it, and sixty years of expert attention, and every mechanical route
closes. A null result, argued axis by axis, was the expected outcome and is
the project's most solid deliverable.

**2. You cannot currently predict *which* problem will fall — and we proved
it on ourselves.** The rubric's shortlist function was tested the only honest
way: score blind, commit, wait, compare. Problems it shortlisted resolved at
7%; problems it ruled out resolved at 14%. On the AI-assisted resolutions
alone: 7.1% versus 7.1%, identical. The shortlist function is retired. Any
future list from this project that claims "these will be solved" would be
using an instrument its own validation killed.

**3. What you *can* identify is which problems are the right *shape* — and an
independent community agrees.** The one criterion that survived validation:
does resolving the problem come down to exhibiting a **finite object that a
program can check exactly**? (A counterexample graph. A set of congruences. A
list of integers.) When the rubric flagged problems with that shape, the Erdős
database maintainers' own labels — assigned in their vocabulary, with no
knowledge of the rubric — agreed 3 for 3, a coincidence with probability
0.00014. This is the defensible unit of prediction: not "this problem will
fall," but "this problem is the kind that falls, and an attempt is cheap
because a hit certifies itself."

**4. The ground is moving faster than the published record implies.** Of 56
ordinary open Erdős problems, **7 resolved in eleven months (12.5%), 4 of
them with AI assistance (7.1%)** — GPT-5.5 Pro, a DeepMind prover agent,
GPT-5.5 with Codex, Claude Opus 4.5 — on problems nobody selected for
tractability. And the constraint that governed 2023–2025 systems — success
concentrates where Lean's math library is mature — was violated by the 2026
resolutions in this very sample. The frontier's operating envelope changed
underneath our instrument during the experiment. Nobody currently has public
evidence for what gates these systems now.

## What "worth attempting" means now

The honest selection doctrine, given all four findings:

- **Pick by shape, not by score.** Finite certificate, exact checker — the one
  externally-corroborated criterion. Attempts on such problems are cheap and
  self-verifying in one direction.
- **Do the literature check first.** In the one near-exhaustive screen on
  record, 8 of 13 "hits" were solutions already in the literature that the
  problem list hadn't absorbed. Rule out "already answered" by actually
  searching, per problem, before spending anything.
- **Prefer neglect.** The corpus's most common success mechanism was a problem
  that was open because nobody senior had spent a month on it, not because it
  was hard. This mechanism is real in the case data and has never been
  properly tested by this project — testing it is the next experiment, and
  the problem folder below is its subject pool.
- **Carry the prior.** In eleven months, 12.5% of a stratified sample of
  ordinary open Erdős problems resolved; 7.1% with AI assistance. A candidate
  list is a pool with odds, not a prophecy. Anyone who quotes a problem from this project without that number
  is misquoting the project.

## Where the work goes next

**`problems/`** — the actionable output. One file per problem: the statement,
its sources, what the finite certificate would be, what is already known, and
what has to be checked before an attempt. Seeded with the strongest cases from
the 43 Erdős problems carrying the community's own finitely-checkable labels,
plus machine-generated graph conjectures (Graffiti.pc) that have the right
shape and the least prior attention. The folder's README states the entry
criteria and the one-problem-at-a-time process. Working the queue *is* the
next experiment: each worked problem discharges the literature-check gate the
validation could never afford, which tests the one mechanism (neglect) the
project never evaluated.

This repository evaluates; it does not attempt. If a problem graduates to an
actual attempt, that happens elsewhere, and the outcome — either way — comes
back here as data.

**Beyond Erdős** (added after the 2026-07-26 pool sweep;
`problems/POOLS.md` is the full map). The Erdős database was where the
labels were free, not where the problems end. The sweep found: the SAT
community's own named next targets (R(3,10) is a one-value gap; ES(7) is
the direct successor to a computation that already fell); a live DeepMind
registry of 67 bound-race problems shipped *with their verifiers*; and a
census of which non-Erdős pools actually produced AI resolutions in the
last eighteen months — Kourovka Notebook 9 (eight of them autonomous and
Lean-verified), OEIS 44 of 492 attempted, and roughly eight cases of the
single most productive mechanism on record: an individual expert bringing
their own working problem to a frontier model. The sharpest of those is
now a corpus case (`corpus/knuth-claude-cycles.yaml`): Knuth's torus
decomposition problem, marked open in our pinned statement source, had
fallen five weeks after the pin — Claude found the odd-case construction
in an hour, and the even case fell in March to a relay of models, ending
in a Lean formalization. Two lessons priced into the folder: target lists
in this territory have a shelf life measured in weeks, and the pools the
labs already harvest industrially (Kourovka, OEIS) are races — the edge
is in the pools their pipelines don't see.

## Reading map

| question | file |
|---|---|
| What were the 20 documented cases? | `corpus/`, one YAML each; `corpus/DISTRIBUTION.md` for the table |
| How does the classification work? | `rubric/TAXONOMY.md`, `rubric/RUBRIC.md` |
| Why is set theory a dead end? | `memo/FINDINGS.md` |
| How was the blind test run, and what failed? | `memo/PREREGISTRATION.md`, then `memo/VALIDATION.md` |
| What should we try to solve? | `problems/README.md` |
| Where do targets beyond Erdős come from? | `problems/POOLS.md` |
| How to work in this repo | `CLAUDE.md` |
