#!/usr/bin/env python3
"""
Emit the Phase 4 survey rows from one reviewable scoring table.

    python3 scripts/mk_survey4.py            # write survey4/*.yaml
    python3 scripts/mk_survey4.py --check    # print the table, write nothing

WHY A GENERATOR. Same reason the taxonomy codings live in scripts/recode.py and
the retrodiction table in scripts/score.py: 56 hand-written YAML files drift
against each other, and the thing a reviewer needs to audit is the axis calls
side by side, not 56 files. Change a call here and re-run; the survey follows.

SCORED BLIND. Every row was scored from `scripts/blind.py` output (statement
with outcomes redacted, earliest available file version) plus the 2025-08-31
cutoff database. The reveal snapshot was not read. See memo/PREREGISTRATION.md.

--------------------------------------------------------------------------
THE B2 BAR -- fixed here BEFORE the reveal, because the survey's headline
number is a function of it and of nothing else.

RUBRIC.md anchors B2=3 as "mathlib covers the ambient theory (Kummer, p-adic
valuations); tasks decompose into subgoals" and B2=2 as "commutative algebra
present, this corner unformalized". Those two admit two readings, and on this
sample they are worth 27 percentage points:

  AMBIENT reading  -- is mathlib mature in the problem's *area*?     -> 36%
  TECHNIQUE reading -- is the specific proof machinery in mathlib?   ->  9%

The technique reading cannot be applied honestly to an open problem: nobody
knows what the proof uses. So the AMBIENT reading is primary, refined by one
mechanical rule that the anchors do support -- an asymptotic question needs
analytic machinery mathlib does not have, whatever its objects are:

  3  finite / arithmetic objects, ambient apparatus mature in mathlib
     (divisibility, congruences, binomials, sigma/omega, gcd-lcm, Q, Fintype,
     SimpleGraph), AND the question is not asymptotic
  2  objects definable but the question is asymptotic, or the area itself
     (Szemeredi-level additive combinatorics, sieve theory, extremal analysis)
     is unformalized
  1  incidence / discrete geometry in R^n -- essentially nothing

This yields 25%. memo/VALIDATION.md reports all three numbers, because the
spread between them IS the result about the instrument.
--------------------------------------------------------------------------
"""

import argparse
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "survey4"

CUTOFF_SHA = "009173b2"
FC_SHA = "5a60e068"

# Files whose EARLIEST version predates the cutoff -- statement source carries
# no post-cutoff information at all. The other 40 use a post-cutoff first
# version; that is a declared residual leak (survey4/SAMPLE.md).
PRECUTOFF = {"1", "3", "9", "11", "39", "41", "142", "200", "212", "213",
             "218", "244", "295", "304", "817", "868"}

