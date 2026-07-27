# Two Structural Reductions for Written on the Wall II, Conjecture 61

*Research note, 27 July 2026*

Publication formats: [PDF](PROOF-061.pdf) · [LaTeX source](PROOF-061.tex)

> **Status.** Conjecture 61 is not proved here. This note proves the
> diameter-at-most-three case, a star-forest certificate theorem, a one-unit
> deletion theorem, and a two-connected escape theorem. It then isolates five
> explicit open statements: any one of (PI), (DL), or (FA) would finish the
> conjecture, (MAXP) implies (PI), and (ESC) would finish the two-connected
> case of (DL). Exact computation supports every open statement on 2,592,586
> graph records, but finite verification is not a proof.

## Abstract

For a nontrivial connected graph $G$, Written on the Wall II, Conjecture 61
asserts

$$
f(G)\ge
\operatorname{residue}(G)
+\left\lceil\frac{\operatorname{diam}(G)}3\right\rceil,
\tag{C61}
$$

where $f(G)$ is the maximum order of an induced forest and
$\operatorname{residue}(G)$ is the Havel–Hakimi residue. We give two
reductions of the conjecture.

The first asks for a distance-three packing $S$ of order
$\lceil\operatorname{diam}(G)/3\rceil$ whose deletion leaves an independent
set of order $\operatorname{residue}(G)$. Such a packing and independent set
induce a disjoint union of stars, proving the desired bound immediately. A
stronger formulation asks that the packing occur among the vertices removed
by a suitable maximum-degree deletion order.

The second reduction is inductive. If $v$ is a maximum-degree vertex and
$C_1,\ldots,C_k$ are the components of $G-v$, then

$$
\sum_{i=1}^k
\left(
  \operatorname{residue}(C_i)
  +\left\lceil\frac{\operatorname{diam}(C_i)}3\right\rceil
\right)
\ge
\operatorname{residue}(G)
+\left\lceil\frac{\operatorname{diam}(G)}3\right\rceil-1.
$$

Thus the natural component induction can lose at most one vertex. We state
the exact deletion lemma that would remove this last unit and prove that it
implies (C61).

Both reductions are then pushed one step further. In a two-connected graph,
deleting a maximum-degree vertex avoided by some diametral pair loses
nothing at all, so a two-connected counterexample to the deletion lemma must
have every maximum-degree vertex inside every diametral pair, diameter
congruent to $1$ modulo $3$, and exact equality in both the residue and
diameter terms after the deletion. Complementing this, a repair statement is
proved: induced forests meeting the component quotas in which no tree holds
two neighbors of the deleted vertex recover the lost unit whenever they
exist. All five open assertions were checked exactly on every
diameter-at-least-four record among the connected unlabeled graphs through
order $9$, equality-boundary extensions through order $11$, and a
path-of-gadgets catalogue through order $20$.

## 1. The problem in plain language

All graphs in this note are finite, simple, and undirected. The conjecture
compares three quantities. Its current Lean transcription is recorded as
research open [6]. The **order** of a graph is its number of vertices.

- $f(G)$ is the largest number of vertices that can be retained while the
  induced subgraph on those vertices has no cycle. Every edge between
  retained vertices must remain; edges cannot be pruned separately.
- $\operatorname{diam}(G)$ is the greatest distance between two vertices.
- $\operatorname{residue}(G)$ is obtained from the degree sequence by the
  Havel–Hakimi process: repeatedly remove the largest degree, subtract one
  from that many following degrees, and sort again. When only zeros remain,
  their number is the residue.

The residue is a classical lower bound on the independence number
$\alpha(G)$, so it already certifies an induced forest of
$\operatorname{residue}(G)$ vertices. Conjecture 61 says that a connected
graph also allows roughly one additional forest vertex for every three steps
of diameter.

Put

$$
r(G)=\operatorname{residue}(G),\qquad
d(G)=\operatorname{diam}(G),\qquad
q(G)=\left\lceil\frac{d(G)}3\right\rceil
$$

and

