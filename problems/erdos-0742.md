# Erdős #742 — Murty–Simon, a finite range too large to enumerate

status: VERDICT — PARK (Füredi's effective threshold is a tower of 2s
of height about \(10^{14}\); current exact computation does not reach the
first open order)
class: W — community label `decidable` (the strongest label: a finite
computation settles it either way)

| | |
|---|---|
| problem page | https://www.erdosproblems.com/742 (human-readable; 403s automation) |
| statement from | `FormalConjectures/ErdosProblems/742.lean` @ `393aa9a` — PRIMARY |
| database status | `decidable`, last_update 2025-08-31 (`sources/erdos-problems.yaml`, pinned 2026-07-25) |
| prize | — |
| tags | graph theory |

## Statement

> **Murty–Simon conjecture.** Let G be a graph on n vertices with diameter 2
> such that deleting any edge increases the diameter. Then G has at most
> ⌊n²/4⌋ edges, with equality conjectured for the balanced complete
> bipartite graph.

## G0 verdict

The finite-range description is logically correct and computationally
misleading. Füredi's proof is effective, but the published upper bound for
its threshold \(n_0\) is no larger than a tower of 2s of height about
\(10^{14}\). Write \(T(1)=2\) and \(T(k+1)=2^{T(k)}\): the result applies
above roughly \(T(10^{14})\), not above a large ordinary integer.

Fan proved the edge bound for \(n\le 24\) and for \(n=26\). Thus the
unresolved orders are \(n=25\) and \(27\le n\le n_0\), not one modest
contiguous interval. A 2014 paper states both the tower-height bound and that
the conjecture remained open at every other order; work from 2019, 2024, and
2025 still describes the full conjecture as open.

**Verdict: PARK.** Exhausting the interval is not a credible attempt. Reopen
only for:

1. a structural proof avoiding the Ruzsa–Szemerédi/regularity bottleneck; or
2. a complete re-quantification that lowers \(n_0\) to the range of exact
   graph search.

## Why modern computation does not bridge it

Kirchweger and Szeider's 2024 SAT-modulo-symmetries computation verified the
conjecture only through \(n=19\). Its aggregate runtime grew from 19.9 hours
at \(n=17\), to 3.4 days at \(n=18\), to 23.7 days at \(n=19\). This is useful
solver work, but it does not move Fan's 1987 mathematical frontier at
\(n=24\) and \(n=26\). The first open case is still \(n=25\).

The certificate shape remains excellent in one direction: a counterexample
is a single graph whose diameter, edge-criticality, and edge count are
quickly checked. Proving the conjecture across the remaining finite interval
requires an infeasible sequence of exhaustive nonexistence certificates.

## Could better removal bounds rescue the threshold?

Füredi's proof uses the Ruzsa–Szemerédi \((6,3)\) theorem. Fox's 2011 graph
removal proof improved the relevant general dependency from a tower whose
height is polynomial in \(1/\varepsilon\) to one whose height is
\(O(\log(1/\varepsilon))\). That can drastically improve the historical
\(10^{14}\)-level height in principle.

It does not produce a practical Murty–Simon cutoff in the literature. The
2024 diameter-critical paper still describes the available quantitative
\((6,3)\) bounds as tower-type and explicitly motivates a
regularity-free proof. No source found in this G0 pass carries Fox's constants
through Füredi's full argument to a new numerical \(n_0\). Treating that
possible improvement as a finite-search plan would be inventing the missing
calculation.

## Sources checked

- [Füredi, *The maximum number of edges in a minimal graph of diameter 2*
  (1992)](https://doi.org/10.1002/jgt.3190160110) — theorem for
  sufficiently large \(n\); PRIMARY. The full text was paywalled in this
  pass, so the quantitative threshold below is corroborated from later
  peer-reviewed papers.
- [Haynes, Henning, van der Merwe, and Yeo, *A maximum degree theorem for
  diameter-2-critical graphs*
  (2014)](https://doi.org/10.2478/s11533-014-0449-3) — states Fan's exact
  finite range, Füredi's tower-height-\(10^{14}\) threshold, and the
  remaining open range; PRIMARY.
- [Dailly, Foucaud, and Hansberg, *Strengthening the Murty–Simon conjecture
  on diameter 2 critical graphs*
  (2019)](https://arxiv.org/abs/1812.08420) — independently repeats the
  tower-height estimate and surveys proved structural subclasses; PRIMARY.
- [Kirchweger and Szeider, *SAT Modulo Symmetries for Graph Generation and
  Enumeration*
  (2024)](https://repositum.tuwien.at/handle/20.500.12708/208555) — exact
  verification through \(n=19\), including runtimes; PRIMARY.
- [Wang, Zhang, and Zhu, *Improved bound on the number of edges of
  diameter-\(k\)-critical graphs*
  (2024)](https://arxiv.org/abs/2409.17491) — says the Murty–Simon
  conjecture remains open and the current quantitative \((6,3)\) bounds are
  tower-type; PRIMARY.
- [Lin and Wang, *Characterize all \(C_5\)-free diameter-2-critical graphs
  with at least \(\lfloor(n-1)^2/4\rfloor+1\) edges*
  (2025)](https://doi.org/10.1016/j.dam.2025.06.025) — recent partial
  structural result still treating Murty–Simon as longstanding; PRIMARY.
- [Fox, *A new proof of the graph removal lemma*
  (2011)](https://annals.math.princeton.edu/2011/174-1/p17) and
  [Fox–Zhao, *Removal lemmas and approximate homomorphisms*
  (2022)](https://doi.org/10.1017/S0963548321000572) — modern quantitative
  context for the only plausible threshold-improvement caveat; PRIMARY.
- [Erdős Problems #742](https://www.erdosproblems.com/742) — current
  `DECIDABLE` status and summary; EXPERT_COMMENTARY.
