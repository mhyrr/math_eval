#!/usr/bin/env python3
"""Independent, brute-force verifier for WOWII graph certificates.

This intentionally shares no code with the C search path.  It is slow in the
useful way: for a proposed small witness it enumerates vertex subsets from
largest to smallest and returns the first exact optimum.
"""

from __future__ import annotations

import argparse
import itertools
import json
import math
from collections import deque


def parse_graph6(record: str) -> list[set[int]]:
    record = record.strip()
    if record.startswith(">>graph6<<"):
        record = record.removeprefix(">>graph6<<")
    if not record:
        raise ValueError("empty graph6 record")
    first = ord(record[0])
    if not 63 <= first <= 125:
        raise ValueError("only short graph6 records are supported")
    order = first - 63
    bit_count = order * (order - 1) // 2
    encoded_count = (bit_count + 5) // 6
    if len(record) != encoded_count + 1:
        raise ValueError("wrong graph6 record length")

    graph = [set() for _ in range(order)]
    bit_index = 0
    for right in range(1, order):
        for left in range(right):
            encoded = ord(record[1 + bit_index // 6]) - 63
            if not 0 <= encoded <= 63:
                raise ValueError("invalid graph6 character")
            if encoded & (1 << (5 - bit_index % 6)):
                graph[left].add(right)
                graph[right].add(left)
            bit_index += 1
    return graph


def edges(graph: list[set[int]]) -> list[list[int]]:
    return [
        [left, right]
        for left, neighbors in enumerate(graph)
        for right in sorted(neighbors)
        if left < right
    ]


def residue(graph: list[set[int]]) -> int:
    degrees = [len(neighbors) for neighbors in graph]
    while degrees:
        degrees.sort(reverse=True)
        degree = degrees.pop(0)
        if degree == 0:
            return len(degrees) + 1
        if degree > len(degrees):
            raise ValueError("nongraphical degree sequence")
        for index in range(degree):
            degrees[index] -= 1
            if degrees[index] < 0:
                raise ValueError("nongraphical degree sequence")
    return 0


def diameter(graph: list[set[int]]) -> int:
    if not graph:
        raise ValueError("empty graph")
    answer = 0
    for source in range(len(graph)):
        distances = [-1] * len(graph)
        distances[source] = 0
        queue = deque([source])
        while queue:
            vertex = queue.popleft()
            for neighbor in graph[vertex]:
                if distances[neighbor] != -1:
                    continue
                distances[neighbor] = distances[vertex] + 1
                queue.append(neighbor)
        if -1 in distances:
            raise ValueError("disconnected graph")
        answer = max(answer, max(distances))
    return answer


def is_forest(graph: list[set[int]], vertices: tuple[int, ...]) -> bool:
    allowed = set(vertices)
    visited: set[int] = set()

    def visit(vertex: int, parent: int | None) -> bool:
        visited.add(vertex)
        for neighbor in graph[vertex] & allowed:
            if neighbor == parent:
                continue
            if neighbor in visited or not visit(neighbor, vertex):
                return False
        return True

    return all(
        vertex in visited or visit(vertex, None)
        for vertex in vertices
    )


def is_bipartite(graph: list[set[int]], vertices: tuple[int, ...]) -> bool:
    allowed = set(vertices)
    colors: dict[int, int] = {}
    for source in vertices:
        if source in colors:
            continue
        colors[source] = 0
        queue = deque([source])
        while queue:
            vertex = queue.popleft()
            for neighbor in graph[vertex] & allowed:
                if neighbor not in colors:
                    colors[neighbor] = 1 - colors[vertex]
                    queue.append(neighbor)
                elif colors[neighbor] == colors[vertex]:
                    return False
    return True


def largest_induced(
    graph: list[set[int]],
    predicate,
) -> tuple[int, ...]:
    vertices = range(len(graph))
    for size in range(len(graph), -1, -1):
        for candidate in itertools.combinations(vertices, size):
            if predicate(graph, candidate):
                return candidate
    raise AssertionError("the empty set should always qualify")


def evaluate(record: str, mode: int) -> dict[str, object]:
    graph = parse_graph6(record)
    if len(graph) < 2:
        raise ValueError("the formal statement assumes a nontrivial graph")
    graph_diameter = diameter(graph)
    graph_residue = residue(graph)
    forest = largest_induced(graph, is_forest)

    result: dict[str, object] = {
        "mode": mode,
        "graph6": record.strip(),
        "n": len(graph),
        "m": sum(map(len, graph)) // 2,
        "edges": edges(graph),
        "residue": graph_residue,
        "diameter": graph_diameter,
        "induced_forest": len(forest),
        "forest_vertices": list(forest),
    }
    if mode == 61:
        diameter_term = (graph_diameter + 2) // 3
        bound = graph_residue + diameter_term
        result["ceil_diameter_over_3"] = diameter_term
    elif mode == 59:
        bipartite = largest_induced(graph, is_bipartite)
        product = graph_residue * len(bipartite)
        root = math.isqrt(product)
        bound = root if root * root == product else root + 1
        result["induced_bipartite"] = len(bipartite)
        result["bipartite_vertices"] = list(bipartite)
    else:
        raise ValueError("mode must be 59 or 61")
    result["bound"] = bound
    result["phi"] = bound - len(forest)
    return result


def self_test() -> None:
    checks = {
        "A_": {
            "n": 2,
            "residue": 1,
            "diameter": 1,
            "induced_forest": 2,
            "phi": 0,
        },
        "Bw": {
            "n": 3,
            "residue": 1,
            "diameter": 1,
            "induced_forest": 2,
            "phi": 0,
        },
        "Cl": {
            "n": 4,
            "residue": 2,
            "diameter": 2,
            "induced_forest": 3,
            "phi": 0,
        },
    }
    for record, expected in checks.items():
        actual = evaluate(record, 61)
        for key, value in expected.items():
            if actual[key] != value:
                raise AssertionError(
                    f"{record}: {key}={actual[key]!r}, expected {value!r}"
                )
    print("self-test: PASS")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("graph6", nargs="?")
    parser.add_argument("--mode", type=int, choices=(59, 61), default=61)
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    if args.self_test:
        self_test()
        return
    if args.graph6 is None:
        parser.error("graph6 is required unless --self-test is used")
    print(json.dumps(evaluate(args.graph6, args.mode), indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
