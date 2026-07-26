#!/usr/bin/env python3
"""
Phase 4 stage 3: reveal outcomes and cross-tabulate against the frozen scores.

    python3 scripts/reveal4.py --repo <formal-conjectures> > survey4/OUTCOMES.md

READS THE REVEAL SNAPSHOT. Do not run before survey4/*.yaml is committed.
Stage 2 was committed at 64dd1f3.

The label being measured is STATUS CHANGE IN A COMMUNITY DATABASE over eleven
months, by anyone, from any cause. It is not "an AI solved it". The
pre-registration says so and the split below is the attempt to separate the
causes: a resolution attributed to pre-2021 literature is the database catching
up, not new work.
"""

import argparse
import re
import subprocess
import sys
from collections import Counter, defaultdict
from math import comb
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import yaml
from score import score
from erdos import RESOLVED, INDEPENDENCE, FINITE_CERT, load, state, updated

ROOT = Path(__file__).resolve().parent.parent
SURVEY = ROOT / "survey4"
CUTOFF_DATE = "2025-08-31"


def frozen():
    """The committed stage-2 verdicts."""
    out = {}
    for f in sorted(SURVEY.glob("*.yaml")):
        d = yaml.safe_load(f.read_text())
        d.setdefault("id", f.stem)
        n = d["id"].split("-")[0][1:]
        out[n] = (d, score(d, optimistic=True))
    return out


def citation_years(repo, n):
    """Every 4-digit year appearing in the HEAD file's references or prose."""
    p = Path(repo) / f"FormalConjectures/ErdosProblems/{n}.lean"
    if not p.exists():
        return [], ""
    body = p.read_text()
    years = sorted({int(y) for y in re.findall(r"\b(19\d\d|20[0-3]\d)\b", body)})
    # bracket keys like [BoPi24] / [GuKa15] carry two-digit years
    for k in re.findall(r"\[[A-Za-z\-'’]+(\d\d)[a-z]?\]", body):
        v = int(k)
        years.append(1900 + v if v > 40 else 2000 + v)
    solved = "category research solved" in body
    return sorted(set(years)), ("solved" if solved else "open")


# Hand-read from each moved problem's HEAD Lean file at 5a60e068. The
# pre-registered rule buckets on the resolving citation's year; these are the
# attributions that rule is supposed to be reading, recorded so a reader can
# check the bucket instead of trusting a regex over every year in the file.
ATTRIB = {
 "42":  ("NEW WORK", "AI", "\"proved for all $M$ by GPT 5.5 Pro (prompted by "
         "Sandhu)\"; carries a formal_proof lean4 link."),
 "152": ("NEW WORK", "AI", "\"proved formally by the DeepMind prover agent\" "
         "[DM26a], 2026; formal_proof in formal_conjectures."),
 "330": ("UNCLEAR", "—", "Database says `proved (Lean)` (2026-05-11) but the "
         "Lean file's main theorem still reads `category research open` with "
         "`answer(sorry)` and cites nothing. NO USABLE CITATION — not guessed."),
 "619": ("NEW WORK", "AI", "\"implemented by GPT 5.5 with Codex\"; two "
         "formal_proof links."),
 "847": ("NEW WORK (rule) / CATCH-UP (substance)", "human",
         "\"A negative answer was given by Reiher, Rödl, and Sales [RRS24]\", "
         "J. Lond. Math. Soc. (2) (2024). The resolving paper PREDATES the "
         "2025-08-31 cutoff by over a year, so substantively the database "
         "caught up. The pre-registered boundary (>=2024 -> NEW WORK) buckets "
         "it as new work. The boundary is wrong here and is not being moved "
         "after the fact."),
 "868": ("NEW WORK", "ambiguous", "\"[LaLa26] Larsen and Larsen, Erdős "
         "problem 868 (2026)\". The file does not say whether a model was "
         "involved; #871 is the same author using Claude Opus 4.5, which is "
         "suggestive and not evidence."),
 "871": ("NEW WORK", "AI", "\"disproved by Larsen using Claude Opus 4.5 - in "
         "fact only a small modification of the argument of [ErNa89] is "
         "required\"; formal_proof lean4 link."),
 "7":   ("RELABEL", "—", "`open` -> `verifiable` with last_update UNCHANGED at "
         "the 2025-08-31 seed. Vocabulary backfill, not an event."),
 "617": ("RELABEL", "—", "`open` -> `falsifiable`, last_update unchanged."),
 "835": ("RELABEL", "—", "`open` -> `verifiable`, last_update unchanged."),
}


