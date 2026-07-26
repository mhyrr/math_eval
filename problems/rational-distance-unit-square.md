# Rational distance problem — a point at rational distance from all four corners

status: DEFINED
class: W — a yes is a single point with four rational distances

| | |
|---|---|
| statement from | `FormalConjectures/Wikipedia/RationalDistanceProblem.lean` @ `393aa9a` — PRIMARY |
| problem lineage | Guy, *Unsolved Problems in Number Theory*, D19 — https://doi.org/10.1007/978-0-387-26677-0 — PRIMARY |
| discussion | https://mathoverflow.net/questions/418260/ — EXPERT_COMMENTARY |
| tags | number theory, rational points, geometry |

## Statement

> Does there exist a point in the plane at rational distance from all four
> vertices of the unit square?

## The certificate

A **yes** is one point — in practice a rational point (x, y) with four
distances whose squares are rational squares — checked by exact arithmetic
instantly. A **no** is a theorem in the arithmetic of quartic surfaces.
Only the affirmative direction is finitely certifiable.

## Known, from the statement source

The Lean file carries the bare question with the Guy D19 and MathOverflow
references. Everything else — parametrizations, known partial results
(points at rational distance from three corners are classical), density
arguments, prior computational sweeps — belongs to the SEARCHED stage.

## Why it is in this folder

Guy-list problems are the pre-Erdős-database analogue of what this project
mines: old, concrete, certificate-shaped. This one has the smallest
possible certificate (one point), a live MathOverflow thread, and a
century-adjacent pedigree without the crowd of the famous conjectures. The
open question the SEARCHED stage must answer is how hard the search space
has already been hit — Guy problems attract amateur computation, and the
verification frontier is not recorded in the statement source.

## Before an attempt

- **G0 not discharged.** The D19 literature and the MathOverflow thread
  must be read; this problem transforms into rational points on a surface,
  and the arithmetic-geometry literature may contain conditional results
  (a Bombieri–Lang-style obstruction would move this to PARK the way #212's
  route died in Phase 4).
- **Feasibility:** naive search enumerates rational points by height; the
  ASSESSED stage needs the transformed surface and any published height
  bounds before estimating cost.
