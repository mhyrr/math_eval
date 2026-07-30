#!/usr/bin/env python3
"""Jointly retain input blocks and synthesize unrestricted replacements.

The model chooses ``--retained-count`` supplied blocks and constructs
``--replacement-count`` new 8-blocks.  This is a larger exact neighborhood
than freezing a hand-picked core.  UNSAT remains a statement about this input
construction, total block count, and replication cap; a timeout proves
nothing.
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import time
from itertools import combinations
from pathlib import Path

import exact_repair


def cardinality(names: list[str], lower: int, upper: int) -> list[str]:
    joined = " ".join(names)
    return [
        f"(assert ((_ at-least {lower}) {joined}))",
        f"(assert ((_ at-most {upper}) {joined}))",
    ]


def emit(
    blocks: list[tuple[int, ...]],
    timeout_ms: int,
    max_replication: int,
    retained_count: int,
    replacement_count: int,
    fix_removed_zero: bool,
) -> str:
    if retained_count < 0 or replacement_count < 1:
        raise ValueError("invalid retained/replacement count")
    if retained_count > len(blocks):
        raise ValueError("retained count exceeds input block count")
    input_count = len(blocks)
    lines = [
        (
            f"; choose {retained_count} input blocks and synthesize "
            f"{replacement_count} arbitrary blocks"
        ),
        f"(set-option :timeout {timeout_ms})",
        "(set-option :produce-models true)",
        "(set-logic QF_FD)",
    ]
    for block in range(input_count):
        lines.append(f"(declare-const keep_{block} Bool)")
    lines.extend(
        cardinality(
            [f"keep_{block}" for block in range(input_count)],
            retained_count,
            retained_count,
        )
    )
    if fix_removed_zero:
        # Translation of a full cyclic seed can move any removed block to zero.
        lines.append("(assert (not keep_0))")

    for row in range(replacement_count):
        names = []
        for point in range(48):
            name = f"x_{row}_{point}"
            names.append(name)
            lines.append(f"(declare-const {name} Bool)")
        lines.extend(cardinality(names, 8, 8))

    for left in range(replacement_count):
        for right in range(left + 1, replacement_count):
            differences = " ".join(
                f"(xor x_{left}_{point} x_{right}_{point})"
                for point in range(48)
            )
            lines.append(f"(assert (or {differences}))")
    for row in range(replacement_count):
        for block_index, block in enumerate(blocks):
            block_points = set(block)
            differences = " ".join(
                (
                    f"(not x_{row}_{point - 1})"
                    if point in block_points
                    else f"x_{row}_{point - 1}"
                )
                for point in range(1, 49)
            )
            lines.append(
                f"(assert (or (not keep_{block_index}) {differences}))"
            )

    containing: dict[tuple[int, int], list[int]] = {
        pair: [] for pair in combinations(range(1, 49), 2)
    }
    for block_index, block in enumerate(blocks):
        for pair in combinations(block, 2):
            containing[pair].append(block_index)
    for (left, right), original_blocks in containing.items():
        alternatives = [f"keep_{index}" for index in original_blocks]
        alternatives.extend(
            f"(and x_{row}_{left - 1} x_{row}_{right - 1})"
            for row in range(replacement_count)
        )
        lines.append(f"(assert (or {' '.join(alternatives)}))")

    point_blocks: dict[int, list[int]] = {point: [] for point in range(1, 49)}
    for block_index, block in enumerate(blocks):
        for point in block:
            point_blocks[point].append(block_index)
    for point in range(1, 49):
        terms = [f"keep_{index}" for index in point_blocks[point]]
        terms.extend(
            f"x_{row}_{point - 1}" for row in range(replacement_count)
        )
        lines.extend(cardinality(terms, 7, max_replication))

    query = [f"keep_{block}" for block in range(input_count)]
    query.extend(
        f"x_{row}_{point}"
        for row in range(replacement_count)
        for point in range(48)
    )
    lines.extend(["(check-sat)", f"(get-value ({' '.join(query)}))"])
    return "\n".join(lines) + "\n"


def parse_truths(text: str, prefix: str) -> dict[tuple[int, ...], bool]:
    if prefix == "keep":
        return {
            (int(index),): truth == "true"
            for index, truth in re.findall(r"\(keep_(\d+)\s+(true|false)\)", text)
        }
    return {
        (int(row), int(point)): truth == "true"
        for row, point, truth in re.findall(
            r"\(x_(\d+)_(\d+)\s+(true|false)\)", text
        )
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("candidate")
    parser.add_argument("--timeout-ms", type=int, default=300000)
    parser.add_argument("--max-replication", type=int, default=9)
    parser.add_argument("--retained-count", type=int, default=40)
    parser.add_argument("--replacement-count", type=int, default=6)
    parser.add_argument("--fix-removed-zero", action="store_true")
    parser.add_argument("--z3", default="/opt/homebrew/bin/z3")
    parser.add_argument("--smt2-output", default="/tmp/cyclic-reduce-z3.smt2")
    parser.add_argument("--output", default="best/cyclic-reduce-z3-46.txt")
    parser.add_argument("--report-output")
    args = parser.parse_args()
    started = time.monotonic()
    try:
        blocks = exact_repair.parse_candidate(args.candidate, v=48, k=8)
        smt_path = Path(args.smt2_output)
        smt_path.write_text(
            emit(
                blocks,
                args.timeout_ms,
                args.max_replication,
                args.retained_count,
                args.replacement_count,
                args.fix_removed_zero,
            ),
            encoding="utf-8",
        )
        z3_started = time.monotonic()
        completed = subprocess.run(
            [args.z3, str(smt_path)],
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=args.timeout_ms / 1000 + 15,
            check=False,
        )
        z3_seconds = time.monotonic() - z3_started
        status = completed.stdout.splitlines()[0] if completed.stdout else "error"
        valid = False
        result: list[tuple[int, ...]] | None = None
        kept_indices: list[int] = []
        if status == "sat":
            keeps = parse_truths(completed.stdout, "keep")
            memberships = parse_truths(completed.stdout, "x")
            if len(keeps) != len(blocks) or len(memberships) != args.replacement_count * 48:
                raise ValueError("incomplete Z3 model")
            kept_indices = [
                index for index in range(len(blocks)) if keeps[(index,)]
            ]
            replacements = [
                tuple(
                    point + 1
                    for point in range(48)
                    if memberships[(row, point)]
                )
                for row in range(args.replacement_count)
            ]
            result = list(
                exact_repair.canonical_blocks(
                    [*(blocks[index] for index in kept_indices), *replacements]
                )
            )
            valid = exact_repair.is_valid_cover(result, v=48, k=8)
            if valid:
                Path(args.output).write_text(
                    exact_repair.canonical_text(result), encoding="utf-8"
                )
        report = {
            "lane": "joint_retain_and_direct_repair",
            "input": args.candidate,
            "input_block_count": len(blocks),
            "retained_block_count": args.retained_count,
            "replacement_count": args.replacement_count,
            "result_block_count": len(result) if result else None,
            "kept_indices_1_based": [index + 1 for index in kept_indices],
            "max_replication": args.max_replication,
            "z3_status": status,
            "z3_seconds": z3_seconds,
            "valid": valid,
            "output": args.output if valid else None,
            "bounded_claim": (
                "unsat applies only to this input construction, block count, "
                "and replication cap; unknown normally means timeout"
            ),
            "elapsed_seconds": time.monotonic() - started,
            "z3_stderr": completed.stderr.strip(),
        }
        if args.report_output:
            Path(args.report_output).write_text(
                json.dumps(report, indent=2, sort_keys=True) + "\n",
                encoding="utf-8",
            )
        print(json.dumps(report, indent=2, sort_keys=True))
        return 0 if valid else 1
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        print(json.dumps({"error": str(error)}, indent=2))
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
