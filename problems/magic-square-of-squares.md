# 3×3 magic square of squares — nine distinct squares, eight equal sums

status: DEFINED
class: W — a yes is nine integers

| | |
|---|---|
| statement from | `FormalConjectures/Wikipedia/MagicSquares.lean` @ `393aa9a` — PRIMARY |
| overview | https://en.wikipedia.org/wiki/Magic_square_of_squares — EXPERT_COMMENTARY |
| record site | http://www.multimagie.com/English/SquaresOfSquaresSearch.htm — the community search-frontier page; **TLS-broken to automated fetch as of 2026-07-26**, needs a browser |
| tags | number theory, diophantine |

## Statement

> Does there exist a 3×3 matrix of nine distinct positive perfect squares
> whose three rows, three columns, and both diagonals all sum to the same
> value?

The same Lean file carries a companion open question: a 3×3 *semi-magic*
square (rows and columns only) of nine distinct positive cubes.

## The certificate

A **yes** is nine integers — the single most famous "one tweet refutes it"
shape in recreational number theory. A **no** is a theorem about rational
points on a K3-adjacent surface. Only the affirmative direction is finitely
certifiable.

## Known, from the record

- Near-misses with seven of eight sums correct exist (the Parker square is
  the popular mascot; Bremner and others have serious papers). Specifics
  belong to SEARCHED.
- The multimagie.com community has run large sweeps and offers prizes; the
  exact current search bound must be read from that site in a browser —
  the automated-fetch failure is recorded above so nobody mistakes fetch
  failure for absence of a frontier.

## Why it is in this folder

Included as the *high-attention control* in the W class: certificate shape
is perfect, but amateur computation has hammered it for decades, so the
neglect mechanism is fully absent. If the folder's doctrine is right, this
problem should score well on shape and badly on G0 — it is here partly to
keep the entry criteria honest. The cubes variant is less searched and may
be the better target of the two.

## Before an attempt

- **G0 not discharged.** Read multimagie's search-status page (browser),
  Bremner's papers, and any elliptic-surface obstruction results. The
  expected SEARCHED outcome is a very deep verified frontier.
- **Feasibility:** parametrized searches (arithmetic progressions of
  squares, elliptic curve families) are the established route; brute force
  is long dead. An ATTEMPT verdict is unlikely; the file exists to make
  the PARK argument concrete and to route attention to the cubes variant.
