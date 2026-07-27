#!/usr/bin/env python3
"""Classify one-vertex-critical graphs for the WOWII 61 proof attack.

The C diagnostic performs the large exact scan.  This independent Python pass
only sees the small critical residue and computes slower structural features:
articulation points, biconnected blocks, twins, alpha, and the exact forest
gap.  It intentionally imports the brute-force certificate verifier rather
than the optimized evaluator.
"""

from __future__ import annotations

import argparse
import collections
import itertools
import json
import sys
from collections.abc import Iterable

from verify import diameter, edges, is_forest, largest_induced, parse_graph6, residue


Graph = list[set[int]]
Edge = tuple[int, int]


def normalized_edge(left: int, right: int) -> Edge:
    return (left, right) if left < right else (right, left)


def components(graph: Graph, skip: int | None = None) -> list[set[int]]:
    unseen = set(range(len(graph)))
    if skip is not None:
        unseen.discard(skip)
    answer: list[set[int]] = []
    while unseen:
        source = next(iter(unseen))
        reached = {source}
        stack = [source]
        unseen.remove(source)
        while stack:
            vertex = stack.pop()
            fresh = (graph[vertex] & unseen) - ({skip} if skip is not None else set())
            reached.update(fresh)
            unseen.difference_update(fresh)
            stack.extend(fresh)
        answer.append(reached)
    return answer


def delete_vertex(graph: Graph, deleted: int) -> Graph:
    mapping = {
        old: new
        for new, old in enumerate(vertex for vertex in range(len(graph)) if vertex != deleted)
    }
    smaller = [set() for _ in range(len(graph) - 1)]
    for left, neighbors in enumerate(graph):
        if left == deleted:
            continue
        for right in neighbors:
            if right == deleted:
                continue
            smaller[mapping[left]].add(mapping[right])
    return smaller


def induced_subgraph(graph: Graph, vertices: set[int]) -> Graph:
    ordered = sorted(vertices)
    mapping = {old: new for new, old in enumerate(ordered)}
    return [
        {mapping[neighbor] for neighbor in graph[old] if neighbor in vertices}
        for old in ordered
    ]


def articulation_points(graph: Graph) -> set[int]:
    if len(graph) <= 2:
        return set()
    return {
        vertex
        for vertex in range(len(graph))
        if len(components(graph, skip=vertex)) > 1
    }


def biconnected_blocks(graph: Graph) -> list[set[Edge]]:
    """Return edge sets of the biconnected blocks, including bridges."""

    order = len(graph)
    discovery = [-1] * order
    low = [-1] * order
    parent = [-1] * order
    edge_stack: list[Edge] = []
    blocks: list[set[Edge]] = []
    clock = 0

    def visit(vertex: int) -> None:
        nonlocal clock
        discovery[vertex] = low[vertex] = clock
        clock += 1
        for neighbor in sorted(graph[vertex]):
            edge = normalized_edge(vertex, neighbor)
            if discovery[neighbor] == -1:
                parent[neighbor] = vertex
                edge_stack.append(edge)
                visit(neighbor)
                low[vertex] = min(low[vertex], low[neighbor])
                if low[neighbor] >= discovery[vertex]:
                    block: set[Edge] = set()
                    while edge_stack:
                        popped = edge_stack.pop()
                        block.add(popped)
                        if popped == edge:
                            break
                    blocks.append(block)
            elif neighbor != parent[vertex] and discovery[neighbor] < discovery[vertex]:
                edge_stack.append(edge)
                low[vertex] = min(low[vertex], discovery[neighbor])

    for source in range(order):
        if discovery[source] == -1:
            visit(source)
        if edge_stack:
            blocks.append(set(edge_stack))
            edge_stack.clear()
    return blocks


