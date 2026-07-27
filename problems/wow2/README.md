# WOWII finite-graph sweeper

This directory is a reusable exact-search core for finite graph inequalities
from *Written on the Wall II*. It currently registers conjectures 59 and 61.
The graph generator and invariant code are conjecture-agnostic; a new WOWII
statement plugs in at the small switch in `conjectures.c`.

## Result for conjecture 61

No counterexample was found in this bounded attempt. The exact exhaustive sweep
covered every connected unlabeled graph through 9 vertices. Targeted searches
then covered 360,267 path-of-gadgets graphs through 20 vertices, 629,041
one-vertex extensions of the order-9 equality cases, 1,342,198 leaf/twin
extensions of the surviving order-10 cases, and about 7.5 million edge-toggle
proposals, with every connected candidate evaluated exactly. The maximum
remained

\[
\Phi(G)=\operatorname{residue}(G)
       +\left\lceil\frac{\operatorname{diam}(G)}3\right\rceil-f(G)=0.
\]

The full run record, including the closest graphs, is in
[`RESULT-061.md`](RESULT-061.md). It supplies evidence only; the conjecture
remains open.

The subsequent proof attack is in [`PROOF-061.md`](PROOF-061.md). It proves
the diameter-at-most-three case, shows that every maximum-degree deletion is
within one unit of a component induction, and proves that a two-connected
counterexample to the deletion route would need every maximum-degree vertex
inside every diametral pair with diameter congruent to 1 modulo 3. The full
conjecture reduces to any one of three open lemmas — packing–independence,
the deletion lemma, or a free-attachment forest lemma — each verified on
all 2,592,586 records in the existing corpora ([`finish61.c`](finish61.c)
holds the endgame checks). The publication rendering is available as
[`PROOF-061.pdf`](PROOF-061.pdf), with source in
[`PROOF-061.tex`](PROOF-061.tex).

## Components

- `graph.c`: graph6 I/O; connectivity and diameter; Havel–Hakimi residue;
  exact maximum induced forest and induced bipartite subgraph. The two induced
  optimizers branch on a certificate cycle and memoize vertex-deletion states.
- `conjectures.c`: the inequality registry. Modes 59 and 61 are deterministic
  consumers of exact invariants.
- `generate.c`: connected-unlabeled generation by canonical vertex
  augmentation. Weisfeiler–Leman color refinement cuts the permutation search;
  exhaustive within-cell permutation supplies the canonical label. Every
  intermediate count is checked against [McKay's published graph tables][mckay-data]
  through order 9.
- `sweep.c`: scan/evaluate/filter CLI. Its self-test compares the optimized
  routines with enumeration and Floyd–Warshall on all 33,867 labeled graphs
  through order 6.
- `verify.py`: independent brute-force certificate oracle. It shares no
  evaluator code with the C path.
- `families.py`: clique, odd-cycle, biclique, barbell, articulation, and
  layered-path constructions.
- `extensions.py`: one-vertex boundary expansion using all neighborhoods or
  just leaves and true/false twins.
- `proof61.c` and `reduction61.c`: exact certificate and one-vertex-induction
  diagnostics for conjecture 61.
- `maxine61.c`: exact dynamic-programming checker for the candidate
  MAX-packing lemma. It distinguishes existence of a successful
  maximum-degree tie-breaking from success under every tie-breaking.
- `reduction_search61.c`: edge-toggle search for a counterexample to the
  candidate component-induction lemma.
- `search.c`: connected edge-toggle optimization. For conjecture 61 it uses
  the exact smoother score
  \(3(\operatorname{residue}-f)+\operatorname{diam}\); this is positive exactly
  when \(\Phi\) is positive.

`geng` remains the right production generator when `nauty` is available.
It was not available on this machine, and both the official tarball and a
public source mirror were unreachable from the execution sandbox. The native
generator removes that dependency for the promised range. `nauty` documents
`geng` and graph6 in its [official user guide][nauty-guide].

## Reproduction

All binaries go to `/tmp`; the repository stays free of build artifacts.

```sh
cd problems/wow2
make test
make check-counts-9
make exhaustive-61
make exhaustive-59
make families-61
make check-critical-61
make check-maxine-61
```

Evaluate one graph and verify it independently:

```sh
/tmp/math-eval-wow2/wow2-sweep --mode 61 --eval 'LN@YI`|OIG^fDH'
python3 verify.py --mode 61 'LN@YI`|OIG^fDH'
```

Run a stochastic search, optionally starting on the equality surface:

```sh
/tmp/math-eval-wow2/wow2-search \
  --mode 61 \
  --start 'LN@YI`|OIG^fDH' \
  --iterations 100000 \
  --restarts 50 \
  --seed 613613
```

Filter equality cases and extend each by every possible neighborhood for one
new vertex:

```sh
/tmp/math-eval-wow2/wow2-generate 9 \
  | /tmp/math-eval-wow2/wow2-sweep --mode 61 --emit-phi-at-least 0 \
  | python3 extensions.py --kind all \
  | /tmp/math-eval-wow2/wow2-sweep --mode 61 --scan
```

## Adding another conjecture

1. Add its mode and exact output fields to `conjectures.h`.
2. Add one branch to `wow2_evaluate` in `conjectures.c`.
3. Add a brute-force formula to `verify.py`.
4. Add family checks or all-small-graph oracle comparisons to `sweep.c`.

The generator and search code should not change. If a new statement requires an
invariant not already present, put the exact implementation in `graph.c`; do not
smuggle conjecture logic into the generator.

[nauty-guide]: https://users.cecs.anu.edu.au/~bdm/nauty/nug29.pdf
[mckay-data]: https://users.cecs.anu.edu.au/~bdm/data/graphs.html
