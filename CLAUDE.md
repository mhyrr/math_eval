# CLAUDE.md — operating rules for this repository

Read `README.md` for what the project is. This file is how to work in it.

---

## Vision

Most writing about AI and mathematics is a genre: a lab announces, the press
amplifies, a correction arrives quietly three weeks later, nobody updates. This
repository exists to be the thing that survives that cycle — a record careful
enough that someone can check it, and a scoring instrument honest enough to tell
you *no*.

The product is not the prediction. **The product is the classification.** If the
final memo says "no candidates exist in this territory, here is the axis-by-axis
argument," that is a complete success. A rigorous null result is the expected
outcome and is worth more than a speculative candidate list.

## Purpose

Build an empirically grounded rubric predicting which open math problems are
tractable for LLM-assisted resolution. Apply it to infinite combinatorics and set
theory — the neighbourhood of CH — to identify candidates or explain rigorously
why none exist.

**This is problem EVALUATION, not problem solving.** Never attempt to prove or
disprove anything in the target territory. If you find yourself sketching a
forcing argument, you have left the project.

---

## Rules

### 1. Source everything, tier everything

Every factual claim about a resolved problem carries a URL and a tier:
`PRIMARY` / `EXPERT_COMMENTARY` / `JOURNALISM` / `SOCIAL_MEDIA`. A claim you
could not verify goes in `unverifiable_claims:` on the case — **never dropped,
never quietly asserted**. 37 such claims are on the record as of Phase 1. That
number going up is fine. It going down without work is a smell.

### 2. Your training data is not a source

Assistant knowledge cutoff predates most of this corpus. Erdős #90, #1196,
AlphaProof Nexus, the Jacobian counterexample, GPT-5.x — all post-cutoff. **Web
search or fetch every claim.** If you recall a fact about 2026 AI mathematics
and cannot find it, you invented it. Exactly one corpus case
(`alphaproof-imo-2024`) rests partly on recall, and it says so in its
`unverifiable_claims`.

### 3. Say "ambiguous," don't guess

The record is genuinely unclear about how much humans contributed versus models
in several cases. Write that down. "Reported, not established" is a legitimate
and frequently correct thing to record.

### 4. Denominators or it didn't happen

Success rates in this literature span two orders of magnitude driven by
*selection freedom*, not model quality:

| rate | regime |
|---|---|
| 6/8 = 75% | Bolzano — problems chosen by the researchers |
| 9/353 = 2.5% | AlphaProof Nexus — curated open Erdős list |
| 5/700 = 0.7% | Gemini screen — near-exhaustive over the open database |

Never quote a hit rate without its regime. When scoring, state which regime you
are in.

### 5. Calibrate against 1–3%

That is the realistic autonomous-resolution rate for a listed open problem in
the *friendliest* field. Any claim implying better needs extraordinary support.
The AlphaProof Nexus authors, on their own strongest system:

> Even most Erdős problems remain out of reach, let alone problems that require
> extensive new theory.

### 6. The rubric rules out; it does not rule in

`RULED_OUT` is a high-confidence verdict. `CANDIDATE` means "in the ~2% pool
worth spending on," never "this will be solved." Any output reporting candidates
without the prior attached is misusing the instrument.

### 7. Classify by mechanism, record provenance separately

`TAXONOMY.md` splits certificate type (what the skeptic is handed) from
production process (how it came to exist). Do not collapse them — that collapse
is what produced 7 contested classifications in v1.0.

---

## Traps this project already hit

**A counterexample is not automatically a witness.** Erdős #90 was a
counterexample whose exponent was *inexplicit* — no finite object existed until
Sawin supplied one weeks later. It scores `D=1`. The Jacobian counterexample is
three polynomials and scores `D=5`. Score the certificate that exists, not the
one the problem's logical form suggests.

**Route W keys on statement FORM, not on expected answer.** A first pass gated
it on whether people expect the conjecture to be false, and ruled out the
Jacobian result — which everyone expected to be true.

**"Open" often means open to the list, not to the field.** 8 of 13 hits in the
Gemini screen were literature identification. Gate G0 exists for this and must be
discharged by actual searching, not judgment.

**Lean certifies less than it looks like.** It proves the proof follows from the
statement. It says nothing about whether the statement is the intended one,
whether the proof was supplied in the input, or whether the theorem is already
known. See `axiom-fel-conjecture-disputed`.