$$
b(G)=r(G)+q(G),\qquad
\Phi(G)=b(G)-f(G).
$$

The conjecture is the assertion $\Phi(G)\le0$ for every nontrivial
connected graph. A single graph with $\Phi(G)>0$ would disprove it.
Graphs with $\Phi(G)=0$ are the equality boundary.

Complete graphs and stars show that equality is common for unrelated
reasons. If $n\ge2$, then $K_n$ has $r(K_n)=1$, $d(K_n)=1$, and
$f(K_n)=2$. The star $K_{1,n-1}$ has residue $n-1$, diameter $2$, and
is itself a forest. Both have $\Phi=0$.

Long paths with dense decorations are the natural pressure point. The long
spine raises the diameter term, while cliques, odd cycles, and dense blocks
attached along the spine try to make every large induced subgraph cyclic.
The computational constructions below were designed to push both effects at
once.

## 2. A star-forest certificate

A **2-packing** is a set $S\subseteq V(G)$ such that every two distinct
vertices of $S$ are at distance at least $3$ in $G$.

### Proposition 1 (packing–independence certificate)

Suppose $G$ contains a 2-packing $S$ and an independent set
$I\subseteq V(G)\setminus S$. Then $G[I\cup S]$ is a disjoint union of
stars and isolated vertices. In particular,

$$
f(G)\ge |I|+|S|.
$$

#### Proof

There are no edges inside $I$, since $I$ is independent, and no edges
inside $S$, since distinct vertices of a 2-packing are not adjacent.
Moreover, each vertex of $I$ has at most one neighbor in $S$. Two such
neighbors would be joined by a path of length two through that vertex,
contrary to the definition of a 2-packing.

Every nontrivial component of $G[I\cup S]$ therefore consists of one vertex
of $S$ and some of its neighbors in $I$. It is a star. The remaining
vertices are isolated, so the entire induced subgraph is a forest. $\square$

Proposition 1 turns Conjecture 61 into the following concrete structural
question.

### Packing–independence lemma (PI, open)

Every nontrivial connected graph $G$ has a 2-packing $S$ such that

$$
|S|=q(G)
\quad\text{and}\quad
\alpha(G-S)\ge r(G).
\tag{PI}
$$

### Corollary 2

If (PI) holds for every nontrivial connected graph, then Conjecture 61 holds.

#### Proof

Choose an independent set $I$ of order $r(G)$ in $G-S$, then apply
Proposition 1:

$$
f(G)\ge |I|+|S|=r(G)+q(G)=b(G).
\qquad\square
$$

This reduction explains the divisor $3$. Vertices chosen three steps apart
cannot acquire a common neighbor in the independent part, so the combined
induced graph has no cycle. Merely choosing mutually nonadjacent vertices
would not be enough.

## 3. A maximum-degree formulation

The classical `MAX` algorithm repeatedly deletes a vertex of maximum degree
until the remaining graph is independent. When several vertices have maximum
degree, the result can depend on the tie-breaking.

Griggs and Kleitman proved that every possible independent set left by `MAX`
has order at least $r(G)$ [2]. If `MAX` stops with more than $r(G)$
vertices, we may continue deleting isolated vertices until exactly $r(G)$
remain; a zero-degree vertex is then still a maximum-degree vertex.

This gives a proof-shaped strengthening of (PI).

### MAX-packing lemma (MAXP, open)

For every nontrivial connected graph $G$, the maximum-degree ties can be
broken so that, after exactly $|V(G)|-r(G)$ deletions, the deleted set
contains a 2-packing of order $q(G)$, with distances measured in the
original graph $G$.

### Proposition 3

$$
\text{(MAXP)}\implies\text{(PI)}\implies\text{(C61)}.
$$

#### Proof

Run the ordering promised by (MAXP), and let $D$ be the first
$|V(G)|-r(G)$ deleted vertices. Its complement $I$ is independent and has
order $r(G)$. If $S\subseteq D$ is the promised 2-packing, then
$I\subseteq V(G)\setminus S$, so $\alpha(G-S)\ge r(G)$. This is (PI), and
Corollary 2 finishes the implication. $\square$

