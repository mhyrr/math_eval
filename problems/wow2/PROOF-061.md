# Two structural reductions for Written on the Wall II, Conjecture 61

*Research note, 27 July 2026*

> **Status.** Conjecture 61 is not proved here. This note proves the
> diameter-at-most-three case, a star-forest certificate theorem, and a
> one-unit deletion theorem. It then isolates three explicit open lemmas, any
> one of which would finish the conjecture. Exact computation supports those
> lemmas on 2,592,586 graph records, but finite verification is not a proof.

## Abstract

For a nontrivial connected graph \(G\), Written on the Wall II, Conjecture 61
asserts

\[
f(G)\ge
\operatorname{residue}(G)
+\left\lceil\frac{\operatorname{diam}(G)}3\right\rceil,
\tag{C61}
\]

where \(f(G)\) is the maximum order of an induced forest and
\(\operatorname{residue}(G)\) is the Havel–Hakimi residue. We give two
reductions of the conjecture.

The first asks for a distance-three packing \(S\) of order
\(\lceil\operatorname{diam}(G)/3\rceil\) whose deletion leaves an independent
set of order \(\operatorname{residue}(G)\). Such a packing and independent set
induce a disjoint union of stars, proving the desired bound immediately. A
stronger formulation asks that the packing occur among the vertices removed
by a suitable maximum-degree deletion order.

The second reduction is inductive. If \(v\) is a maximum-degree vertex and
\(C_1,\ldots,C_k\) are the components of \(G-v\), then

\[
\sum_{i=1}^k
\left(
  \operatorname{residue}(C_i)
  +\left\lceil\frac{\operatorname{diam}(C_i)}3\right\rceil
\right)
\ge
\operatorname{residue}(G)
+\left\lceil\frac{\operatorname{diam}(G)}3\right\rceil-1.
\]

Thus the natural component induction can lose at most one vertex. We state
the exact deletion lemma that would remove this last unit and prove that it
implies (C61). All three open assertions were checked exactly on every
diameter-at-least-four record among the connected unlabeled graphs through
order \(9\), equality-boundary extensions through order \(11\), and a
path-of-gadgets catalogue through order \(20\).

## 1. The problem in plain language

All graphs in this note are finite, simple, and undirected. The conjecture
compares three quantities. Its current Lean transcription is recorded as
research open [6]. The **order** of a graph is its number of vertices.

- \(f(G)\) is the largest number of vertices that can be retained while the
  induced subgraph on those vertices has no cycle. Every edge between
  retained vertices must remain; edges cannot be pruned separately.
- \(\operatorname{diam}(G)\) is the greatest distance between two vertices.
- \(\operatorname{residue}(G)\) is obtained from the degree sequence by the
  Havel–Hakimi process: repeatedly remove the largest degree, subtract one
  from that many following degrees, and sort again. When only zeros remain,
  their number is the residue.

The residue is a classical lower bound on the independence number
\(\alpha(G)\), so it already certifies an induced forest of
\(\operatorname{residue}(G)\) vertices. Conjecture 61 says that a connected
graph also allows roughly one additional forest vertex for every three steps
of diameter.

Put

\[
r(G)=\operatorname{residue}(G),\qquad
d(G)=\operatorname{diam}(G),\qquad
q(G)=\left\lceil\frac{d(G)}3\right\rceil
\]

and

\[
b(G)=r(G)+q(G),\qquad
\Phi(G)=b(G)-f(G).
\]

The conjecture is the assertion \(\Phi(G)\le0\) for every nontrivial
connected graph. A single graph with \(\Phi(G)>0\) would disprove it.
Graphs with \(\Phi(G)=0\) are the equality boundary.

Complete graphs and stars show that equality is common for unrelated
reasons. If \(n\ge2\), then \(K_n\) has \(r(K_n)=1\), \(d(K_n)=1\), and
\(f(K_n)=2\). The star \(K_{1,n-1}\) has residue \(n-1\), diameter \(2\), and
is itself a forest. Both have \(\Phi=0\).

Long paths with dense decorations are the natural pressure point. The long
spine raises the diameter term, while cliques, odd cycles, and dense blocks
attached along the spine try to make every large induced subgraph cyclic.
The computational constructions below were designed to push both effects at
once.

## 2. A star-forest certificate

A **2-packing** is a set \(S\subseteq V(G)\) such that every two distinct
vertices of \(S\) are at distance at least \(3\) in \(G\).

### Proposition 1 (packing–independence certificate)

