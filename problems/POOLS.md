# POOLS.md — where targets come from, beyond the Erdős queue

The doctrine (three doors, four stages) is in [README.md](README.md). This
file is the map of the territory the doors open onto: every pool of open
problems with the right shape that this project has located and verified,
with maintenance status and — where the record exists — AI-resolution
yield with denominators. Compiled 2026-07-26; every claim web-verified on
that date unless marked otherwise.

## 1. The statement warehouse: `formal-conjectures` (pinned @ `393aa9a`)

DeepMind's repo is the project's statement source for Erdős problems; it
also carries ten other directories. Files with at least one
`category research open` statement, counted from the pinned clone:

| directory | open files | character |
|---|---|---|
| Wikipedia | 122 | named conjectures — mostly maximum-attention celebrities (Collatz, Goldbach, ABC), with a certificate-shaped minority now seeded here |
| GreensOpenProblems | ~47 | Ben Green's 100-problems list — mostly asymptotic; one already fell to AlphaProof Nexus (see §2) |
| Paper | 24 | one-off conjectures from individual papers — where Claude's Cycles lived |
| OEIS | 22 files | Zhi-Wei Sun-style representability — see §2 for why this pool is now picked over |
| Mathoverflow | 11 | individual MO questions, formalized |
| Arxiv | 10 | conjectures tied to specific arXiv papers |
| Books | 7 | Bugeaud distribution-mod-one problems and similar |
| Kourovka | 2 | group theory — the repo barely samples it; the real notebook is ~1000 problems (see §2) |
| WrittenOnTheWallII | 22 conjectures | Graffiti.pc machine-generated graph inequalities — see §5 |

**The staleness lesson, learned this session.** `Paper/ClaudesCycles.lean`
at the pin says "the even case m > 2 remains open." Knuth's own note
(revised 2026-04-14) says the even case is no longer in doubt — it fell
in March, five weeks after the pin, to a relay of models
(`corpus/knuth-claude-cycles.yaml`). `Wikipedia/CasasAlvero.lean` carries
its own staleness note: a claimed proof (Ghosh, arXiv:2501.09272, revised
2026-03-21) has stood since January 2025. **A pinned `research open` tag
is a statement source, never a status source.** Every DEFINED file must
check status live, and the SEARCHED stage re-checks it.

## 2. Pools with verified AI resolutions, 2025-01 → 2026-07 (denominators attached)

The point of this table: it says where the mechanism actually strikes
outside the Erdős database, which is what pool selection should key on.
Sources: primary papers listed per row; census compiled this session.

| pool | yield | source |
|---|---|---|
| **Kourovka Notebook** (group theory, running since 1965) | 9 problems in the window — 1 with DeepMind's AI co-mathematician + Lackenby (arXiv:2605.10402), **8 autonomously by Harmonic's Aristotle, Lean-verified** (arXiv:2607.17477, 2026-07-20) | PRIMARY |
| **OEIS open conjectures** | **44 / 492** formalized conjectures proved by AlphaProof Nexus, manually reviewed (arXiv:2605.22763) — a ~9% regime, the largest denominator-honest non-Erdős screen | PRIMARY |
| **Green's 100 open problems** | 1 — Problem 57's intended form disproved via a ℤ/3ℤ counterexample (same Nexus paper, Green in the loop on intent) | PRIMARY |
| **COLT open problems** | 1 — learning-curve monotonicity for MLE (GPT-5.2 Pro; Sellke write-up) | PRIMARY |
| **Machine-conjecture corpora** (Graffiti, TxGraffiti, IRIS, solubilizer) | ~12 refutations + 1 proof — the Demonstrandum refutation pipeline plus a Grok agent run; artifacts public (github.com/demonstrandum-research/artifacts) | PRIMARY |
| **Individual researchers' working problems** (no list) | ~8 cases: Knuth (corpus case), Aaronson–Witteveen QMA, Carbery/Grok Lᵖ counterexample, Ivanisvili–Xie NICD, Ryu NAG point convergence, Nourdin fourth-moment, the Jacobian counterexample, Coester online algorithms | PRIMARY per case |
| **Bound races** (see §3) | ~20 record improvements by evolve-style systems | PRIMARY |

**Zero-yield pools in the same window:** Guy's UPINT as a cited list, AIM
problem lists, open MathOverflow questions, DARPA expMath (13 funded teams,
no results yet), and — the instructive null — Tao's Equational Theories
Project, where classical ATPs beat LLMs on cost for 22M implications.

**The strategic read.** Kourovka and OEIS are now being harvested
industrially by Lean-armed lab systems (Aristotle's Kourovka paper is six
days old at compilation time). Entering those pools means racing labs on
their home turf. The pools where a small operation has edge are the ones
the labs' pipelines don't see: individual working problems, dormant bound
races (superpermutations), and machine-conjecture corpora *before* the
refutation pipelines sweep them. This is Door N applied to pools instead
of problems.