def fisher_one_sided(a, b, c, d):
    """P(>= a) for the 2x2 table [[a,b],[c,d]]; one-sided, CANDIDATE enriched."""
    n = a + b + c + d
    r1, c1 = a + b, a + c
    tot = comb(n, c1)
    return sum(comb(r1, i) * comb(n - r1, c1 - i)
               for i in range(a, min(r1, c1) + 1)) / tot


def _binom_cdf(k, n, p):
    return sum(comb(n, i) * p**i * (1 - p)**(n - i) for i in range(k + 1))


def clopper_pearson(k, n, alpha=0.05):
    """Exact binomial CI by bisecting the binomial CDF. No scipy in this venv."""
    def bisect(f, lo, hi):
        for _ in range(200):
            mid = (lo + hi) / 2
            if f(mid) > 0:
                lo = mid
            else:
                hi = mid
        return (lo + hi) / 2
    low = 0.0 if k == 0 else bisect(
        lambda p: _binom_cdf(k - 1, n, p) - (1 - alpha / 2), 0.0, 1.0)
    high = 1.0 if k == n else bisect(
        lambda p: _binom_cdf(k, n, p) - alpha / 2, 0.0, 1.0)
    return low, high


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", required=True)
    a = ap.parse_args()

    fz = frozen()
    rev = {p["number"]: p for p in load()}
    cut = {p["number"]: p for p in load(cutoff=True)}
    nums = sorted(fz, key=int)

    print("# Phase 4 outcomes — the reveal\n")
    print("Generated by `scripts/reveal4.py` **after** `survey4/*.yaml` was "
          "committed at `64dd1f3`. Reveal snapshot `35ba233a`, "
          "2026-07-25T14:09:39Z, pinned in the pre-registration before scoring "
          "began.\n")

    rows = []
    for n in nums:
        d, r = fz[n]
        cs, rs = state(cut[n]), state(rev[n])
        moved = rs != cs
        resolved = rs in RESOLVED
        years, fcstate = citation_years(a.repo, n)
        upd = updated(rev[n])
        rows.append(dict(n=n, area=d["area"], verdict=r["verdict"],
                         cleared=[c.split()[0] for c in r["cleared"]],
                         cut=cs, rev=rs, moved=moved, resolved=resolved,
                         # an EVENT touched last_update; a RELABEL did not
                         event=upd > CUTOFF_DATE,
                         upd=upd, years=years, fc=fcstate,
                         fincert=rs in FINITE_CERT))

    print("## Every row\n")
    print("| # | tag | scored | cutoff | now | last_update | moved? |")
    print("|---|---|---|---|---|---|---|")
    for r in rows:
        mark = "**yes**" if r["moved"] else "—"
        v = ("**CANDIDATE** (" + ",".join(r["cleared"]) + ")"
             if r["verdict"] == "CANDIDATE" else r["verdict"])
        print(f"| {r['n']} | {r['area']} | {v} | {r['cut']} | {r['rev']} | "
              f"{r['upd']} | {mark} |")
    print()

    moved = [r for r in rows if r["moved"]]
    res = [r for r in rows if r["resolved"]]
    ev = [r for r in moved if r["event"]]
    relabel = [r for r in moved if not r["event"]]
    touched = [r for r in rows if r["event"] and not r["moved"]]
    print(f"**{len(moved)} of {len(rows)} changed state** since the cutoff. But "
          f"that count mixes two different things, and separating them is the "
          f"first thing the reveal forced:\n")
    print(f"- **{len(ev)} EVENTS** — state changed *and* `last_update` moved "
          f"off the 2025-08-31 seed. Something happened. All {len(ev)} are now "
          f"in a RESOLVED state.")
    print(f"- **{len(relabel)} RELABELS** — state changed but `last_update` is "
          f"still the seed date: "
          f"{', '.join('#' + r['n'] for r in relabel)}. The database added the "
          f"`verifiable`/`falsifiable`/`decidable` vocabulary after the seed "
          f"and backfilled it. Nothing happened to these problems; they were "
          f"always finitely checkable and are still open.\n")
    print(f"A third group is worth naming: **{len(touched)} problems were "
          f"touched but not moved** — `last_update` bumped, state still "
          f"`open`: {', '.join('#' + r['n'] for r in touched)}. Maintainer "
          f"attention without resolution.\n")
    print(f"This is category 3 of the three things the pre-registration warned "
          f"\"resolved since the cutoff\" would mix, and it is not cosmetic: "
          f"**3 of the 4 CANDIDATE rows that \"moved\" are backfill.** Any P3 "
          f"computed on raw status change is measuring the maintainers' "
          f"vocabulary, so the operative row below is the resolved one.\n")

    print("## P3 — do CANDIDATEs resolve more often?\n")
    for label, sel in (("resolved — THE OPERATIVE ROW",
                        lambda r: r["resolved"]),
                       ("any status change (CONTAMINATED by relabels; "
                        "shown for completeness, not to be quoted)",
                        lambda r: r["moved"]),
                       ("touched at all (last_update moved, resolved or not)",
                        lambda r: r["event"])):
        ca = [r for r in rows if r["verdict"] == "CANDIDATE"]
        ro = [r for r in rows if r["verdict"] != "CANDIDATE"]
        a_, b_ = sum(1 for r in ca if sel(r)), sum(1 for r in ca if not sel(r))
        c_, d_ = sum(1 for r in ro if sel(r)), sum(1 for r in ro if not sel(r))
        p = fisher_one_sided(a_, b_, c_, d_)
        cl, ch = clopper_pearson(a_, len(ca))
        rl, rh = clopper_pearson(c_, len(ro))
        short = label.split("(")[0].split("—")[0].strip()
        print(f"### Outcome = {label}\n")
        print(f"| | {short} | not | rate | 95% CI (Clopper-Pearson) |")
        print(f"|---|---|---|---|---|")
        print(f"| CANDIDATE (n={len(ca)}) | {a_} | {b_} | {a_/len(ca):.0%} | "
              f"{cl:.0%} – {ch:.0%} |")
        print(f"| RULED_OUT (n={len(ro)}) | {c_} | {d_} | {c_/len(ro):.0%} | "
              f"{rl:.0%} – {rh:.0%} |")
        print(f"\nFisher one-sided (CANDIDATE enriched): **p = {p:.3f}**. "
              f"The two intervals overlap across almost their whole range; at "
              f"this n the data cannot distinguish a real effect in either "
              f"direction from none.\n")

    print("## The label split — catch-up vs new work\n")
    print("Pre-registered rule: year of the resolving citation in the "
          "`formal-conjectures` reference block. ≤2020 → CATCH-UP (the problem "
          "was already answered and the database caught up); ≥2024 → NEW WORK; "
          "between, or no usable citation → UNCLEAR, reported separately.\n")
    print("| # | now | scored | rule bucket | agent | attribution in the Lean file |")
    print("|---|---|---|---|---|---|")
    for r in sorted(moved, key=lambda x: int(x["n"])):
        b, who, why = ATTRIB[r["n"]]
        print(f"| {r['n']} | {r['rev']} | "
              f"{'**CAND**' if r['verdict'] == 'CANDIDATE' else 'ruled out'} | "
              f"{b} | {who} | {why} |")
    print()
    print("**No CATCH-UP rows, and that is partly the boundary's fault.** Of "
          "the 7 events, 5 are unambiguous 2026 work, 1 (#330) gives no usable "
          "citation and is reported UNCLEAR rather than guessed, and 1 (#847) "
          "resolves on a paper published in 2024 — *before* the cutoff — which "
          "the pre-registered `>=2024 -> NEW WORK` boundary mis-buckets. The "
          "boundary is not being moved after seeing the data; it is being "
          "reported as wrong for that row.\n")
    ai = [n for n, (b, w, _) in ATTRIB.items() if w == "AI"]
    print(f"**{len(ai)} of the 7 resolutions are explicitly AI-assisted** "
          f"(#{', #'.join(sorted(ai, key=int))}): GPT-5.5 Pro, a DeepMind "
          f"prover agent, GPT-5.5 with Codex, and Claude Opus 4.5. This is "
          f"better than the pre-registration feared — it worried the label "
          f"would measure status change rather than AI-tractability, and on "
          f"this sample the two largely coincide. It also makes the P3 result "
          f"below harder to explain away.\n")
    aic = [n for n in ai if fz[n][1]["verdict"] == "CANDIDATE"]
    print(f"Of those {len(ai)} AI-assisted resolutions, **{len(aic)} was "
          f"scored CANDIDATE** (#{', #'.join(aic)}) — a rate of "
          f"{len(aic)/len(ai):.0%} against an overall CANDIDATE rate of "
          f"{len([r for r in rows if r['verdict']=='CANDIDATE'])/len(rows):.0%}. "
          f"The filter selected AI-resolved problems at its own base rate.\n")

    print("## The one clean hit: Route W against the community's own labels\n")
    w = sorted((r["n"] for r in rows if "W" in r["cleared"]), key=int)
    fc = sorted((r["n"] for r in rows if r["fincert"]), key=int)
    inside = [n for n in fc if n in w]
    p = comb(len(w), len(inside)) / comb(len(rows), len(fc)) if fc else 1.0
    print(f"The database labels a problem `decidable`, `falsifiable` or "
          f"`verifiable` when a finite computation would settle it. That is "
          f"Route W's requirement — `D >= 4` (the answer can be a finite "
          f"object) and `B1 >= 4` (an exact automatic checker exists) — stated "
          f"in the maintainers' vocabulary instead of the rubric's, by people "
          f"with no knowledge of the rubric.\n")
    print(f"- Route W cleared, blind: **{', '.join('#' + n for n in w)}**")
    print(f"- Community finite-certificate labels: "
          f"**{', '.join('#' + n for n in fc)}**")
    print(f"- Overlap: **{len(inside)} of {len(fc)}**, all inside Route W. No "
          f"RULED_OUT row carries the label.\n")
    print(f"Under random assignment of {len(fc)} labels among {len(rows)} "
          f"problems, the chance that all of them land inside a nominated set "
          f"of {len(w)} is **{p:.5f}**.\n")
    print("**This is a construct-validity check, not an outcome prediction.** "
          "Route W's conditions and the community's label mean close to the "
          "same thing, so agreement tests whether the axes were *applied* "
          "correctly across 56 blind judgements, not whether they predict "
          "anything. It is still the only place in this project where an "
          "independent party, using its own vocabulary, picked out the same "
          "problems the rubric did.\n")

    print("## Per-tag outcomes\n")
    bt = defaultdict(list)
    for r in rows:
        bt[r["area"]].append(r)
    print("| tag | n | CANDIDATE | moved | resolved |")
    print("|---|---|---|---|---|")
    for t in sorted(bt):
        ps = bt[t]
        print(f"| {t} | {len(ps)} | "
              f"{sum(1 for r in ps if r['verdict']=='CANDIDATE')} | "
              f"{sum(1 for r in ps if r['moved'])} | "
              f"{sum(1 for r in ps if r['resolved'])} |")
    print()

    print("## Community labels now carried by the sample\n")
    c = Counter(r["rev"] for r in rows)
    print("| state | n |")
    print("|---|---|")
    for k, v in c.most_common():
        print(f"| {k} | {v} |")
    fin = [r for r in rows if r["rev"] in FINITE_CERT]
    ind = [r for r in rows if r["rev"] in INDEPENDENCE]
    print(f"\nFinite-certificate labels (`decidable`/`falsifiable`/"
          f"`verifiable`) — the external signal for Axis D and B1: "
          f"{len(fin)} rows{' (' + ', '.join(r['n'] for r in fin) + ')' if fin else ''}.")
    print(f"Independence labels — the external signal for Axis E: "
          f"{len(ind)} rows{' (' + ', '.join(r['n'] for r in ind) + ')' if ind else ''}.")


if __name__ == "__main__":
    main()
