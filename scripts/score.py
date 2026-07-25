#!/usr/bin/env python3
"""
Score a problem for AI-tractability against rubric/RUBRIC.md.

    python3 scripts/score.py --retrodict          # score the Phase 1 corpus
    python3 scripts/score.py --score problem.yaml # score one problem
    python3 scripts/score.py --template           # emit a blank input

Verdicts are RULED_OUT / CANDIDATE / UNDETERMINED. A CANDIDATE means "in the
~1-3% pool worth spending on", never "this will be solved". See RUBRIC.md
Part 4 on why the instrument is deliberately better at ruling out.
"""

import argparse
import sys
from pathlib import Path
from collections import Counter

try:
    import yaml
except ImportError:
    sys.exit("PyYAML required:  ./.venv/bin/pip install pyyaml")

ROOT = Path(__file__).resolve().parent.parent

GATES = ["open_to_field", "no_new_theory_required", "verification_standard_exists"]
AXES = ["certificate_cost", "evaluator_availability", "formal_library_coverage",
        "reservoir_proximity", "witness_representability", "independence_risk"]
FORMS = ["UNIVERSAL", "EXISTENTIAL", "NEITHER"]

# NOTE (v1.1): axis B was split after retrodiction. A single "verifier
# availability" score conflated two independent facts -- whether an exact
# evaluator exists for candidate objects (drives Route W) and whether a formal
# library covers the ambient theory (drives Route F). AlphaEvolve has the first
# and none of the second; it was wrongly clearing Route F. Axis D was likewise
# split from search-space structure: D now measures ONLY whether the answer can
# be a finite object, not how hard it is to find.
#
# NOTE (v1.2, Phase 3): axis E was REBUILT. The v1.1 anchors graded
# proof-theoretic strength on the reverse-mathematics Big Five at 5-3 and our
# epistemic state at 2-0 -- two different things on one axis, the same defect
# TAXONOMY.md diagnosed in the flat four-mechanism scheme. It also had a range
# error: the Big Five live in second-order arithmetic, which reaches only
# "structures that are either themselves countable, or which can be represented
# by countable codes" (SEP, Reverse Mathematics), and tops out at Pi-1-1-CA0 --
# far below ZFC, let alone the CH neighbourhood. Right units, wrong range.
#
# E is now a DECISION-STATUS scale. Its only job in this instrument is gating
# Route F, and Phase 3 established what that gate actually turns on: a proof
# assistant works inside one fixed model, so a ZFC theorem is expressible and a
# consistency statement is not. The corpus is unaffected -- every case was 4-5
# under both readings -- but the sub-3 anchors are now grounded on observed
# set-theoretic facts rather than constructed from reverse math. See RUBRIC.md
# Part 2, Axis E, and memo/FINDINGS.md section 3.


# --------------------------------------------------------------------------
# Routes. Each is a mechanism observed in the corpus, with necessary
# conditions read off the cases that used it. A problem is a CANDIDATE if it
# clears ANY route. Deliberately not a weighted sum -- see RUBRIC.md Part 1.
# --------------------------------------------------------------------------
def route_w(p):
    """Witness / refutation: exhibit a finite object that settles it."""
    reasons = []
    if p["form"] == "NEITHER":
        reasons.append("statement form admits no finite witness")
    if p["witness_representability"] < 4:
        reasons.append(f"witness_representability {p['witness_representability']} < 4")
    if p["evaluator_availability"] < 4:
        reasons.append(f"evaluator_availability {p['evaluator_availability']} < 4")
    return reasons


def route_f(p):
    """Formal: generate against a proof assistant until it compiles."""
    reasons = []
    if p["formal_library_coverage"] < 3:
        reasons.append(f"formal_library_coverage {p['formal_library_coverage']} < 3 "
                       "(no mature formal library for the ambient theory)")
    if p["independence_risk"] < 3:
        reasons.append(f"independence_risk {p['independence_risk']} < 3 "
                       "(axioms may not settle it)")
    if not p.get("formalizable_without_ambiguity", True):
        reasons.append("statement not formalizable without ambiguity")
    return reasons


def route_r(p):
    """Reservoir: import machinery from a field that has not been asked."""
    reasons = []
    if p["reservoir_proximity"] < 4:
        reasons.append(f"reservoir_proximity {p['reservoir_proximity']} < 4")
    if p["certificate_cost"] < 2:
        reasons.append(f"certificate_cost {p['certificate_cost']} < 2 "
                       "(verification too slow for the transfer to pay)")
    return reasons


