# problems/ — the target pool

One file per problem: what it says, where the statement comes from, what a
resolving object would look like, what is already known, and what must be
checked before anyone spends effort on it. This folder is the actionable
output of the project; `CONCLUSIONS.md` is the argument for why it is built
this way.

## What this folder is not

It is not a prediction list. The project's own blind hold-out
(`memo/VALIDATION.md`) showed that its shortlist verdict did not predict which
problems resolve — 7% for shortlisted versus 14% for ruled-out, identical on
the AI-assisted subset. So no problem is here because a score says it will
fall. The prior that applies to everything below: **in the eleven months of
the hold-out window, 12.5% of the sample resolved, 7.1% with AI assistance.**
A pool with odds, not a prophecy.

## Entry criteria

A problem earns a file through one of three doors, in order of how much
evidence backs the door:

**W — finite certificate.** Resolving the problem (in at least one direction)
comes down to exhibiting a finite object — a graph, a set of congruences, a
list of integers — that a program can check exactly. This is the one
criterion that survived validation: when the rubric flagged this shape blind,
the Erdős database maintainers' independent `decidable` / `falsifiable` /
`verifiable` labels agreed 3 for 3 (p = 0.00014). Attempts are cheap and a
hit certifies itself. The database carries **43 open problems** with these
labels; they are the queue below.

**N — plausibly neglected.** The problem looks open because nobody senior
spent a month on it, not because it is hard — the corpus's most common
success mechanism, and the shape of the actual 2026 resolutions in the
hold-out (one was "a small modification of the argument of [ErNa89]").
Membership in this class **cannot be read off metadata**; it is established
per problem, by the G0 literature search in the process below. This door is
untested by the project — working the queue is the test.