**Cross-cutting monitor:** https://aimath.robertj1.com/ — a community
tracker of AI-resolution claims (245 entries at compilation, with
per-entry verification labels). Operator unidentified; treat as signal,
not record — but its labels matched primary sources on every spot-check
this session. Partially fills the census gap left when the teorth wiki
froze (2026-06-30); see `NEXT-SESSION.md` item 5.

## 3. Class B registries — bound races with exact evaluators

**The registry that opens the door:**
[google-deepmind/alphaevolve_repository_of_problems](https://github.com/google-deepmind/alphaevolve_repository_of_problems)
(Georgiev–Gómez-Serrano–Tao–Wagner, arXiv:2511.02864, live, PRs accepted).
67 problems, each shipped with **prompt, verification code, and current
best program** — the record-verification problem class B was blocked on is
solved by construction. Machine-readable `status.json` (read 2026-07-26):

| status | count | meaning for us |
|---|---|---|
| world_record | 19 | AlphaEvolve holds the record — beat-the-machine targets |
| former_record | 4 | already retaken by others — proof the races are alive |
| worse_than_record | 8 | AlphaEvolve failed to reach SOTA — the hard tier |
| matched, not known optimal | 24 | **the cleanest open targets** |
| matched_optimal | 12 | closed, no race |

Tao's caveats (his 2025-11-05 blog, EXPERT_COMMENTARY): evolve systems
game sloppy verifiers (float tolerance, clipping) and are weakest on
analytic number theory. Score with exact/interval arithmetic only.

**Authoritative record tables, with maintenance status:**

| race | table | state |
|---|---|---|
| Small Ramsey numbers | Radziszowski DS1, **rev. #18, 2026-04-24** | live; already absorbed 9 AlphaEvolve lower bounds ("NaRT", arXiv:2603.09172) — LLM records entering the field's canonical table |
| Sorting networks (size n=13–16, depth n=17+) | bertdobbelaere.github.io/sorting_networks.html | updated 2025-11-07 |
| Packing (squares, circles) | Friedman's index (updated 2026-07-03); squares-in-squares handed to Ellsworth (kingbird.myphotos.cc, Feb 2026); Packomania (updated 2026-07-25) | all live, active amateur+pro community |
| Kissing numbers | cohn.mit.edu/kissing-numbers | live; dim 11: AlphaEvolve's 593 (May 2025) beaten by AI agents at 604 (arXiv:2606.10402, June 2026) — AI racing AI |
| Cages | DS16 (stale, 2013); live front is arXiv:2511.07247 (11 records, rev. 2026-07-22) + House of Graphs | split between stale survey and live papers |
| Degree-diameter | combinatoricswiki.org (**TLS-broken to automated fetch**); Wikipedia mirror dated June 2024 | maintenance unverifiable this session |
| Superpermutations | Egan's page (2019) | **dormant 7 years** — seeded, [superpermutation-n7](superpermutation-n7.md) |

Verified LLM/agent bound movements in the window, for calibration: the 9
Ramsey lower bounds (Mar 2026); kissing dim 11 604 (Jun 2026); Steiner
ratio certified 0.824 → 0.8559, first movement in ~30 years
(arXiv:2601.22365); TTT-Discover retaking two AlphaEvolve records at ~$300
per problem (arXiv:2601.16175); GPT-5 Pro's 1.5/L convex-optimization
bound — correct, verified, and behind the already-published 1.75/L
(`corpus/gpt5pro-convex-optimization-bound.yaml`).

## 4. The SAT frontier (the Heule program)

Settled precedents 2016–2024: Boolean Pythagorean triples, Schur five,
Keller dim 7, packing chromatic 15, empty hexagon. The community's own
named next targets (Li–Duggan–Bright–Ganesh, IJCAI 2025, PRIMARY):
**R(3,10)** ([seeded](ramsey-r3-10.md)), **R(4,6)**, R(5,5). The natural
successor by machinery is **ES(7) = 33** ([seeded](erdos-szekeres-g7.md)).
Heule's own named marquee targets (talk abstracts 2024–2025, PRIMARY):
Hadwiger–Nelson, optimal 3×3 matrix multiplication, Collatz — all open,
none moved by SAT since 2018.

Parked by the community's own verdicts: **W(2,7)** ("not computable at
this time, and perhaps never" — the authors of the last two exact values);
**BB(6)** (formally classified Hard: requires deciding Antihydra, a
Collatz-like problem; current bound BB(6) > 2↑↑↑5); **MOLS(10) triple**
(pair-based strategies exhausted, arXiv:2503.10504).

Pattern worth carrying: 2024–2026 the class's center of gravity moved from
*resolving* to *certifying* (R(3,8)/R(3,9) certificates, Keller and empty
hexagon formalized). Genuinely new SAT resolutions are getting smaller.

## 5. Machine-generated conjecture corpora

**Graffiti.pc "Written on the Wall II" is ten times bigger than the
formalized slice.** DeLaViña's own status page (archived; her UHD server
is dead as of 2026-07-26 — Wayback only) counts **~213–230 open** of ~500
numbered conjectures, frozen since April 2020. DeepMind formalized 22 —
all 22 appear in her open list, so the codings agree; the selection is
just ~11% of the pool. Two seeded here as [wow2-059](wow2-059.md) /
[wow2-061](wow2-061.md). Authoritative archived sources:
menu/counts — https://web.archive.org/web/20240726224805/http://cms.dt.uh.edu/faculty/delavinae/research/wowII/menu.htm (PRIMARY);
full statused list — https://web.archive.org/web/20260723161837/http://cms.dt.uh.edu/faculty/delavinae/research/wowII/all.html (PRIMARY).
One calibration from her own ledger: 132 of the historical corpus fell to
counterexamples over the years — the 2020 survivors are enriched for
*true* statements, so a refutation sweep may come up dry and the likelier
resolution mode is proof.

