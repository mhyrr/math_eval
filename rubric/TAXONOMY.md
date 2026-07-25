# Taxonomy: two coding dimensions and three flags

Replaces the flat four-mechanism vocabulary from the Phase 1 spec. That
vocabulary produced 7 contested classifications out of 20, and the reason is
structural rather than a matter of judgment: its four values are not on the
same axis.

| original mechanism | what it actually measures |
|---|---|
| `LITERATURE_RECALL` | a **process** (retrieval) |
| `CONSTRUCTION` | a **certificate** (finite witness) |
| `NOVEL_ARGUMENT` | a certificate **plus** a frontier claim |
| `SEARCH_PLUS_VERIFIER` | a **process** (guided search) |

Asking whether Erdős #728 is `SEARCH_PLUS_VERIFIER` or `NOVEL_ARGUMENT` is
asking whether to describe it by its process or its product. There is no right
answer, which is why it wouldn't sit still. Split the axis and the question
dissolves: it is a **formal proof** (type) produced by **guided search**
(process), and both facts are recorded.

---

## Dimension 1 — CERTIFICATE TYPE

*What is the skeptic handed?* A property of the result, independent of who or
what produced it. This is the dimension that governs verification cost.

| value | what it is | cost to check |
|---|---|---|
| `WITNESS` | explicit finite object — a polynomial map, a point configuration, a decomposition | minutes; exact arithmetic |
| `FORMAL_PROOF` | machine-checked derivation in Lean or equivalent | compile time — but see the statement-audit caveat below |
| `INFORMAL_PROOF` | human-readable argument | hours to months of expert reading |
| `CITATION` | pointer to existing work that already answered it | time to read the referenced paper |
| `EVIDENCE` | numerics, patterns, computed small cases | **not a certificate**; establishes nothing alone |

Grounded in the constructive/nonconstructive distinction, which is the oldest
classification in this space and cuts exactly here: `WITNESS` is the
constructive case, `INFORMAL_PROOF` covers nonconstructive existence.

**The distinction press coverage always loses.** A counterexample sounds like a
`WITNESS` by definition. Erdős #90 was a counterexample whose exponent was
*inexplicit* — there was no finite object to hand anyone until Sawin supplied
one weeks later. Code the certificate that exists, not the one the problem's
logical form implies.

### `WITNESS` is only available on the refutation side

Proof by counterexample refutes a universal by proving the negated existential:

    ¬∀x P(x)  ≡  ∃x ¬P(x)

and an existential is proved the cheap way — exhibit one. In complexity terms,
refuting a Π₁ statement with a Σ₁ certificate. In Lakatos' vocabulary
(*Proofs and Refutations*) the Jacobian map is a **global counterexample**: it
kills the main conjecture outright, as opposed to a *local* counterexample that
only kills a lemma.

The asymmetry is absolute. **You can refute a universal by example; you can
never confirm one that way.** So the cheapest certificate type in this taxonomy
is structurally unavailable to any problem whose expected answer is "yes, the
conjecture holds."

This is visible in the corpus once you look for it. The frontier-advancing
results of 2026 are disproportionately *destructive* — Jacobian (false), unit
distance (false), sum-product (false, by humans following the same route). That
is not a fact about what models find easy. It is certificate economics: the
refutation side is where cheap certificates live, so that is where cheap
results appear.

**Scoring consequence.** Before any tractability axis is applied, the analyst
must record the problem's expected polarity:

| expected answer | cheapest available certificate |
|---|---|
| conjecture is **false** | `WITNESS` — in principle a minutes-long check |
| conjecture is **true** | `INFORMAL_PROOF` or `FORMAL_PROOF` — expert time or mathlib coverage |
| **independent** of ZFC | see below — the hazard case |

### The forcing trap (flagged here, resolved in Phase 3)

Set-theoretic consistency questions look like they inherit the good case. You
answer "is X consistent with ZFC" by **constructing a forcing notion** —
exhibiting a partial order P and showing it forces φ. That is witness-*shaped*.

But verifying that P forces φ is a proof, not a computation. There is no
determinant to evaluate, no collision to check. The exhibition is cheap and the
verification is not.

Objects that look like witnesses and verify like informal proofs may be the
worst combination in this taxonomy, because they invite the specific error of
believing the problem reduces to "find the right forcing." Phase 3 should test
this explicitly rather than assume it.

**The `FORMAL_PROOF` caveat.** Lean certifies that the proof follows from the
statement. It certifies nothing about whether the statement is the intended one,
whether the proof was found or supplied, or whether the theorem is already
known. All three residual risks are where the disputed cases in this corpus
live. See `axiom-fel-conjecture-disputed`.

## Dimension 2 — PRODUCTION PROCESS

*How did the certificate come to exist?* A property of the pipeline.

| value | what happened |
|---|---|
| `RETRIEVAL` | surfaced existing work from training data or literature |
| `DIRECT_GENERATION` | model emitted it in one or few shots, no external checker in the loop |
| `GUIDED_SEARCH` | iterated generation against an automatic evaluator (Lean, CAS, fitness function) |
| `TRANSLATION` | restated a supplied human argument in another formalism |
| `SEEDED_COLLABORATION` | model supplied a germ; humans built the result around it |
| `HUMAN_CONVENTIONAL` | no AI; present for the control cases |

