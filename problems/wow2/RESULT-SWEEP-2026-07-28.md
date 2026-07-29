# Sweep of the 21 open WOWII conjectures

Date: 2026-07-28

## Verdict

**No counterexample to any open conjecture.** Every one of the 21 open WOWII
statements this machinery can evaluate survived every lane. The pipeline that
produced this null was first measured against three conjectures whose
counterexamples are published, and it found all three
([`RESULT-CALIBRATION.md`](RESULT-CALIBRATION.md)) — including a new, smaller
one for conjecture 58. That is what makes this null worth reading.

DeLaViña's own ledger predicts it: 132 conjectures in the historical corpus fell
to counterexamples over the years, so the 2020 survivors are enriched for *true*
statements. A dry refutation sweep is the expected outcome, not a
disappointment.

## The pool

DeepMind's `formal-conjectures` carries 22 WOWII statements marked
`category research open` at HEAD (`f776d2f`, 2026-07-27) — the same 22 as at the
project's pin, so nothing moved in or out. Conjecture 19 is excluded here: a
preprint (preprints.org 202607.0114, 2 July 2026) claims a Lean-verified proof,
which the warehouse tag has not caught up with. The remaining 21 are registered.

## Exhaustive lane — every connected unlabeled graph through order 9

261,080 graphs per mode, all invariants exact.

| mode | statement | equality / applicable | witnesses |
|---|---|---:|---:|
| 2 | Ls ≥ 2(l_avg − 1) | 1 | 0 |
| 40 | f ≥ ⌈(path cover + b + 1)/2⌉ | 6,098 | 0 |
| 59 | f ≥ ⌈√(residue·b)⌉ | 12,336 | 0 |
| 61 | f ≥ residue + ⌈diam/3⌉ | 1,231 | 0 |
| 65 | f ≥ distMin(A) + ⌈distMin(M)/3⌉ | 0 | 0 |
| 100 | α ≤ ⌈(max l(v) + ½·degreeL2Norm(Ḡ))/2⌉ | 339 | 0 |
| 133 | path ≥ radius + ⌊l_avg⌋^[C4-free] | 245 | 0 |
| 141 | tree ≥ ⌊girth/2⌋ − 1 + max l(v) | 95 | 0 |
| 142 | tree ≥ ⅔·girth + eccSet(B) | 294 | 0 |
| 144 | tree ≥ girth − 1 + ecc(center) | 50 | 0 |
| 145 | 2·eccSet(B) ≤ tree · lMin(Ḡ) | 961 | 0 |
| 146 | 2·eccSet(B) ≤ tree · radius(G²) | 1,272 | 0 |
| 160 | Ls ≥ max l(v) + max T(v)·[C4-free] | 335 | 0 |
| 194 | α ≤ 1 + l_avg ⟹ Hamiltonian path | 98,994 | 0 |
| 198a | b ≤ 2 + ecc_avg ⟹ Hamiltonian path | 1,639 | 0 |
| 200 | tree = ⌈1 + l_avg⌉ ⟹ Hamiltonian path | 3,790 | 0 |
| 217 | Ls ≤ 4·[residue=2] + 2 ⟹ Hamiltonian path | 7,915 | 0 |
| 291 | γₜ ≤ Havel–Hakimi zero step + freq min T | 930 | 0 |
| 314 | triangle-free ∧ path ≤ 4 ⟹ well tot. dominated | 74 | 0 |
| 316 | avg deg(Ḡ) ≤ pendants ⟹ well tot. dominated | 12 | 0 |
| 322 | max l(v) ≤ 1 ⟹ well tot. dominated | 1 | 0 |

For inequalities the middle column counts the equality surface; for
implications it counts graphs satisfying the hypothesis. Mode 145 skipped
12,346 graphs and mode 100 skipped 13,588, each failing that statement's own
side condition (a positive lMin of the complement, a connected complement) —
those graphs lie outside the conjecture rather than surviving it.

Two columns worth pausing on. Mode 65 has an **empty equality surface**: the
bound is never tight anywhere at order 9, so it has slack everywhere and is the
least likely of these to be sharp. Mode 322's hypothesis holds on **one graph in
261,080**, which is what a filter that severe buys — it can be swept far deeper
for the same compute.

## Exhaustive lane — every connected unlabeled graph through order 10

Added 2026-07-29. The generator was capped at order 9, which is what had blocked
the "exhaust order 10" step [`RESULT-061.md`](RESULT-061.md) named as its next
material lane. Raising the cap reproduces McKay's published count exactly —
**11,716,571** connected unlabelled graphs — in 7 minutes 49 seconds, and every
intermediate order still matches.

