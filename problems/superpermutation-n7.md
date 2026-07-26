# Minimal superpermutation, n = 7 — a 22-symbol gap, dormant since 2019

status: DEFINED
class: B — bound race with a trivial exact evaluator

| | |
|---|---|
| record page | Greg Egan, *Superpermutations* — https://www.gregegan.net/SCIENCE/Superpermutations/Superpermutations.html — PRIMARY (hosts the record strings; last revised 2019-06-13) |
| overview | https://en.wikipedia.org/wiki/Superpermutation — EXPERT_COMMENTARY |
| tags | combinatorics of words, permutations |

## Statement

> Find the shortest string over 7 symbols containing all 7! = 5040
> permutations as contiguous substrings.

Current bounds: **5884 ≤ L(7) ≤ 5906**. The upper bound is Egan's
construction (2019-02-27, building on Houston's method); the lower bound is
the anonymous-4chan argument as written up by Houston–Pantone–Vatter
(2018). Neither bound has moved since February 2019.

## The certificate

A candidate string is checked by a linear scan for all 5040 permutations —
the cheapest evaluator in this folder. An improved construction is
self-certifying. Improving the *lower* bound is a proof, not a certificate.

## Known, from the record

- n = 6 is also unresolved: 872 (Houston 2014) is best known and not proven
  minimal — a smaller instance of the same race.
- The 2018–2019 flurry (4chan lower bound, Egan's constructions) was the
  last movement. Seven years of silence on a problem with a 22-symbol gap
  and a trivial evaluator.
- Fetch trap: OEIS A180632 returns 403 to automated fetch; use Egan's page.

## Why it is in this folder

The strongest *neglect* case in the bound-race class: an exact evaluator,
a narrow gap, an amateur-accessible statement, and no recorded activity
since before the entire LLM-search era. FunSearch, AlphaEvolve, and their
successors have published record tables in adjacent territory; nothing on
record says any of them was ever pointed here. That absence is the
experiment.

## Before an attempt

- **G0 not discharged.** The SEARCHED stage must check for post-2019
  improvements outside Egan's page (arXiv, the superpermutators community,
  OEIS via browser) before trusting the 5906 figure.
- **Feasibility:** the search space is unbounded but the race is
  incremental — any string in [5884, 5905] wins. The ASSESSED stage should
  reconstruct Egan's construction family and ask what an evolve-style
  search would mutate.
