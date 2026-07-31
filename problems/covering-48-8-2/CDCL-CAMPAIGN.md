# Dedicated CDCL campaign

Date opened: 2026-07-30

## Decision

The next search uses a dedicated SAT solver, with one hard calibration gate:
the unrestricted 46-block encoding must recover a valid covering before the
same solver and encoding are trusted on 45 or 44 blocks.

The campaign order is:

1. unrestricted 46 blocks;
2. unrestricted 45 blocks;
3. the two complete essential-pair branches at 44 blocks.

Every search uses exact pair witnesses. A witness variable is true exactly
when its row contains the corresponding pair. This adds no variables and
adds one ternary clause per row/pair, exposing repeated pair coverage to
CDCL propagation.

Timeouts are watchdogs, not evidence and not the search strategy. Calibration
and frontier runs use six-hour wall-clock limits unless a short probe has a
specific diagnostic purpose. SAT and UNSAT stop immediately; `unknown` only
records that the watchdog fired.

## 44-block split

Every 44-block cover would be irredundant because the trusted lower bound is
44. The pair-multiplicity count also guarantees a pair occurring in at least
four blocks. Relabel that pair as `{1,2}`, relabel one containing block as
`{1,...,8}`, and place four containing rows first.

The first block has a pair unique to it, and that pair is not `{1,2}`. Under
the stabilizer of `{1,2}`, there are two cases:

- `shared`: the unique pair is `{1,3}`;
- `disjoint`: the unique pair is `{3,4}`.

The two CNFs together cover all possible 44-block constructions. An UNSAT
result for only one branch proves nothing globally. Global nonexistence would
require both CNFs to finish UNSAT and both proof artifacts to pass an
independent checker.

## Solver interface

`incidence_sat.py` accepts `--solver z3|cadical|kissat`,
`--solver-path`, repeatable `--solver-param`, `--proof-output`, and
`--proof-checker`. Reports include the executable version and SHA-256, exact
command, timeout state, CNF hash, proof hash, and proof-checker result.

`--no-column-lex` retains the fixed first block and strict row ordering but
omits residual column ordering. This is an exact alternative representation,
not a relaxation of pair coverage. It is the current long Z3 calibration
because the strengthened DoubleLex formula again timed out after 600 seconds.

CaDiCaL and Kissat are single-threaded in this interface. The Python process
enforces the wall-clock limit; solver-specific internal timeout flags should
only be added through `--solver-param` after checking the installed binary's
help.

## Reproducible commands

Set a directory outside the repository for large CNFs and logs:

```sh
CAMPAIGN_DIR=/tmp/math-eval-covering-48-8-2/cdcl-campaign
mkdir -p "$CAMPAIGN_DIR"
```

Calibration:

```sh
python3 incidence_sat.py \
  --blocks 46 \
  --exact-pair-witnesses \
  --solver cadical \
  --solver-path /path/to/cadical \
  --seed 7301 \
  --timeout 21600 \
  --cnf-output "$CAMPAIGN_DIR/free46.cnf" \
  --metadata-output "$CAMPAIGN_DIR/free46.meta.json" \
  --report-output "$CAMPAIGN_DIR/free46.report.json" \
  --output "$CAMPAIGN_DIR/free46.txt"
```

Only after that report says `solver_status: sat`, `valid: true`, and both
verification paths are valid:

```sh
python3 incidence_sat.py \
  --blocks 45 \
  --exact-pair-witnesses \
  --solver cadical \
  --solver-path /path/to/cadical \
  --seed 7302 \
  --timeout 21600 \
  --cnf-output "$CAMPAIGN_DIR/free45.cnf" \
  --metadata-output "$CAMPAIGN_DIR/free45.meta.json" \
  --report-output "$CAMPAIGN_DIR/free45.report.json" \
  --output "$CAMPAIGN_DIR/free45.txt"

python3 incidence_sat.py \
  --blocks 44 \
  --essential-pair-branch shared \
  --exact-pair-witnesses \
  --solver cadical \
  --solver-path /path/to/cadical \
  --seed 7303 \
  --timeout 21600 \
  --cnf-output "$CAMPAIGN_DIR/free44-shared.cnf" \
  --metadata-output "$CAMPAIGN_DIR/free44-shared.meta.json" \
  --report-output "$CAMPAIGN_DIR/free44-shared.report.json" \
  --output "$CAMPAIGN_DIR/free44-shared.txt"

python3 incidence_sat.py \
  --blocks 44 \
  --essential-pair-branch disjoint \
  --exact-pair-witnesses \
  --solver cadical \
  --solver-path /path/to/cadical \
  --seed 7304 \
  --timeout 21600 \
  --cnf-output "$CAMPAIGN_DIR/free44-disjoint.cnf" \
  --metadata-output "$CAMPAIGN_DIR/free44-disjoint.meta.json" \
  --report-output "$CAMPAIGN_DIR/free44-disjoint.report.json" \
  --output "$CAMPAIGN_DIR/free44-disjoint.txt"
```

For a completed UNSAT run, rerun the identical CNF with `--proof-output`.
A proof is not complete evidence until `--proof-checker` independently
accepts it. Z3 4.16 can emit textual DRAT and internally check it, but checking
its own proof is not independent certification.

## Local provisioning state

The 2026-07-30 audit found Z3 4.16 but no CaDiCaL, Kissat, MiniSat,
CryptoMiniSat, DRAT checker, LRAT checker, cached bottle, or cached source
tree. Apple clang 21 and GNU Make are available. Homebrew and direct GitHub
fetches both failed because this environment could not resolve external DNS.

Once a CaDiCaL 3.0.1 source tree or binary is placed on the machine, verify
its `--help` and `--version`, build it outside the repository, then use the
commands above. Do not interpret an uncalibrated timeout at 45 or 44 as
mathematical evidence.
