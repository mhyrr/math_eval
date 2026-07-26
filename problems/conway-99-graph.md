# Conway's 99-graph — does an SRG(99, 14, 1, 2) exist?

status: DEFINED
class: W — a yes is a single graph on 99 vertices

| | |
|---|---|
| statement from | `FormalConjectures/Wikipedia/Conway99Graph.lean` @ `393aa9a` — PRIMARY |
| overview | https://en.wikipedia.org/wiki/Conway%27s_99-graph_problem — EXPERT_COMMENTARY |
| feasibility evidence | Keramatipour, *Approaching the Conway-99 problem using SAT solvers*, 2026-04 — https://arxiv.org/abs/2604.23037 — PRIMARY |
| structural constraints | *On the automorphism group of a putative Conway 99-graph*, Algebraic Combinatorics — https://alco.centre-mersenne.org/item/10.5802/alco.418.pdf — PRIMARY |
| prize | $1000 (Conway, from his 2017 five-problems list; per the Wikipedia article) |
| tags | graph theory, strongly regular graphs |

## Statement

> Does there exist a graph on 99 vertices in which every two adjacent
> vertices have exactly one common neighbour and every two non-adjacent
> vertices have exactly two?

Equivalently: a strongly regular graph with parameters (99, 14, 1, 2) —
every edge in a unique triangle, every non-edge a diagonal of a unique
quadrilateral.

## The certificate

A **yes** is one adjacency matrix, checked in milliseconds. A **no** is an
exhaustion of a finite but enormous space. Only the affirmative direction
has a small certificate — the same asymmetry as Erdős #7.

## Known, from the record

- The question goes back to Norman Biggs (1969) and was popularized by
  Conway with a $1000 bounty (Wikipedia, EXPERT_COMMENTARY).
- A 2026 SAT attack (PRIMARY above) reports that direct SAT encodings
  cannot handle the problem "in a reasonable time" — the negative-direction
  search is out of reach for current general-purpose solvers, and the paper
  exists to document that.
- The automorphism-group literature (PRIMARY above) has been squeezing the
  symmetric case for years: a putative graph's automorphism group is
  heavily constrained, so highly symmetric constructions are largely ruled
  out. Any surviving example is probably ugly — which is bad news for
  human search and neutral-to-interesting news for machine search.

## Why it is in this folder

The purest single-finite-object existence question in the non-Erdős pool,
with real prize money and fresh (2026) documentation of exactly where the
computational barrier sits. It enters as the class exemplar for "perfect
certificate, hostile search space" — the Conway-99 analogue of what #307
is inside the Erdős queue.

## Before an attempt

- **G0 not discharged.** The SEARCHED stage must establish which parameter
  constraints and partial exhaustions are already published (the SRG
  literature is large and well-indexed).
- **Feasibility:** the 2026 SAT paper is the starting point — extract what
  encodings were tried and where they died. Any ATTEMPT verdict would need
  a genuinely new search idea (orbit-based, algebraic seeding), not more
  solver hours on the same encoding. PARK-leaning.
