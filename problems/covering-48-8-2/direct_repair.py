#!/usr/bin/env python3
"""Direct Z3 repair with arbitrary replacement blocks.

This complements exact_repair.py's finite candidate pool.  It freezes a core
and represents each replacement block by 48 Boolean membership variables, so
SAT yields an unrestricted repair for that particular removed-block set.
UNSAT is still local to the frozen core; it is not a lower-bound proof.
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import tempfile
import time
from collections import Counter
from itertools import combinations
from pathlib import Path

import exact_repair


def signature(row: int, v: int) -> str:
    terms = " ".join(
        f"(ite x_{row}_{point} {1 << point} 0)" for point in range(v)
    )
    return f"(+ {terms})"


def emit_direct_smt2(
    *,
    core: list[tuple[int, ...]],
    deficits: tuple[tuple[int, int], ...],
    replacements: int,
    v: int,
    k: int,
    max_replication: int,
    timeout_ms: int,
) -> str:
    lines = [
        "; unrestricted fixed-core covering repair",
        f"(set-option :timeout {timeout_ms})",
        "(set-option :produce-models true)",
        "(set-logic QF_LIA)",
    ]
    for row in range(replacements):
        for point in range(v):
            lines.append(f"(declare-const x_{row}_{point} Bool)")
        cardinality = " ".join(
            f"(ite x_{row}_{point} 1 0)" for point in range(v)
        )
        lines.append(f"(assert (= (+ {cardinality}) {k}))")

    for left, right in deficits:
        alternatives = " ".join(
            f"(and x_{row}_{left - 1} x_{row}_{right - 1})"
            for row in range(replacements)
        )
        lines.append(f"(assert (or {alternatives}))")

    core_replication = Counter(point for block in core for point in block)
    for point in range(1, v + 1):
        total = " ".join(
            f"(ite x_{row}_{point - 1} 1 0)" for row in range(replacements)
        )
        minimum = max(0, (v - 1 + k - 2) // (k - 1) - core_replication[point])
        maximum = max_replication - core_replication[point]
        if maximum < minimum:
            lines.append("(assert false)")
        else:
            lines.append(f"(assert (>= (+ {total}) {minimum}))")
            lines.append(f"(assert (<= (+ {total}) {maximum}))")

    # Canonical row order removes the replacement-block permutation symmetry.
    row_signatures = [signature(row, v) for row in range(replacements)]
    for left in range(replacements - 1):
        lines.append(
            f"(assert (< {row_signatures[left]} {row_signatures[left + 1]}))"
        )
    core_masks = [
        sum(1 << (point - 1) for point in block)
        for block in core
    ]
    for row_signature in row_signatures:
        for mask in core_masks:
            lines.append(f"(assert (distinct {row_signature} {mask}))")

    names = " ".join(
        f"x_{row}_{point}"
        for row in range(replacements)
        for point in range(v)
    )
    lines.extend(["(check-sat)", f"(get-value ({names}))"])
    return "\n".join(lines) + "\n"


def parse_model(text: str, replacements: int, v: int) -> list[tuple[int, ...]]:
    values = {
        (int(row), int(point)): truth == "true"
        for row, point, truth in re.findall(
            r"\(x_(\d+)_(\d+)\s+(true|false)\)", text
        )
    }
    if len(values) != replacements * v:
        raise ValueError("Z3 model did not contain every membership variable")
    return [
        tuple(point + 1 for point in range(v) if values[(row, point)])
        for row in range(replacements)
    ]


def run(args: argparse.Namespace) -> dict[str, object]:
    started = time.monotonic()
    blocks = exact_repair.parse_candidate(args.candidate, v=48, k=8)
    if args.remove_indices:
        removed_indices = exact_repair.parse_remove_indices(
            args.remove_indices, block_count=len(blocks)
        )
        if len(removed_indices) != args.remove_count:
            raise ValueError("--remove-indices count differs from --remove-count")
    else:
        removed_indices = exact_repair.choose_weak_blocks(
            blocks,
            args.remove_count,
            v=48,
            seed=args.seed,
            trials=args.selection_trials,
            selection_width=args.selection_width,
        )
    removed_set = set(removed_indices)
    core = [block for index, block in enumerate(blocks) if index not in removed_set]
    deficits = exact_repair.uncovered_pairs(core, 48)
    smt2 = emit_direct_smt2(
        core=core,
        deficits=deficits,
        replacements=args.replacement_count,
        v=48,
        k=8,
        max_replication=args.max_replication,
        timeout_ms=args.timeout_ms,
    )
    if args.smt2_output:
        smt_path = Path(args.smt2_output)
        smt_path.write_text(smt2, encoding="utf-8")
        temporary = None
    else:
        temporary = tempfile.TemporaryDirectory()
        smt_path = Path(temporary.name) / "direct-repair.smt2"
        smt_path.write_text(smt2, encoding="utf-8")
    z3_started = time.monotonic()
    completed = subprocess.run(
        [args.z3, str(smt_path)],
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=args.timeout_ms / 1000 + 10,
        check=False,
    )
    z3_seconds = time.monotonic() - z3_started
    first_line = completed.stdout.splitlines()[0] if completed.stdout else ""
    result_blocks: list[tuple[int, ...]] | None = None
    valid = False
    if first_line == "sat":
        replacements = parse_model(completed.stdout, args.replacement_count, 48)
        result_blocks = list(exact_repair.canonical_blocks([*core, *replacements]))
        valid = exact_repair.is_valid_cover(result_blocks, v=48, k=8)
        if valid:
            Path(args.output).write_text(
                exact_repair.canonical_text(result_blocks), encoding="utf-8"
            )
    report = {
        "lane": "direct_fixed_core_repair",
        "seed": args.seed,
        "input": args.candidate,
        "input_block_count": len(blocks),
        "removed_indices_1_based": [index + 1 for index in removed_indices],
        "core_block_count": len(core),
        "core_uncovered_pair_count": len(deficits),
        "replacement_count": args.replacement_count,
        "max_replication": args.max_replication,
        "z3_status": first_line or "error",
        "z3_seconds": z3_seconds,
        "valid": valid,
        "result_block_count": len(result_blocks) if result_blocks else None,
        "output": args.output if valid else None,
        "bounded_claim": (
            "unsat applies only to this frozen core and replication cap; "
            "unknown normally means the configured timeout"
        ),
        "elapsed_seconds": time.monotonic() - started,
        "z3_stderr": completed.stderr.strip(),
    }
    if args.report_output:
        Path(args.report_output).write_text(
            json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8"
        )
    if temporary is not None:
        temporary.cleanup()
    return report


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("candidate")
    parser.add_argument("--seed", type=int, default=1)
    parser.add_argument("--remove-count", type=int, required=True)
    parser.add_argument("--replacement-count", type=int)
    parser.add_argument("--remove-indices")
    parser.add_argument("--selection-trials", type=int, default=64)
    parser.add_argument("--selection-width", type=int, default=8)
    parser.add_argument("--max-replication", type=int, default=9)
    parser.add_argument("--timeout-ms", type=int, default=60000)
    parser.add_argument("--z3", default="/opt/homebrew/bin/z3")
    parser.add_argument("--output", default="best/direct-repair.txt")
    parser.add_argument("--smt2-output")
    parser.add_argument("--report-output")
    args = parser.parse_args()
    if args.replacement_count is None:
        args.replacement_count = args.remove_count
    return args


def main() -> int:
    try:
        report = run(parse_args())
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        print(json.dumps({"error": str(error)}, indent=2))
        return 2
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0 if report["valid"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
