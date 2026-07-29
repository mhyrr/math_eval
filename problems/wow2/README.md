# WOWII finite-graph sweeper

A reusable exact-search core for the graph inequalities in *Written on the Wall
II*. It registers **24 conjectures**: the 21 of DeepMind's 22 open WOWII
statements that this machinery can evaluate, plus three already refuted
upstream that are kept as a calibration set.

Every quantity is exact. Rounding is done on integers and rationals, never on
floating point — the one transcendental term in the pool (conjecture 103's
natural logarithm) is decided against certified rational enclosures of `e^k`.

## Start here

```sh
make test          # every self-test, including the invariant oracle
make list          # the registry
make calibrate     # the three conjectures whose answers are already known
```

## The registry

`conjectures.c` holds one descriptor and one evaluator per statement, in a
table meant to stay on one screen. Two shapes cover the pool:

- **inequality** — asserts `lhs <= rhs`; a counterexample has `phi = lhs - rhs`
  positive, and `phi = 0` is the equality surface.
- **implication** — asserts `hypothesis -> conclusion`; a counterexample
  satisfies the first and fails the second. The hypothesis is a filter, which
  is what makes these cheap to sweep deeply. Conjecture 322's holds on exactly
  one graph at order 9 and one at order 10, out of 11,716,571.

Statements are transcribed from the theorem bodies in DeepMind's
`formal-conjectures`, **never from the module docstrings** — they drift apart
inside the same file. Conjecture 100's docstring still describes a superseded
revision using the complement's diameter; the theorem uses `degreeL2Norm`.
Implementing the docstring produced witnesses at order 9 that evaporated once
the real statement was used. If a surviving open conjecture suddenly falls to a
small simple graph, suspect the transcription first.

## Search lanes, and what each one can reach

`RESULT-CALIBRATION.md` is the measurement; this is the summary.

| lane | reaches | blind to |
|---|---|---|
| exhaustive unlabeled, order ≤ 10 | everything small | every counterexample in this pool, which start at order 11 |
| `families.py` path-of-gadgets, ≤ 20 | chains of blocks | joins, coronas, cones |
| `search.c` edge toggles | local structure | anything needing a global construction |
| `parametric.py` blueprint families | joins, cones, coronas, at any order | shapes not in the catalogue |

The parametric lane is the one that finds this pool's counterexamples. It
builds graphs from the operations they actually use — disjoint union, join,
cone, pendant attachment — and sweeps each integer parameter. Against the
calibration set it recovers conjecture 109's published counterexample exactly,
finds a valid one for 103, and finds a **new, smaller counterexample to
conjecture 58 at order 41** against the published order 79.

Two lessons are now baked into the code:

**Rounding flattens the search.** Because nearly every statement ends in a
floor or a ceiling, `phi` is integer valued and a hill climber plateaus at zero.
Against conjecture 109 at the order where a counterexample was already known,
the edge-toggle search made two improvements in 16 million evaluations.
`set_inequality_smooth` hands the search the statement *before* its rounding
step. The verdict still comes from the rounded statement.

**A family can outrun the representation.** Conjecture 58's published
counterexample is at order 79; the graph type stops at 63 and graph6's short
form at 62. `--trend` therefore reports families whose unrounded bound is still
climbing when the vertex budget runs out, with the extrapolated crossing, so
they are flagged rather than silently counted as survivors.

## Results

- [`RESULT-SWEEP-2026-07-28.md`](RESULT-SWEEP-2026-07-28.md) — all 21 open
  conjectures through both lanes. No counterexamples.
- [`RESULT-CALIBRATION.md`](RESULT-CALIBRATION.md) — what the pipeline finds
  when pointed at conjectures whose answers are already published, and the
  order-41 counterexample to conjecture 58 that came out of it.
- [`RESULT-061.md`](RESULT-061.md), [`PROOF-061.md`](PROOF-061.md) — the
  2026-07-26 attempt on conjecture 61 and its proof attack.

## Result for conjecture 61

No counterexample, from any lane. The 2026-07-26 attempt covered every
connected unlabeled graph through order 9, 360,267 structured graphs through
order 20, and about 7.5 million edge toggles; 2026-07-29 added every connected
unlabeled graph through **order 10** (11,716,571 graphs, 5,124 equality cases)
and the parametric families. Maximum Φ is 0 everywhere. The run record is
[`RESULT-061.md`](RESULT-061.md); the proof attack, which reduces the conjecture
to any one of three open lemmas verified on all 2,592,586 records, is
[`PROOF-061.md`](PROOF-061.md) with the rendering in
[`PROOF-061.pdf`](PROOF-061.pdf).

