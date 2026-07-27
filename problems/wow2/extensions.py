#!/usr/bin/env python3
"""Extend graph6 records by one vertex for targeted boundary searches."""

from __future__ import annotations

import argparse
import sys

from verify import parse_graph6


def encode_bits(order: int, bits: list[int]) -> str:
    padded = bits + [0] * (-len(bits) % 6)
    output = [chr(order + 63)]
    for start in range(0, len(padded), 6):
        value = 0
        for bit in padded[start : start + 6]:
            value = 2 * value + bit
        output.append(chr(value + 63))
    return "".join(output)


def base_bits(graph: list[set[int]]) -> list[int]:
    return [
        int(left in graph[right])
        for right in range(1, len(graph))
        for left in range(right)
    ]


def neighborhood_masks(
    graph: list[set[int]],
    kind: str,
) -> range | list[int]:
    order = len(graph)
    if kind == "all":
        return range(1, 1 << order)

    masks: set[int] = set()
    for vertex, neighbors in enumerate(graph):
        masks.add(1 << vertex)  # leaf
        false_twin = sum(1 << neighbor for neighbor in neighbors)
        if false_twin:
            masks.add(false_twin)
        masks.add(false_twin | (1 << vertex))  # true twin
    return sorted(masks)


def extensions(record: str, kind: str):
    graph = parse_graph6(record)
    if len(graph) >= 61:
        raise ValueError("extension would exceed short graph6")
    prefix = base_bits(graph)
    for mask in neighborhood_masks(graph, kind):
        neighbor_bits = [
            int(bool(mask & (1 << vertex)))
            for vertex in range(len(graph))
        ]
        yield encode_bits(len(graph) + 1, prefix + neighbor_bits)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--kind",
        choices=("all", "local"),
        default="all",
        help="all nonempty neighborhoods, or only leaves and true/false twins",
    )
    args = parser.parse_args()

    inputs = 0
    outputs = 0
    try:
        for line in sys.stdin:
            record = line.strip()
            if not record or record == ">>graph6<<":
                continue
            inputs += 1
            for child in extensions(record, args.kind):
                print(child)
                outputs += 1
    except BrokenPipeError:
        return
    print(
        f"extensions inputs={inputs} outputs={outputs} kind={args.kind}",
        file=sys.stderr,
    )


if __name__ == "__main__":
    main()
