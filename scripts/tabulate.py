#!/usr/bin/env python3
"""
Tabulate the Phase 1 corpus.

Reads corpus/*.yaml and emits the mechanism-distribution table plus the
cross-tabs that matter for Phase 2 rubric extraction.

Usage:
    python3 scripts/tabulate.py                 # markdown to stdout
    python3 scripts/tabulate.py --exclude-thin  # drop data_quality: THIN rows

Design note: this deliberately reports THIN and CONTESTED counts separately
rather than silently folding them in. A distribution table that hides its own
uncertainty is worse than no table.
"""

import argparse
import sys
from collections import Counter, defaultdict
from pathlib import Path

try:
    import yaml
except ImportError:
    sys.exit("PyYAML required:  pip3 install pyyaml")

CORPUS = Path(__file__).resolve().parent.parent / "corpus"

MECHANISMS = [
    "LITERATURE_RECALL",
    "CONSTRUCTION",
    "NOVEL_ARGUMENT",
    "SEARCH_PLUS_VERIFIER",
]
CERT_SIZES = ["SMALL", "MEDIUM", "LARGE"]


def load(exclude_thin=False):
    cases = []
    for path in sorted(CORPUS.glob("*.yaml")):
        with open(path) as fh:
            try:
                data = yaml.safe_load(fh)
            except yaml.YAMLError as exc:
                sys.exit(f"YAML parse failure in {path.name}: {exc}")
        if not isinstance(data, dict):
            sys.exit(f"{path.name} did not parse to a mapping")
        data["_file"] = path.name
        if exclude_thin and data.get("data_quality") == "THIN":
            continue
        cases.append(data)
    return cases


def pct(n, d):
    return f"{100.0 * n / d:.0f}%" if d else "-"


def bar(n, d, width=20):
    filled = round(width * n / d) if d else 0
    return "#" * filled + "." * (width - filled)


def table(rows, headers):
    widths = [max(len(str(r[i])) for r in [headers] + rows) for i in range(len(headers))]
    out = ["| " + " | ".join(str(h).ljust(w) for h, w in zip(headers, widths)) + " |"]
    out.append("|" + "|".join("-" * (w + 2) for w in widths) + "|")
    for r in rows:
        out.append("| " + " | ".join(str(c).ljust(w) for c, w in zip(r, widths)) + " |")
    return "\n".join(out)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--exclude-thin", action="store_true")
    args = ap.parse_args()

    cases = load(args.exclude_thin)
    n = len(cases)
    if not n:
        sys.exit("no cases loaded")

    def mech(c):
        return (c.get("resolution_mechanism") or {}).get("primary")

    def contested(c):
        return bool((c.get("resolution_mechanism") or {}).get("contested"))

    print(f"# Phase 1 corpus: mechanism distribution\n")
    print(f"Cases: **{n}**"
          + ("  (THIN rows excluded)" if args.exclude_thin else "")
          + "\n")

    # --- primary mechanism ---
    prim = Counter(mech(c) for c in cases)
    print("## Primary resolution mechanism\n")
    rows = []
    for m in MECHANISMS:
        rows.append([m, prim.get(m, 0), pct(prim.get(m, 0), n), bar(prim.get(m, 0), n)])
    unknown = sum(v for k, v in prim.items() if k not in MECHANISMS)
    if unknown:
        rows.append(["(unmapped)", unknown, pct(unknown, n), bar(unknown, n)])
    print(table(rows, ["mechanism", "n", "share", ""]))

    n_contested = sum(1 for c in cases if contested(c))
    print(f"\n**{n_contested} of {n} primary classifications are flagged `contested: true`.** "
          f"Read the distribution above with that in mind.\n")

    # --- secondary mechanism (multi-label) ---
    sec = Counter()
    for c in cases:
        for s in (c.get("resolution_mechanism") or {}).get("secondary") or []:
            sec[s] += 1
    print("## Mechanisms appearing anywhere (primary or secondary)\n")
    rows = []
    for m in MECHANISMS:
        tot = prim.get(m, 0) + sec.get(m, 0)
        rows.append([m, prim.get(m, 0), sec.get(m, 0), tot])
    print(table(rows, ["mechanism", "as primary", "as secondary", "total"]))

    # --- certificate size ---
    cert = Counter(c.get("certificate_size") for c in cases)
    print("\n## Certificate size\n")
    rows = [[s, cert.get(s, 0), pct(cert.get(s, 0), n), bar(cert.get(s, 0), n)]
            for s in CERT_SIZES]
    print(table(rows, ["size", "n", "share", ""]))

    # --- mechanism x certificate ---
    print("\n## Mechanism x certificate size\n")
    grid = defaultdict(Counter)
    for c in cases:
        grid[mech(c)][c.get("certificate_size")] += 1
    rows = []
    for m in MECHANISMS:
        rows.append([m] + [grid[m].get(s, 0) for s in CERT_SIZES])
    print(table(rows, ["mechanism"] + CERT_SIZES))

    # --- verification ---
    ver = Counter(c.get("verification_method") for c in cases)
    print("\n## Verification method\n")
    rows = [[k or "(none)", v, pct(v, n)] for k, v in ver.most_common()]
    print(table(rows, ["method", "n", "share"]))

    # --- status ---
    st = Counter(c.get("status") for c in cases)
    print("\n## Status\n")
    rows = [[k or "(none)", v, pct(v, n)] for k, v in st.most_common()]
    print(table(rows, ["status", "n", "share"]))

    # --- autonomy ---
    aut = Counter(str(c.get("autonomy")) for c in cases)
    print("\n## Autonomy\n")
    rows = [[k, v, pct(v, n)] for k, v in aut.most_common()]
    print(table(rows, ["autonomy", "n", "share"]))

    # --- cross-field transfer ---
    xf = [c for c in cases if (c.get("cross_field_transfer") or {}).get("occurred")]
    print(f"\n## Cross-field technique transfer\n")
    print(f"Occurred in **{len(xf)} of {n}** cases ({pct(len(xf), n)}).\n")
    rows = [[c["id"],
             (c.get("cross_field_transfer") or {}).get("from", "")[:52],
             (c.get("cross_field_transfer") or {}).get("to", "")[:28]] for c in xf]
    if rows:
        print(table(rows, ["case", "from", "to"]))

    # --- field spread ---
    fld = Counter(c.get("field") for c in cases)
    print("\n## Field\n")
    rows = [[k or "(none)", v] for k, v in fld.most_common()]
    print(table(rows, ["field", "n"]))

    # --- data hygiene ---
    thin = [c["id"] for c in cases if c.get("data_quality") == "THIN"]
    unver = sum(len(c.get("unverifiable_claims") or []) for c in cases)
    nosrc = [c["id"] for c in cases if not c.get("sources")]
    primary_only = [c["id"] for c in cases
                    if c.get("sources")
                    and not any(s.get("tier") == "PRIMARY" for s in c["sources"])]
    print("\n## Data hygiene\n")
    print(f"- Flagged-unverifiable claims recorded across corpus: **{unver}**")
    print(f"- Cases flagged `data_quality: THIN`: {thin or 'none'}")
    print(f"- Cases with no sources at all: {nosrc or 'none'}")
    print(f"- Cases resting on NO primary source: {primary_only or 'none'}")


if __name__ == "__main__":
    main()