Suppose \(G\) contains a 2-packing \(S\) and an independent set
\(I\subseteq V(G)\setminus S\). Then \(G[I\cup S]\) is a disjoint union of
stars and isolated vertices. In particular,

\[
f(G)\ge |I|+|S|.
\]

#### Proof

There are no edges inside \(I\), since \(I\) is independent, and no edges
inside \(S\), since distinct vertices of a 2-packing are not adjacent.
Moreover, each vertex of \(I\) has at most one neighbor in \(S\). Two such
neighbors would be joined by a path of length two through that vertex,
contrary to the definition of a 2-packing.

Every nontrivial component of \(G[I\cup S]\) therefore consists of one vertex
of \(S\) and some of its neighbors in \(I\). It is a star. The remaining
vertices are isolated, so the entire induced subgraph is a forest. \(\square\)

Proposition 1 turns Conjecture 61 into the following concrete structural
question.

### Packing–independence lemma (PI, open)

Every nontrivial connected graph \(G\) has a 2-packing \(S\) such that

\[
|S|=q(G)
\quad\text{and}\quad
\alpha(G-S)\ge r(G).
\tag{PI}
\]

### Corollary 2

If (PI) holds for every nontrivial connected graph, then Conjecture 61 holds.

#### Proof

Choose an independent set \(I\) of order \(r(G)\) in \(G-S\), then apply
Proposition 1:

\[
f(G)\ge |I|+|S|=r(G)+q(G)=b(G).
\qquad\square
\]

This reduction explains the divisor \(3\). Vertices chosen three steps apart
cannot acquire a common neighbor in the independent part, so the combined
induced graph has no cycle. Merely choosing mutually nonadjacent vertices
would not be enough.

## 3. A maximum-degree formulation

The classical `MAX` algorithm repeatedly deletes a vertex of maximum degree
until the remaining graph is independent. When several vertices have maximum
degree, the result can depend on the tie-breaking.

Griggs and Kleitman proved that every possible independent set left by `MAX`
has order at least \(r(G)\) [2]. If `MAX` stops with more than \(r(G)\)
vertices, we may continue deleting isolated vertices until exactly \(r(G)\)
remain; a zero-degree vertex is then still a maximum-degree vertex.

This gives a proof-shaped strengthening of (PI).

### MAX-packing lemma (MAXP, open)

For every nontrivial connected graph \(G\), the maximum-degree ties can be
broken so that, after exactly \(|V(G)|-r(G)\) deletions, the deleted set
contains a 2-packing of order \(q(G)\), with distances measured in the
original graph \(G\).

### Proposition 3

\[
\text{(MAXP)}\implies\text{(PI)}\implies\text{(C61)}.
\]

#### Proof

Run the ordering promised by (MAXP), and let \(D\) be the first
\(|V(G)|-r(G)\) deleted vertices. Its complement \(I\) is independent and has
order \(r(G)\). If \(S\subseteq D\) is the promised 2-packing, then
\(I\subseteq V(G)\setminus S\), so \(\alpha(G-S)\ge r(G)\). This is (PI), and
Corollary 2 finishes the implication. \(\square\)

The existential tie-breaking clause cannot simply be dropped. Among the
21,332 connected unlabeled graphs of order \(9\) and diameter at least \(4\),
all admit at least one successful maximum-degree ordering, but only 10,604
work for every ordering.

There is also a technical trap in any recursive proof of (MAXP): deletion can
increase distances. A set that is a 2-packing in a later residual graph need
not have been a 2-packing in the original graph. An induction must therefore
remember original distance-two conflicts, not only the current graph.

## 4. What can already be proved

### Proposition 4 (one vertex beyond independence)

Every nontrivial connected graph satisfies

\[
f(G)\ge\alpha(G)+1.
\]

#### Proof

Let \(A\) be a maximum independent set. Since \(G\) is connected and has at
least two vertices, \(A\ne V(G)\); choose \(v\notin A\). The graph induced by
\(A\cup\{v\}\) is a star centered at \(v\), together with isolated vertices.
It is an induced forest of order \(\alpha(G)+1\). \(\square\)

### Corollary 5 (diameter at most three)

Conjecture 61 holds whenever \(d(G)\le3\).

#### Proof

A nontrivial connected graph has \(1\le d(G)\le3\), hence \(q(G)=1\).
The classical residue bound \(r(G)\le\alpha(G)\) [1] and Proposition 4 give

\[
f(G)\ge\alpha(G)+1\ge r(G)+q(G).
\qquad\square
\]

