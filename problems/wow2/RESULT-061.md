# WOWII 61 — bounded counterexample search

Date: 2026-07-26

## Verdict

**No counterexample found.** This attempt reached the stated stop condition
without producing a graph with \(\Phi(G)>0\). It leaves exact reusable
machinery and a bounded null result. The conjecture remains open.

## Exact target

For each nontrivial connected simple graph \(G\), the evaluator computes

\[
\Phi(G)=\operatorname{residue}(G)
       +\left\lceil\frac{\operatorname{diam}(G)}3\right\rceil-f(G),
\]

where residue is the number of zeros left by the Havel–Hakimi process and
\(f(G)\) is the largest order of an induced forest.

## Verification

The optimized C evaluator was compared against separate exhaustive oracles on
all 33,867 labeled graphs through order 6:

- manual Havel–Hakimi versus the production residue routine;
- Floyd–Warshall versus repeated-BFS diameter;
- all vertex subsets versus the cycle-branching induced-forest optimizer;
- all vertex subsets versus the odd-cycle-branching induced-bipartite
  optimizer used by WOWII 59;
- graph6 encode/decode round trips.

It also checks complete graphs, paths, and stars through order 20. A separate
Python program re-evaluated the reported boundary graphs by descending subset
enumeration.

## Search lanes

### 1. Every connected unlabeled graph through order 9

The native canonical-augmentation generator reproduced the connected-unlabeled
counts in [McKay's graph tables](https://users.cecs.anu.edu.au/~bdm/data/graphs.html):

| order | graphs |
|---:|---:|
| 1 | 1 |
| 2 | 1 |
| 3 | 2 |
| 4 | 6 |
| 5 | 21 |
| 6 | 112 |
| 7 | 853 |
| 8 | 11,117 |
| 9 | 261,080 |

Order 9 result:

```text
SUMMARY mode=61 graphs=261080 equality=1231 witnesses=0 max_phi=0
BEST mode=61 graph6=HJ?K[Zo n=9 m=14 residue=3 diameter=4
ceil_diameter_over_3=2 bound=5 induced_forest=5 phi=0
```

### 2. Path-of-gadgets catalogue

The catalogue generated clique, odd-cycle, complete-bipartite, barbell,
articulation, mixed-block, and layered-path families through 20 vertices.

```text
path-of-gadgets records=360267 max_vertices=20
SUMMARY mode=61 graphs=360267 equality=196 witnesses=0 max_phi=0
```

### 3. Boundary extensions

Every nonempty neighborhood for a new vertex was applied to all 1,231
order-9 equality graphs:

```text
extensions inputs=1231 outputs=629041 kind=all
SUMMARY mode=61 graphs=629041 equality=49521 witnesses=0 max_phi=0
```

Leaves and true/false twins were then applied to those surviving records:

```text
extensions inputs=49521 outputs=1342198 kind=local
SUMMARY mode=61 graphs=1342198 equality=690671 witnesses=0 max_phi=0
```

These are labeled extension records and include isomorphic duplicates; the
reported denominator is the number actually evaluated.

### 4. Direct optimization

Completed stochastic runs at orders 10, 12, 13, 16, and 18 considered about
7.5 million edge toggles, evaluating every connected candidate exactly. The
strongest run began from this order-13 equality graph:

```text
graph6=LN@YI`|OIG^fDH
n=13 m=34 residue=8 diameter=3
ceil_diameter_over_3=1 induced_forest=9 phi=0
```

Five million further mutations across 50 restarts did not improve its smoother
score \(3(\operatorname{residue}-f)+\operatorname{diam}=0\). The independent
Python oracle agrees on every displayed quantity.

## Interpretation

The substantial equality surface shows that the search repeatedly reached zero
slack. The attempted pressure families and direct optimizer reach \(\Phi=0\),
then fail to cross it.

Further random mutations would repeat an exhausted idea. A materially
different attempt would either:

1. exhaust order 10 with `geng` or another independently established
   unlabeled generator; or
2. turn the observed boundary behavior into a proof attack, likely by relating
   the residue independent-set witness to vertices selected along a diametral
   geodesic.

This attempt stops before pretending either one happened.

## Subsequent proof attack

The second route has now been carried out as far as one explicit structural
lemma. A distance-three packing \(S\) of order
\(\lceil\operatorname{diam}(G)/3\rceil\), disjoint from an independent
\(\operatorname{residue}(G)\)-set, immediately induces the required star
forest. The exact checker found such a certificate on every record in all
four corpora. A stronger formulation through maximum-degree (`MAX`) deletion
also survived every record.

Neither finite result proves the structural lemma. The proved reductions,
2,592,586-record verification table, and exact stopping point are in
[`PROOF-061.md`](PROOF-061.md).

## Endgame checks (2026-07-27)

Three further exact checks, implemented in [`finish61.c`](finish61.c), ran
over the same four corpora (2,592,586 records; 880,890 of diameter at least
4). Reproduce with `make check-finish-61` for the order-9 slice, or pipe any
graph6 stream through `/tmp/math-eval-wow2/wow2-finish61`.

**Lemma 7 stress test.** For every maximum-degree vertex of every record —
5,246,681 deletions, all diameters — the component residues of \(G-v\)
summed to at least \(\operatorname{residue}(G)\). Zero violations. This
exercises the majorization step of Lemma 7, the one proof step resting on a
citation.

**Two-connected collapse.** Lemma 6 loses a metric unit only when the
deleted vertex separates the diametral pair or is one of its endpoints, and
no vertex separates a 2-connected graph. So a maximum-degree vertex outside
some diametral pair satisfies (DL) directly. Among all 53,096 two-connected
records of diameter at least 4 (orders 8–20), **every graph has such a
vertex**; the deletion bound was re-verified exactly at each one, with zero
failures. The trapped class — every maximum-degree vertex inside every
diametral pair — is empty on everything tested, so the residual case
(trapped with \(d\equiv1\pmod3\)) never materializes. Whether trapped
2-connected graphs exist at all is now its own open question.

**Free-attachment repair.** At every maximum-degree deletion that is
one-unit tight (\(b^\oplus(G-v)=b(G)-1\)) — 58,809 configurations — each
component admits an induced forest meeting its \(b(C_i)\) quota in which no
tree contains two neighbors of \(v\). The union plus \(v\) is then a
\(b(G)\)-forest. Zero failures, zero skipped records. This is exact support
for a strengthened induction hypothesis (a bound-sized forest with a free
attachment point), which would close the conjecture even where (DL)-style
deletion loses its unit.

| corpus | records | \(d\ge4\) | 2-conn | trapped | tight | repaired |
|---|---:|---:|---:|---:|---:|---:|
| connected order 9 | 261,080 | 21,332 | 1,944 | 0 | 854 | 854 |
| order-10 extensions | 629,041 | 119,687 | 18,037 | 0 | 7,113 | 7,113 |
| order-11 leaf/twin extensions | 1,342,198 | 379,980 | 30,380 | 0 | 28,936 | 28,936 |
| path-of-gadgets through 20 | 360,267 | 359,891 | 2,735 | 0 | 21,906 | 21,906 |
| **total** | **2,592,586** | **880,890** | **53,096** | **0** | **58,809** | **58,809** |

The deletion census inside `finish61.c` independently reproduced the
published order-9 numbers (21,136 connected-reducible, 15,568 at maximum
degree, 196 cut-vertex rescues, 0 without a (DL) vertex).
