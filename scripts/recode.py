#!/usr/bin/env python3
"""
Apply the two-dimensional taxonomy (rubric/TAXONOMY.md) to the Phase 1 corpus.

The coding table below is THE artifact to review. Every judgment call in the
re-classification lives here, in one screen, rather than scattered across 20
YAML files. Change a call here and re-run; the corpus follows.

Appends a `classification:` block to each corpus YAML (append rather than
rewrite, so the hand-written comments and notes in those files survive).

Usage:
    python3 scripts/recode.py --check     # show the table, write nothing
    python3 scripts/recode.py             # append classification blocks
    python3 scripts/recode.py --force     # rewrite existing blocks
"""

import argparse
import re
import sys
from pathlib import Path

CORPUS = Path(__file__).resolve().parent.parent / "corpus"

CERT_TYPES = ["WITNESS", "FORMAL_PROOF", "INFORMAL_PROOF", "CITATION", "EVIDENCE"]
PROCESSES = ["RETRIEVAL", "DIRECT_GENERATION", "GUIDED_SEARCH", "TRANSLATION",
             "SEEDED_COLLABORATION", "HUMAN_CONVENTIONAL"]
FRONTIERS = ["ADVANCES", "MATCHES", "BEHIND", "SURFACES"]
PEIRCE = ["TRANSPORT", "DEDUCTION", "INDUCTION", "ABDUCTION"]
CULTURES = ["PROBLEM_SOLVING", "THEORY_BUILDING", "MIXED"]

