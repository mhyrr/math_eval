# R(3,10) — the nearest open Ramsey number

status: DEFINED
class: W — finite certificate in both directions; a one-value gap

| | |
|---|---|
| authoritative record | Radziszowski, *Small Ramsey Numbers*, dynamic survey DS1, **revision #18, 2026-04-24** — https://www.cs.rit.edu/~spr/ElJC/sur.pdf — PRIMARY |
| survey landing page | https://www.combinatorics.org/ojs/index.php/eljc/article/view/DS1 — PRIMARY |
| named as a target by | Li–Duggan–Bright–Ganesh, IJCAI 2025 — https://arxiv.org/html/2502.06055 — PRIMARY |
| tags | ramsey theory, graph theory |

## Statement

> Determine R(3,10): the smallest N such that every 2-colouring of the edges
> of K_N contains a red triangle or a blue K₁₀.

Current bounds: **40 ≤ R(3,10) ≤ 41** (DS1 rev. #18, Table Ia). This is the
smallest classical Ramsey number whose value is unknown, and the gap is a
single integer.

## The certificate

- **R(3,10) = 41** is witnessed by one graph: a triangle-free graph on 40
  vertices with no independent set of size 10. Checking it is polynomial
  and trivial.
- **R(3,10) = 40** is an exhaustion: no such graph on 40 vertices exists.
  The modern form is a SAT UNSAT certificate — large but mechanically
  checkable, exactly the artifact Li–Duggan–Bright–Ganesh produced for
  R(3,8) and R(3,9) in 2025 (R(3,9): 2,486 cubes, ~26 h wall-clock, 289 GiB
  of proof).

Either answer closes the problem with a finite, checkable object. That is
rarer than it sounds: most Ramsey gaps are ranges, not coin flips.

## Known, from the record

- DS1 rev. #18 note (d): more than **43 million (3,10)-graphs on 39
  vertices** are already catalogued — the witness pool for the lower bound
  is enormous and thoroughly mined, which is evidence *against* an easy
  40-vertex witness existing.
- The SAT+CAS group that certified R(3,8) and R(3,9) names R(3,10) first in
  its list of open targets: "determining the values of R(3,10), R(4,6), or
  R(5,5), which remain open problems" (IJCAI 2025 paper, PRIMARY).
- Neighbouring frontier, same survey: R(4,6) ∈ [36, 40] (Table Ib carries
  an unpublished Angeltveit–McKay ≤ 40; the published Table Ia says 41);
  R(5,5) ∈ [43, 46] after Angeltveit–McKay 2024
  (https://arxiv.org/abs/2409.15709, PRIMARY — LP plus case checking,
  independently implemented twice, not a SAT proof).

## Why it is in this folder

Not neglect — this is one of the most attended computational problems in
combinatorics. It is here because the certificate shape is perfect, the gap
is one value, and the group with the working pipeline has publicly queued
it. The honest framing: this is the field's own next domino, and the open
question for us is whether an LLM-assisted attack adds anything beyond what
the SAT+CAS pipeline will do on its own schedule.

## Before an attempt

- **G0 substantially discharged at definition time** by DS1 rev. #18
  (2026-04-24) — the survey *is* the literature search for status. A full
  SEARCHED pass should still check for post-April-2026 announcements and
  any bbchallenge-style community effort.
- **Feasibility:** the R(3,9) certificate cost 289 GiB; R(3,10) is one
  exponential step up. The ASSESSED stage should get a cost estimate from
  the IJCAI 2025 paper's scaling data before recommending anything.
