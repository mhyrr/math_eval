# Soft-bridge result

status: CLOSED — the attempted bridge failed; no solution claimed

run: 2026-07-26, exact segmented sieve through \(n=10^9\)

## Result

The relaxed condition and the historical condition did not come close to
meeting.

The analyzer recovered exactly the ten known solutions

\[
1,2,3,4,5,6,8,10,12,24
\]

and found no solution above 24 through \(10^9\). Its internal check compared
every computed \(\tau(n)\) through 10,000 with trial division.

Among \(24<n\le10^9\):

| window \(K\) | locally good \(n\) | best historical defect |
|---:|---:|---:|
| 4 | 5,535 | 8 |
| 5 | 760 | 22 |
| 6 | 45 | 22 |
| 7 | 19 | 38 |
| 8 | 5 | 38 |
| 9 | 1 | 44 |
| 10 | 0 | — |

The defect is

\[
R(n-K-1)-(n+2).
\]

A bridge requires it to be nonpositive. Every locally good value had positive
defect.

## The five last-eight windows

Each locally good \(n\) for \(K=8\) was still covered by an older interval:

| \(n\) | old blocker \(m\) | \(n-m\) | \(\tau(m)\) | defect |
|---:|---:|---:|---:|---:|
| 10,780,560 | 10,780,536 | 24 | 64 | 38 |
| 62,198,640 | 62,198,400 | 240 | 384 | 142 |
| 158,608,800 | 158,608,736 | 64 | 144 | 78 |
| 381,147,480 | 381,147,360 | 120 | 192 | 70 |
| 629,992,440 | 629,992,230 | 210 | 256 | 44 |

The last value is the only \(n\le10^9\) satisfying the first nine local
inequalities. It fails at the next shift:

\[
629992440-10
  =2\cdot5\cdot251\cdot250993,
\qquad
\tau(629992430)=16>12.
\]

Even removing that failure would not make it a solution: the 256-divisor
integer 210 places earlier still exceeds its allowance by 44.

## The complementary near miss

The smallest global excess in \(10^8\le n<10^9\) was 10, at
\(n=444444000\). Here the historical envelope was unusually close:

\[
444443904=2^8\cdot3^2\cdot23\cdot8387,
\qquad \tau(444443904)=108.
\]

But the local conditions fail immediately after \(j=1\):

\[
444443998=2\cdot8863\cdot25073,
\qquad \tau(444443998)=8>4.
\]

The two desirable events were orthogonal in the run:

- strong local windows retained a substantial historical blocker;
- low historical backlog came with an immediate local failure.

## What the quote does and does not suggest

Each integer \(m\) blocks the interval

\[
I_m=\{m+1,\ldots,m+\tau(m)-3\}.
\]

A full solution is an integer outside the union of all earlier \(I_m\). The
relaxed conjecture removes every covering interval generated before \(n-K\).
That is why Schinzel's Hypothesis H can plausibly produce its prime patterns.
The full problem asks those patterns to occur exactly when the entire older
interval cover has also expired.

The five blockers above have different factorizations and distances. The only
common explanation is the one already present in the original problem: an
older number has enough divisors to reach forward past the locally quiet
window. Turning that observation into “every locally good window has an old
blocker” would simply restate the missing theorem.

## Stop reason

This attempt found no infinite-family pattern and no witness. Extending the
sieve would repeat stronger published searches, which already reach
\(10^{12}\) directly and \(6.157\times10^{17}\) on the reduced frontier.

The soft bridge is closed. A future attempt should attack the synchronization
problem itself — record-envelope catch-up together with the forced prime chain
— rather than generating relaxed windows first and hoping the history happens
to cooperate.
