#!/usr/bin/env python3
"""Exact pair-counting verifier for covering-design certificates."""

from __future__ import annotations

import argparse
import json
from collections import Counter
from dataclasses import asdict, dataclass
from itertools import combinations
from pathlib import Path
from typing import Iterable, Sequence


Pair = tuple[int, int]
Block = tuple[int, ...]


@dataclass(frozen=True)
class VerificationReport:
    valid: bool
    v: int
    k: int
    block_count: int
    used_points: list[int]
    missing_points: list[int]
    unexpected_points: list[int]
    errors: list[str]
    uncovered_pairs: list[Pair]
    pair_multiplicity_histogram: dict[int, int]
    pair_multiplicities: dict[str, int]
    repeated_pair_incidences: int
    redundant_blocks: list[int]


def parse_text(text: str) -> list[Block]:
    blocks: list[Block] = []
    for line_number, raw_line in enumerate(text.splitlines(), start=1):
        line = raw_line.split("#", 1)[0].strip()
        if not line:
            continue
        fields = line.split()
        try:
            block = tuple(int(field, 10) for field in fields)
        except ValueError as exc:
            raise ValueError(f"line {line_number}: non-integer point label") from exc
        blocks.append(block)
    return blocks


def parse_file(path: str | Path) -> list[Block]:
    return parse_text(Path(path).read_text(encoding="utf-8"))


def canonicalize(blocks: Iterable[Sequence[int]]) -> tuple[Block, ...]:
    return tuple(sorted(tuple(sorted(block)) for block in blocks))


def canonical_text(blocks: Iterable[Sequence[int]]) -> str:
    return "".join(" ".join(map(str, block)) + "\n" for block in canonicalize(blocks))


def verify(
    blocks: Sequence[Sequence[int]],
    *,
    v: int = 48,
    k: int = 8,
) -> VerificationReport:
    if v < 2:
        raise ValueError("v must be at least 2")
    if not 2 <= k <= v:
        raise ValueError("k must satisfy 2 <= k <= v")

    errors: list[str] = []
    expected_points = set(range(1, v + 1))
    used_points: set[int] = set()
    normalized_blocks: list[Block] = []
    well_formed: list[bool] = []

    for index, raw_block in enumerate(blocks, start=1):
        block = tuple(raw_block)
        normalized_blocks.append(block)
        used_points.update(block)
        good = True
        if len(block) != k:
            errors.append(f"block {index}: has {len(block)} entries, expected {k}")
            good = False
        if len(set(block)) != len(block):
            errors.append(f"block {index}: contains duplicate point labels")
            good = False
        bad = sorted(set(block) - expected_points)
        if bad:
            errors.append(f"block {index}: out-of-range points {bad}")
            good = False
        well_formed.append(good)

    seen: dict[frozenset[int], int] = {}
    for index, (block, good) in enumerate(
        zip(normalized_blocks, well_formed, strict=True), start=1
    ):
        if not good:
            continue
        key = frozenset(block)
        if key in seen:
            errors.append(f"block {index}: duplicates block {seen[key]}")
        else:
            seen[key] = index

    pair_counts: Counter[Pair] = Counter()
    for block, good in zip(normalized_blocks, well_formed, strict=True):
        if not good:
            continue
        for left, right in combinations(sorted(block), 2):
            pair_counts[(left, right)] += 1

    all_pairs = list(combinations(range(1, v + 1), 2))
    uncovered = [pair for pair in all_pairs if pair_counts[pair] == 0]
    histogram_counter = Counter(pair_counts[pair] for pair in all_pairs)
    histogram = dict(sorted(histogram_counter.items()))
    repeated = sum(max(0, count - 1) for count in pair_counts.values())

    redundant: list[int] = []
    for index, (block, good) in enumerate(
        zip(normalized_blocks, well_formed, strict=True), start=1
    ):
        if good and all(pair_counts[pair] >= 2 for pair in combinations(sorted(block), 2)):
            redundant.append(index)

    missing_points = sorted(expected_points - used_points)
    unexpected_points = sorted(used_points - expected_points)
    if missing_points:
        errors.append(f"point universe is missing {missing_points}")
    if unexpected_points:
        errors.append(f"point universe has unexpected labels {unexpected_points}")
    if uncovered:
        errors.append(f"{len(uncovered)} unordered pairs are uncovered")

    multiplicities = {
        f"{left},{right}": pair_counts[(left, right)] for left, right in all_pairs
    }
    return VerificationReport(
        valid=not errors,
        v=v,
        k=k,
        block_count=len(blocks),
        used_points=sorted(used_points & expected_points),
        missing_points=missing_points,
        unexpected_points=unexpected_points,
        errors=errors,
        uncovered_pairs=uncovered,
        pair_multiplicity_histogram=histogram,
        pair_multiplicities=multiplicities,
        repeated_pair_incidences=repeated,
        redundant_blocks=redundant,
    )


def render_human(report: VerificationReport) -> str:
    histogram = ", ".join(
        f"{multiplicity}:{count}"
        for multiplicity, count in report.pair_multiplicity_histogram.items()
    )
    lines = [
        f"valid: {'yes' if report.valid else 'no'}",
        f"parameters: v={report.v} k={report.k}",
        f"blocks: {report.block_count}",
        f"points used: {len(report.used_points)}/{report.v}",
        f"uncovered pairs: {len(report.uncovered_pairs)}",
        f"pair multiplicities (multiplicity:number_of_pairs): {histogram}",
        f"repeated pair incidences: {report.repeated_pair_incidences}",
        "redundant blocks (1-based): "
        + (", ".join(map(str, report.redundant_blocks)) or "none"),
    ]
    if report.uncovered_pairs:
        sample = " ".join(f"({a},{b})" for a, b in report.uncovered_pairs[:40])
        suffix = " ..." if len(report.uncovered_pairs) > 40 else ""
        lines.append(f"uncovered pair sample: {sample}{suffix}")
    if report.errors:
        lines.append("errors:")
        lines.extend(f"  - {error}" for error in report.errors)
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("candidate", help="one block per line")
    parser.add_argument("--v", type=int, default=48)
    parser.add_argument("--k", type=int, default=8)
    parser.add_argument("--json", action="store_true")
    parser.add_argument(
        "--canonical-output",
        metavar="PATH",
        help="write the parsed blocks in canonical form after verification",
    )
    args = parser.parse_args()

    try:
        blocks = parse_file(args.candidate)
        report = verify(blocks, v=args.v, k=args.k)
    except (OSError, ValueError) as exc:
        print(f"parse error: {exc}")
        return 2

    if args.canonical_output:
        Path(args.canonical_output).write_text(canonical_text(blocks), encoding="utf-8")
    if args.json:
        print(json.dumps(asdict(report), sort_keys=True, indent=2))
    else:
        print(render_human(report))
    return 0 if report.valid else 1


if __name__ == "__main__":
    raise SystemExit(main())