The only unresolved case therefore has diameter at least \(4\).

## 5. The one-unit deletion theorem

For a graph \(H\) with connected components \(C_1,\ldots,C_k\), define the
component bound

\[
b^\oplus(H)=
\sum_{i=1}^k
\left(
  r(C_i)+q(C_i)
\right).
\]

For an isolated vertex \(K_1\), we use \(r(K_1)=1\), \(d(K_1)=0\), and
\(q(K_1)=0\), so its contribution to \(b^\oplus\) is \(1\).

### Lemma 6 (diameter loss under one deletion)

If \(G\) is connected, \(v\in V(G)\), and \(C_1,\ldots,C_k\) are the
components of \(G-v\), then

\[
\sum_{i=1}^k q(C_i)\ge q(G)-1.
\tag{1}
\]

#### Proof

Let

\[
P=x_0x_1\cdots x_d
\]

be a diametral geodesic of \(G\), so \(d=d(G)\). If \(v\notin V(P)\), the
whole path survives in one component of \(G-v\). Distances cannot decrease
when a vertex is deleted, so that component has diameter at least \(d\), and
the left side of (1) is at least \(q(G)\).

Now suppose \(v=x_i\). If neither endpoint was deleted and \(x_0,x_d\) lie
in one component of \(G-v\), their distance there is at least \(d\), and the
same argument applies. Otherwise, the surviving arm or arms of \(P-v\) lie
in their respective components. Let their edge-lengths be \(a,b\ge0\),
taking the missing arm to have length \(0\) when an endpoint of \(P\) is
deleted. Each nonempty arm remains a geodesic in its component, and

\[
d\le a+b+2.
\]

Consequently,

\[
\left\lceil\frac a3\right\rceil+
\left\lceil\frac b3\right\rceil
\ge
\left\lceil\frac{a+b+2}{3}\right\rceil-1
\ge
\left\lceil\frac d3\right\rceil-1.
\]

For the first inequality, use
\(a\le3\lceil a/3\rceil\) and
\(b\le3\lceil b/3\rceil\). Each existing arm lies in a component whose
diameter is at least its length; a missing arm contributes \(0\). This proves
(1). \(\square\)

### Lemma 7 (residue under a maximum-degree deletion)

Let \(v\) be a maximum-degree vertex of \(G\), and let
\(C_1,\ldots,C_k\) be the components of \(G-v\). Then

\[
\sum_{i=1}^k r(C_i)\ge r(G).
\tag{2}
\]

#### Proof

