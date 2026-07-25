#!/usr/bin/env python3
"""
Query the teorth/erdosproblems community database.

    python3 scripts/erdos.py --fetch          # download data/problems.yaml
    python3 scripts/erdos.py --status 623 501 # status of specific problems
    python3 scripts/erdos.py --tag "set theory"
    python3 scripts/erdos.py --corroborate    # cross-check rubric axes vs community labels

WHY THIS EXISTS. `erdosproblems.com` returns 403 to automated fetch, which was
Phase 1's largest sourcing weakness -- every corpus problem statement is
paraphrased from commentary rather than read from the database. The database
itself, however, is a public YAML file in a GitHub repo, actively maintained
(1217 problems, updated daily), and carries per-problem `status.state`,
`status.last_update`, `tags`, and `formalized.state`.

It does NOT carry problem statements. Those still have to come from DeepMind's
formal-conjectures Lean files or the literature. What it does carry is the one
thing this project most needed and could not get: an authoritative, dated,
community-maintained resolution status. That is what gate G0 is about.

Fetched via `gh api` rather than curl -- the repo is public but curl is not in
this environment's permitted set.
"""

import argparse
import json
import subprocess
import sys
from collections import Counter, defaultdict
from pathlib import Path

try:
    import yaml
except ImportError:
    sys.exit("PyYAML required:  ./.venv/bin/pip install pyyaml")

ROOT = Path(__file__).resolve().parent.parent
CACHE = ROOT / "sources" / "erdos-problems.yaml"       # REVEAL snapshot
CUTOFF = ROOT / "sources" / "erdos-cutoff.yaml"        # status as of 2025-08-31

REPO = "teorth/erdosproblems"
PATH = "data/problems.yaml"

# Phase 4 pins, fixed in memo/PREREGISTRATION.md before any scoring.
CUTOFF_SHA = "009173b2"   # 2025-08-31T23:55:32Z -- last commit before the cutoff
REVEAL_SHA = "35ba233a"   # 2026-07-25T14:09:39Z -- outcome snapshot, frozen

# The community's own status vocabulary, grouped by what it means for this rubric.
RESOLVED = {"proved", "proved (Lean)", "disproved", "disproved (Lean)",
            "solved", "solved (Lean)"}
# Settled, but settled by showing ZFC does not settle it. Axis E territory.
INDEPENDENCE = {"independent", "not provable", "not disprovable"}
# Open, but a finite computation would settle it. This is an EXTERNAL signal for
# Axis D (witness representability) and Axis B1 (evaluator availability),
# maintained by mathematicians rather than by this project.
FINITE_CERT = {"decidable", "falsifiable", "verifiable"}


def _pull(ref=None):
    url = f"repos/{REPO}/contents/{PATH}" + (f"?ref={ref}" if ref else "")
    out = subprocess.run(["gh", "api", url, "-H", "Accept: application/vnd.github.raw"],
                         capture_output=True, text=True)
    if out.returncode:
        sys.exit(f"gh api failed:\n{out.stderr}")
    return out.stdout


def fetch(cutoff=False):
    CACHE.parent.mkdir(parents=True, exist_ok=True)
    if cutoff:
        body = _pull(CUTOFF_SHA)
        CUTOFF.write_text(
            f"# Erdos problem database AS OF THE PHASE 4 CUTOFF.\n"
            f"# Commit {CUTOFF_SHA}, 2025-08-31T23:55:32Z. Do not regenerate.\n"
            f"# This is the SCORING-TIME view. Status here is what was known at the\n"
            f"# cutoff; it contains no outcome information.\n" + body)
        print(f"wrote {CUTOFF.relative_to(ROOT)} -- "
              f"{len(yaml.safe_load(body))} problems (cutoff view)")
    else:
        body = _pull(REVEAL_SHA)
        CACHE.write_text(
            f"# Erdos problem database, PHASE 4 REVEAL SNAPSHOT.\n"
            f"# Commit {REVEAL_SHA}, 2026-07-25T14:09:39Z.\n"
            f"#\n"
            f"# *** CONTAINS OUTCOMES. Do not consult while scoring Phase 4. ***\n"
            f"# Stage 2 of memo/PREREGISTRATION.md requires scores to be committed\n"
            f"# before this file is read. Use erdos-cutoff.yaml until then.\n" + body)
        print(f"wrote {CACHE.relative_to(ROOT)} -- "
              f"{len(yaml.safe_load(body))} problems (REVEAL -- contains outcomes)")