def block_features(block: set[Edge]) -> dict[str, object]:
    vertices = {vertex for edge in block for vertex in edge}
    degrees = collections.Counter(vertex for edge in block for vertex in edge)
    order = len(vertices)
    size = len(block)
    complete = size == order * (order - 1) // 2
    cycle = order >= 3 and size == order and set(degrees.values()) == {2}

    colors: dict[int, int] = {}
    block_neighbors = {vertex: set() for vertex in vertices}
    for left, right in block:
        block_neighbors[left].add(right)
        block_neighbors[right].add(left)
    bipartite = True
    for source in vertices:
        if source in colors:
            continue
        colors[source] = 0
        stack = [source]
        while stack:
            vertex = stack.pop()
            for neighbor in block_neighbors[vertex]:
                if neighbor not in colors:
                    colors[neighbor] = 1 - colors[vertex]
                    stack.append(neighbor)
                elif colors[neighbor] == colors[vertex]:
                    bipartite = False
    complete_bipartite = False
    if bipartite:
        left = {vertex for vertex, color in colors.items() if color == 0}
        right = vertices - left
        complete_bipartite = (
            bool(left)
            and bool(right)
            and size == len(left) * len(right)
        )

    if size == 1:
        kind = "edge"
    elif cycle:
        kind = f"cycle{order}"
    elif complete:
        kind = f"clique{order}"
    elif complete_bipartite:
        sides = sorted((sum(color == 0 for color in colors.values()),
                        sum(color == 1 for color in colors.values())))
        kind = f"biclique{sides[0]}x{sides[1]}"
    else:
        kind = f"other{order}v{size}e"
    return {
        "vertices": order,
        "edges": size,
        "complete": complete,
        "cycle": cycle,
        "complete_bipartite": complete_bipartite,
        "kind": kind,
    }


def is_independent(graph: Graph, vertices: tuple[int, ...]) -> bool:
    chosen = set(vertices)
    return all(not (graph[vertex] & chosen) for vertex in vertices)


def all_pairs_distances(graph: Graph) -> list[list[int]]:
    matrix: list[list[int]] = []
    for source in range(len(graph)):
        distances = [-1] * len(graph)
        distances[source] = 0
        queue = collections.deque([source])
        while queue:
            vertex = queue.popleft()
            for neighbor in graph[vertex]:
                if distances[neighbor] == -1:
                    distances[neighbor] = distances[vertex] + 1
                    queue.append(neighbor)
        matrix.append(distances)
    return matrix


def twin_counts(graph: Graph) -> tuple[int, int]:
    false_twins = 0
    true_twins = 0
    for left, right in itertools.combinations(range(len(graph)), 2):
        if graph[left] == graph[right]:
            false_twins += 1
        closed_left = graph[left] | {left}
        closed_right = graph[right] | {right}
        if closed_left == closed_right:
            true_twins += 1
    return false_twins, true_twins


def bound(graph: Graph) -> int:
    return residue(graph) + (diameter(graph) + 2) // 3


def deletion_critical(graph: Graph) -> bool:
    original = bound(graph)
    for vertex in range(len(graph)):
        smaller = delete_vertex(graph, vertex)
        if len(components(smaller)) == 1 and bound(smaller) >= original:
            return False
    return True


def profile(record: str) -> dict[str, object]:
    graph = parse_graph6(record)
    graph_edges = edges(graph)
    graph_residue = residue(graph)
    graph_diameter = diameter(graph)
    forest = largest_induced(graph, is_forest)
    independent = largest_induced(graph, is_independent)
    cuts = articulation_points(graph)
    raw_blocks = biconnected_blocks(graph)
    blocks = [block_features(block) for block in raw_blocks]
    degree_sequence = sorted((len(neighbors) for neighbors in graph), reverse=True)
    maximum = max(degree_sequence)
    maximum_vertices = {
        vertex for vertex, neighbors in enumerate(graph) if len(neighbors) == maximum
    }
    false_twins, true_twins = twin_counts(graph)
    distances = all_pairs_distances(graph)
    peripheral = {
        vertex
        for vertex, row in enumerate(distances)
        if max(row) == graph_diameter
    }
    cyclomatic = len(graph_edges) - len(graph) + 1
    cactus = all(
        bool(features["edges"] == 1 or features["cycle"])
        for features in blocks
    )
    block_graph = all(bool(features["complete"]) for features in blocks)
    diameter_term = (graph_diameter + 2) // 3
    graph_bound = graph_residue + diameter_term
    cut_component_margins: dict[int, int] = {}
    for cut in cuts:
        component_bound = sum(
            bound(induced_subgraph(graph, component))
            for component in components(graph, skip=cut)
        )
        cut_component_margins[cut] = component_bound - graph_bound
    best_cut_component_margin = (
        max(cut_component_margins.values())
        if cut_component_margins
        else -len(graph)
    )
    maximum_cut_component_margin = (
        max(
            (
                margin
                for vertex, margin in cut_component_margins.items()
                if vertex in maximum_vertices
            ),
            default=-len(graph),
        )
    )

    return {
        "graph6": record.strip(),
        "n": len(graph),
        "m": len(graph_edges),
        "degree_sequence": degree_sequence,
        "minimum_degree": min(degree_sequence),
        "maximum_degree": maximum,
        "leaves": degree_sequence.count(1),
        "articulation_points": len(cuts),
        "all_maximum_degree_vertices_are_cuts": maximum_vertices <= cuts,
        "best_cut_component_margin": best_cut_component_margin,
        "maximum_cut_component_margin": maximum_cut_component_margin,
        "cut_component_reducible": best_cut_component_margin >= 0,
        "maximum_cut_component_reducible": maximum_cut_component_margin >= 0,
        "connected_vertex_deletions": len(graph) - len(cuts),
        "blocks": [features["kind"] for features in blocks],
        "block_count": len(blocks),
        "cactus": cactus,
        "block_graph": block_graph,
        "cyclomatic": cyclomatic,
        "false_twin_pairs": false_twins,
        "true_twin_pairs": true_twins,
        "peripheral_vertices": len(peripheral),
        "residue": graph_residue,
        "alpha": len(independent),
        "alpha_minus_residue": len(independent) - graph_residue,
        "diameter": graph_diameter,
        "diameter_term": diameter_term,
        "forest": len(forest),
        "forest_minus_alpha": len(forest) - len(independent),
        "bound": graph_bound,
        "slack": len(forest) - graph_bound,
        "critical_verified": deletion_critical(graph),
    }