def route_n(p):
    """Neglect: open because nobody tried, not because it is hard.

    Added after retrodiction -- see RETRODICTION-NOTES.md. The Gemini screen's
    own diagnosis of why problems sat open was "obscurity rather than
    difficulty". This route is one literature search away from being a G0
    failure, so it demands a rigorous G0 rather than a nominal one.
    """
    reasons = []
    if p.get("prior_attention", "HIGH") != "LOW":
        reasons.append(f"prior_attention {p.get('prior_attention')} != LOW")
    if p["certificate_cost"] < 3:
        reasons.append(f"certificate_cost {p['certificate_cost']} < 3")
    if not p.get("g0_checked_rigorously", False):
        reasons.append("G0 not discharged by actual literature work "
                       "(required for this route specifically)")
    return reasons


ROUTES = [("W", "witness/refutation", route_w),
          ("F", "formal/mathlib", route_f),
          ("R", "reservoir/transfer", route_r),
          ("N", "neglect/low-attention", route_n)]


def score(p, optimistic=False):
    """Score one problem.

    optimistic=True grants every unassessed gate. This is NOT a way to get a
    nicer answer -- it is the strong form of a falsification test. If a
    territory still fails to clear a route after being handed every gate it
    could not discharge, the routes did the work, not the gates. Phase 3 needs
    this because G0 ("open to the field, not just to the list") is undischarged
    for most of the CH-neighbourhood survey, and an all-UNDETERMINED table
    would hide the axis-by-axis argument that is the actual deliverable.
    """
    out = {"id": p.get("id", "?"), "cleared": [], "blocked": {}, "notes": []}

    # gates
    failed = [g for g in GATES if p.get(g) is False]
    unknown = [g for g in GATES if p.get(g) is None]
    if failed:
        out["verdict"] = "RULED_OUT"
        out["why"] = "gate failure: " + ", ".join(failed)
        return out
    if unknown:
        if not optimistic:
            out["verdict"] = "UNDETERMINED"
            out["why"] = "gate not assessed: " + ", ".join(unknown)
            return out
        out["granted"] = unknown

    for tag, name, fn in ROUTES:
        blockers = fn(p)
        if blockers:
            out["blocked"][f"{tag} ({name})"] = blockers
        else:
            out["cleared"].append(f"{tag} ({name})")

    if out["cleared"]:
        out["verdict"] = "CANDIDATE"
        out["why"] = "clears " + ", ".join(out["cleared"])
    else:
        out["verdict"] = "RULED_OUT"
        out["why"] = "no route clears"
    return p and out


def render(r, verbose=True):
    mark = {"CANDIDATE": "+", "RULED_OUT": "-", "UNDETERMINED": "?"}[r["verdict"]]
    lines = [f"{mark} {r['id']}: {r['verdict']} -- {r['why']}"]
    if r.get("granted"):
        lines.append(f"    [gates GRANTED, not discharged: {', '.join(r['granted'])}]")
    if verbose and r.get("blocked"):
        for route, reasons in r["blocked"].items():
            lines.append(f"    blocked {route}: {'; '.join(reasons)}")
    return "\n".join(lines)


