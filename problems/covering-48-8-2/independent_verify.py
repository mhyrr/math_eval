#!/usr/bin/env python3
"""Independent incidence-intersection verifier for covering designs.

This file intentionally imports no implementation from verifier.py.
"""

from __future__ import annotations

import argparse
import json
from collections import Counter
from pathlib import Path


def read_blocks(path: str) -> list[tuple[int, ...]]:
    result: list[tuple[int, ...]] = []
    for number, raw in enumerate(Path(path).read_text(encoding="utf-8").splitlines(), 1):
        content = raw.partition("#")[0].strip()
        if not content:
            continue
        try:
            result.append(tuple(map(int, content.split())))
        except ValueError as exc:
            raise ValueError(f"line {number} is not a list of integers") from exc
    return result


def check(
    blocks: list[tuple[int, ...]] | list[list[int]],
    v: int = 48,
    k: int = 8,
) -> dict[str, object]:
    problems: list[str] = []
    expected = set(range(1, v + 1))
    incidence = [set() for _ in range(v + 1)]
    block_sets: list[set[int] | None] = []
    used: set[int] = set()
    fingerprints: dict[tuple[int, ...], int] = {}

    for block_number, original in enumerate(blocks, 1):
        block = tuple(original)
        used.update(block)
        if len(block) != k:
            problems.append(f"block {block_number} has wrong size {len(block)}")
        if len(set(block)) != len(block):
            problems.append(f"block {block_number} repeats a point")
        outside = set(block).difference(expected)
        if outside:
            problems.append(
                f"block {block_number} has labels outside 1..{v}: {sorted(outside)}"
            )
        if len(block) == k and len(set(block)) == k and not outside:
            points = set(block)
            block_sets.append(points)
            fingerprint = tuple(sorted(points))
            if fingerprint in fingerprints:
                problems.append(
                    f"block {block_number} duplicates block {fingerprints[fingerprint]}"
                )
            else:
                fingerprints[fingerprint] = block_number
            for point in points:
                incidence[point].add(block_number)
        else:
            block_sets.append(None)

    missing_points = sorted(expected.difference(used))
    extra_points = sorted(used.difference(expected))
    if missing_points:
        problems.append(f"missing points: {missing_points}")
    if extra_points:
        problems.append(f"unexpected points: {extra_points}")

    uncovered: list[tuple[int, int]] = []
    histogram: Counter[int] = Counter()
    multiplicities: dict[str, int] = {}
    for left in range(1, v):
        for right in range(left + 1, v + 1):
            multiplicity = len(incidence[left].intersection(incidence[right]))
            histogram[multiplicity] += 1
            multiplicities[f"{left},{right}"] = multiplicity
            if multiplicity == 0:
                uncovered.append((left, right))
    if uncovered:
        problems.append(f"{len(uncovered)} pairs have empty incidence intersection")

    redundant: list[int] = []
    for block_number, points in enumerate(block_sets, 1):
        if points is None:
            continue
        can_remove = True
        ordered = sorted(points)
        for offset, left in enumerate(ordered):
            for right in ordered[offset + 1 :]:
                common_elsewhere = incidence[left].intersection(incidence[right])
                common_elsewhere.discard(block_number)
                if not common_elsewhere:
                    can_remove = False
                    break
            if not can_remove:
                break
        if can_remove:
            redundant.append(block_number)

    repeated = sum(
        max(0, multiplicity - 1) * pair_count
        for multiplicity, pair_count in histogram.items()
    )
    return {
        "valid": not problems,
        "v": v,
        "k": k,
        "block_count": len(blocks),
        "uncovered_pairs": uncovered,
        "pair_multiplicity_histogram": dict(sorted(histogram.items())),
        "pair_multiplicities": multiplicities,
        "repeated_pair_incidences": repeated,
        "redundant_blocks": redundant,
        "errors": problems,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("candidate")
    parser.add_argument("--v", type=int, default=48)
    parser.add_argument("--k", type=int, default=8)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    try:
        report = check(read_blocks(args.candidate), args.v, args.k)
    except (OSError, ValueError) as exc:
        print(f"parse error: {exc}")
        return 2
    if args.json:
        print(json.dumps(report, indent=2, sort_keys=True))
    else:
        print(f"valid: {'yes' if report['valid'] else 'no'}")
        print(f"parameters: v={args.v} k={args.k}")
        print(f"blocks: {report['block_count']}")
        print(f"uncovered pairs: {len(report['uncovered_pairs'])}")
        print(
            "pair multiplicities (multiplicity:number_of_pairs): "
            + ", ".join(
                f"{multiplicity}:{count}"
                for multiplicity, count in report[
                    "pair_multiplicity_histogram"
                ].items()
            )
        )
        print(f"repeated pair incidences: {report['repeated_pair_incidences']}")
        print(
            "redundant blocks (1-based): "
            + (", ".join(map(str, report["redundant_blocks"])) or "none")
        )
        for error in report["errors"]:
            print(f"error: {error}")
    return 0 if report["valid"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
