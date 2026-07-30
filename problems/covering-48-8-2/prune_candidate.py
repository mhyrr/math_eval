#!/usr/bin/env python3
"""Remove the requested number of blocks with exact greedy lookahead."""

from __future__ import annotations

import argparse
from itertools import combinations
from pathlib import Path

import verifier


def uncovered_count(blocks: list[tuple[int, ...]], v: int = 48) -> int:
    covered = {
        pair for block in blocks for pair in combinations(tuple(sorted(block)), 2)
    }
    return v * (v - 1) // 2 - len(covered)


def prune(
    blocks: list[tuple[int, ...]], target: int
) -> tuple[list[tuple[int, ...]], list[int]]:
    current = list(blocks)
    original_indices = list(range(len(blocks)))
    removed: list[int] = []
    while len(current) > target:
        choice = min(
            range(len(current)),
            key=lambda index: (
                uncovered_count(current[:index] + current[index + 1 :]),
                tuple(sorted(current[index])),
            ),
        )
        removed.append(original_indices.pop(choice))
        current.pop(choice)
    return current, removed


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("candidate")
    parser.add_argument("--target", type=int, required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()
    blocks = verifier.parse_file(args.candidate)
    if not 0 < args.target <= len(blocks):
        parser.error("--target must be between 1 and the input block count")
    result, removed = prune(blocks, args.target)
    Path(args.output).write_text(verifier.canonical_text(result), encoding="utf-8")
    print(
        f"input_blocks={len(blocks)} target_blocks={len(result)} "
        f"removed_1_based={','.join(str(index + 1) for index in removed)} "
        f"uncovered={uncovered_count(result)} output={args.output}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