**B — bound race with an exact evaluator.** Not "resolve the conjecture" but
"beat the best known construction," where the score of a candidate is
mechanically computable — the FunSearch cap-set / AlphaEvolve mechanism, plus
the GPT-5 Pro convex-optimization case for improving a constant in a recent
paper. This door is now open: DeepMind's live
[repository of problems](https://github.com/google-deepmind/alphaevolve_repository_of_problems)
ships 67 targets **with their verifiers included**, which solves the
record-verification problem by construction — 24 of them sit at
matched-but-not-known-optimal, the cleanest race position. The registry
survey, with maintenance status for every record table this project
verified, is [POOLS.md §3](POOLS.md). First seeded file in the class:
[superpermutation-n7](superpermutation-n7.md), dormant since 2019.

## The process — one problem at a time

Each problem file moves through four stages, recorded in its `status:` line:

1. **DEFINED** — statement transcribed from a primary source, certificate
   shape stated, known results listed from the statement source only.
2. **SEARCHED** — G0 discharged rigorously: an actual literature search for
   an existing resolution, recorded in the file with sources and dates. In
   the one near-exhaustive AI screen on record, 8 of 13 "hits" were
   already-published solutions; this stage exists so we never rediscover one.
   *This stage is also the experiment: it is the gate the Phase 4 validation
   could not afford to discharge 56 times, and discharging it here tests the
   neglect mechanism (Route N) the project never evaluated.*
3. **ASSESSED** — feasibility of the finite check, honestly. A finite
   certificate is not a feasible search: #307's solutions are two finite sets
   of primes, and a 2026 machine-checked barrier puts the smallest such sets
   at ≥ 59 primes with products ≥ 2·10⁵⁶. The file must say what the search
   space actually is and what known barriers exist.
4. **VERDICT** — `ATTEMPT` (worth pointing a system or a person at, with the
   mechanism named) or `PARK` (with the reason: attention too high, search
   infeasible, statement ambiguous...). Attempts happen **outside this
   repository** — this repo evaluates; results come back as data.

Sourcing rules are the repository's (`CLAUDE.md`): every claim carries a URL
and a tier; statements come from DeepMind's `formal-conjectures` Lean files at
a pinned commit (erdosproblems.com 403s automated fetch; its database YAML is
public and pinned in `sources/`); recall is not a source.

## Seeded files

Eleven Erdős problems from the labeled pool — chosen for label strength
(`decidable` > the one-sided labels), prize money (a proxy for the community
finding the problem real), Phase 4's blind Route W picks, and two problems
whose unresolved part is literally a **finite range** (#742, #848: proved for
all large n, open below) — plus one unlabeled problem the rubric picked blind
(#11), and two machine-generated graph conjectures as exemplars of the
lowest-attention class in reach.

Beyond Erdős (the 2026-07-26 sweep; sourcing and pool map in
[POOLS.md](POOLS.md)): the two problems the SAT community itself names as
next to fall, and four single-finite-object questions from the pinned
`formal-conjectures` warehouse, including one deliberate high-attention
control.

| file | one line |
|---|---|
| [erdos-0007](erdos-0007.md) | covering system with all moduli odd? |
| [erdos-0011](erdos-0011.md) | odd n = squarefree + power of 2? |
| [erdos-0064](erdos-0064.md) | min degree 3 ⇒ cycle of length 2^k? ($1000) |
| [erdos-0097](erdos-0097.md) | convex polygon vertex with no 4 equidistant vertices? ($100) |
| [erdos-0128](erdos-0128.md) | dense induced subgraphs force a triangle? ($250) |
| [erdos-0287](erdos-0287.md) | Egyptian fractions for 1 need a gap ≥ 3? |
| [erdos-0307](erdos-0307.md) | two prime sets with reciprocal-sum product 1? |
| [erdos-0617](erdos-0617.md) | Erdős–Gyárfás: unbalanced r-colourings of K_{r²+1}? |
| [erdos-0647](erdos-0647.md) | some n > 24 with max(m + τ(m)) ≤ n + 2? |
| [erdos-0742](erdos-0742.md) | Murty–Simon, open only on a finite range |
| [erdos-0835](erdos-0835.md) | Johnson graph J(2k,k) chromatic number k+1? |
| [erdos-0848](erdos-0848.md) | non-squarefree ab+1 sets, open only for small N |
| [wow2-059](wow2-059.md) | Graffiti.pc 59: induced forest vs residue (exemplar) |
| [wow2-061](wow2-061.md) | Graffiti.pc 61: induced forest vs residue + diameter (exemplar) |
| [ramsey-r3-10](ramsey-r3-10.md) | R(3,10) ∈ {40, 41} — the nearest open Ramsey number |
| [erdos-szekeres-g7](erdos-szekeres-g7.md) | ES(7) = 33? — successor to the empty-hexagon computation |
| [conway-99-graph](conway-99-graph.md) | SRG(99,14,1,2) existence ($1000; perfect certificate, hostile search) |
| [superpermutation-n7](superpermutation-n7.md) | L(7) ∈ [5884, 5906] — bound race dormant since 2019 |
| [rational-distance-unit-square](rational-distance-unit-square.md) | point at rational distance from all 4 corners (Guy D19) |
| [magic-square-of-squares](magic-square-of-squares.md) | 3×3 magic square of squares (high-attention control) |

## The queue — Erdős problems with community finite-certificate labels

All 43, from the database snapshot pinned 2026-07-25
(`sources/erdos-problems.yaml`, `teorth/erdosproblems`). "Statement file"
means `formal-conjectures` has a Lean statement to transcribe; **missing**
rows need their statement sourced from the literature first (the site 403s).

| # | label | prize | tags | statement file | here |
|---|---|---|---|---|---|
| 7 | verifiable | — | number theory, covering systems | yes | [erdos-0007](erdos-0007.md) |
| 19 | decidable | $500 | graph theory, chromatic number | **missing** | queue |
| 23 | falsifiable | — | graph theory | yes | queue |
| 64 | falsifiable | $1000 | graph theory, cycles | yes | [erdos-0064](erdos-0064.md) |
| 97 | falsifiable | $100 | geometry, distances, convex | yes | [erdos-0097](erdos-0097.md) |
| 106 | falsifiable | — | geometry | **missing** | queue |
| 107 | falsifiable | $500 | geometry, convex | yes | queue |
| 114 | falsifiable | $250 | polynomials, analysis | **missing** | queue |
| 128 | falsifiable | $250 | graph theory | yes | [erdos-0128](erdos-0128.md) |
| 167 | falsifiable | — | graph theory | **missing** | queue |
| 242 | falsifiable | — | number theory, unit fractions | yes | queue |
| 287 | falsifiable | — | number theory, unit fractions | yes | [erdos-0287](erdos-0287.md) |
| 307 | verifiable | — | number theory, unit fractions | yes | [erdos-0307](erdos-0307.md) |
| 364 | verifiable | — | number theory | yes | queue |
| 366 | verifiable | — | number theory | yes | queue |
| 375 | falsifiable | — | number theory | yes | queue |
| 398 | falsifiable | — | number theory, factorials | yes | queue |
| 458 | falsifiable | — | number theory, primes | yes | queue |
| 475 | decidable | — | number theory, additive combinatorics | **missing** | queue |
| 488 | falsifiable | — | number theory | yes | queue |
| 506 | decidable | — | geometry | **missing** | queue |
| 547 | decidable | — | graph theory, ramsey theory | **missing** | queue |
| 548 | falsifiable | — | graph theory | **missing** | queue |
| 551 | decidable | — | graph theory, ramsey theory | **missing** | queue |
| 556 | decidable | — | graph theory, ramsey theory | **missing** | queue |
| 580 | decidable | — | graph theory | **missing** | queue |
| 583 | falsifiable | — | graph theory | **missing** | queue |
| 617 | falsifiable | — | graph theory | yes | [erdos-0617](erdos-0617.md) |
| 628 | falsifiable | — | graph theory, chromatic number | **missing** | queue |
| 647 | verifiable | £25 | number theory | yes | [erdos-0647](erdos-0647.md) |
| 672 | verifiable | — | number theory | yes | queue |
| 699 | falsifiable | — | number theory, binomial coefficients | yes | queue |
| 723 | falsifiable | — | combinatorics | yes | queue |
| 742 | decidable | — | graph theory | yes | [erdos-0742](erdos-0742.md) |
| 743 | falsifiable | — | graph theory | **missing** | queue |
| 779 | falsifiable | — | number theory, primes | yes | queue |
| 835 | verifiable | — | graph theory, hypergraphs | yes | [erdos-0835](erdos-0835.md) |
| 848 | decidable | — | number theory | yes | [erdos-0848](erdos-0848.md) |
| 982 | falsifiable | — | geometry, convex, distances | yes | queue |
| 993 | falsifiable | — | graph theory | **missing** | queue |
| 1020 | falsifiable | — | graph theory, hypergraphs | **missing** | queue |
| 1041 | falsifiable | — | analysis | yes | queue |
| 1082 | falsifiable | — | geometry, distances | yes | queue |

**Plus #11** — no community label, but it was one of the four problems Route W
nominated blind in Phase 4 (the other three all turned out to carry community
labels), it has a 2^50 computational verification on record, and its file
explains why it belongs.

Beyond this table, the full pool map — every non-Erdős source this project
has located, with AI-resolution yields and denominators, record-table
maintenance status, and the pools already being harvested industrially
(Kourovka, OEIS) versus the ones still quiet — is **[POOLS.md](POOLS.md)**.
Three updates from the 2026-07-26 sweep: the Graffiti.pc pool is ten times
bigger than the formalized slice (~213–230 open per DeLaViña's archived
list, against DeepMind's 22 — POOLS.md §5); refutation pipelines began
sweeping the *adjacent* machine-conjecture corpora in June 2026, so wow2
G0 checks now include the Demonstrandum artifacts; and the pinned
statement warehouse proved stale on at least one problem (Claude's Cycles
fell five weeks after the pin; `corpus/knuth-claude-cycles.yaml`), so
DEFINED always includes a live status check.