def load(cutoff=False):
    p = CUTOFF if cutoff else CACHE
    if not p.exists():
        sys.exit(f"no cache at {p}. Run:  scripts/erdos.py --fetch"
                 f"{'-cutoff' if cutoff else ''}")
    return yaml.safe_load(p.read_text())


def state(p):
    return (p.get("status") or {}).get("state")


def updated(p):
    return (p.get("status") or {}).get("last_update") or ""


def show_status(probs, numbers):
    D = {p["number"]: p for p in probs}
    for n in numbers:
        p = D.get(str(n))
        if not p:
            print(f"  #{n}: NOT IN DATABASE")
            continue
        st, fm = p.get("status") or {}, p.get("formalized") or {}
        print(f"  #{n:<6} state={str(st.get('state')):<18} "
              f"last_update={st.get('last_update')}  "
              f"formalized={str(fm.get('state')):<5} tags={p.get('tags')}")
        if st.get("note"):
            print(f"          note: {st['note']}")


def show_tag(probs, tag):
    ps = [p for p in probs if tag in (p.get("tags") or [])]
    if not ps:
        sys.exit(f"no problems tagged {tag!r}")
    c = Counter(state(p) for p in ps)
    print(f"# tag {tag!r}: {len(ps)} of {len(probs)} problems "
          f"({len(ps)/len(probs):.1%})\n")
    for k, v in c.most_common():
        print(f"  {str(k):<20} {v}")
    op = sorted((p["number"] for p in ps if state(p) == "open"), key=int)
    print(f"\n  open ({len(op)}): {' '.join(op)}")


def corroborate(probs):
    """Cross-check the rubric's axis calls against community labels.

    This is the closest thing to an external check the project has. The labels
    were assigned by mathematicians curating a problem database, with no
    knowledge of this rubric, which makes them independent in a way nothing in
    corpus/ is.
    """
    bytag = defaultdict(list)
    for p in probs:
        for t in (p.get("tags") or []):
            bytag[t].append(p)

    print("# Community labels vs rubric axes\n")
    print("## Finite-certificate labels by field  (external signal for D and B1)\n")
    print("`decidable` / `falsifiable` / `verifiable` mean a finite computation")
    print("settles the problem -- which is exactly what Route W requires.\n")
    print(f"| tag | n | finite-cert | share | independence-labelled |")
    print(f"|---|---|---|---|---|")
    rows = []
    for t, ps in bytag.items():
        if len(ps) < 25:
            continue
        f = sum(1 for p in ps if state(p) in FINITE_CERT)
        i = sum(1 for p in ps if state(p) in INDEPENDENCE)
        rows.append((f / len(ps), t, len(ps), f, i))
    for share, t, n, f, i in sorted(rows, reverse=True):
        print(f"| {t} | {n} | {f} | {share:.1%} | {i} |")

    print("\n## Every independence-labelled problem in the database\n")
    ind = [p for p in probs if state(p) in INDEPENDENCE]
    print(f"{len(ind)} of {len(probs)} problems.\n")
    print("| # | label | tags |")
    print("|---|---|---|")
    for p in sorted(ind, key=lambda x: int(x["number"])):
        print(f"| {p['number']} | {state(p)} | {', '.join(p.get('tags') or [])} |")

    st_adj = sum(1 for p in ind
                 if {"set theory", "chromatic number", "ramsey theory"}
                 & set(p.get("tags") or []))
    print(f"\n**{st_adj} of {len(ind)} carry a set-theory, chromatic-number or "
          f"ramsey-theory tag** -- i.e. sit in or beside the Phase 3 survey "
          f"territory. That is independent corroboration that Axis E is "
          f"measuring a real property concentrated in one place.")


