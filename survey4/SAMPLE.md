# Phase 4 sample — drawn 2026-07-25, before any scoring

The 56 problems the validation test runs on. Produced by the deterministic rule
in `scripts/erdos.py --sample`, which is the pre-registered selection rule as
code (`memo/PREREGISTRATION.md`, Appendix C). **Nothing here was hand-adjusted.**

## Reproduction

```sh
git clone https://github.com/google-deepmind/formal-conjectures
cd formal-conjectures && git checkout 5a60e068
./.venv/bin/python scripts/erdos.py --sample <that>/FormalConjectures/ErdosProblems
```

A full clone, not `--filter=blob:none`: the blinding harness below reads the
historical version of every file, and on-demand blob fetches against the
promisor remote fail partway through. The repo is 7.9 MB of `.git`.

| pin | commit | date |
|---|---|---|
| `teorth/erdosproblems` cutoff snapshot | `009173b2` | 2025-08-31T23:55:32Z |
| `google-deepmind/formal-conjectures` | `5a60e068` | 2026-07-25T14:08:30Z |
| seed | `20260725` | — |

**Pool: 313** problems open at the cutoff, not on the exclusion list, carrying a
Lean statement. **8 strata × 7 = n = 56.**

## The sample

| tag | pool | drawn | problems |
|---|---|---|---|
| additive basis | 14 | 7 | 9 11 32 330 868 871 881 |
| additive combinatorics | 15 | 7 | 1 138 142 170 789 817 847 |
| distances | 15 | 7 | 89 96 100 212 213 503 653 |
| graph theory | 15 | 7 | 61 85 184 600 617 619 835 |
| number theory | 138 | 7 | 3 7 342 455 539 830 849 |
| primes | 24 | 7 | 200 218 244 681 853 855 890 |
| sidon sets | 15 | 7 | 14 39 41 42 152 153 241 |
| unit fractions | 17 | 7 | 282 288 291 295 304 312 317 |

```
1 3 7 9 11 14 32 39 41 42 61 85 89 96 100 138 142 152 153 170 184 200 212 213
218 241 244 282 288 291 295 304 312 317 330 342 455 503 539 600 617 619 653 681
789 817 830 835 847 849 853 855 868 871 881 890
```

## Reproduction check against the pre-registration

Appendix C of `memo/PREREGISTRATION.md` recorded the strata table when the rule
was first run, in the contaminated session. This session re-ran it from a fresh
clone. **The pool counts and draw counts are identical in all eight strata**
(14/15/15/15/138/24/15/17, seven drawn from each, n = 56). Appendix C did not
record the individual problem numbers, so the check is at strata level — which
is the level at which the rule could have drifted.

---

## Blindness hazard found at stage 1 — read this before stage 2

The plan sources problem statements from the `formal-conjectures` Lean files.
**Those files are pinned at 2026-07-25 and contain the answer key.** From
`728.lean` (on the exclusion list, so quoting it costs nothing):

```lean
@[category research solved, AMS 11, formal_proof using lean4 at "https://..."]
theorem erdos_728 : answer(True) ↔ ...
```

and, in the docstring above it, prose naming who resolved it, with what model,
and when.

Aggregate over all 510 Erdős files in the repo — counts only, no per-problem
breakdown, since a breakdown would itself be the reveal:

| leak | files |
|---|---|
| `@[category research solved]` | 358 |
| `answer(...)` — the resolved value, inline | 436 |
| `formal_proof using ...` — a machine-checked solution exists | 113 |
| mentions a year in 2022–2026 | 509 |

Reading these files raw at stage 2 is reading the outcome. That is not a
degradation of the test; it is the end of it.

### Two mitigations, both mechanical

1. **Earliest version of each file, not HEAD.** `git log --diff-filter=A` gives
   the commit that added each file; `git show` gives its content then. For the
   16 of 56 that existed at the cutoff, this is a genuinely pre-cutoff source
   with no outcome information at all. For the other 40 the file was created
   after the cutoff, but its first version predates any later "solved" edit.
2. **Redaction of what remains**, applied uniformly to all 56 so the treatment
   itself carries no signal: the `@[category ...]` attribute, every `answer(...)`
   payload, and docstring sentences matching outcome patterns.

Both live in `scripts/blind.py` rather than in a promise, for the same reason
Appendix C put the selection rule in code.

### The harness is tested, not asserted

```sh
./.venv/bin/python scripts/blind.py --repo <fc> --selftest
# checked 453 non-sampled files (56 sampled files skipped by design)
# PASS -- no status marker, answer payload or proof link survived
```

It redacts every Erdős file **except the 56** — a self-test that printed which
of *those* leaked would be the leak — and fails if any `category research
solved`/`open`, unredacted `answer(...)`, or `formal_proof using` survives. 453
files (509 `.lean` + README = 510 entries, minus the 56). All 56 sampled
problems have a Lean file.

Two bugs it caught, both found on exclusion-list problems:

- Case-insensitive `Aleph` (the prover) matched `aleph` (the cardinal) and
  redacted the theorem statements themselves. Agent names are now
  case-sensitive, and prose redaction is confined to comment regions — Lean code
  gets only the structural redactions.
- The bibliography-kill pattern read `[ErHa66]` as a 2066 paper and deleted a
  1966 reference. Two-digit citation keys now only trip on `25`–`29`.

Bare `proved` / `shown` / `established` are deliberately **not** redacted.
"Komjáth [Ko13] proved that it is consistent that the answer is no" is
pre-cutoff background and is exactly the input `prior_attention` and Axis C
need. Redacting it would trade blindness for a survey scored on nothing. What
dies is resolution-of-this-problem language, undated recency, agent names, and
any date at or after the cutoff.

### Residual leak, declared

- **40 of 56 statements come from a post-cutoff file version.** Redaction is a
  blacklist and blacklists are incomplete.
- **The amount redacted per problem varies**, and "this file had a lot to say"
  correlates weakly with the file having something to say about a resolution.
  Where a redaction sat adjacent to the statement I was reading, the affected
  row records it in `unverifiable_claims`.
- 16 of 56 use a pre-cutoff file and 40 do not. Whether a problem was formalized
  early is a fact about the repo's coverage growth, not about the problem's
  outcome — but it is not certified independent of it.