# tag, title, form, attention, A, B1, B2, C, D, E, rationale, [extra claims]
#
# A  certificate cost      5 minutes-to-check .. 1 weeks-to-months
# B1 evaluator             5 exact checker IS the certificate .. 0 nothing
# B2 formal library        see the B2 bar above
# C  reservoir             5 another field's untried machinery .. 0 exhausted
# D  witness               5 answer is a finite object .. 0 about all models
# E  decision status       5 ZFC theorem, countable objects, reverse-math range
T = {
"1": ("additive combinatorics", "Erdos-Moser sum-distinct sets: is N >> 2^n?",
    "NEITHER", "HIGH", 2, 0, 2, 2, 1, 5,
    "Form is exists-C-forall-N, so no finite object confirms it and refuting needs "
    "an infinite family: D=1, B1=0. The file's own ladder (Erdos-Moser 1/4, then "
    "Elkies-Gleason sqrt(2/pi) unpublished) is Fourier-analytic work inside the "
    "field, so C=2. B2=2: Finset subset sums are in mathlib, the analytic apparatus "
    "behind the record constants is not.",
    ["$500 prize, read from the cutoff database, is the basis for prior_attention "
     "HIGH. Prize size is a proxy for Erdos's own difficulty estimate, not a "
     "measure of how much work the problem has actually received."]),
"3": ("number theory", "Erdos-Turan: does sum 1/n = infinity force arbitrarily long APs?",
    "UNIVERSAL", "HIGH", 1, 0, 2, 2, 1, 5,
    "A=1: any resolution is a landmark paper at or beyond Szemeredi's theorem. "
    "D=1 because a refuting set is infinite. B2=2 -- Szemeredi's theorem is not in "
    "mathlib and the ambient additive-combinatorial machinery is absent.",
    ["$5000 prize (cutoff database), the second largest in the corpus. I did not "
     "attempt to source an expert statement that this awaits a new framework, so "
     "G1 is null rather than false even though that is the likely call."]),
"7": ("number theory", "Is there a covering system with all moduli odd and > 1?",
    "EXISTENTIAL", "HIGH", 5, 4, 3, 2, 5, 5,
    "The clearest Route W shape in the sample. A witness is a finite set of "
    "congruences (D=5) and checking coverage is exact CRT arithmetic over the "
    "period (B1=4, the funsearch-cap-set anchor: cheap evaluator over explicit "
    "finite objects). A=5 follows. B2=3: congruences and CRT are mature in mathlib "
    "and the question is not asymptotic.",
    ["Route W clearance here says the certificate is cheap IF one exists and is "
     "found. The search space over odd moduli is astronomically large and the "
     "problem has resisted since the 1950s. CANDIDATE means 'in the ~2% pool', "
     "which is exactly the caveat this row needs."]),
"9": ("additive basis", "Positive upper density of odd n not equal to a prime plus two powers of 2?",
    "NEITHER", "HIGH", 3, 1, 2, 2, 1, 5,
    "A density question, so no finite witness (D=1) and no evaluator (B1=1). "
    "B2=2 under the fixed bar: the objects (primes, powers of 2, upper density) are "
    "in mathlib but the question is asymptotic. C=2 -- covering congruences are the "
    "field's own tool and are already deployed here.",
    ["The file credits Schinzel, via Erdos [Er77c], with infinitude of the set but "
     "records that Erdos 'gives no reference'. I could not chase that citation "
     "under the blindness protocol."]),
"11": ("additive basis", "Is every odd n the sum of a squarefree number and a power of 2?",
    "UNIVERSAL", "HIGH", 5, 5, 3, 2, 5, 5,
    "Scored on FORM, per the rubric's Jacobian trap: the expected answer is yes, "
    "but the statement is universal, so a counterexample would be a single integer "
    "n. Checking it is exact and cheap -- test squarefreeness of n - 2^l for each "
    "l -- giving B1=5, D=5, A=5. B2=3: Squarefree and powers are mature in mathlib "
    "and the statement is not asymptotic.",
    ["The file records verification to 10^7 (Odlyzko) and 2^50 (Hercher). Route W "
     "clears on form, but the finite-witness search has already been run through "
     "roughly 1.1e15 without a hit, which is evidence against the route paying "
     "here even though it does not change the axis call."]),
"14": ("sidon sets", "Unique-representation sums: is |{1..N} \\ B| >> N^(1/2-eps)?",
    "UNIVERSAL", "HIGH", 2, 0, 2, 2, 1, 5,
    "Asymptotic on both sides: refutation is an infinite set A, so D=1 and B1=0. "
    "Erdos-Fuchs territory, worked from inside the field, C=2.", []),
"32": ("additive basis", "Additive complement to the primes of size o((log N)^2)?",
    "EXISTENTIAL", "HIGH", 2, 0, 2, 3, 2, 5,
    "Existential but the witness is an infinite set; Erdos's own construction is "
    "given by a finite recipe, which is the sawin-unit-distance anchor's shape, so "
    "D=2 rather than 1. C=3: Ruzsa's e^gamma lower bound came from analytic methods "
    "adjacent to the combinatorial framing.", []),
"39": ("sidon sets", "Infinite Sidon set with |A cap [1,N]| >> N^(1/2-eps)?",
    "EXISTENTIAL", "HIGH", 2, 0, 2, 3, 2, 5,
    "$500 prize, so attention HIGH. The witness is an infinite Sidon set given by a "
    "construction (D=2). C=3 -- the best constructions have come from algebraic and "
    "probabilistic imports rather than from Sidon-set technique alone.", []),
"41": ("sidon sets", "Triple-sum-distinct sets: is liminf |A cap [1,N]|/N^(1/3) = 0?",
    "UNIVERSAL", "HIGH", 2, 0, 2, 2, 1, 5,
    "$500 prize. Universal over infinite sets A, asymptotic conclusion: D=1, B1=0.", []),
"42": ("sidon sets", "Every maximal Sidon set admits a size-M Sidon set with disjoint differences?",
    "UNIVERSAL", "LOW", 3, 3, 2, 2, 5, 5,
    "A refutation is a finite object -- a specific N, M and maximal Sidon set A -- "
    "so D=5. But certifying it means showing NO Sidon B of size M has disjoint "
    "differences, an exhaustive search over subsets of [1,N]. That is the 'expensive "
    "exact check' anchor, B1=3, which blocks Route W. attention LOW: the file cites "
    "no literature beyond the problem page.",
    ["prior_attention LOW rests on the absence of references in the Lean file, which "
     "is weak evidence -- the repo's reference blocks are uneven in depth."]),
"61": ("graph theory", "Erdos-Hajnal conjecture: polynomial clique or independent set in H-free graphs",
    "UNIVERSAL", "HIGH", 1, 0, 2, 3, 1, 5,
    "A major named conjecture with a documented ladder (Erdos-Hajnal 1989 "
    "exp(c sqrt(log n)), Bucic-Nguyen-Scott-Seymour a loglog step). The gap to n^c "
    "is enormous, so A=1. D=1: a refuting H needs an asymptotic argument, not an "
    "object. B2=2 -- SimpleGraph is in mathlib, Ramsey-type machinery is not.", []),
"85": ("graph theory", "Is f(n+1) >= f(n), where f forces a C4 by minimum degree?",
    "UNIVERSAL", "LOW", 4, 3, 3, 2, 5, 5,
    "A counterexample is a single n (D=5), but certifying it means computing f at "
    "two points, each a search over all graphs on n vertices -- exact and "
    "superexponential, so B1=3 and Route W blocks. B2=3: minDegree, cycleGraph and "
    "subgraph containment are mature in mathlib and the statement is a finite "
    "monotonicity claim, not an asymptotic one. attention LOW -- the file cites no "
    "literature and the TODO suggests it is not a worked problem.", []),
"89": ("distances", "Distinct distances: does every n-point planar set determine >> n/sqrt(log n)?",
    "UNIVERSAL", "HIGH", 1, 0, 1, 3, 1, 5,
    "Guth-Katz got n/log n with the polynomial method; the remaining sqrt(log n) is "
    "the famous last gap, so A=1. B2=1: incidence geometry in R^2 has essentially no "
    "mathlib presence. C=3 -- the last big advance was itself an import (algebraic "
    "geometry), but that reservoir has now been tapped.", []),
"96": ("distances", "Do convex n-gon vertices determine O(n) unit distances?",
    "UNIVERSAL", "HIGH", 2, 1, 1, 3, 2, 5,
    "Refutation is a family of convex polygons, not a single one, so D=2. B2=1 for "
    "the same reason as #89.", []),
"100": ("distances", "Distances separated by >= 1: is the diameter >> n?",
    "NEITHER", "HIGH", 2, 2, 1, 3, 2, 5,
    "B1=2 rather than 0 because candidate configurations are finite point sets and "
    "the file records one (Piepmeyer, 9 points, diameter < 5) -- but that certifies "
    "small cases, not the linear lower bound, so D=2 and Route W blocks.", []),
"138": ("additive combinatorics", "Van der Waerden numbers: does W(k)^(1/k) tend to infinity?",
    "NEITHER", "HIGH", 2, 0, 2, 2, 1, 5,
    "A limit statement, D=1. The file's ladder (Berlekamp lower bound, Gowers upper) "
    "shows a gap of several exponentials. B2=2: van der Waerden numbers are not in "
    "mathlib and Gowers's bound certainly is not.", []),
"142": ("additive combinatorics", "Asymptotic formula for r_k(N), the largest AP-free subset",
    "NEITHER", "HIGH", 1, 0, 2, 2, 1, 5,
    "$10000, the largest prize in the database. Asks for a formula, so D=1 and A=1.",
    ["G2 is marked true, but 'prove an asymptotic formula' is the weakest "
     "verification standard in the sample -- the Lean file itself leaves the answer "
     "as an unknown function. A reader could reasonably score G2 false here."]),
"152": ("sidon sets", "Sidon sumsets: must the isolated-element count tend to infinity?",
    "NEITHER", "LOW", 3, 0, 2, 2, 1, 5,
    "A limit statement over Sidon sets, D=1. attention LOW -- one reference "
    "([ESS94]) and no later work in the file. A=3: a resolution would be a normal "
    "paper in a mature elementary area.", []),
"153": ("sidon sets", "Sidon sumsets: must the normalised squared-gap sum tend to infinity?",
    "NEITHER", "LOW", 3, 0, 2, 2, 1, 5,
    "Same family and same reference as #152; scored identically.", []),
"170": ("additive combinatorics", "Perfect rulers: determine lim F(N)/sqrt(N)",
    "NEITHER", "LOW", 3, 4, 2, 2, 1, 5,
    "B1=4 is real -- a candidate ruler is an explicit finite set with a cheap exact "
    "check, and F(N) is computable by finite search. But the problem asks for the "
    "value of a limit, which is not a finite object, so D=1 and Route W blocks. This "
    "is the cleanest case in the sample of a good evaluator attached to a question "
    "that is not witnessable. attention LOW: Leech 1956 and Wichmann 1963, nothing "
    "since in the file.", []),
"184": ("graph theory", "Erdos-Gallai: can every n-vertex graph be decomposed into O(n) cycles and edges?",
    "NEITHER", "HIGH", 2, 0, 2, 3, 1, 5,
    "Active recent work (Conlon-Fox-Sudakov, Bucic-Montgomery n log* n), so "
    "attention HIGH and the reservoir is being drawn on already: C=3. D=1 -- a "
    "refutation is an asymptotic statement about a family.", []),
"200": ("primes", "Is the longest AP of primes in [1,N] of length o(log N)?",
    "NEITHER", "HIGH", 2, 0, 2, 3, 1, 5,
    "Green-Tao adjacent, attention HIGH. Asymptotic, D=1. B2=2: mathlib has primes "
    "and now the prime number theorem, but not the machinery this needs.", []),
"212": ("distances", "Erdos-Ulam: is there a dense subset of R^2 with all pairwise distances rational?",
    "EXISTENTIAL", "HIGH", 2, 1, 1, 4, 2, 5,
    "The one Route R clearance in the sample. C=4 on the erdos-90 pattern: a problem "
    "posed in discrete geometry whose known conditional resolution comes from "
    "arithmetic geometry, a field that was not asked. D=2 -- the witness is a dense "
    "hence infinite set, though a construction would be finitely described. B2=1.",
    ["C=4 rests on my recollection that Erdos-Ulam follows from the Bombieri-Lang "
     "conjecture. That is decades-old classical knowledge rather than a post-cutoff "
     "fact, but the blindness protocol barred me from fetching a source for it, and "
     "the Lean file does not mention it. This is the single least-sourced axis call "
     "in the survey and it is the only thing holding the sample's Route R "
     "clearance."]),
"213": ("distances", "n points, no 3 collinear, no 4 concyclic, all distances integers",
    "UNIVERSAL", "HIGH", 4, 4, 1, 3, 3, 5,
    "B1=4: a candidate configuration is exactly checkable in rational or algebraic "
    "arithmetic. But the statement quantifies over all n >= 4, so the answer is a "
    "construction scheme, not one object -- D=3, the sawin anchor, and Route W "
    "blocks. That gap between B1 and D is the same split v1.1 introduced for "
    "ch-erdos-918. The file's best construction is n = 7 (Kreisel-Kurz).", []),
"218": ("primes", "Prime gaps: density 1/2 for d_n <= d_(n+1), and infinitely many equal consecutive gaps",
    "NEITHER", "HIGH", 2, 0, 2, 2, 1, 5,
    "Density statements about prime gaps, D=1. B2=2: the ambient theory is analytic "
    "and sieve-theoretic, not in mathlib. The file notes the third part is "
    "equivalent to infinitely many 3-term APs in prime gaps, which places it near "
    "known-hard territory.", []),
"241": ("sidon sets", "Bose-Chowla: is f(N) ~ N^(1/3) for triple-sum-distinct sets?",
    "NEITHER", "HIGH", 2, 0, 2, 3, 1, 5,
    "$100 prize; Bose-Chowla lower bound and Green's upper bound bracket it, so the "
    "field is active and C=3. Asymptotic, D=1.", []),
"244": ("primes", "Does {p + floor(C^k)} have positive density?",
    "NEITHER", "LOW", 3, 0, 2, 3, 1, 5,
    "The Romanoff analogue with C^k replaced by floor(C^k). C=3 -- Romanoff's "
    "theorem and its sieve apparatus are the adjacent machinery. B2=2: sieve theory "
    "is not in mathlib. attention LOW, one reference in the file.", []),
"282": ("unit fractions", "Does the odd greedy Egyptian-fraction algorithm always terminate?",
    "UNIVERSAL", "HIGH", 3, 2, 3, 2, 2, 5,
    "B1=2: the algorithm runs, so you can gather evidence, but non-termination is "
    "not finitely certifiable -- evidence is not a certificate, per TAXONOMY.md. "
    "D=2. B2=3 under the fixed bar: the objects are rationals and the question is "
    "not asymptotic. Graham's theorems give the field real structure, attention "
    "HIGH.", []),
"288": ("unit fractions", "Only finitely many interval pairs with integral reciprocal sum?",
    "NEITHER", "LOW", 3, 1, 3, 2, 1, 5,
    "Finiteness claims are not finitely certifiable in either direction, D=1. "
    "B2=3: harmonic sums over intervals are elementary rational arithmetic, mature "
    "in mathlib, and the question is not asymptotic. attention LOW -- no references "
    "in the file at all.", []),
"291": ("unit fractions", "Is gcd(a_n, L_n) = 1 for infinitely many n?",
    "NEITHER", "HIGH", 2, 0, 3, 3, 1, 5,
    "B2=3 and this is the closest row in the sample to the erdos-728 anchor: the "
    "file's own Steinerberger generalisation is base-p digit reasoning plus "
    "Wolstenholme, which is exactly the Kummer / p-adic-valuation shape mathlib "
    "covers. Attention HIGH -- Shiu, Wu-Yan and Steinerberger all appear. D=1: an "
    "infinitude claim.",
    ["Whether mathlib specifically carries Wolstenholme's theorem and a harmonic-"
     "number API is the load-bearing question for B2=3 here, and I could not check "
     "mathlib under the blindness protocol. If it does not, this row drops to B2=2 "
     "and loses its Route F clearance."]),
"295": ("unit fractions", "Does k(N) - (e-1)N tend to infinity?",
    "NEITHER", "LOW", 3, 0, 2, 2, 1, 5,
    "B2=2, not 3: the question is asymptotic and the Erdos-Straus bounds it sits "
    "between are analytic. D=1. attention LOW.", []),
"304": ("unit fractions", "Is N(b) << log log b?",
    "NEITHER", "HIGH", 2, 0, 2, 3, 1, 5,
    "Asymptotic, so B2=2 under the fixed bar despite the elementary objects. The "
    "file's ladder is Erdos 1950 (log b / log log b) then Vose 1985 (sqrt(log b)) "
    "against a log log b target -- a real gap, A=2. C=3.", []),
"312": ("unit fractions", "Subset of a reciprocal-heavy multiset summing close to 1",
    "NEITHER", "LOW", 3, 1, 2, 2, 1, 5,
    "Exists-c-forall-K form, so no finite witness: D=1. B2=2 -- asymptotic in K. "
    "attention LOW: the file carries no references.", []),
"317": ("unit fractions", "Small nonzero signed reciprocal sums below c/2^n",
    "NEITHER", "LOW", 4, 4, 3, 2, 3, 5,
    "B1=4: for a fixed n a candidate sign vector is exactly checkable in rational "
    "arithmetic, cheaply. D=3 rather than 5 because the statement is exists-c-"
    "forall-n, so the answer is a scheme producing a vector for every n -- the "
    "sawin anchor again, and Route W blocks on it. B2=3: lcm and Q arithmetic are "
    "mature and the bound is explicit rather than asymptotic.", []),
"330": ("additive basis", "Minimal basis of positive density: is the unrepresentable set of positive density?",
    "UNIVERSAL", "LOW", 3, 0, 2, 2, 1, 5,
    "Density conclusion, D=1. attention LOW -- no references in the file.", []),
"342": ("number theory", "Ulam sequence: pairs (a, a+2), eventual periodicity of differences, density",
    "NEITHER", "HIGH", 2, 2, 2, 3, 1, 5,
    "B1=2: the sequence is computable and there is a great deal of numerical "
    "evidence, which is EVIDENCE in the taxonomy's sense and certifies nothing. "
    "B2=2: the objects are elementary but all three parts are asymptotic, and there "
    "is essentially no theory of the Ulam sequence to formalise. attention HIGH -- "
    "the file cites Ben Green's open-problem list.", []),
"455": ("number theory", "Monotone prime gaps: must q_n / n^2 tend to infinity?",
    "UNIVERSAL", "LOW", 3, 0, 2, 2, 1, 5,
    "Asymptotic, D=1, B2=2. attention LOW: one reference (Richter 1976) and nothing "
    "after it in the file.", []),
"503": ("distances", "Largest isosceles set in R^n",
    "NEITHER", "HIGH", 4, 4, 1, 3, 3, 5,
    "B1=4 -- a candidate isosceles set is exactly checkable -- but the problem asks "
    "for the extremal size as a function of n, so D=3 and Route W blocks. The known "
    "bracket is Alweiss's C(n+1,2) lower against Blokhuis's C(n+2,2) upper, a narrow "
    "gap, hence A=4. B2=1: R^n distance geometry is not in mathlib.", []),
"539": ("number theory", "Estimate h(n), the cofactor threshold, between sqrt(n) and n^(2/3)",
    "NEITHER", "LOW", 3, 0, 2, 3, 1, 5,
    "Asymptotic Theta estimate, so B2=2 despite gcd and Finset being mature. C=3 -- "
    "Granville-Roesler's upper bound came from outside the original framing. "
    "attention LOW: two references, decades apart.", []),
"600": ("graph theory", "Ruzsa-Szemeredi: does e(n,r+1) - e(n,r) tend to infinity?",
    "NEITHER", "HIGH", 2, 0, 2, 3, 1, 5,
    "The (6,3)-problem's neighbourhood; attention HIGH. Asymptotic, D=1. B2=2: "
    "regularity-method machinery is absent from mathlib.", []),
"617": ("graph theory", "Erdos-Gyarfas: r-colourings of K_(r^2+1) and a missing colour on r+1 vertices",
    "UNIVERSAL", "LOW", 5, 4, 3, 2, 5, 5,
    "The strongest Route W row that is also cheap. A counterexample is a single "
    "colouring of a single finite complete graph (D=5) and checking it means "
    "scanning the (r+1)-subsets -- exact, and trivial for the small r where the "
    "problem is open (B1=4, A=5). B2=3: Sym2, Fintype and finite colourings are "
    "mature in mathlib and nothing here is asymptotic. attention LOW: the file cites "
    "only Erdos-Gyarfas 1999, which settled r=3 and r=4, and nothing since.",
    ["prior_attention LOW is inferred from a single reference in the Lean file. If "
     "this problem has in fact been attacked computationally since 1999, the LOW "
     "call is wrong and the conditional Route N count should not include it. Route W "
     "clearance does not depend on it."]),
"619": ("graph theory", "Triangle-free graphs: is h_4(G) < (1-c)n?",
    "NEITHER", "LOW", 3, 2, 3, 2, 2, 5,
    "B2=3: finite graphs, diameter and triangle-freeness are all mature in mathlib "
    "and the claim is a linear bound on finite objects rather than an asymptotic "
    "formula. B1=2 -- computing h_4(G) for a given G is itself a hard search. D=2: "
    "a refutation is a family. attention LOW: Erdos-Gyarfas-Ruszinko 1998 and "
    "Erdos 1999, nothing since in the file.", []),
"653": ("distances", "Distinct-distance multiplicities: is g(n) >= (1-o(1))n?",
    "NEITHER", "LOW", 3, 1, 1, 3, 1, 5,
    "Asymptotic, D=1, and B2=1 as for the rest of the distances stratum. attention "
    "LOW -- no references in the file.", []),
"681": ("primes", "For large n, is there k with n+k composite and least prime factor > k^2?",
    "UNIVERSAL", "LOW", 4, 4, 3, 3, 3, 5,
    "B1=4: for a given n, a candidate k is checked by factoring n+k -- exact and "
    "cheap. D=3 because the statement is for-all-large-n, so the answer is a scheme "
    "rather than one object, and Route W blocks. B2=3: least prime factor and "
    "compositeness are mature in mathlib and the statement is not asymptotic. "
    "attention LOW -- the file carries no references at all.", []),
"789": ("additive combinatorics", "Estimate h(n), the subset-sum separating threshold",
    "NEITHER", "LOW", 3, 0, 2, 3, 1, 5,
    "Theta estimate between (n log n)^(1/3) and sqrt(n), so asymptotic: B2=2, D=1. "
    "attention LOW -- Straus 1966, Erdos 1962, Choi 1974, and nothing since.", []),
"817": ("additive combinatorics", "Is g_3(n) >> 3^n?",
    "UNIVERSAL", "LOW", 3, 0, 2, 2, 1, 5,
    "Asymptotic lower bound; the file records Erdos-Sarkozy's 3^n/n^O(1). D=1. "
    "attention LOW.", []),
"830": ("number theory", "Are there infinitely many amicable pairs?",
    "NEITHER", "HIGH", 2, 1, 3, 2, 1, 5,
    "B2=3: sigma is mature in mathlib and part (i) is a plain infinitude claim about "
    "divisor sums, not an asymptotic one. B1=1 -- individual amicable pairs are "
    "trivially checkable but that certifies nothing about infinitude, and D=1. "
    "attention HIGH: a classical problem with Pomerance-grade upper bounds in the "
    "file.",
    ["Part (ii) of this problem (A(x) > x^(1-o(1))) is asymptotic and would score "
     "B2=2 on its own. The row is scored on part (i), which is the headline "
     "question; a reader splitting the parts would score them differently."]),
"835": ("graph theory", "Is there k > 2 whose k-subsets of a 2k-set admit a rainbow (k+1)-colouring?",
    "EXISTENTIAL", "HIGH", 5, 4, 2, 3, 5, 5,
    "The purest FunSearch shape in the sample: an existential whose witness is a "
    "single finite colouring (D=5) with an exact, cheap, monotone-scoreable "
    "evaluator -- count the (k+1)-subsets that see all colours (B1=4, A=5). Route W "
    "clears. B2=2, not 3: Johnson graphs and their chromatic numbers are defined in "
    "the conjectures repo, not in mathlib. C=3 -- coding theory supplies the Johnson "
    "bound and is already being used. attention HIGH: a maintained table of known "
    "cases plus Ma-Tang's result.", []),
"847": ("additive combinatorics", "Does few-3APs force a finite union of 3AP-free sets?",
    "UNIVERSAL", "LOW", 3, 0, 2, 3, 1, 5,
    "Universal over infinite sets with an infinite-decomposition conclusion, D=1. "
    "attention LOW -- no references in the file.", []),
"849": ("number theory", "For every t, is there a with exactly t solutions to C(n,k) = a?",
    "UNIVERSAL", "HIGH", 4, 4, 3, 3, 3, 5,
    "Singmaster's neighbourhood. B1=4: for a candidate a the solution set is "
    "effectively boundable and checkable. D=3, not 5, because the claim is for every "
    "t -- one a settles one t, so the answer is a scheme, and Route W blocks. B2=3: "
    "Nat.choose is mature in mathlib and the statement is not asymptotic.", []),
"853": ("primes", "Does r(x), the least even gap not yet attained, tend to infinity?",
    "NEITHER", "LOW", 3, 0, 2, 2, 1, 5,
    "B2=2: prime-gap distribution is analytic territory mathlib does not cover. "
    "D=1. attention LOW -- no references in the file.", []),
"855": ("primes", "Second Hardy-Littlewood conjecture in eventually-form: pi(x+y) <= pi(x) + pi(y)",
    "UNIVERSAL", "HIGH", 2, 1, 2, 3, 1, 5,
    "Universal in form, but the 'eventually' quantifiers mean a single pair (x,y) "
    "does not refute it -- D=1, and this is the sample's clearest case of statement "
    "form promising a witness the statement's quantifiers then withdraw. attention "
    "HIGH: a named Hardy-Littlewood conjecture.", []),
"868": ("additive basis", "Must a basis of order 2 with f(n) -> infinity contain a minimal basis?",
    "UNIVERSAL", "LOW", 3, 0, 2, 2, 1, 5,
    "The witness B is an infinite set, D=1. attention LOW: Erdos-Nathanson and "
    "Hartter-Nathanson, then nothing in the file.", []),
"871": ("additive basis", "Can such a basis be partitioned into two disjoint bases of order 2?",
    "UNIVERSAL", "LOW", 3, 0, 2, 2, 1, 5,
    "Same family as #868, same shape, scored identically. The file records "
    "Erdos-Nathanson 1988 and 1989.", []),
"881": ("additive basis", "Minimal basis of order k: is there infinite B with A \\ B a basis of order k+1?",
    "UNIVERSAL", "LOW", 3, 0, 2, 2, 1, 5,
    "Same family again; the witness is an infinite subset, D=1. attention LOW -- no "
    "references in the file.", []),
"890": ("primes", "Erdos-Selfridge: liminf of summed omega over k consecutive integers",
    "NEITHER", "LOW", 3, 0, 2, 2, 1, 5,
    "liminf and limsup statements, D=1, and the ambient (Polya on smooth numbers, "
    "omega asymptotics) is analytic, B2=2. attention LOW: Erdos-Selfridge 1967 and "
    "nothing since in the file.", []),
}

