# Erdős #647 — one attempt

This directory records one bounded attempt on Erdős Problem #647. The parent
repository evaluates problems rather than solving them; Greg explicitly opened
this exception on 2026-07-26 so that the evaluation can be tested against real
use.

## Question

Let \(\tau(m)\) be the number of positive divisors of \(m\). Does some \(n>24\)
satisfy

\[
\max_{m<n}(m+\tau(m))\le n+2?
\]

Equivalently,

\[
\tau(n-j)\le j+2\qquad(1\le j<n).
\]

A positive answer is an integer and an exact divisor-count check. A negative
answer is a theorem.

## Prior frontier

This attempt does not restart the existing search.

- OEIS A087280 records no example through \(10^{12}\).
- Scott Hughes's proof-chain forces every candidate \(n>84\) into
  \(n=2520N\), with \(N\) in 41 residue classes modulo 46189.
- Its finite-range certificate rules out
  \(24<n\le615736321200000000\).
- Its Stage-1 boundary retires fixed-depth positive-footprint congruence trees:
  a Chinese-remainder all-avoid branch survives every finite prime pool of
  that form.

Sources:

- <https://oeis.org/A087280>
- <https://www.erdosproblems.com/forum/thread/647>
- <https://github.com/scottdhughes/erdos647-proof-chain>
- <https://github.com/scottdhughes/erdos647-proof-chain/blob/main/docs/stage1_boundary.md>

## The attempted idea: bridge the relaxed problem

Erdős expected that for every fixed \(K\), infinitely many \(n\) satisfy the
condition only in the last \(K\) positions. That suggests a possible soft
route: find a locally good window which also escapes every older divisor peak.

Define

\[
R(x)=\max_{1\le m\le x}(m+\tau(m)).
\]

For \(n>K\), the full condition splits exactly into:

1. **local window:** \(\tau(n-j)\le j+2\) for \(1\le j\le K\);
2. **historical bridge:** \(R(n-K-1)\le n+2\).

The experiment measures the intersection rather than treating the relaxed
conjecture as evidence by analogy. For several \(K\), it will:

- enumerate locally good \(n\);
- compute their exact historical defect \(R(n-K-1)-(n+2)\);
- identify the old integer \(m\) responsible for the best failed bridge;
- verify the known full solutions and the absence of another small solution.

## Success and stop conditions

This lane succeeds if it finds either:

- a full witness; or
- a stable arithmetic pattern in the best bridge failures which yields an
  infinite-family lemma not covered by the retired fixed-depth tree.

It stops after one implementation and one inspected run if it merely reproduces
the known separation between fixed local windows and the accumulated global
maximum. Extending the numerical frontier alone is not a result.

## Reproduction

The analyzer uses a segmented multiplicative sieve. It computes every divisor
count exactly while retaining only one block and a short ring of prefix data.

```sh
cc -std=c17 -O3 -Wall -Wextra -Wpedantic -Werror \
  problems/erdos647/soft_bridge.c -lm -o /tmp/erdos647-soft-bridge

/tmp/erdos647-soft-bridge \
  --limit 1000000000 \
  --block-size 2097152 \
  --trace-window 8 \
  --self-test
```

`--self-test` compares the segmented sieve with trial division through 10,000
and checks the complete known solution set through 24.

The completed run and its interpretation are in [RESULT.md](RESULT.md).