**Zero witnesses on all 21 conjectures.** The column below is the equality
surface for inequalities and the satisfied hypothesis for implications, at each
order, with its share of that order's population.

| mode | order 9 | order 10 | order 9 share | order 10 share |
|---|---:|---:|---:|---:|
| 2 | 1 | 3 | 0.000% | 0.000% |
| 40 | 6,098 | 115,154 | 2.336% | 0.983% |
| 59 | 12,336 | 99,765 | 4.725% | 0.851% |
| 61 | 1,231 | 5,124 | 0.472% | 0.044% |
| 65 | 0 | 0 | 0.000% | 0.000% |
| 100 | 339 | 63 (skipped 288,597) | 0.130% | 0.001% |
| 133 | 245 | 782 | 0.094% | 0.007% |
| 141 | 95 | 332 | 0.036% | 0.003% |
| 142 | 294 | 916 | 0.113% | 0.008% |
| 144 | 50 | 70 | 0.019% | 0.001% |
| 145 | 961 | 7,799 (skipped 274,668) | 0.368% | 0.067% |
| 146 | 1,272 | 11,554 | 0.487% | 0.099% |
| 160 | 335 | 759 | 0.128% | 0.006% |
| 194 | 98,994 | 3,226,062 | 37.917% | 27.534% |
| 198a | 1,639 | 10,612 | 0.628% | 0.091% |
| 200 | 3,790 | 50,779 | 1.452% | 0.433% |
| 217 | 7,915 | 31 | 3.032% | 0.000% |
| 291 | 930 | 11,703 | 0.356% | 0.100% |
| 314 | 74 | 152 | 0.028% | 0.001% |
| 316 | 12 | 14 | 0.005% | 0.000% |
| 322 | 1 | 1 | 0.000% | 0.000% |

### Every constraint loosens with order, and that cuts against exhaustion

The share column falls for **all 21**. Generic graphs carry more slack at order
10 than at order 9; the bounds are not tightening as the order grows.

Read carelessly that is evidence the conjectures hold. Read against the
calibration data it says something more useful and less comforting. The
counterexamples in this pool are not generic — conjecture 58's is a cone, 109's
is a join, 103's is a corona, and each is a vanishing fraction of its order.
"Generic slack is increasing" and "a structured family tips the bound at c = 36"
are perfectly compatible, and both are true here.

So the exhaustive lane samples precisely the regime where these statements are
least stressed, and each further order costs far more for a thinner slice of
what matters: order 11 is about a billion graphs, 86 times order 10, and by this
trend its equality surface would be thinner still. **Exhaustion past order 10 is
not where the next counterexample in this pool comes from.** That is the same
conclusion the calibration reached from the other direction.

Two modes stand out at the edges. **Mode 65** still has an empty equality
surface at 11.7 million graphs, so its bound is nowhere tight and it is the
least likely of these to be sharp. **Mode 217**'s hypothesis fires on 31 graphs
in 11,716,571, down from 3% at order 9, and is heading toward vacuity — which
makes it a plausible target for a *proof* rather than a refutation, since a
hypothesis that restrictive may be characterizable outright.

## Parametric lane — blueprint families

14 blueprint families built from disjoint union, join, cone, and pendant
attachment, swept across their integer parameters. This is the lane that found
all three calibration counterexamples.

**Zero witnesses on all 21 open conjectures.** Order ceiling 30 for the modes
whose invariants are polynomial, 20 for those needing a Hamiltonian-path table
or a minimal-total-dominating-set enumeration; the ceiling is printed on every
summary line.

Families still climbing when the vertex budget ran out — reported so they are
not silently counted as survivors:

| mode | climbing families |
|---|---:|
| 2 | 5 |
| 133 | 5 |
| 40 | 2 |
| 59 | 2 |
| 141 | 2 |
| all others | 0 |

A climbing flag is a lower bound on where to look, not a claim that a
counterexample exists: the extrapolation is linear and these approaches are
typically concave, so a true crossing would land later than predicted, if it
exists at all. Conjecture 58's refuting family is exactly this shape, which is
why the flag is worth carrying.

## What this does not establish

The exhaustive lane stops at order 9 and this pool's known counterexamples
start at 11. The parametric lane reaches further but only along the shapes in
its catalogue. A counterexample that is neither small nor a member of a
join/cone/corona family would be missed by everything here.

The honest summary: 21 conjectures survived a search that is now measured to
find real counterexamples of three known kinds, at orders where two of the three
older lanes could not.

## Reproduction

```sh
make test                     # every self-test including the invariant oracle
make exhaustive-61            # any single mode over order 9
make parametric-61            # any single mode over the blueprint families
make calibrate                # the three conjectures with known answers
```