Let \(\pi'\) be the Havel–Hakimi reduction of the degree sequence of \(G\).
It removes the maximum degree and subtracts one from the largest available
terms. Deleting the actual vertex \(v\) instead subtracts one from the terms
corresponding to its neighbors. After sorting, this actual residual degree
sequence dominates \(\pi'\), meaning its initial partial sums are at least
as large: subtracting from any terms other than the largest ones can only
make the sequence more concentrated. Residue is monotone under this
dominance order [1], so

\[
r(G-v)\ge r(\pi')=r(G).
\]

The residue of a disjoint union is at most the sum of the residues of its
components [3]. Therefore

\[
r(G)
\le r(G-v)
\le \sum_{i=1}^k r(C_i).
\qquad\square
\]

### Theorem 8 (one-unit deletion theorem)

If \(v\) is a maximum-degree vertex of a connected graph \(G\), then

\[
b^\oplus(G-v)\ge b(G)-1.
\tag{3}
\]

#### Proof

Add inequalities (1) and (2). \(\square\)

The theorem says that the natural induction is never badly wrong. A
maximum-degree deletion preserves the full residue contribution and can lose
at most one unit from the rounded diameter contribution.

## 6. The exact inductive finishing lemma

The one missing unit can be isolated as a second open statement.

### Deletion lemma (DL, open)

Every connected graph \(G\) with \(d(G)\ge4\) has a vertex \(v\) such that

\[
b^\oplus(G-v)\ge b(G).
\tag{DL}
\]

The vertex in (DL) is not required to have maximum degree, and \(G-v\) may be
disconnected.

### Theorem 9

If (DL) holds, then Conjecture 61 holds.

#### Proof

Proceed by strong induction on \(|V(G)|\). The case \(d(G)\le3\) is
Corollary 5. Otherwise choose \(v\) as in (DL), and let
\(C_1,\ldots,C_k\) be the components of \(G-v\).

Each nontrivial component has fewer vertices than \(G\), so by induction it
contains an induced forest of order at least \(b(C_i)\). An isolated
component contributes its single vertex. The union of these forests is an
induced forest in \(G-v\), and hence in \(G\), of order at least

\[
\sum_{i=1}^k b(C_i)
=b^\oplus(G-v)
\ge b(G).
\]

This is (C61). \(\square\)

Theorem 8 sharply constrains a smallest counterexample to Conjecture 61. If
\(G\) were such a graph, component induction would give
\(b^\oplus(G-v)\le b(G)-1\) for every vertex \(v\). When \(v\) has maximum
degree, integrality and (3) would therefore force

\[
b^\oplus(G-v)=b(G)-1
\]

for every such \(v\). Both ingredients would have to be tight:

\[
\sum_i r(C_i)=r(G),
\qquad
\sum_i q(C_i)=q(G)-1.
\tag{4}
\]

This reduces the next proof attempt to the equality structure in (4):
compare two maximum-degree deletions, and show that either another vertex
satisfies (DL) or an induced forest through the deleted vertex recovers the
missing unit. The computation says this happens on every graph tested. It
does not yet explain why it must happen in every graph.

## 7. Exact computational evidence

The computation had two roles: search directly for \(\Phi(G)>0\), and test
the proposed structural lemmas without replacing them by heuristics.

### 7.1 Evaluator validation

The optimized C evaluator was compared with separate exhaustive oracles on
all 33,867 labeled graphs through order \(6\):

- manual Havel–Hakimi reduction against the production residue routine;
- Floyd–Warshall against repeated-BFS diameter;
- all vertex subsets against the cycle-branching induced-forest optimizer;
- graph6 encoding and decoding round trips.

Complete graphs, paths, and stars were also checked through order \(20\).
An independent Python program re-evaluated reported boundary graphs by
descending subset enumeration.

### 7.2 Counterexample search

Four complementary searches were used.

1. **Exhaustive small graphs.** Every connected unlabeled graph through order
   \(9\) was generated. The order-\(9\) count of 261,080 agrees with McKay's
   published table [4].
2. **Path-of-gadgets constructions.** The catalogue joins cliques, odd
   cycles, complete bipartite blocks, barbells, articulation blocks, mixed
   blocks, and layered paths along thin necks, through order \(20\).
3. **Boundary extensions.** Every nonempty neighborhood for a new vertex was
   attached to each of the 1,231 order-\(9\) equality graphs. Leaves and true
   or false twins were then attached to the 49,521 surviving order-\(10\)
   equality records.
4. **Direct optimization.** Edge additions and deletions preserving
   connectivity hill-climbed \(\Phi\) and the smoother score
   \(3(r(G)-f(G))+d(G)\). About 7.5 million edge toggles were considered at
   orders \(10,12,13,16,\) and \(18\), with every connected candidate
   evaluated exactly.

No graph with \(\Phi(G)>0\) was found. This is a bounded null result, not a
proof of (C61).

### 7.3 Structural-lemma checks

[`proof61.c`](proof61.c) enumerates 2-packings and solves the remaining
independent-set decision problem exactly. [`maxine61.c`](maxine61.c) uses
dynamic programming over remaining-vertex masks, explores every allowed
maximum-degree tie, and inspects the frontier at exactly \(r(G)\) remaining
vertices.

| corpus | records | \(d\le3\) | \(d\ge4\) | (PI) | some MAX order | every MAX order |
|---|---:|---:|---:|---:|---:|---:|
| connected unlabeled graphs of order 9 | 261,080 | 239,748 | 21,332 | 21,332 | 21,332 | 10,604 |
| all one-vertex extensions of order-9 equality graphs | 629,041 | 509,354 | 119,687 | 119,687 | 119,687 | 37,579 |
| leaf/twin extensions of surviving order-10 equality records | 1,342,198 | 962,218 | 379,980 | 379,980 | 379,980 | 111,752 |
| path-of-gadgets catalogue through order 20 | 360,267 | 376 | 359,891 | 359,891 | 359,891 | 213,501 |
| **total evaluated records** | **2,592,586** | **1,711,696** | **880,890** | **880,890** | **880,890** | **373,436** |

The extension and family rows contain isomorphic duplicates. The totals
therefore count exact evaluations, not distinct isomorphism classes.
A separate component-bound scan found a vertex satisfying (DL) in all
880,890 records of diameter at least \(4\).

The deletion reduction gives a more detailed picture at order \(9\). Of the
21,332 graphs not covered by Corollary 5:

- 15,568 have a successful maximum-degree deletion that leaves a connected
  graph;
- 21,136 have some successful connected deletion;
- the remaining 196 have no successful connected deletion, but all 196 have
  a cut-vertex deletion satisfying (DL).

Thus no order-\(9\) graph is induction-critical. Thirty-one of the final 196
graphs also lie on the equality boundary \(\Phi=0\).

## 8. False strengthenings

Exact data rules out three tempting shortcuts.

1. Replacing residue by independence is false:

   \[
   f(G)\ge\alpha(G)+q(G)
   \]

   fails on 539 connected graphs of order \(9\).

2. Letting \(\rho_2(G)\) be the maximum order of a 2-packing, the stronger
   inequality

   \[
   f(G)\ge r(G)+\rho_2(G)
   \]

   fails on 114 order-\(9\) graphs with diameter at least \(4\). A maximum
   packing need not leave enough room for the independent part.

3. Replacing \(\alpha(G-S)\ge r(G)\) in (PI) by
   \(r(G-S)\ge r(G)\) is too strong. Ten connected order-\(9\) graphs have no
   target-size packing satisfying the residue condition, although each has a
   packing satisfying the independence condition in (PI).

These failures locate the useful level of generality. The target-size
packing and the independent set left after its deletion survive; the more
obvious stronger statements do not.

## 9. What remains

There are now three explicit proof targets:

\[
\text{(MAXP)}\implies\text{(PI)}\implies\text{(C61)}
\]

and, independently,

\[
\text{(DL)}\implies\text{(C61)}.
\]

Of these, (DL) is the closest to the proved material. Theorem 8 leaves only a
one-unit equality case, and a proof of (DL) would close the conjecture by
Theorem 9. A counterexample to (DL), (PI), or (MAXP) would not necessarily be
a counterexample to Conjecture 61; it would only rule out that proof route.

Further unconstrained edge mutation is unlikely to resolve the question.
The next mathematical step is to classify equality in (4), especially when
deleting a maximum-degree vertex separates a diametral geodesic. The next
computational step, if needed, is to enumerate the equality states relevant
to that classification rather than merely generate more graphs.

The related WOWII Conjecture 17 says that the maximum order of an induced
bipartite subgraph is at least
\(\alpha(G)+\lceil d(G)/3\rceil\); it is listed as resolved on the historical
WOWII page [5]. Its diameter term has the same spacing geometry, but
bipartiteness permits even cycles. Proposition 1 uses distance three to
obtain the stronger conclusion that the witness is a forest.

## 10. Reproduction

The implementation is in this directory. All build products are written to
`/tmp`.

```sh
cd problems/wow2
make test
make check-counts-9
make exhaustive-61
make families-61
make check-critical-61
make check-maxine-61
```

The direct order-\(9\) packing scan is:

```sh
/tmp/math-eval-wow2/wow2-generate 9 \
  | /tmp/math-eval-wow2/wow2-proof61 --scan-packing
```

The exact evaluator and graph routines are in [`graph.c`](graph.c) and
[`conjectures.c`](conjectures.c). The independent verifier is
[`verify.py`](verify.py). The complete bounded-search log, including graph6
records on the equality boundary, is in [`RESULT-061.md`](RESULT-061.md).

## References

1. O. Favaron, M. Mahéo, and J.-F. Saclé, “On the residue of a graph,”
   *Journal of Graph Theory* **15** (1991), 39–64.
   <https://doi.org/10.1002/jgt.3190150107>
2. J. R. Griggs and D. J. Kleitman, “Independence and the Havel–Hakimi
   residue,” *Discrete Mathematics* **127** (1994), 209–212.
   <https://doi.org/10.1016/0012-365X(92)00479-B>
3. D. Amos, R. Davila, and R. Pepper, “On the \(k\)-residue of disjoint
   unions of graphs with applications to \(k\)-independence,”
   *Discrete Mathematics* **321** (2014), 24–34.
   <https://doi.org/10.1016/j.disc.2013.12.013>
4. B. D. McKay, “Combinatorial data: graphs.”
   <https://users.cecs.anu.edu.au/~bdm/data/graphs.html>
5. E. DeLaViña, “Written on the Wall II: Resolved Conjectures.”
   <https://cms.dt.uh.edu/faculty/delavinae/research/wowII/resolvedT.htm>
6. Google DeepMind, “FormalConjectures/WrittenOnTheWallII/
   GraphConjecture61.lean.”
   <https://github.com/google-deepmind/formal-conjectures/blob/main/FormalConjectures/WrittenOnTheWallII/GraphConjecture61.lean>