# --------------------------------------------------------------------------
# Survey mode: score a directory of problems and report the aggregate.
#
# The aggregate is the point. Phase 3 is a falsification test, and the number
# that decides it is the CANDIDATE fraction, not any individual verdict. A
# correctly calibrated instrument must reproduce the observed fact that nothing
# in set theory has fallen to these methods; a rubric nominating a large share
# of this survey has failed regardless of how good its per-problem reasoning
# looks. See RUBRIC.md Part 4.
# --------------------------------------------------------------------------
def survey(path, optimistic, verbose):
    files = sorted(p for p in Path(path).glob("*.yaml")
                   if not p.name.startswith("SMOKE-TEST"))
    if not files:
        sys.exit(f"no survey YAML files in {path}")

    probs = []
    for f in files:
        d = yaml.safe_load(f.read_text())
        d.setdefault("id", f.stem)
        probs.append(d)

    results = [(p, score(p, optimistic)) for p in probs]

    mode = "OPTIMISTIC (unassessed gates granted)" if optimistic else "STRICT"
    print(f"# Survey: {len(results)} problems, {mode}\n")

    by_area = {}
    for p, r in results:
        by_area.setdefault(p.get("area", "unclassified"), []).append((p, r))

    for area in sorted(by_area):
        print(f"## {area}\n")
        print("```")
        for p, r in sorted(by_area[area], key=lambda x: x[0]["id"]):
            print(render(r, verbose))
        print("```\n")

    verdicts = Counter(r["verdict"] for _, r in results)
    n = len(results)
    print("## Aggregate\n")
    print(f"| verdict | n | share |")
    print(f"|---|---|---|")
    for v in ("CANDIDATE", "RULED_OUT", "UNDETERMINED"):
        c = verdicts.get(v, 0)
        print(f"| {v} | {c} | {c / n:.0%} |")
    print()

    routes = Counter()
    for _, r in results:
        for c in r["cleared"]:
            routes[c.split()[0]] += 1
    print("Route clearances (a problem may clear more than one):\n")
    for tag, name, _ in ROUTES:
        print(f"- Route {tag} ({name}): {routes.get(tag, 0)}")

    # Which axis does the blocking? This is the axis-by-axis argument.
    blockers = Counter()
    for _, r in results:
        for reasons in r.get("blocked", {}).values():
            for reason in reasons:
                blockers[reason.split()[0]] += 1
    if blockers:
        print("\nBlocking conditions by frequency:\n")
        for k, v in blockers.most_common():
            print(f"- {k}: {v}")

    return results