AXES = ["certificate_cost", "evaluator_availability", "formal_library_coverage",
        "reservoir_proximity", "witness_representability", "independence_risk"]

# Every row carries these. Stated once here rather than copy-pasted 56 times.
STANDING_CLAIMS = [
    "G0 is discharged NOMINALLY, from the cutoff database's `open` state at "
    "2025-08-31, not by a literature search. That is exactly the list status "
    "Bloom described -- a statement about the list's coverage, not humanity's "
    "knowledge -- so g0_checked_rigorously is false and Route N cannot clear.",
    "G1 (does resolution require extensive new theory?) is null. It is not "
    "answerable for an open problem from a redacted Lean file and a status "
    "database, in either direction, and guessing it would be the whole result. "
    "Under --optimistic this is the only gate being granted.",
    "Axis C is the least sourceable axis under the blindness protocol. Scoring "
    "'is there untried machinery in an adjacent field' properly needs a "
    "literature survey per problem; these calls are judgements against the "
    "RUBRIC.md anchors from the references visible in the Lean file.",
]

TMPL = """\
# Phase 4 hold-out row. GENERATED by scripts/mk_survey4.py -- edit the table
# there, not this file. Scored blind against the 2025-08-31 cutoff; the reveal
# snapshot was not read. See memo/PREREGISTRATION.md.
id: {id}
area: {area}
title: "Erdos #{n} -- {title}"
statement: >
  {title}

sources:
  - url: https://github.com/google-deepmind/formal-conjectures/blob/{fc}/FormalConjectures/ErdosProblems/{n}.lean
    what: "Lean statement, formal-conjectures at pinned commit {fc}. Read via
      scripts/blind.py, which serves the EARLIEST version of the file ({era})
      with the category attribute, every answer(...) payload, post-cutoff
      bibliography entries and outcome prose redacted. The unredacted file
      carries the resolution status and was not read."
    tier: PRIMARY
  - url: https://github.com/teorth/erdosproblems/blob/{cut}/data/problems.yaml
    what: "Cutoff snapshot of the community database, commit {cut}
      (2025-08-31T23:55:32Z). Source for tags, prize and the `open` state used
      to discharge G0 nominally. Contains no outcome information."
    tier: PRIMARY
  - url: https://www.erdosproblems.com/{n}
    what: "Canonical problem page. NOT FETCHED -- the site returns 403 to
      automated requests (CLAUDE.md), and it would in any case show the current
      status. Listed as the pointer a reader should follow, not as a source
      consulted."
    tier: PRIMARY

# --- gates ---
open_to_field: true
no_new_theory_required: null
verification_standard_exists: true
g0_checked_rigorously: false

form: {form}
prior_attention: {attention}

# --- axes ---
certificate_cost: {A}
evaluator_availability: {B1}
formal_library_coverage: {B2}
reservoir_proximity: {C}
witness_representability: {D}
independence_risk: {E}

formalizable_without_ambiguity: true

axis_rationale: >
  {rationale}

unverifiable_claims:
{claims}"""