def signature(item: dict[str, object]) -> tuple[object, ...]:
    return (
        item["diameter"],
        item["cyclomatic"],
        item["leaves"],
        item["articulation_points"],
        item["all_maximum_degree_vertices_are_cuts"],
        item["maximum_cut_component_margin"],
        tuple(item["blocks"]),
        item["alpha_minus_residue"],
        item["forest_minus_alpha"],
        item["slack"],
    )


def print_summary(profiles: list[dict[str, object]], top: int) -> None:
    if not profiles:
        raise SystemExit("error: no graph6 records were read")
    if not all(item["critical_verified"] for item in profiles):
        raise SystemExit("error: input contains a graph that is not deletion-critical")

    print(
        "SUMMARY"
        f" records={len(profiles)}"
        f" equality={sum(item['slack'] == 0 for item in profiles)}"
        f" all_max_degree_cut={sum(bool(item['all_maximum_degree_vertices_are_cuts']) for item in profiles)}"
        f" cut_component_reducible={sum(bool(item['cut_component_reducible']) for item in profiles)}"
        f" max_cut_component_reducible={sum(bool(item['maximum_cut_component_reducible']) for item in profiles)}"
        f" cactus={sum(bool(item['cactus']) for item in profiles)}"
        f" block_graph={sum(bool(item['block_graph']) for item in profiles)}"
        f" with_leaves={sum(int(item['leaves']) > 0 for item in profiles)}"
        f" with_twins={sum(int(item['false_twin_pairs']) + int(item['true_twin_pairs']) > 0 for item in profiles)}"
    )
    for field in (
        "diameter",
        "cyclomatic",
        "leaves",
        "articulation_points",
        "block_count",
        "best_cut_component_margin",
        "maximum_cut_component_margin",
        "alpha_minus_residue",
        "forest_minus_alpha",
        "slack",
    ):
        counts = collections.Counter(item[field] for item in profiles)
        encoded = ",".join(f"{value}:{count}" for value, count in sorted(counts.items()))
        print(f"DISTRIBUTION field={field} values={encoded}")

    signatures = collections.Counter(signature(item) for item in profiles)
    for value, count in signatures.most_common(top):
        print(f"SIGNATURE count={count} value={value!r}")


def self_test() -> None:
    path = parse_graph6("DhC")
    if len(biconnected_blocks(path)) != 4:
        raise AssertionError("path should have four bridge blocks")

    critical = profile("H????~e")
    expected = {
        "residue": 6,
        "alpha": 7,
        "diameter": 4,
        "forest": 8,
        "slack": 0,
        "critical_verified": True,
        "all_maximum_degree_vertices_are_cuts": True,
    }
    for key, value in expected.items():
        if critical[key] != value:
            raise AssertionError(
                f"critical profile {key}={critical[key]!r}, expected {value!r}"
            )
    print("self-test: PASS")


def records_from_stream(stream: Iterable[str]) -> list[str]:
    return [
        line.strip()
        for line in stream
        if line.strip() and not line.startswith(">>graph6<<")
    ]


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--records", action="store_true")
    parser.add_argument("--self-test", action="store_true")
    parser.add_argument("--top", type=int, default=20)
    args = parser.parse_args()
    if args.self_test:
        self_test()
        return

    profiles = [profile(record) for record in records_from_stream(sys.stdin)]
    if args.json:
        print(json.dumps(profiles, indent=2, sort_keys=True))
    elif args.records:
        for item in profiles:
            print(json.dumps(item, sort_keys=True, separators=(",", ":")))
    else:
        print_summary(profiles, args.top)


if __name__ == "__main__":
    main()