# --------------------------------------------------------------------------
# Retrodiction table.
#
# HINDSIGHT WARNING. These are scored as the problem stood BEFORE resolution,
# but they are scored by someone who knows every outcome. Treat agreement as
# a sanity check that catches gross errors, NOT as validation. RUBRIC.md
# Part 4 explains why real validation is unavailable: no lab publishes which
# problems its system attempted and failed.
#
# fields: form, A cert_cost, Be evaluator, Bf formal-library, C reservoir,
#         D witness, E independence, prior_attention, gate overrides
# --------------------------------------------------------------------------
RETRO = {
    "jacobian-conjecture-c3": dict(
        form="UNIVERSAL", A=5, Be=5, Bf=2, C=1, D=5, E=5, attention="HIGH",
        note="Widely believed TRUE pre-resolution. Route W is available on "
             "statement FORM, not on expected answer -- a first pass keyed on "
             "expectation and wrongly ruled out the biggest result of 2026."),
    "erdos-90-unit-distance": dict(
        form="UNIVERSAL", A=2, Be=1, Bf=1, C=5, D=1, E=5, attention="HIGH",
        note="A counterexample here is an infinite family, so D=1 blocks "
             "Route W despite the result being called a 'counterexample'. "
             "Route R carries it. This is the press-coverage trap, scored."),
    "erdos-1196-primitive-sets": dict(
        form="NEITHER", A=2, Be=1, Bf=1, C=4, D=1, E=5, attention="HIGH",
        note="A is the fragile score: blind you estimate 'a paper' (2); "
             "post-hoc you know it is 35 pages (1). At A=1 Route R fails and "
             "this case flips to RULED_OUT. Sensitivity flagged."),
    "erdos-728-factorial-divisibility": dict(
        form="EXISTENTIAL", A=4, Be=2, Bf=3, C=1, D=1, E=5, attention="LOW",
        note="Clears F and N. mathlib covers Kummer / p-adic valuations."),
    "erdos-1026-monotone-subsequence": dict(
        form="NEITHER", A=3, Be=3, Bf=3, C=3, D=2, E=5, attention="LOW",
        gates=dict(open_to_field=None),
        note="Closed by a 2024 paper a human found. G0 correctly returns "
             "UNDETERMINED -- the rubric says 'go search the literature first', "
             "which is exactly what would have been right."),
    "gpt5-erdos-claim-october-2025": dict(
        form="NEITHER", A=5, Be=1, Bf=3, C=1, D=1, E=5, attention="LOW",
        gates=dict(open_to_field=False),
        note="NEGATIVE CASE. Rubric declines on G0. Correct."),
    "gpt5pro-convex-optimization-bound": dict(
        form="NEITHER", A=3, Be=1, Bf=2, C=1, D=1, E=5, attention="LOW",
        gates=dict(open_to_field=False),
        note="NEGATIVE CASE. The 1.75/L bound was already published, so the "
             "problem was not open. Rubric declines on G0. Correct, and for "
             "the right reason."),
    "axiom-fel-conjecture-disputed": dict(
        form="NEITHER", A=4, Be=1, Bf=3, C=1, D=1, E=5, attention="LOW",
        gates=dict(open_to_field=False),
        note="NEGATIVE CASE. Proof was supplied in the input. Declines on G0."),
    "alphaproof-nexus-erdos-nine": dict(
        form="EXISTENTIAL", A=4, Be=2, Bf=3, C=1, D=2, E=5, attention="LOW",
        note="Clears F and N. The 2.5% base rate lives here."),
    "erdos-397-formalization": dict(
        form="EXISTENTIAL", A=4, Be=2, Bf=3, C=1, D=1, E=5, attention="LOW",
        note="THIN sourcing; scores inherit that."),
    "alphaevolve-math-constructions": dict(
        form="EXISTENTIAL", A=5, Be=5, Bf=1, C=1, D=5, E=5, attention="HIGH",
        note="Clears W only. Bf=1 correctly blocks F -- these were never "
             "formalized, and the pre-split axis was wrongly clearing it."),
    "alphaevolve-matmul-4x4": dict(
        form="EXISTENTIAL", A=5, Be=5, Bf=1, C=1, D=5, E=5, attention="HIGH",
        note="Clears W. Most favourable instance in the corpus."),
    "funsearch-cap-set": dict(
        form="EXISTENTIAL", A=5, Be=4, Bf=1, C=1, D=5, E=5, attention="HIGH",
        note="D corrected to 5 after the split: a cap set IS a finite explicit "
             "object. The old D=3 was scoring search difficulty, not witness "
             "representability."),
    "erdos-52-sum-product-human-followup": dict(
        form="UNIVERSAL", A=3, Be=1, Bf=1, C=5, D=4, E=5, attention="HIGH",
        note="CONTROL (human). Clears R. Be=1 correctly blocks W: no automatic "
             "checker exists for sum-product constructions."),
    "sawin-unit-distance-explicit": dict(
        form="EXISTENTIAL", A=3, Be=1, Bf=1, C=5, D=3, E=5, attention="HIGH",
        note="CONTROL (human). Clears R."),
    "gemini-aletheia-autonomous-erdos": dict(
        form="NEITHER", A=3, Be=1, Bf=1, C=3, D=2, E=5, attention="LOW",
        note="Motivated adding Route N. C=3 blocks R, Bf=1 blocks F, yet the "
             "problems resolved -- because they were neglected, not because a "
             "reservoir was tapped. DeepMind declines to call them major."),
    "bolzano-eight-problems": dict(
        form="NEITHER", A=3, Be=2, Bf=2, C=3, D=2, E=5, attention="LOW",
        note="Same shape as above. Clears N only."),
    "alphaproof-imo-2024": dict(
        form="EXISTENTIAL", A=4, Be=2, Bf=3, C=1, D=2, E=5, attention="LOW",
        gates=dict(open_to_field=False),
        note="Competition problems are not open. Rubric declines on G0, which "
             "is the correct treatment of a benchmark."),
    "gemini-erdos-survey-700": dict(skip="aggregate study, not a problem"),
    "teorth-wiki-aggregate": dict(skip="aggregate record, not a problem"),
}