`TRANSLATION` is **the category the original vocabulary was missing**, and on
the aggregate evidence it is the largest one in the real world: formalization is
category 2(b) on the teorth wiki with over 200 of ~600 logged entries. It is not
retrieval (nothing is recalled), not construction (no witness), not novel
argument (not novel), and only technically search-against-a-verifier — the
search is over tactics for a proof you were already handed.

---

## Flag A — FRONTIER

*Does it advance the state of the art?* Orthogonal to both dimensions, and the
corpus contains a case that separates it from everything else.

| value | meaning |
|---|---|
| `ADVANCES` | new to humanity |
| `MATCHES` | independently rediscovers something already known |
| `BEHIND` | correct, novel to the session, and weaker than published work |
| `SURFACES` | makes existing work findable without adding to it |

`gpt5pro-convex-optimization-bound` is why this flag exists. GPT-5 Pro produced
an argument that was **new** and **correct** and **behind the frontier** — the
1.75/L bound was already published; the model derived 1.5/L from scratch. Every
other scheme in this space, including the teorth wiki's 1(a)–1(d), measures
provenance and would score that case as a success.

A tractability rubric that fires on "a model will produce a correct new proof of
something already known" is worse than useless. It fires constantly, on exactly
the problems where it has no value.

## Flag B — INFERENCE MODE (Peirce)

From Zahavy, *Position: LLMs can't jump* (Google DeepMind, 2026), which argues
via Peirce's trichotomy that current systems do deduction and induction but not
abduction — the leap to a premise with no symbolic precedent.

| value | meaning |
|---|---|
| `TRANSPORT` | no inference; moving existing content between places or formalisms |
| `DEDUCTION` | derivation of consequences within fixed premises |
| `INDUCTION` | recombination and pattern extraction over the existing symbolic space |
| `ABDUCTION` | a new premise without precedent in that space |

**Read the resulting count carefully — this is the trap.** The corpus contains
zero `ABDUCTION` cases. That is *not* confirmation of Zahavy's thesis, because
the three human control cases (`erdos-52`, `sawin-unit-distance-explicit`, and
the human portion of `erdos-1026`) also code as non-abductive. Erdős-style
problems do not require new axioms **from anyone**. The corpus is drawn from a
culture where abduction is not the binding constraint, so it cannot test the
claim. It can only show that nothing here needed the jump.

Recording this honestly matters more than scoring a point with it.

## Flag C — GOWERS CULTURE

From Gowers, *The Two Cultures of Mathematics* (2000): mathematicians and
subjects divide by priority into **problem-solvers** and **theory-builders**.
Erdős is his exemplar of the first, Atiyah of the second.

| value | meaning |
|---|---|
| `PROBLEM_SOLVING` | Gowers' sense: "problems that it is reasonable to attack more or less from first principles" |
| `THEORY_BUILDING` | progress via accumulated machinery and expertise |
| `MIXED` | problem stated in one culture, resolved with the other's tools |

### Why this flag turned out to be the important one

Gowers on problem-solving fields (graph theory as his example):

> the interesting problems tend to be open precisely because the established
> techniques cannot easily be applied

Gowers on theory-building fields (algebraic number theory, geometry):

> progress is often the result of clever combinations of a wide range of
> existing results

Read those together against the corpus and the naive story inverts. **Broad
recall across a wide literature is the theory-builder's advantage.** It is also
the single thing a large model most reliably has, and the thing a specialist
human most reliably lacks.

Every cross-field transfer in the corpus is a model applying the theory-builder's
mode to a problem-solver's question:

- **Erdős #90** — unit distances (problem-solving) cracked with class field
  towers and Golod–Shafarevich (theory-building). Gowers himself, co-signing the
  digest: "many of the ideas needed for the proof were present in the literature
  already."
- **Erdős #1196** — primitive sets (problem-solving) cracked with Markov chains,
  a frame the paper says was "overlooked by the prior literature since Erdős's
  seminal 1935 paper."
- **Erdős #1026** — monotone subsequences cracked via square-packing results.

The tractable region is not "problem-solving culture." It is the **intersection**:
a problem stated in the problem-solving culture, so the certificate is cheap and
the statement is attackable from first principles, whose solution happens to sit
in some other field's existing machinery. Code that as `MIXED`.

That intersection is the rubric's actual target, and naming it is the main thing
Phase 2 gets from the classification literature.

---

## What the literature contributed, and what it didn't

| source | contributed | status |
|---|---|---|
| Peirce / Zahavy, *LLMs can't jump* | Flag B; the mechanism behind zero-theory-building | **untestable on this corpus** — see Flag B |
| Gowers, *Two Cultures* | Flag C; the intersection insight above | **adopted, load-bearing** |
| Constructive vs nonconstructive | the `WITNESS` / `INFORMAL_PROOF` split | adopted |
| HorizonMath (arXiv 2603.15617) | "discovery is hard, verification is efficient" — independently arrived at the certificate axis | corroborating |
| DeepMind Level 1–4 significance scale | precedent for a frontier flag | corroborating |
| Reverse mathematics — the Big Five<br>(RCA₀, WKL₀, ACA₀, ATR₀, Π¹₁-CA₀) | a real proof-theoretic difficulty ladder | **deferred to Phase 3** |

Reverse mathematics is the one to hold for the survey rather than the corpus.
Every case here sits at or below ACA₀ in strength; the ladder does no work
separating them. It becomes the natural instrument in the target territory,
where the question "which axioms does this need" *is* the question, and where
its set-theoretic analogue — the consistency-strength hierarchy — turns Greg's
"independence risk" axis from a vague hazard into a graded scale with a
literature behind it.