The existential tie-breaking clause cannot simply be dropped. Among the
21,332 connected unlabeled graphs of order $9$ and diameter at least $4$,
all admit at least one successful maximum-degree ordering, but only 10,604
work for every ordering.

There is also a technical trap in any recursive proof of (MAXP): deletion can
increase distances. A set that is a 2-packing in a later residual graph need
not have been a 2-packing in the original graph. An induction must therefore
remember original distance-two conflicts, not only the current graph.

## 4. What can already be proved

### Proposition 4 (one vertex beyond independence)

Every nontrivial connected graph satisfies

$$
f(G)\ge\alpha(G)+1.
$$

#### Proof

Let $A$ be a maximum independent set. Since $G$ is connected and has at
least two vertices, $A\ne V(G)$; choose $v\notin A$. The graph induced by
$A\cup\{v\}$ is a star centered at $v$, together with isolated vertices.
It is an induced forest of order $\alpha(G)+1$. $\square$

### Corollary 5 (diameter at most three)

Conjecture 61 holds whenever $d(G)\le3$.

#### Proof

A nontrivial connected graph has $1\le d(G)\le3$, hence $q(G)=1$.
The classical residue bound $r(G)\le\alpha(G)$ [1] and Proposition 4 give

$$
f(G)\ge\alpha(G)+1\ge r(G)+q(G).
\qquad\square
$$

The only unresolved case therefore has diameter at least $4$.

## 5. The one-unit deletion theorem

For a graph $H$ with connected components $C_1,\ldots,C_k$, define the
component bound

$$
b^\oplus(H)=
\sum_{i=1}^k
\left(
  r(C_i)+q(C_i)
\right).
$$

For an isolated vertex $K_1$, we use $r(K_1)=1$, $d(K_1)=0$, and
$q(K_1)=0$, so its contribution to $b^\oplus$ is $1$.

### Lemma 6 (diameter loss under one deletion)

If $G$ is connected, $v\in V(G)$, and $C_1,\ldots,C_k$ are the
components of $G-v$, then

$$
\sum_{i=1}^k q(C_i)\ge q(G)-1.
\tag{1}
$$

#### Proof

Let

$$
P=x_0x_1\cdots x_d
$$

be a diametral geodesic of $G$, so $d=d(G)$. If $v\notin V(P)$, the
whole path survives in one component of $G-v$. Distances cannot decrease
when a vertex is deleted, so that component has diameter at least $d$, and
the left side of (1) is at least $q(G)$.

Now suppose $v=x_i$. If neither endpoint was deleted and $x_0,x_d$ lie
in one component of $G-v$, their distance there is at least $d$, and the
same argument applies. Otherwise, the surviving arm or arms of $P-v$ lie
in their respective components. Let their edge-lengths be $a,b\ge0$,
taking the missing arm to have length $0$ when an endpoint of $P$ is
deleted. Each nonempty arm remains a geodesic in its component, and

$$
d\le a+b+2.
$$

Consequently,

$$
\left\lceil\frac a3\right\rceil+
\left\lceil\frac b3\right\rceil
\ge
\left\lceil\frac{a+b+2}{3}\right\rceil-1
\ge
\left\lceil\frac d3\right\rceil-1.
$$

For the first inequality, use
$a\le3\lceil a/3\rceil$ and
$b\le3\lceil b/3\rceil$. Each existing arm lies in a component whose
diameter is at least its length; a missing arm contributes $0$. This proves
(1). $\square$

### Lemma 7 (residue under a maximum-degree deletion)

Let $v$ be a maximum-degree vertex of $G$, and let
$C_1,\ldots,C_k$ be the components of $G-v$. Then

$$
\sum_{i=1}^k r(C_i)\ge r(G).
\tag{2}
$$

#### Proof