def retrodict(verbose):
    corpus = {p.stem for p in (ROOT / "corpus").glob("*.yaml")}
    missing = corpus - set(RETRO)
    if missing:
        print(f"WARNING: corpus cases with no retrodiction entry: {sorted(missing)}\n")

    results, skipped = [], []
    for cid, d in sorted(RETRO.items()):
        if "skip" in d:
            skipped.append((cid, d["skip"]))
            continue
        p = {"id": cid, "form": d["form"],
             "certificate_cost": d["A"], "evaluator_availability": d["Be"],
             "formal_library_coverage": d["Bf"],
             "reservoir_proximity": d["C"], "witness_representability": d["D"],
             "independence_risk": d["E"], "prior_attention": d["attention"],
             "open_to_field": True, "no_new_theory_required": True,
             "verification_standard_exists": True,
             "g0_checked_rigorously": True}
        p.update(d.get("gates") or {})
        r = score(p)
        r["note"] = d.get("note", "")
        results.append(r)

    print("# Retrodiction: rubric v1 against the Phase 1 corpus\n")
    print("**Hindsight warning.** Scored by someone who knows every outcome. "
          "This catches gross errors; it is not validation. See RUBRIC.md Part 4.\n")

    resolved = [r for r in results if r["id"] not in
                ("gpt5-erdos-claim-october-2025", "gpt5pro-convex-optimization-bound",
                 "axiom-fel-conjecture-disputed", "alphaproof-imo-2024")]
    negatives = [r for r in results if r["id"] in
                 ("gpt5-erdos-claim-october-2025", "gpt5pro-convex-optimization-bound",
                  "axiom-fel-conjecture-disputed", "alphaproof-imo-2024")]

    print(f"## Resolved cases (n={len(resolved)}) -- should score CANDIDATE\n")
    print("```")
    for r in sorted(resolved, key=lambda x: x["id"]):
        print(render(r, verbose))
    print("```\n")
    hits = sum(1 for r in resolved if r["verdict"] == "CANDIDATE")
    print(f"**{hits}/{len(resolved)} resolved cases score CANDIDATE.**\n")

    print(f"## Negative / declined cases (n={len(negatives)}) -- should NOT score CANDIDATE\n")
    print("```")
    for r in sorted(negatives, key=lambda x: x["id"]):
        print(render(r, verbose))
    print("```\n")
    ok = sum(1 for r in negatives if r["verdict"] != "CANDIDATE")
    print(f"**{ok}/{len(negatives)} negatives correctly declined.**\n")

    if skipped:
        print("## Excluded\n")
        for cid, why in skipped:
            print(f"- `{cid}` -- {why}")
        print()

    print("## Per-case notes\n")
    for r in sorted(results, key=lambda x: x["id"]):
        if r.get("note"):
            print(f"- **{r['id']}** -- {r['note']}")

    # route usage
    print("\n## Route usage across resolved cases\n")
    from collections import Counter
    c = Counter()
    for r in resolved:
        for route in r["cleared"]:
            c[route.split()[0]] += 1
    for tag, name, _ in ROUTES:
        print(f"- Route {tag} ({name}): {c.get(tag, 0)}")


TEMPLATE = """\
# Input for scripts/score.py -- see rubric/RUBRIC.md for anchors.
id: my-problem
statement: >
  One paragraph, informal.

# --- gates: true / false / null(=not assessed) ---
open_to_field: null                  # G0 -- discharge by literature work, not judgment
no_new_theory_required: null         # G1
verification_standard_exists: null   # G2
g0_checked_rigorously: false         # required for Route N specifically

# --- statement form: does its logic admit a finite witness? ---
# UNIVERSAL   -> a counterexample would be a finite witness
# EXISTENTIAL -> a construction would be a finite witness
# NEITHER     -> asymptotic, consistency, or otherwise not witnessable
form: NEITHER

prior_attention: HIGH                # HIGH | LOW

# --- axes, 0-5, anchored in RUBRIC.md Part 2 ---
certificate_cost: 0                  # A  high = cheap to check
evaluator_availability: 0            # B1 exact automatic checker for candidate objects
formal_library_coverage: 0           # B2 mathlib-style coverage of the ambient theory
reservoir_proximity: 0               # C  unexploited machinery in an adjacent field
witness_representability: 0          # D  can the answer be a finite object
independence_risk: 0                 # E  decision status, v1.2 -- see RUBRIC.md Part 2
                                     #  5 ZFC thm, countable objects, reverse-math range
                                     #  4 ZFC thm, countable objects, ACA0-ATR0
                                     #  3 ZFC thm expected, uncountable objects
                                     #  2 ZFC status genuinely unknown; specialists split
                                     #  1 known independent; live question is cons. strength
                                     #  0 the question IS a consistency question

formalizable_without_ambiguity: true
"""


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--retrodict", action="store_true")
    ap.add_argument("--score", metavar="FILE")
    ap.add_argument("--survey", metavar="DIR",
                    help="score every *.yaml in DIR and report the aggregate")
    ap.add_argument("--optimistic", action="store_true",
                    help="grant unassessed gates (strong-form falsification test)")
    ap.add_argument("--template", action="store_true")
    ap.add_argument("--quiet", action="store_true")
    a = ap.parse_args()

    if a.template:
        print(TEMPLATE)
    elif a.retrodict:
        retrodict(not a.quiet)
    elif a.survey:
        survey(a.survey, a.optimistic, not a.quiet)
    elif a.score:
        p = yaml.safe_load(Path(a.score).read_text())
        print(render(score(p, a.optimistic)))
    else:
        ap.print_help()


if __name__ == "__main__":
    main()