# id -> (certificate_type, process, frontier, inference_mode, culture, rationale)
CODING = {
    "jacobian-conjecture-c3": (
        "WITNESS", "DIRECT_GENERATION", "ADVANCES", "INDUCTION", "THEORY_BUILDING",
        "The corpus's only THEORY_BUILDING field with a WITNESS certificate. Affine "
        "algebraic geometry is machinery-heavy, but the conjecture was falsifiable by "
        "example, so the certificate collapsed to three polynomials. Refutes any rubric "
        "rule of the form 'theory-building fields are untractable' - what matters is "
        "whether the CLAIM admits a finite counterexample, not how deep the field is."),

    "erdos-90-unit-distance": (
        "INFORMAL_PROOF", "DIRECT_GENERATION", "ADVANCES", "INDUCTION", "MIXED",
        "NOT a WITNESS despite being a counterexample: the model's exponent was "
        "inexplicit. Culture MIXED is the whole point - a problem-solving question "
        "(Erdos, unit distances) closed with theory-building machinery (class field "
        "towers). Gowers, co-signing the digest: the ideas 'were present in the "
        "literature already'."),

    "erdos-1196-primitive-sets": (
        "INFORMAL_PROOF", "SEEDED_COLLABORATION", "ADVANCES", "INDUCTION", "MIXED",
        "Model supplied the frame (von Mangoldt Markov chains); seven mathematicians "
        "supplied the 35 pages. SEEDED_COLLABORATION is exactly this shape. MIXED: "
        "number theory question, probabilistic machinery 'overlooked since 1935'."),

    "erdos-728-factorial-divisibility": (
        "FORMAL_PROOF", "GUIDED_SEARCH", "ADVANCES", "DEDUCTION", "PROBLEM_SOLVING",
        "The case that motivated splitting the axis. Formerly contested between "
        "SEARCH_PLUS_VERIFIER (process) and NOVEL_ARGUMENT (product); now both are "
        "recorded and the conflict is gone. Kummer's theorem is native to the field - "
        "no import, so PROBLEM_SOLVING not MIXED."),

    "erdos-1026-monotone-subsequence": (
        "CITATION", "RETRIEVAL", "SURFACES", "TRANSPORT", "MIXED",
        "Coded by what CLOSED it: a human locating Baek-Koizumi-Ueoro (2024). Four AI "
        "tools contributed upstream (see contributions list in the file) but none closed "
        "it, and AI deep-research explicitly failed to find the paper. MIXED: monotone "
        "subsequences closed via square-packing."),

    "gpt5-erdos-claim-october-2025": (
        "CITATION", "RETRIEVAL", "SURFACES", "TRANSPORT", "PROBLEM_SOLVING",
        "Clean under the new scheme where it was awkward before. SURFACES, not "
        "ADVANCES - the frontier flag does the work the old vocabulary couldn't."),

    "alphaproof-nexus-erdos-nine": (
        "FORMAL_PROOF", "GUIDED_SEARCH", "ADVANCES", "DEDUCTION", "PROBLEM_SOLVING",
        "Purest GUIDED_SEARCH in the corpus. DEDUCTION because Lean is the ground "
        "truth and the loop is generate-then-derive. Base rate 9/353."),

    "gemini-erdos-survey-700": (
        "CITATION", "RETRIEVAL", "SURFACES", "TRANSPORT", "PROBLEM_SOLVING",
        "Coded by modal outcome: 8 of 13 hits were literature identification. The 5 "
        "novel solutions would code INFORMAL_PROOF / DIRECT_GENERATION / ADVANCES. "
        "Aggregate row - see the caveat in DISTRIBUTION."),

    "gpt5pro-convex-optimization-bound": (
        "INFORMAL_PROOF", "DIRECT_GENERATION", "BEHIND", "INDUCTION", "PROBLEM_SOLVING",
        "The reason Flag A exists. New, correct, and behind the published 1.75/L. The "
        "only BEHIND in the corpus, and no other classification scheme in this space "
        "can express it."),

    "erdos-52-sum-product-human-followup": (
        "WITNESS", "HUMAN_CONVENTIONAL", "ADVANCES", "INDUCTION", "MIXED",
        "CONTROL CASE - no AI. Coded to test Flag B: if humans also code non-abductive, "
        "the zero-abduction finding is a property of the problem culture, not of the "
        "models. They do. See TAXONOMY.md Flag B."),

    "sawin-unit-distance-explicit": (
        "WITNESS", "HUMAN_CONVENTIONAL", "ADVANCES", "INDUCTION", "MIXED",
        "CONTROL CASE - no AI. Converts the inexplicit #90 existence proof into an "
        "explicit exponent, i.e. INFORMAL_PROOF -> WITNESS. Shows the certificate type "
        "of a RESULT changes over time; code the snapshot, and date it."),

    "alphaevolve-math-constructions": (
        "WITNESS", "GUIDED_SEARCH", "ADVANCES", "INDUCTION", "PROBLEM_SOLVING",
        "WITNESS x GUIDED_SEARCH is the cell where the evaluator IS the certificate "
        "checker. That coincidence is the mechanism's entire applicability condition."),

    "alphaevolve-matmul-4x4": (
        "WITNESS", "GUIDED_SEARCH", "ADVANCES", "INDUCTION", "PROBLEM_SOLVING",
        "Same cell, most favourable instance: finite parameterized space, integer "
        "objective, exact verifier, measurable partial progress."),

    "funsearch-cap-set": (
        "WITNESS", "GUIDED_SEARCH", "ADVANCES", "INDUCTION", "PROBLEM_SOLVING",
        "Same cell, 2023. The mechanism has not changed in 2.5 years - only the range "
        "of problems with a cheap evaluator has. Relevant to extrapolation."),

    "alphaproof-imo-2024": (
        "FORMAL_PROOF", "GUIDED_SEARCH", "MATCHES", "DEDUCTION", "PROBLEM_SOLVING",
        "MATCHES, not ADVANCES: competition problems come with a guarantee that a "
        "solution exists and is findable in hours. Included to keep a benchmark axis "
        "out of the rubric."),

    "gemini-aletheia-autonomous-erdos": (
        "INFORMAL_PROOF", "DIRECT_GENERATION", "ADVANCES", "INDUCTION", "PROBLEM_SOLVING",
        "Expert-graded natural language, not Lean. DeepMind explicitly declines to "
        "claim Level 3/4 significance - ADVANCES is the right flag but the magnitude "
        "is small, which the flag does not capture. Possible Phase 2 refinement."),

    "bolzano-eight-problems": (
        "INFORMAL_PROOF", "GUIDED_SEARCH", "ADVANCES", "INDUCTION", "MIXED",
        "GUIDED_SEARCH with an LLM verifier rather than a proof assistant - a "
        "materially weaker guarantee than the Lean cases sharing this cell. The "
        "taxonomy does not currently distinguish verifier strength. Phase 2 gap."),

    "axiom-fel-conjecture-disputed": (
        "FORMAL_PROOF", "TRANSLATION", "SURFACES", "TRANSPORT", "PROBLEM_SOLVING",
        "THE case for adding TRANSLATION. Under the old vocabulary this had no home "
        "and was force-fitted to LITERATURE_RECALL. FORMAL_PROOF x TRANSLATION names "
        "it exactly: machine-checked, and mathematically empty."),

    "teorth-wiki-aggregate": (
        "FORMAL_PROOF", "TRANSLATION", "SURFACES", "TRANSPORT", "PROBLEM_SOLVING",
        "Coded by modal category: formalization is >200 of ~600 entries. Landing the "
        "largest real-world bucket in the cell the old vocabulary had no name for is "
        "the strongest evidence that TRANSLATION was a genuine omission rather than a "
        "convenience."),

    "erdos-397-formalization": (
        "FORMAL_PROOF", "GUIDED_SEARCH", "ADVANCES", "DEDUCTION", "PROBLEM_SOLVING",
        "THIN source quality - coding is provisional and inherits the file's caveats."),

    "knuth-claude-cycles": (
        "FORMAL_PROOF", "GUIDED_SEARCH", "ADVANCES", "INDUCTION", "PROBLEM_SOLVING",
        "Coded on the odd-m leg, the event the note documents: construction found by "
        "iterated scripted exploration, pattern generalized from m=3, then proved "
        "(Knuth) and Lean-formalized (Morrison) - INDUCTION with a FORMAL_PROOF "
        "certificate. The even-m leg would code INFORMAL_PROOF (machine-written "
        "proof, no independent verification on record). Native technique: the "
        "construction is the modular m-ary Gray code, recognized by Knuth."),
}


