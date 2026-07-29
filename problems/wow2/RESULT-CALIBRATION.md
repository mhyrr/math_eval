# Calibrating the sweeper against conjectures with known answers

Date: 2026-07-28

## Why this exists

The 2026-07-26 attempt on WOWII 61 exhausted every connected graph through
order 9, built 360,267 structured graphs through order 20, and ran about 7.5
million edge toggles. It found nothing, and the write-up said so. What it could
not say was whether "nothing" meant the conjecture is true or the search was
pointed the wrong way.

That question has an answer, because three WOWII conjectures were refuted
upstream during 2026-06 and 2026-07 and their counterexamples are published.
Registering them as modes turns the pipeline on problems whose answers are
already known. Whatever it fails to find is a measurement of the search.

| conjecture | published counterexample | order |
|---|---|---:|
| 58 | K(3,3) with one vertex coned over K(73) | 79 |
| 103 | a triangle with four leaves on each of two vertices | 11 |
| 109 | an empty graph on 7 joined to two disjoint triangles | 13 |

Every counterexample in this pool sits **above** order 9 and every one is an
algebraic family with an integer parameter pushed until the inequality tips.

## What each lane found

| lane | 58 | 103 | 109 |
|---|---|---|---|
| exhaustive, connected unlabeled through order 9 | out of range | out of range | out of range |
| path-of-gadgets catalogue, 360,267 graphs through order 20 | miss | miss | miss |
| edge-toggle search, 16 million evaluations at the counterexample's own order | miss | miss | miss |
| **parametric blueprint families** | **hit** | **hit** | **hit** |

The first row is arithmetic: the exhaustive lane stops at 9 and the
counterexamples start at 11. The second and third rows are the finding.

### The edge-toggle lane was stuck on a plateau

Run against conjecture 109 at order 13 — the order at which a counterexample was
already known to exist — the search produced **two improvements in 16 million
evaluations** and never left phi = 0.

The cause is that almost every WOWII inequality ends in a floor, a ceiling, or
an integer square root, so phi is integer valued. The search reaches the
equality surface in a few hundred toggles and from there every neighbour scores
identically. Conjecture 61 had a hand-written smoother score for exactly this
reason; generalising the registry reintroduced the problem everywhere else.

The fix is to hand the search the statement *before* its rounding step
(`set_inequality_smooth` in `conjectures.c`). With the gradient restored the
same run reaches independence number 7 and induced bipartite size 9 — the
counterexample's own values — missing only residue 2 against 3. It still does
not find the graph. A hill climber over single edge toggles does not build a
join.

### The family catalogue had the wrong shape

`families.py` generates paths of gadgets: chains of cliques, cycles and
bicliques joined end to end. None of the three counterexamples has that shape.
They are a join, a corona, and a cone. 360,267 records could not contain any of
them.

`parametric.py` builds families from the operations the counterexamples
actually use — disjoint union, join, cone, pendant attachment — and sweeps each
integer parameter. Against the same three conjectures it finds:

```text
mode=109  WITNESS family=join_empty_cliques params=(7, 2, 3) n=13 phi=1
mode=103  WITNESS family=triangle_pendants  params=(1, 7)    n=11 phi=1
mode=58   WITNESS family=cone_biclique_clique params=(3, 2, 36) n=41 phi=1
```

The first is the published conjecture 109 counterexample exactly. The second is
a different member of the published family for 103. The third is new.

## A smaller counterexample to conjecture 58

The published refutation of conjecture 58 is K(3,3) with one vertex coned over
K(73), on 79 vertices. The sweep found

**K(3,2) with one vertex coned over K(36), on 41 vertices.**

```text
graph6  hFyCKMF`{No~`~`~o~{N~`~}F~{N~{N~}F~~`~~{N~~o~~~`~~~`~~~o~~~{N~~~`~~~
        }F~~~{N~~~{N~~~}F~~~~`~~~~{N~~~~o~~~~~`~~~~~`~~~~~o~~~~~{N~~~~~`~~~~~{

l_avg = 49/41        b = 6        f = 5        ceil(b / l_avg) = 6 > 5
```

The family closes in one line. For K(a,b) with one vertex of the a-side coned
over K(c), the local independence numbers are 3 at the apex, b at each of the
other a-side vertices, a at each b-side vertex, and 1 at every clique vertex.
At (a,b) = (3,2) that sums to 13 + c over an order of 5 + c, so

$$\frac{b(G)}{l_{\text{avg}}} = \frac{6(5+c)}{13+c} > 5 \iff c > 35,$$

and f(G) = 5 throughout. The first witness is therefore c = 36, which is
exactly where the sweep found one. Verified three ways: the C evaluator, the
algebra above, and `oracle.py`, which shares no code with the C path.

This refutes a conjecture that was already refuted, so its value is
calibration, not novelty. What it establishes is that the lane reaches real
counterexamples at orders the other lanes cannot touch.

## The limit that is not fixable by searching harder

Conjecture 58's *published* counterexample is at order 79. The sweeper's graph
representation stops at 63 vertices and graph6's short form at 62. No
instantiation this pipeline can hold would ever have shown it.

That is the case `--trend` exists for. When a family's unrounded bound is still
climbing at the largest representable parameter, the family is reported as
CLIMBING with the extrapolated crossing, rather than counted as a survivor. A
conjecture that survives a sweep with climbing families attached has not been
tested the way one with none has, and the report says which it is.

## Reproduction

```sh
make calibrate
```

Runs the independent oracle's self-test and the three trends. Conjecture 58's
family needs `--max-order 45` to reach order 41; the sweep across every mode
runs at a lower ceiling because the exact induced-forest search costs far more
on a dense graph than a sparse one, and the ceiling is printed on the summary
line rather than assumed. The counterexample above can be re-checked on its own:

```sh
python3 oracle.py --mode 58 --check-witness "$(python3 -c "
import sys; sys.path.insert(0,'.')
from parametric import cone, biclique, clique, to_graph6
print(to_graph6(cone(biclique(3,2), 0, clique(36))))")"
```

## What this says about conjecture 61

Nothing directly, and that is the point. The 2026-07-26 null result for
conjecture 61 was produced by two lanes now known to miss counterexamples that
exist, plus an exhaustive lane below the order where this pool's
counterexamples live. The null stands as a statement about what was searched.
It is weaker evidence for the conjecture than it looked before this measurement,
and conjecture 61 should be re-run through the parametric lane before anyone
leans on it.