**Retrodiction is circular here.** The routes were derived from these cases and
you know every outcome. It catches gross errors. It is not validation, and no
hold-out is possible — no lab publishes which problems its system attempted and
failed.

**A keyword search over a corpus indexed by identifier returns a clean-looking
false negative.** Phase 1 searched the teorth wiki for "set theory", "forcing",
"cardinal" and found nothing, and wrote down that the category was empty. The
wiki indexes by *problem number*. Eleven entries were there. Search by the
corpus's own index, not by the vocabulary of the thing you're looking for.

**Consistency statements are not expressible in a proof assistant** — Lean works
inside one fixed model, so "consistent with ZFC" cannot be stated without leaving
ZFC. DeepMind's own formal-conjectures repo says so in
`ErdosProblems/1175.lean`. This is not a mathlib coverage gap that will close; it
is structural. CH-*equivalences* are formalizable (Paulson did Wetzel's problem);
CH-*independence* is not.

**Fetch failures to expect:** `erdosproblems.com` returns **403** to automated
fetch (every problem statement in the *corpus* is paraphrased from papers and
commentary as a result — the largest sourcing weakness in Phase 1). Hacker News
returns **429** under load. arXiv abstract pages work; PDFs sometimes return
binary, in which case `Read` the saved file with a `pages:` range.

**Working around the 403 — do this before paraphrasing anything.** The *site* is
closed to automated fetch. The *data* is not. Three public repos cover almost
everything Phase 1 had to guess at:

| what you need | where |
|---|---|
| **status, dates, tags** (1217 problems, live) | `scripts/erdos.py --fetch` → `teorth/erdosproblems`, `data/problems.yaml` |
| **statements**, formalized, with references | `git clone --filter=blob:none` [formal-conjectures](https://github.com/google-deepmind/formal-conjectures); set-theory slice via `grep -rl --include='*.lean' -E "AMS 3([ ,\]]|$)"` |
| **the AI census** (frozen 2026-06-30) | `git clone https://github.com/teorth/erdosproblems.wiki.git` — raw markdown, greppable and countable in a way the rendered page is not |

The status database also carries community labels that map onto rubric axes and
were assigned with no knowledge of it: `independent` / `not provable` /
`not disprovable` for Axis E, and `decidable` / `falsifiable` / `verifiable` for
Axis D and B1. `scripts/erdos.py --corroborate` cross-tabulates them. It is the
only external check this project has — use it, and note that the finite-cert
share is low in *every* field, so absence in one field is weak evidence alone.

---

## Mechanics

Python is externally managed on this machine (PEP 668). Use the venv:

```sh
python3 -m venv .venv && ./.venv/bin/pip install pyyaml
```

| task | command |
|---|---|
| regenerate distribution table | `./.venv/bin/python scripts/tabulate.py > corpus/DISTRIBUTION.md` |
| same, dropping weak rows | `./.venv/bin/python scripts/tabulate.py --exclude-thin` |
| review all taxonomy codings | `./.venv/bin/python scripts/recode.py --check` |
| re-apply codings after edits | `./.venv/bin/python scripts/recode.py --force` |
| retrodict the rubric | `./.venv/bin/python scripts/score.py --retrodict > rubric/RETRODICTION.md` |
| blank scoring input | `./.venv/bin/python scripts/score.py --template` |
| score one problem | `./.venv/bin/python scripts/score.py --score survey/foo.yaml` |
| score the whole survey | `./.venv/bin/python scripts/score.py --survey survey/` |
| same, granting unassessed gates | `./.venv/bin/python scripts/score.py --survey survey/ --optimistic` |

`--optimistic` grants every gate that could not be discharged. It is the
**strong** form of the falsification test, not the lenient one: if a territory
still clears no route after being handed the gates, the routes did the work.
Always report both columns.

**The taxonomy coding table lives in `scripts/recode.py`, not in the YAML
files.** It is deliberately one reviewable screen. Change a call there and
re-run; the corpus follows. Same pattern for the retrodiction table in
`scripts/score.py`.

After any corpus edit, regenerate `DISTRIBUTION.md` — it is generated output and
will silently go stale.

---

## What "done" means here

- Would a skeptical mathematician be able to check every claim from the links?
- Are the uncertainties recorded *as* uncertainties, or smoothed away?
- Does the conclusion survive the base rate, or does it need the base rate to be
  wrong?
- If this is read in a year, will the corrections have been to the confident
  parts or the hedged parts?

A finding you cannot make concrete is not a finding. Cut it.