# --------------------------------------------------------------------------
# Phase 4 sampling. The selection rule from memo/PREREGISTRATION.md, as CODE.
#
# This exists so the rule cannot drift. A pre-registered selection rule that
# lives only in prose is a promise; one that lives in a deterministic function
# is a constraint. Nothing here reads the reveal snapshot -- it operates
# entirely on the cutoff view plus a fixed exclusion list.
# --------------------------------------------------------------------------
EXCLUDE = set("""
52 70 75 90 164 397 401 501 590 591 592 593 594 595 596 597 598 601 602 623
728 729 740 846 857 918 949 965 1026 1028 1034 1036 1037 1039 1044 1048 1067
1068 1071 1080 1095 1098 1102 1119 1123 1127 1128 1154 1167 1168 1169 1170
1171 1172 1173 1174 1175 1176 1177 1196 1217
""".split())

SEED = 20260725          # fixed; changing it invalidates the pre-registration
N_TARGET = 60
N_TAGS = 8
MIN_TAG_DB = 25          # tag must have >=25 problems in the database
MIN_TAG_POOL = 3         # and >=3 usable problems in the pool


def sample(lean_dir):
    import random
    cut = load(cutoff=True)
    lean = {p.stem for p in Path(lean_dir).glob("*.lean")}

    pool = [p for p in cut
            if state(p) == "open"
            and p["number"] not in EXCLUDE
            and p["number"] in lean]

    dbcount = Counter()
    for p in cut:
        for t in (p.get("tags") or []):
            dbcount[t] += 1

    poolcount = Counter()
    for p in pool:
        for t in (p.get("tags") or []):
            poolcount[t] += 1

    qual = {t for t in poolcount
            if dbcount[t] >= MIN_TAG_DB and poolcount[t] >= MIN_TAG_POOL}

    # assign each problem to its RAREST qualifying tag, so big fields do not
    # swamp small ones (PREREGISTRATION.md, field selection rule)
    assigned = defaultdict(list)
    for p in pool:
        ts = [t for t in (p.get("tags") or []) if t in qual]
        if ts:
            assigned[min(ts, key=lambda t: poolcount[t])].append(p["number"])

    top = sorted(assigned, key=lambda t: -len(assigned[t]))[:N_TAGS]
    per = N_TARGET // len(top)
    rng = random.Random(SEED)

    print("# Phase 4 sample\n")
    print(f"Selection rule: `scripts/erdos.py --sample`, seed {SEED}, "
          f"cutoff commit {CUTOFF_SHA}.")
    print(f"Pool: {len(pool)} problems open at cutoff, not excluded, "
          f"with a Lean statement.\n")
    print(f"Strata: {len(top)} tags x {per} problems. "
          f"Each problem assigned to its rarest qualifying tag.\n")
    print("| tag | pool | drawn | problems |")
    print("|---|---|---|---|")
    chosen = []
    for t in sorted(top):
        avail = sorted(assigned[t], key=int)
        take = sorted(rng.sample(avail, min(per, len(avail))), key=int)
        chosen += take
        print(f"| {t} | {len(avail)} | {len(take)} | {' '.join(take)} |")
    print(f"\n**n = {len(chosen)}**\n")
    print("```")
    print(" ".join(sorted(chosen, key=int)))
    print("```")
    return chosen


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--fetch", action="store_true",
                    help="pull the REVEAL snapshot (contains outcomes)")
    ap.add_argument("--fetch-cutoff", action="store_true",
                    help="pull the cutoff snapshot (scoring-time view)")
    ap.add_argument("--status", nargs="+", metavar="N")
    ap.add_argument("--tag", metavar="TAG")
    ap.add_argument("--corroborate", action="store_true")
    ap.add_argument("--sample", metavar="LEAN_DIR",
                    help="draw the Phase 4 sample from a formal-conjectures "
                         "ErdosProblems directory")
    a = ap.parse_args()

    if a.fetch_cutoff:
        return fetch(cutoff=True)
    if a.fetch:
        return fetch()
    if a.sample:
        return sample(a.sample)
    if not (a.status or a.tag or a.corroborate):
        return ap.print_help()

    probs = load()
    if a.status:
        show_status(probs, a.status)
    if a.tag:
        show_tag(probs, a.tag)
    if a.corroborate:
        corroborate(probs)


if __name__ == "__main__":
    main()
