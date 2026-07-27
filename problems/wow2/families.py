#!/usr/bin/env python3
"""Generate thin-neck and path-of-gadgets graphs as graph6 records."""

from __future__ import annotations

import argparse
import itertools
import sys
from dataclasses import dataclass


@dataclass
class Gadget:
    adjacency: list[set[int]]
    left: int
    right: int


def empty_graph(order: int) -> list[set[int]]:
    return [set() for _ in range(order)]


def add_edge(graph: list[set[int]], left: int, right: int) -> None:
    if left == right:
        raise ValueError("loops are not allowed")
    graph[left].add(right)
    graph[right].add(left)


def clique(order: int) -> Gadget:
    graph = empty_graph(order)
    for left in range(order):
        for right in range(left + 1, order):
            add_edge(graph, left, right)
    return Gadget(graph, 0, order - 1)


def cycle(order: int) -> Gadget:
    graph = empty_graph(order)
    for vertex in range(order):
        add_edge(graph, vertex, (vertex + 1) % order)
    return Gadget(graph, 0, order // 2)


def biclique(left_size: int, right_size: int) -> Gadget:
    graph = empty_graph(left_size + right_size)
    for left in range(left_size):
        for right in range(left_size, left_size + right_size):
            add_edge(graph, left, right)
    far = left_size - 1 if left_size > 1 else left_size
    return Gadget(graph, 0, far)


def append_disjoint(
    target: list[set[int]],
    source: list[set[int]],
) -> int:
    offset = len(target)
    target.extend(set() for _ in source)
    for left, neighbors in enumerate(source):
        for right in neighbors:
            if left < right:
                add_edge(target, offset + left, offset + right)
    return offset


def bridge_chain(gadgets: tuple[Gadget, ...]) -> list[set[int]]:
    graph: list[set[int]] = []
    previous_right: int | None = None
    for gadget in gadgets:
        offset = append_disjoint(graph, gadget.adjacency)
        left = offset + gadget.left
        right = offset + gadget.right
        if previous_right is not None:
            add_edge(graph, previous_right, left)
        previous_right = right
    return graph


def articulation_chain(gadgets: tuple[Gadget, ...]) -> list[set[int]]:
    first, *rest = gadgets
    graph = [set(neighbors) for neighbors in first.adjacency]
    previous_right = first.right
    for gadget in rest:
        mapping: dict[int, int] = {gadget.left: previous_right}
        for vertex in range(len(gadget.adjacency)):
            if vertex != gadget.left:
                mapping[vertex] = len(graph)
                graph.append(set())
        for left, neighbors in enumerate(gadget.adjacency):
            for right in neighbors:
                if left < right:
                    add_edge(graph, mapping[left], mapping[right])
        previous_right = mapping[gadget.right]
    return graph


def layered_path(
    sizes: tuple[int, ...],
    inside: str,
    join: str,
) -> list[set[int]]:
    order = sum(sizes)
    graph = empty_graph(order)
    layers: list[list[int]] = []
    start = 0
    for size in sizes:
        layer = list(range(start, start + size))
        layers.append(layer)
        start += size
        if inside == "clique":
            for left, right in itertools.combinations(layer, 2):
                add_edge(graph, left, right)

    for left_layer, right_layer in itertools.pairwise(layers):
        if join == "complete":
            pairs = itertools.product(left_layer, right_layer)
        elif join == "matching":
            pairs = zip(
                left_layer,
                itertools.cycle(right_layer),
                strict=False,
            )
        elif join == "bridge":
            pairs = [(left_layer[-1], right_layer[0])]
        else:
            raise ValueError(join)
        for left, right in pairs:
            add_edge(graph, left, right)
        if join == "matching" and len(right_layer) > len(left_layer):
            for index in range(len(left_layer), len(right_layer)):
                add_edge(
                    graph,
                    left_layer[index % len(left_layer)],
                    right_layer[index],
                )
    return graph


def barbell(
    left_size: int,
    right_size: int,
    neck_edges: int,
) -> list[set[int]]:
    left = clique(left_size)
    right = clique(right_size)
    graph = [set(neighbors) for neighbors in left.adjacency]
    previous = left.right
    for _ in range(max(0, neck_edges - 1)):
        vertex = len(graph)
        graph.append(set())
        add_edge(graph, previous, vertex)
        previous = vertex
    offset = append_disjoint(graph, right.adjacency)
    add_edge(graph, previous, offset + right.left)
    return graph


def graph6(graph: list[set[int]]) -> str:
    order = len(graph)
    if not 0 <= order <= 62:
        raise ValueError("short graph6 supports at most 62 vertices")
    bits = [
        int(left in graph[right])
        for right in range(1, order)
        for left in range(right)
    ]
    while len(bits) % 6:
        bits.append(0)
    encoded = [chr(order + 63)]
    for start in range(0, len(bits), 6):
        value = 0
        for bit in bits[start : start + 6]:
            value = 2 * value + bit
        encoded.append(chr(value + 63))
    return "".join(encoded)


def gadget_catalog() -> tuple[Gadget, ...]:
    gadgets: list[Gadget] = []
    gadgets.extend(clique(order) for order in range(3, 8))
    gadgets.extend(cycle(order) for order in (3, 5, 7, 9))
    gadgets.extend(
        biclique(left, right)
        for left in range(2, 5)
        for right in range(left, 6)
    )
    return tuple(gadgets)


def catalog(max_vertices: int) -> list[str]:
    records: set[str] = set()
    gadgets = gadget_catalog()

    for gadget in gadgets:
        for block_count in range(2, 9):
            blocks = (gadget,) * block_count
            for constructor in (bridge_chain, articulation_chain):
                graph = constructor(blocks)
                if len(graph) <= max_vertices:
                    records.add(graph6(graph))

    small = tuple(
        gadget
        for gadget in gadgets
        if len(gadget.adjacency) <= 5
    )
    for block_count in range(2, 6):
        for blocks in itertools.product(small, repeat=block_count):
            if sum(len(block.adjacency) for block in blocks) > max_vertices:
                continue
            records.add(graph6(bridge_chain(blocks)))
            articulation_order = (
                sum(len(block.adjacency) for block in blocks)
                - block_count
                + 1
            )
            if articulation_order <= max_vertices:
                records.add(graph6(articulation_chain(blocks)))

    for layer_count in range(2, 13):
        for width in range(1, 6):
            sizes = (width,) * layer_count
            if sum(sizes) > max_vertices:
                continue
            for inside in ("empty", "clique"):
                for join in ("bridge", "matching", "complete"):
                    if (
                        inside == "empty"
                        and width > 1
                        and join != "complete"
                    ):
                        continue
                    records.add(graph6(layered_path(sizes, inside, join)))

        for widths in itertools.product((1, 2, 3), repeat=layer_count):
            if sum(widths) > max_vertices:
                continue
            for inside, join in (
                ("clique", "complete"),
                ("empty", "complete"),
                ("clique", "bridge"),
            ):
                records.add(graph6(layered_path(widths, inside, join)))

    for left_size in range(3, 10):
        for right_size in range(3, 10):
            for neck_edges in range(1, 13):
                graph = barbell(left_size, right_size, neck_edges)
                if len(graph) <= max_vertices:
                    records.add(graph6(graph))

    return sorted(records, key=lambda record: (ord(record[0]) - 63, record))


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--max-vertices", type=int, default=20)
    parser.add_argument("--count-only", action="store_true")
    args = parser.parse_args()
    if not 2 <= args.max_vertices <= 30:
        parser.error("--max-vertices must be from 2 through 30")
    records = catalog(args.max_vertices)
    print(
        f"path-of-gadgets records={len(records)} "
        f"max_vertices={args.max_vertices}",
        file=sys.stderr,
    )
    if not args.count_only:
        print(*records, sep="\n")


if __name__ == "__main__":
    main()