Two caveats keep this from being stronger than it is. The lanes that produced
the original null are now measured to miss counterexamples that exist
(`RESULT-CALIBRATION.md`), and conjecture 61's equality surface *thins* from
0.472% of order 9 to 0.044% of order 10 — so exhaustion is sampling the regime
where the bound is least stressed, and buying order 11 would buy less again.
Conjecture 61 did survive the parametric lane, but through a catalogue built
from the shapes that refute conjectures 58, 103 and 109, not from its own.

## Components

Kernel, conjecture-agnostic:

- `graph.c` — graph6 I/O, connectivity, diameter, Havel–Hakimi residue, exact
  maximum induced forest and induced bipartite subgraph. The two optimizers
  branch on a certificate cycle and memoize vertex-deletion states.
- `rational.c` — exact rational arithmetic; aborts on overflow rather than
  wrapping.
- `invariants.c` — the rest of the exact invariants, computed lazily so a
  statement needing only distances never pays for a Hamiltonian-path search:
  the eccentricity family, girth, local independence, largest induced tree and
  path, path cover number, max-leaf spanning tree, Hamiltonian path, total
  domination and well-total-domination, triangle counts, the C(4) indicator.
- `generate.c` — connected-unlabeled generation by canonical vertex
  augmentation, checked against [McKay's tables][mckay-data] through order 10
  (11,716,571 graphs, about eight minutes).

Conjecture-aware:

- `conjectures.c` — the registry. Add a mode here and a formula in `oracle.py`.
- `sweep.c` — scan, filter, evaluate, `--report`, `--list`.
- `search.c` — connected edge-toggle optimization against a mode's gradient.
- `parametric.py` — blueprint families and `--trend`.
- `families.py`, `extensions.py` — the older structured and boundary lanes.
- `proof61.c`, `reduction61.c`, `maxine61.c`, `reduction_search61.c`,
  `finish61.c` — conjecture 61's proof-attack diagnostics.

Oracles, sharing no code with the C path:

- `verify.py` — subset enumeration from the top. Correct by inspection, and
  limited to small graphs.
- `invariants_test.c` — every invariant against brute force on all 33,867
  labeled graphs through order 6. This is what caught `Ls(K_1) = 0`, and it
  validates the connected-dominating-set identity behind the fast max-leaf
  routine and the upward-closure shortcut behind minimal total domination
  rather than assuming either.
- `oracle.py` — the same 24 statements at the sizes the parametric lane
  reaches, in Python, with maximum independent set through complement cliques
  instead of the C search's degree branching. `--check-witness` exits non-zero
  unless a graph really refutes its conjecture.

## Coverage that was not achieved is reported

Invariants that enumerate over vertex subsets stop at order 20, the path cover
at 18, and a statement below its own minimum order is outside it. Each case is
counted and printed as `skipped=` on the summary line. A family still climbing
at the vertex budget is printed as `CLIMBING`. Nothing is silently truncated.

## Adding another conjecture

1. Add a descriptor and an evaluator to `conjectures.c`.
2. Add the same formula to `oracle.py`.
3. If it needs a new invariant, put the exact implementation in `invariants.c`
   and its brute-force twin in `invariants_test.c`.

The generator and the search do not change. Do not put conjecture logic in the
generator.

## Reproduction

Binaries go to `/tmp`; the repository stays free of build artifacts.

```sh
cd problems/wow2
make test
make check-counts-9
make exhaustive-61
make calibrate
make parametric-61          # any registered mode

/tmp/math-eval-wow2/wow2-sweep --mode 61 --eval 'LN@YI`|OIG^fDH'
python3 oracle.py --mode 61 'LN@YI`|OIG^fDH'
```

`geng` remains the right production generator when `nauty` is available. It was
not available here and both the official tarball and a public mirror were
unreachable from the sandbox, so the native generator removes that dependency
for the promised range. `nauty` documents `geng` and graph6 in its
[user guide][nauty-guide].

[nauty-guide]: https://users.cecs.anu.edu.au/~bdm/nauty/nug29.pdf
[mckay-data]: https://users.cecs.anu.edu.au/~bdm/data/graphs.html