def wrap(text, indent, width=76):
    words, lines, cur = text.split(), [], indent
    for w in words:
        if len(cur) + len(w) + 1 > width and cur.strip():
            lines.append(cur.rstrip())
            cur = indent + w + " "
        else:
            cur += w + " "
    if cur.strip():
        lines.append(cur.rstrip())
    return "\n".join(lines).lstrip()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--check", action="store_true")
    a = ap.parse_args()

    if a.check:
        print(f"{'#':>5} {'tag':<24} {'form':<12} {'att':<5} "
              f"{'A':>2} {'B1':>2} {'B2':>2} {'C':>2} {'D':>2} {'E':>2}")
        for n, r in sorted(T.items(), key=lambda kv: int(kv[0])):
            print(f"{n:>5} {r[0]:<24} {r[2]:<12} {r[3]:<5} "
                  f"{r[4]:>2} {r[5]:>2} {r[6]:>2} {r[7]:>2} {r[8]:>2} {r[9]:>2}")
        print(f"\n{len(T)} rows")
        return

    OUT.mkdir(exist_ok=True)
    for n, r in T.items():
        tag, title, form, att, A, B1, B2, C, D, E, rationale, extra = r
        claims = "\n".join(f"  - \"{wrap(c, '    ')}\""
                           for c in (extra + STANDING_CLAIMS))
        body = TMPL.format(
            id=f"e{n}-{tag.replace(' ', '-')}", area=tag, n=n, title=title,
            fc=FC_SHA, cut=CUTOFF_SHA,
            era="pre-cutoff" if n in PRECUTOFF else "post-cutoff; declared leak",
            form=form, attention=att, A=A, B1=B1, B2=B2, C=C, D=D, E=E,
            rationale=wrap(rationale, "  "), claims=claims)
        (OUT / f"e{n}-{tag.replace(' ', '-')}.yaml").write_text(body + "\n")
    print(f"wrote {len(T)} rows to {OUT.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