**Attention has reached WOWII proper.** A preprint submitted 1 July and posted
2 July 2026 claims a source-scoped, Lean-verified proof of WOWII 19:
Z. Chen, Q. Wang, Y. Feng, ["A Source-Scoped Lean-Verified Proof of WOWII
Conjecture 19"](https://www.preprints.org/manuscript/202607.0114) (PRIMARY;
not peer reviewed). The authors pin the Lean artifact and explicitly disclaim
a broader literature-priority claim. One proof does not establish a class-wide
sweep. Demonstrandum and at least one Grok agent run are also
refuting conjectures from the *adjacent* Fajtlowicz Graffiti and TxGraffiti
corpora with machine-checkable artifacts (§2). The SEARCHED stage for any
wow2 file must check the Demonstrandum artifacts repo, the tracker (§2), new
WOWII-specific work, and the archived DeLaViña list.

**TxGraffiti** (Davila): the flagship open set is the four conjectures in
the ten-year retrospective (arXiv:2507.17780); one fell to a *human* in
June 2026 (arXiv:2606.29553), three remain, and the paper explicitly
invites AI attempts — the low-attention window is closing. The system
itself is live (PyPI `txgraffiti` v0.4.1, 2026-01) but publishes no
canonical open-conjecture registry. Fajtlowicz's original Graffiti list
was never publicly hosted; Larson–Van Cleemput's `conjecturing` is dormant
(last push 2023, domain dead).

## 5b. Design existence — one sweet spot, and the parked celebrities

**The sweet spot: open instances from the Handbook of Combinatorial
Designs.** Rosin's CPro1 protocol (arXiv:2501.17725, arXiv:2505.23881)
had LLMs generate search programs for open Handbook instances and solved
previously-open instances in **7 of 16 design types tested** (weighing
matrices, packing arrays, Bhaskar Rao designs, a nested Steiner quadruple
system, and more). That is direct calibration: Handbook-scale open design
instances sit inside the AI-search feasibility envelope, the certificates
are by definition finite, per-instance attention is near zero — and ~9 of
16 types resisted, so the pool is not picked clean. The natural next
seeding pass for this folder draws from the resistant types' smallest
open instances.

**Parked, with reasons on the record (all verified 2026-07-26):**

| problem | why parked |
|---|---|
| Projective plane of order 12 | smallest undecided order; 2023 result caps any collineation group at order ≤ 3, i.e. no symmetry left to exploit; order 10 took two decades of computation (SAT program stopped there) |
| Hadamard matrix of order 668 | smallest open order since 2005; a 2026 simulated-annealing campaign over a 10³⁷-per-sequence space got nowhere (ulam.ai report); *and* it now has active AI-community attention (Epoch FrontierMath open-problems list) — fails feasibility and neglect both |
| Lonely Runner, next case | recent cases k = 8 through 12 fell to specialized human machinery in eight months (Rosenfeld 2025; Sungkawichai–Trakulthongchai 2026) — an actively harvested vein; Tao's finite reduction is n^{O(n²)}, astronomically infeasible naively |
| Frankl union-closed | not finite-certificate in either direction (no known reduction; verified only to 12-element universes); the constant race has been stalled at ≈ 0.38 since mid-2023 — wrong shape for this folder |

## 6. Fetch traps found this session

Extending the `CLAUDE.md` table: `combinatoricswiki.org` — TLS certificate
failure; `multimagie.com` — TLS failure; OEIS pages — 403 to automated
fetch; `houseofgraphs.org` — serves an empty SPA shell to fetch;
`web.archive.org` — blocked; large arXiv PDFs (>10 MB) — fetch fails, use
the saved-file + `Read` route (that route successfully read Knuth's PDF
this session).