def validate():
    bad = []
    for cid, (ct, pr, fr, pe, cu, _) in CODING.items():
        for val, allowed, name in [(ct, CERT_TYPES, "certificate_type"),
                                   (pr, PROCESSES, "process"),
                                   (fr, FRONTIERS, "frontier"),
                                   (pe, PEIRCE, "inference_mode"),
                                   (cu, CULTURES, "culture")]:
            if val not in allowed:
                bad.append(f"{cid}: {name}={val!r} not in {allowed}")
    if bad:
        sys.exit("invalid coding:\n  " + "\n  ".join(bad))


def block(cid):
    ct, pr, fr, pe, cu, why = CODING[cid]
    wrapped = "\n".join("    " + line for line in
                        __import__("textwrap").wrap(why, 74))
    return (f"\n# ---- applied by scripts/recode.py; see rubric/TAXONOMY.md ----\n"
            f"classification:\n"
            f"  certificate_type: {ct}\n"
            f"  process: {pr}\n"
            f"  frontier: {fr}\n"
            f"  inference_mode: {pe}\n"
            f"  culture: {cu}\n"
            f"  rationale: >\n{wrapped}\n")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--force", action="store_true")
    args = ap.parse_args()

    validate()

    files = {p.stem: p for p in CORPUS.glob("*.yaml")}
    missing = set(CODING) - set(files)
    uncoded = set(files) - set(CODING)
    if missing:
        sys.exit(f"coded but no such file: {sorted(missing)}")
    if uncoded:
        sys.exit(f"corpus file with no coding: {sorted(uncoded)}")

    if args.check:
        w = max(len(c) for c in CODING)
        print(f"{'case'.ljust(w)}  {'certificate'.ljust(15)} {'process'.ljust(21)} "
              f"{'frontier'.ljust(9)} {'peirce'.ljust(10)} culture")
        print("-" * (w + 70))
        for cid in sorted(CODING):
            ct, pr, fr, pe, cu, _ = CODING[cid]
            print(f"{cid.ljust(w)}  {ct.ljust(15)} {pr.ljust(21)} "
                  f"{fr.ljust(9)} {pe.ljust(10)} {cu}")
        print(f"\n{len(CODING)} cases coded.")
        return

    written = 0
    for cid, path in sorted(files.items()):
        text = path.read_text()
        has = re.search(r"^classification:", text, re.M)
        if has and not args.force:
            continue
        if has:
            text = re.sub(r"\n# ---- applied by scripts/recode\.py.*?(?=\n\w|\Z)",
                          "", text, flags=re.S)
        path.write_text(text.rstrip("\n") + "\n" + block(cid))
        written += 1
    print(f"wrote classification blocks to {written} file(s)")


if __name__ == "__main__":
    main()