Let $\pi'$ be the Havel–Hakimi reduction of the degree sequence of $G$.
It removes the maximum degree and subtracts one from the largest available
terms. Deleting the actual vertex $v$ instead subtracts one from the terms
corresponding to its neighbors. After sorting, this actual residual degree
sequence dominates $\pi'$, meaning its initial partial sums are at least
as large: subtracting from any terms other than the largest ones can only
make the sequence more concentrated. Residue is monotone under this
dominance order [1], so

$$
r(G-v)\ge r(\pi')=r(G).
$$

The residue of a disjoint union is at most the sum of the residues of its
components [3]. Therefore

$$
r(G)
\le r(G-v)
\le \sum_{i=1}^k r(C_i).
\qquad\square
$$

### Theorem 8 (one-unit deletion theorem)

If $v$ is a maximum-degree vertex of a connected graph $G$, then

$$
b^\oplus(G-v)\ge b(G)-1.
\tag{3}
$$

#### Proof

Add inequalities (1) and (2). $\square$

The theorem says that the natural induction is never badly wrong. A
maximum-degree deletion preserves the full residue contribution and can lose
at most one unit from the rounded diameter contribution.

## 6. The exact inductive finishing lemma

The one missing unit can be isolated as a second open statement.

### Deletion lemma (DL, open)

Every connected graph $G$ with $d(G)\ge4$ has a vertex $v$ such that

$$
b^\oplus(G-v)\ge b(G).
\tag{DL}
$$

The vertex in (DL) is not required to have maximum degree, and $G-v$ may be
disconnected.

### Theorem 9

If (DL) holds, then Conjecture 61 holds.

#### Proof

Proceed by strong induction on $|V(G)|$. The case $d(G)\le3$ is
Corollary 5. Otherwise choose $v$ as in (DL), and let
$C_1,\ldots,C_k$ be the components of $G-v$.

Each nontrivial component has fewer vertices than $G$, so by induction it
contains an induced forest of order at least $b(C_i)$. An isolated
component contributes its single vertex. The union of these forests is an
induced forest in $G-v$, and hence in $G$, of order at least

$$
\sum_{i=1}^k b(C_i)
=b^\oplus(G-v)
\ge b(G).
$$

This is (C61). $\square$

Theorem 8 sharply constrains a smallest counterexample to Conjecture 61. If
$G$ were such a graph, component induction would give
$b^\oplus(G-v)\le b(G)-1$ for every vertex $v$. When $v$ has maximum
degree, integrality and (3) would therefore force

$$
b^\oplus(G-v)=b(G)-1
$$

for every such $v$. Both ingredients would have to be tight:

$$
\sum_i r(C_i)=r(G),
\qquad
\sum_i q(C_i)=q(G)-1.
\tag{4}
$$

This reduces the next proof attempt to the equality structure in (4):
show that either another vertex satisfies (DL) or an induced forest through
the deleted vertex recovers the missing unit. The next two sections carry
both alternatives as far as they currently go.

## 7. The two-connected case

In the equality structure (4), the diameter term loses its unit only under a
genuine separation. Making this precise closes the two-connected case up to
a single structural question.

### Lemma 10 (metric loss requires separation)

Let $v$ be a vertex of a connected graph $G$, and suppose some diametral
pair $\{x,y\}$ of $G$ satisfies $v\notin\{x,y\}$, with $x$ and $y$ in the
same component $C$ of $G-v$. Then

$$
\sum_{i=1}^k q(C_i)\ge q(G).
$$

#### Proof

Deleting a vertex cannot decrease distances, so the distance between $x$
and $y$ inside $C$ is at least $d(G)$. Hence $d(C)\ge d(G)$ and
$q(C)\ge q(G)$. $\square$

This is the first case in the proof of Lemma 6, restated because its
contrapositive is what matters here: losing the diameter unit forces the
deleted vertex to separate, or to belong to, every diametral pair. No
vertex separates a two-connected graph.

### Theorem 11 (two-connected escape)

Let $G$ be two-connected with $d(G)\ge4$, and let $v$ be a maximum-degree
vertex avoided by some diametral pair. Then $G-v$ is connected and

$$
b(G-v)\ge b(G).
$$

In particular, $v$ satisfies (DL).

#### Proof

Since $G$ is two-connected, $G-v$ is connected, so the avoided diametral
pair lies in the single component $G-v$ and Lemma 10 gives
$q(G-v)\ge q(G)$. Lemma 7 gives $r(G-v)\ge r(G)$. Adding the two
inequalities proves the claim. $\square$

### Corollary 12 (structure of a trapped counterexample)

Suppose $G$ is two-connected with $d(G)\ge4$ and no vertex of $G$
satisfies (DL). Then:

1. every maximum-degree vertex lies in every diametral pair; consequently
   $G$ has at most two maximum-degree vertices, and if it has exactly two,
   the diametral pair is unique;
2. $d(G)\equiv1\pmod3$;
3. for every maximum-degree vertex $v$, both $d(G-v)=d(G)-1$ and
   $r(G-v)=r(G)$ hold exactly;
4. for every diametral pair $\{v,y\}$ with $v$ of maximum degree, $v$ is
   the only vertex of $G$ at distance $d(G)$ from $y$.

#### Proof

Claim 1 is Theorem 11 read contrapositively; a diametral pair has two
elements, and two distinct maximum-degree vertices can both lie in every
pair only if every pair is exactly those two vertices.

For claim 4, if some $z\ne v$ had $\operatorname{dist}(z,y)=d(G)$, then
$\{z,y\}$ would be a diametral pair avoiding $v$, against claim 1.

For claims 2 and 3, fix a maximum-degree $v$ and a diametral pair
$\{v,y\}$. The graph $G-v$ is connected and retains the arm of a diametral
geodesic from $y$, so $d(G-v)\ge d(G)-1$, while Lemma 7 gives
$r(G-v)\ge r(G)$. If $d(G)\not\equiv1\pmod3$, then
$\lceil(d(G)-1)/3\rceil=q(G)$ and $b(G-v)\ge b(G)$, contradicting the
absence of a (DL) vertex. So $d(G)\equiv1\pmod3$, and
$b(G-v)\le b(G)-1$ forces equality throughout: $r(G-v)=r(G)$ and
$q(G-v)=q(G)-1$. Since $d(G)\equiv1\pmod3$ gives $3(q(G)-1)=d(G)-1$,
the ceiling equality caps $d(G-v)\le d(G)-1$, so $d(G-v)=d(G)-1$
exactly. $\square$

The remaining question is whether trapped graphs exist at all.

### Escape lemma (ESC, open)

Every two-connected graph with $d(G)\ge4$ has a maximum-degree vertex
avoided by some diametral pair.

By Theorem 11, (ESC) implies (DL) for every two-connected graph. All
53,096 two-connected diameter-at-least-four records in the four corpora
satisfy (ESC); no trapped graph has been seen (Section 9.4). Two cautions
attach to that evidence. First, coverage: in a two-connected graph every
intermediate breadth-first level from any vertex has at least two vertices,
since a singleton level would be a cut vertex separating what lies beyond
it; hence $|V(G)|\ge2\,d(G)$, and the exhaustive order-at-most-$9$ slice
tests (ESC) only at $d(G)=4$. Evidence at larger diameters comes from the
structured families alone. Second, a trapped graph would not by itself
refute (DL) or Conjecture 61: Corollary 12 would still leave its rigid
residual structure open to a direct argument.

## 8. Forests through the deleted vertex

The second alternative in the equality analysis of (4) keeps the deleted
vertex in the forest.

### Proposition 13 (free-attachment repair)

Let $v$ be a maximum-degree vertex of a connected graph $G$ with
$d(G)\ge4$, and let $C_1,\ldots,C_k$ be the components of $G-v$. Suppose
each $C_i$ contains an induced forest $F_i$ with $|F_i|\ge b(C_i)$ such
that no tree of $C_i[F_i]$ contains two neighbors of $v$. Then

$$
f(G)\ge b(G).
$$

#### Proof

The union $F=F_1\cup\cdots\cup F_k$ is an induced forest of $G-v$. Any
cycle in $G[F\cup\{v\}]$ would have to pass through $v$, and its remainder
would be a path in the forest $G[F]$ joining two neighbors of $v$. A path
in a forest stays inside one tree, so some tree would contain two neighbors
of $v$, contrary to hypothesis. Adding $v$ merges the trees it touches
into one and creates no cycle, so $G[F\cup\{v\}]$ is an induced forest of
order at least

$$
1+\sum_{i=1}^k b(C_i)
=1+b^\oplus(G-v)
\ge b(G)
$$

by Theorem 8. $\square$

### Free-attachment lemma (FA, open)

Every connected graph $G$ with $d(G)\ge4$ has a maximum-degree vertex $v$
admitting forests as in Proposition 13.

### Proposition 14

If (FA) holds, then Conjecture 61 holds.

#### Proof

Corollary 5 covers $d(G)\le3$; Proposition 13 covers $d(G)\ge4$
directly. $\square$

The hypotheses of (FA) cannot simply be dropped. If a component is a single
edge with both endpoints adjacent to $v$, the only forest meeting the quota
$b(K_2)=2$ is the edge itself, and its single tree holds both neighbors.
That configuration is a triangle and has diameter $1$; the open content of
(FA) is that diameter at least $4$ and maximum degree leave enough room.
The star-forest witness of Section 2 is the natural candidate for the
$F_i$: stars are hard to enter twice, which is one more sign that the
packing reduction and the deletion reduction aim at the same object.

At every maximum-degree deletion in the corpora that is one-unit tight —
$b^\oplus(G-v)=b(G)-1$, the only case Theorem 8 leaves open, since
otherwise $v$ itself satisfies (DL) — the constrained quota forests exist:
58,809 configurations, no exceptions (Section 9.4).

## 9. Exact computational evidence

The computation had two roles: search directly for $\Phi(G)>0$, and test
the proposed structural lemmas without replacing them by heuristics.

### 9.1 Evaluator validation

The optimized C evaluator was compared with separate exhaustive oracles on
all 33,867 labeled graphs through order $6$:

- manual Havel–Hakimi reduction against the production residue routine;
- Floyd–Warshall against repeated-BFS diameter;
- all vertex subsets against the cycle-branching induced-forest optimizer;
- graph6 encoding and decoding round trips.

Complete graphs, paths, and stars were also checked through order $20$.
An independent Python program re-evaluated reported boundary graphs by
descending subset enumeration.

### 9.2 Counterexample search

Four complementary searches were used.

1. **Exhaustive small graphs.** Every connected unlabeled graph through order
   $9$ was generated. The order-$9$ count of 261,080 agrees with McKay's
   published table [4].
2. **Path-of-gadgets constructions.** The catalogue joins cliques, odd
   cycles, complete bipartite blocks, barbells, articulation blocks, mixed
   blocks, and layered paths along thin necks, through order $20$.
3. **Boundary extensions.** Every nonempty neighborhood for a new vertex was
   attached to each of the 1,231 order-$9$ equality graphs. Leaves and true
   or false twins were then attached to the 49,521 surviving order-$10$
   equality records.
4. **Direct optimization.** Edge additions and deletions preserving
   connectivity hill-climbed $\Phi$ and the smoother score
   $3(r(G)-f(G))+d(G)$. About 7.5 million edge toggles were considered at
   orders $10,12,13,16,$ and $18$, with every connected candidate
   evaluated exactly.

No graph with $\Phi(G)>0$ was found. This is a bounded null result, not a
proof of (C61).

### 9.3 Structural-lemma checks

[`proof61.c`](proof61.c) enumerates 2-packings and solves the remaining
independent-set decision problem exactly. [`maxine61.c`](maxine61.c) uses
dynamic programming over remaining-vertex masks, explores every allowed
maximum-degree tie, and inspects the frontier at exactly $r(G)$ remaining
vertices.

| corpus | records | $d\le3$ | $d\ge4$ | (PI) | some MAX order | every MAX order |
|---|---:|---:|---:|---:|---:|---:|
| connected unlabeled graphs of order 9 | 261,080 | 239,748 | 21,332 | 21,332 | 21,332 | 10,604 |
| all one-vertex extensions of order-9 equality graphs | 629,041 | 509,354 | 119,687 | 119,687 | 119,687 | 37,579 |
| leaf/twin extensions of surviving order-10 equality records | 1,342,198 | 962,218 | 379,980 | 379,980 | 379,980 | 111,752 |
| path-of-gadgets catalogue through order 20 | 360,267 | 376 | 359,891 | 359,891 | 359,891 | 213,501 |
| **total evaluated records** | **2,592,586** | **1,711,696** | **880,890** | **880,890** | **880,890** | **373,436** |

The extension and family rows contain isomorphic duplicates. The totals
therefore count exact evaluations, not distinct isomorphism classes.
A separate component-bound scan found a vertex satisfying (DL) in all
880,890 records of diameter at least $4$.

The deletion reduction gives a more detailed picture at order $9$. Of the
21,332 graphs not covered by Corollary 5:

- 15,568 have a successful maximum-degree deletion that leaves a connected
  graph;
- 21,136 have some successful connected deletion;
- the remaining 196 have no successful connected deletion, but all 196 have
  a cut-vertex deletion satisfying (DL).

Thus no order-$9$ graph is induction-critical. Thirty-one of the final 196
graphs also lie on the equality boundary $\Phi=0$.

### 9.4 Endgame checks

[`finish61.c`](finish61.c) ran three further exact checks across the same
four corpora. Its internal deletion census independently reproduced the
order-$9$ numbers above.

**Lemma 7 stress test.** All 5,246,681 maximum-degree deletions across the
2,592,586 records satisfied $\sum_i r(C_i)\ge r(G)$, with zero violations.
This exercises the dominance step of Lemma 7 — the one step of the proved
material resting on a monotonicity property cited from [1] — on every
available record.

**(ESC) and (FA) census.** Every two-connected record of diameter at least
$4$ has a maximum-degree vertex avoided by some diametral pair, and the
conclusion $b(G-v)\ge b(G)$ of Theorem 11 was re-verified by direct
computation at each such vertex, with no failures. Every one-unit-tight
maximum-degree deletion admits the constrained quota forests of
Proposition 13; the per-component search was exhaustive over vertex
subsets, with no records skipped.

| corpus | 2-connected $d\ge4$ | trapped | tight deletions | repaired |
|---|---:|---:|---:|---:|
| connected unlabeled graphs of order 9 | 1,944 | 0 | 854 | 854 |
| all one-vertex extensions of order-9 equality graphs | 18,037 | 0 | 7,113 | 7,113 |
| leaf/twin extensions of surviving order-10 equality records | 30,380 | 0 | 28,936 | 28,936 |
| path-of-gadgets catalogue through order 20 | 2,735 | 0 | 21,906 | 21,906 |
| **total** | **53,096** | **0** | **58,809** | **58,809** |

## 10. False strengthenings

Exact data rules out three tempting shortcuts.

1. Replacing residue by independence is false:

   $$
   f(G)\ge\alpha(G)+q(G)
   $$

   fails on 539 connected graphs of order $9$.

2. Letting $\rho_2(G)$ be the maximum order of a 2-packing, the stronger
   inequality

   $$
   f(G)\ge r(G)+\rho_2(G)
   $$

   fails on 114 order-$9$ graphs with diameter at least $4$. A maximum
   packing need not leave enough room for the independent part.

3. Replacing $\alpha(G-S)\ge r(G)$ in (PI) by
   $r(G-S)\ge r(G)$ is too strong. Ten connected order-$9$ graphs have no
   target-size packing satisfying the residue condition, although each has a
   packing satisfying the independence condition in (PI).

These failures locate the useful level of generality. The target-size
packing and the independent set left after its deletion survive; the more
obvious stronger statements do not.

## 11. What remains

There are now three independent finishing routes,

$$
\text{(MAXP)}\implies\text{(PI)}\implies\text{(C61)},
\qquad
\text{(DL)}\implies\text{(C61)},
\qquad
\text{(FA)}\implies\text{(C61)},
$$

and one partial route: by Theorem 11, (ESC) implies the two-connected case
of (DL).

The two-connected case is the most constrained. Corollary 12 pins any
counterexample there to diameter congruent to $1$ modulo $3$, at most two
maximum-degree vertices locked inside every diametral pair, and exact
equality in both deletion terms. Settling (ESC) is the natural next step.
A trapped graph, if one exists, has at least $2\,d(G)\ge14$ vertices when
$d(G)\ge7$, so it lies beyond the exhaustive corpus; targeted
constructions — a high-degree fan feeding a long ladder, at diameters $7$
and $10$ — would either produce one or sharpen the evidence. For $d(G)=4$
the exhaustive data covers only orders $8$ and $9$.

On the block-tree side, (FA) is the statement that still lacks a mechanism.
The 58,809 successful repairs say the constrained forests always exist in
the tested range; they do not say why. The mod-$3$ arithmetic of Lemma 6
favors cut vertices at positions congruent to $1$ modulo $3$ along a
diametral geodesic — the same spacing as the packing skeleton of
Section 2 — but such a vertex must also preserve the residue sum, and
residue is unprotected for deletions below maximum degree.

A counterexample to (DL), (PI), (MAXP), (ESC), or (FA) would not
necessarily be a counterexample to Conjecture 61; it would only rule out
that proof route. Further unconstrained edge mutation is unlikely to
resolve the question; the equality classification that Section 6 called for
has now been carried into Sections 7 and 8, and what remains of it is
exactly (ESC) and (FA).

The related WOWII Conjecture 17 says that the maximum order of an induced
bipartite subgraph is at least
$\alpha(G)+\lceil d(G)/3\rceil$; it is listed as resolved on the historical
WOWII page [5]. Its diameter term has the same spacing geometry, but
bipartiteness permits even cycles. Proposition 1 uses distance three to
obtain the stronger conclusion that the witness is a forest.

## 12. Reproduction

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
make check-finish-61
```

The direct order-$9$ packing scan is:

```sh
/tmp/math-eval-wow2/wow2-generate 9 \
  | /tmp/math-eval-wow2/wow2-proof61 --scan-packing
```

The exact evaluator and graph routines are in [`graph.c`](graph.c) and
[`conjectures.c`](conjectures.c). The endgame checks of Section 9.4 are
implemented in [`finish61.c`](finish61.c); any graph6 stream can be piped
through the resulting `wow2-finish61`. The independent verifier is
[`verify.py`](verify.py). The complete bounded-search log, including graph6
records on the equality boundary, is in [`RESULT-061.md`](RESULT-061.md).

## References

1. O. Favaron, M. Mahéo, and J.-F. Saclé, “On the residue of a graph,”
   *Journal of Graph Theory* **15** (1991), 39–64.
   <https://doi.org/10.1002/jgt.3190150107>
2. J. R. Griggs and D. J. Kleitman, “Independence and the Havel–Hakimi
   residue,” *Discrete Mathematics* **127** (1994), 209–212.
   <https://doi.org/10.1016/0012-365X(92)00479-B>
3. D. Amos, R. Davila, and R. Pepper, “On the $k$-residue of disjoint
   unions of graphs with applications to $k$-independence,”
   *Discrete Mathematics* **321** (2014), 24–34.
   <https://doi.org/10.1016/j.disc.2013.12.013>
4. B. D. McKay, “Combinatorial data: graphs.”
   <https://users.cecs.anu.edu.au/~bdm/data/graphs.html>
5. E. DeLaViña, “Written on the Wall II: Resolved Conjectures.”
   <https://cms.dt.uh.edu/faculty/delavinae/research/wowII/resolvedT.htm>
6. Google DeepMind, “FormalConjectures/WrittenOnTheWallII/
   GraphConjecture61.lean.”
   <https://github.com/google-deepmind/formal-conjectures/blob/main/FormalConjectures/WrittenOnTheWallII/GraphConjecture61.lean>
