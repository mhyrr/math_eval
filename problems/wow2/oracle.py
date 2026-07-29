#!/usr/bin/env python3
"""Independent exact oracle for every registered WOWII conjecture.

`verify.py` stays what it has always been: subset enumeration from the top,
slow in the useful way, correct by inspection, and limited to small graphs. It
is the right check for a nine-vertex record and useless for a forty-one vertex
one, which is what the parametric lane produces.

This file covers the same statements at the sizes that lane reaches. It shares
no code with the C path -- different language, integer bitmasks instead of
`uint64_t`, maximum independent set through complement cliques with a colouring
bound rather than the C search's degree branching -- so agreement between the
two is evidence rather than a tautology. Where a quantity is small enough that
enumeration is honest, it enumerates.

    python3 oracle.py --mode 58 GRAPH6
    python3 oracle.py --self-test
    python3 oracle.py --mode 109 --check-witness GRAPH6

The last form exits non-zero unless the graph really is a counterexample,
which is what a claimed witness should have to survive.
"""

from __future__ import annotations

import argparse
import itertools
import json
import sys
from fractions import Fraction

sys.setrecursionlimit(100000)


# ------------------------------------------------------------------ graph io

def parse_graph6(record: str) -> list[int]:
    """Returns adjacency as a list of integer bitmasks."""
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
    if len(record) != (bit_count + 5) // 6 + 1:
        raise ValueError("wrong graph6 record length")

    adjacency = [0] * order
    index = 0
    for right in range(1, order):
        for left in range(right):
            encoded = ord(record[1 + index // 6]) - 63
            if encoded & (1 << (5 - index % 6)):
                adjacency[left] |= 1 << right
                adjacency[right] |= 1 << left
            index += 1
    return adjacency


def bits(mask: int):
    while mask:
        low = mask & -mask
        yield low.bit_length() - 1
        mask ^= low


def popcount(mask: int) -> int:
    return bin(mask).count("1")


def full_mask(adjacency: list[int]) -> int:
    return (1 << len(adjacency)) - 1


# --------------------------------------------------------------- basic facts

def degree(adjacency: list[int], vertex: int) -> int:
    return popcount(adjacency[vertex])


def edge_count(adjacency: list[int]) -> int:
    return sum(popcount(row) for row in adjacency) // 2


def complement(adjacency: list[int]) -> list[int]:
    universe = full_mask(adjacency)
    return [universe & ~row & ~(1 << v) for v, row in enumerate(adjacency)]


def connected(adjacency: list[int], within: int | None = None) -> bool:
    within = full_mask(adjacency) if within is None else within
    if within == 0:
        return False
    start = (within & -within).bit_length() - 1
    seen = 1 << start
    stack = [start]
    while stack:
        vertex = stack.pop()
        fresh = adjacency[vertex] & within & ~seen
        seen |= fresh
        stack.extend(bits(fresh))
    return seen == within


def distances(adjacency: list[int]) -> list[list[int]]:
    order = len(adjacency)
    table = []
    for source in range(order):
        row = [-1] * order
        row[source] = 0
        frontier = [source]
        while frontier:
            nxt = []
            for vertex in frontier:
                for neighbor in bits(adjacency[vertex]):
                    if row[neighbor] < 0:
                        row[neighbor] = row[vertex] + 1
                        nxt.append(neighbor)
            frontier = nxt
        table.append(row)
    return table


def residue(adjacency: list[int]) -> int:
    sequence = sorted((popcount(row) for row in adjacency), reverse=True)
    while sequence:
        head = sequence.pop(0)
        if head == 0:
            return len(sequence) + 1
        if head > len(sequence):
            raise ValueError("nongraphical degree sequence")
        for index in range(head):
            sequence[index] -= 1
        sequence.sort(reverse=True)
    return 0


def havel_hakimi_zero_step(adjacency: list[int]) -> int:
    sequence = sorted((popcount(row) for row in adjacency), reverse=True)
    step = 0
    while True:
        if not sequence or sequence[-1] == 0:
            return step
        head = sequence.pop(0)
        for index in range(head):
            sequence[index] -= 1
        sequence.sort(reverse=True)
        step += 1


def girth(adjacency: list[int]) -> int:
    """Shortest cycle length, 0 when acyclic, by enumerating induced cycles.

    A shortest cycle has no chord, so it is induced; searching only induced
    2-regular connected sets is a different route to the same number than the
    C side's breadth-first sweep.
    """
    order = len(adjacency)
    best = 0
    for size in range(3, order + 1):
        for candidate in itertools.combinations(range(order), size):
            mask = 0
            for vertex in candidate:
                mask |= 1 << vertex
            if all(popcount(adjacency[v] & mask) == 2 for v in candidate):
                if connected(adjacency, mask):
                    return size
        if best:
            break
    return 0


# ------------------------------------------------- independence and cliques

def max_clique(adjacency: list[int], within: int) -> int:
    """Tomita-style branch and bound with a greedy colouring bound."""

    best = 0

    def colour_order(mask: int) -> list[tuple[int, int]]:
        ordered: list[tuple[int, int]] = []
        colour = 0
        remaining = mask
        while remaining:
            colour += 1
            available = remaining
            while available:
                vertex = (available & -available).bit_length() - 1
                available &= ~(1 << vertex)
                available &= ~adjacency[vertex]
                remaining &= ~(1 << vertex)
                ordered.append((vertex, colour))
        return ordered

    def expand(mask: int, size: int) -> None:
        nonlocal best
        if mask == 0:
            best = max(best, size)
            return
        for vertex, colour in reversed(colour_order(mask)):
            if size + colour <= best:
                return
            expand(mask & adjacency[vertex] & ~((1 << (vertex + 1)) - 1)
                   | (mask & adjacency[vertex] & ((1 << vertex) - 1)),
                   size + 1)
            mask &= ~(1 << vertex)

    def simple(mask: int, size: int) -> None:
        nonlocal best
        if size > best:
            best = size
        ordered = colour_order(mask)
        for vertex, colour in reversed(ordered):
            if size + colour <= best:
                return
            simple(mask & adjacency[vertex], size + 1)
            mask &= ~(1 << vertex)

    simple(within, 0)
    return best


def independence_number(adjacency: list[int], within: int | None = None) -> int:
    """Maximum independent set as a maximum clique in the complement."""
    within = full_mask(adjacency) if within is None else within
    return max_clique(complement(adjacency), within)


def local_independence(adjacency: list[int], vertex: int) -> int:
    return independence_number(adjacency, adjacency[vertex])


def local_independence_values(adjacency: list[int]) -> list[int]:
    return [local_independence(adjacency, v) for v in range(len(adjacency))]


# ------------------------------------------------- induced forests and trees

def subset_is_forest(adjacency: list[int], mask: int) -> bool:
    edges_inside = sum(popcount(adjacency[v] & mask) for v in bits(mask)) // 2
    components = 0
    remaining = mask
    while remaining:
        components += 1
        start = (remaining & -remaining).bit_length() - 1
        seen = 1 << start
        stack = [start]
        while stack:
            vertex = stack.pop()
            fresh = adjacency[vertex] & remaining & ~seen
            seen |= fresh
            stack.extend(bits(fresh))
        remaining &= ~seen
    return edges_inside + components == popcount(mask)


def subset_is_bipartite(adjacency: list[int], mask: int) -> bool:
    colour: dict[int, int] = {}
    for source in bits(mask):
        if source in colour:
            continue
        colour[source] = 0
        stack = [source]
        while stack:
            vertex = stack.pop()
            for neighbor in bits(adjacency[vertex] & mask):
                if neighbor not in colour:
                    colour[neighbor] = 1 - colour[vertex]
                    stack.append(neighbor)
                elif colour[neighbor] == colour[vertex]:
                    return False
    return True


def _find_obstruction(adjacency: list[int], mask: int, odd_only: bool) -> int:
    """Returns the vertex set of a cycle inside `mask`, or 0 if there is none."""
    if not odd_only:
        parent: dict[int, int] = {}
        seen = 0
        for source in bits(mask):
            if seen & (1 << source):
                continue
            stack = [(source, -1)]
            parent[source] = -1
            seen |= 1 << source
            while stack:
                vertex, came_from = stack.pop()
                for neighbor in bits(adjacency[vertex] & mask):
                    if neighbor == came_from:
                        continue
                    if seen & (1 << neighbor):
                        cycle = (1 << vertex) | (1 << neighbor)
                        walk = vertex
                        depth = 0
                        while walk != neighbor and walk != -1 and depth <= len(adjacency):
                            walk = parent.get(walk, -1)
                            depth += 1
                            if walk != -1:
                                cycle |= 1 << walk
                        return cycle
                    seen |= 1 << neighbor
                    parent[neighbor] = vertex
                    stack.append((neighbor, vertex))
        return 0

    colour: dict[int, int] = {}
    parent = {}
    depth = {}
    for source in bits(mask):
        if source in colour:
            continue
        colour[source] = 0
        parent[source] = -1
        depth[source] = 0
        frontier = [source]
        while frontier:
            nxt = []
            for vertex in frontier:
                for neighbor in bits(adjacency[vertex] & mask):
                    if neighbor not in colour:
                        colour[neighbor] = 1 - colour[vertex]
                        parent[neighbor] = vertex
                        depth[neighbor] = depth[vertex] + 1
                        nxt.append(neighbor)
                    elif colour[neighbor] == colour[vertex]:
                        left, right = vertex, neighbor
                        cycle = 0
                        while depth[left] > depth[right]:
                            cycle |= 1 << left
                            left = parent[left]
                        while depth[right] > depth[left]:
                            cycle |= 1 << right
                            right = parent[right]
                        while left != right:
                            cycle |= (1 << left) | (1 << right)
                            left = parent[left]
                            right = parent[right]
                        return cycle | (1 << left)
            frontier = nxt
    return 0


def _largest_avoiding(adjacency: list[int], odd_only: bool) -> int:
    universe = full_mask(adjacency)
    best = [0]
    seen: set[int] = set()

    def search(mask: int) -> None:
        if popcount(mask) <= best[0] or mask in seen:
            return
        seen.add(mask)
        obstruction = _find_obstruction(adjacency, mask, odd_only)
        if obstruction == 0:
            best[0] = max(best[0], popcount(mask))
            return
        for vertex in bits(obstruction):
            search(mask & ~(1 << vertex))

    search(universe)
    return best[0]


def largest_induced_forest(adjacency: list[int]) -> int:
    return _largest_avoiding(adjacency, odd_only=False)


def largest_induced_bipartite(adjacency: list[int]) -> int:
    return _largest_avoiding(adjacency, odd_only=True)


def _largest_induced_growth(adjacency: list[int], cap: int,
                            paths_only: bool) -> int:
    order = len(adjacency)
    if order == 0:
        return 0
    best = [1]

    def grow(chosen: int, banned: int) -> None:
        best[0] = max(best[0], popcount(chosen))
        if best[0] >= cap:
            return
        reach = chosen
        stack = list(bits(chosen))
        while stack:
            vertex = stack.pop()
            fresh = adjacency[vertex] & ~reach & ~banned
            reach |= fresh
            stack.extend(bits(fresh))
        if popcount(reach) <= best[0]:
            return
        frontier = 0
        for vertex in bits(chosen):
            frontier |= adjacency[vertex]
        frontier &= ~chosen & ~banned
        for vertex in bits(frontier):
            inside = adjacency[vertex] & chosen
            if popcount(inside) != 1:
                banned |= 1 << vertex
                continue
            if paths_only:
                anchor = (inside & -inside).bit_length() - 1
                if popcount(adjacency[anchor] & chosen) > 1:
                    banned |= 1 << vertex
                    continue
            grow(chosen | (1 << vertex), banned)
            banned |= 1 << vertex
            if best[0] >= cap:
                return

    banned = 0
    for vertex in range(order):
        if best[0] >= cap:
            break
        grow(1 << vertex, banned)
        banned |= 1 << vertex
    return best[0]


def largest_induced_tree(adjacency: list[int]) -> int:
    return _largest_induced_growth(adjacency, largest_induced_forest(adjacency),
                                   False)


def largest_induced_path(adjacency: list[int]) -> int:
    return _largest_induced_growth(adjacency, largest_induced_tree(adjacency),
                                   True)


# --------------------------------------- spanning trees, paths, domination

def max_leaf_spanning_tree(adjacency: list[int]) -> int:
    """Order minus the smallest connected dominating set, checked directly.

    The identity fails at the two smallest orders, which are handled on their
    own terms: K(1) is a spanning tree with no degree-one vertex, K(2) is one
    edge with two of them.
    """
    order = len(adjacency)
    if not connected(adjacency):
        return 0
    if order <= 2:
        return 0 if order == 1 else 2
    universe = full_mask(adjacency)
    for size in range(1, order):
        for candidate in itertools.combinations(range(order), size):
            mask = 0
            for vertex in candidate:
                mask |= 1 << vertex
            dominated = mask
            for vertex in candidate:
                dominated |= adjacency[vertex]
            if dominated != universe:
                continue
            if connected(adjacency, mask):
                return order - size
    return 0


def has_hamiltonian_path(adjacency: list[int]) -> int:
    order = len(adjacency)
    if order == 1:
        return True
    ends = [0] * (1 << order)
    for vertex in range(order):
        ends[1 << vertex] = 1 << vertex
    universe = full_mask(adjacency)
    for mask in range(1, 1 << order):
        reachable = ends[mask]
        if not reachable:
            continue
        for vertex in bits(universe & ~mask):
            if adjacency[vertex] & reachable:
                ends[mask | (1 << vertex)] |= 1 << vertex
    return ends[universe] != 0


def path_cover_number(adjacency: list[int]) -> int:
    """Order minus the most edges in a spanning linear forest."""
    order = len(adjacency)
    infinity = order + 1
    cost = [[infinity] * order for _ in range(1 << order)]
    for vertex in range(order):
        cost[1 << vertex][vertex] = 1
    universe = full_mask(adjacency)
    for mask in range(1, 1 << order):
        row = cost[mask]
        for vertex in bits(mask):
            here = row[vertex]
            if here >= infinity:
                continue
            rest = universe & ~mask
            for nxt in bits(adjacency[vertex] & rest):
                target = cost[mask | (1 << nxt)]
                if here < target[nxt]:
                    target[nxt] = here
            for nxt in bits(rest):
                target = cost[mask | (1 << nxt)]
                if here + 1 < target[nxt]:
                    target[nxt] = here + 1
    return min(cost[universe])


def is_total_dominating(adjacency: list[int], mask: int) -> bool:
    return all(adjacency[v] & mask for v in range(len(adjacency)))


def total_domination(adjacency: list[int]) -> tuple[int, bool]:
    """Returns the total domination number and whether G is well dominated.

    Minimality is checked against every proper subset, not just single-vertex
    removals, so this does not assume the upward-closure shortcut the C side
    uses.
    """
    order = len(adjacency)
    smallest = None
    minimal_sizes: set[int] = set()
    for mask in range(1, 1 << order):
        if not is_total_dominating(adjacency, mask):
            continue
        size = popcount(mask)
        if smallest is None or size < smallest:
            smallest = size
        minimal = True
        submask = (mask - 1) & mask
        while True:
            if is_total_dominating(adjacency, submask):
                minimal = False
                break
            if submask == 0:
                break
            submask = (submask - 1) & mask
        if minimal:
            minimal_sizes.add(size)
    if smallest is None:
        return 0, True
    return smallest, len(minimal_sizes) == 1


# ------------------------------------------------------------- eccentricity

def eccentricities(adjacency: list[int]) -> list[int]:
    return [max(row) for row in distances(adjacency)]


def distance_to_set(table: list[list[int]], vertex: int, members: list[int]) -> int:
    if not members:
        return 0
    return min(table[vertex][member] for member in members)


def set_ecc(adjacency: list[int], members: list[int], include_members: bool) -> int:
    table = distances(adjacency)
    order = len(adjacency)
    if include_members:
        if not members:
            return 0
        return max(distance_to_set(table, v, members) for v in range(order))
    outside = [v for v in range(order) if v not in set(members)]
    if not outside:
        return 0
    return max(distance_to_set(table, v, members) for v in outside)


def set_dist_min(adjacency: list[int], members: list[int]) -> int:
    table = distances(adjacency)
    outside = [v for v in range(len(adjacency)) if v not in set(members)]
    if not outside:
        return 0
    return min(distance_to_set(table, v, members) for v in outside)


def graph_square_radius(adjacency: list[int]) -> int:
    table = distances(adjacency)
    order = len(adjacency)
    return min(max((table[u][v] + 1) // 2 for v in range(order))
               for u in range(order))


def has_four_cycle(adjacency: list[int]) -> bool:
    order = len(adjacency)
    return any(popcount(adjacency[u] & adjacency[v]) >= 2
               for u in range(order) for v in range(u + 1, order))


def triangles_at(adjacency: list[int], vertex: int) -> int:
    neighbors = adjacency[vertex]
    return sum(popcount(adjacency[u] & neighbors) for u in bits(neighbors)) // 2


def ceil_natural_log(value: Fraction) -> int:
    """Least k with e^k >= value, decided against certified enclosures of e^k."""
    lower = [
        Fraction(1),
        Fraction(2718281828459, 10 ** 12),
        Fraction(7389056098930, 10 ** 12),
        Fraction(20085536923187, 10 ** 12),
        Fraction(54598150033144, 10 ** 12),
        Fraction(148413159102576, 10 ** 12),
        Fraction(403428793492735, 10 ** 12),
    ]
    if value < 1:
        raise ValueError("average eccentricity below one")
    for power, bound in enumerate(lower):
        if value <= bound:
            return power
        if power and value < bound + Fraction(1, 10 ** 12):
            raise ValueError("value sits inside the enclosure of e^k")
    raise ValueError("average eccentricity beyond e^6")


# ----------------------------------------------------------------- the modes

def evaluate(record: str, mode: str) -> dict[str, object]:
    adjacency = parse_graph6(record)
    order = len(adjacency)
    if order < 2:
        raise ValueError("the statements assume a nontrivial graph")
    if not connected(adjacency):
        raise ValueError("the statements assume a connected graph")

    result: dict[str, object] = {
        "mode": mode,
        "graph6": record.strip(),
        "n": order,
        "m": edge_count(adjacency),
    }

    def finish_inequality(lhs: Fraction, rhs: Fraction) -> dict[str, object]:
        result["lhs"] = str(lhs)
        result["rhs"] = str(rhs)
        result["phi"] = str(lhs - rhs)
        result["witness"] = lhs > rhs
        return result

    def finish_implication(hypothesis: bool, conclusion: bool):
        result["hypothesis"] = hypothesis
        result["conclusion"] = conclusion
        result["witness"] = hypothesis and not conclusion
        return result

    locals_ = None

    def local_values() -> list[int]:
        nonlocal locals_
        if locals_ is None:
            locals_ = local_independence_values(adjacency)
        return locals_

    def local_average() -> Fraction:
        return Fraction(sum(local_values()), order)

    if mode == "59":
        product = residue(adjacency) * largest_induced_bipartite(adjacency)
        root = 0
        while root * root < product:
            root += 1
        result["residue"] = residue(adjacency)
        result["induced_bipartite"] = largest_induced_bipartite(adjacency)
        result["induced_forest"] = largest_induced_forest(adjacency)
        return finish_inequality(Fraction(root),
                                 Fraction(largest_induced_forest(adjacency)))

    if mode == "61":
        table = distances(adjacency)
        diameter = max(max(row) for row in table)
        forest = largest_induced_forest(adjacency)
        result["residue"] = residue(adjacency)
        result["diameter"] = diameter
        result["induced_forest"] = forest
        return finish_inequality(
            Fraction(residue(adjacency) + -(-diameter // 3)), Fraction(forest))

    if mode == "2":
        leaves = max_leaf_spanning_tree(adjacency)
        result["local_independence_average"] = str(local_average())
        result["max_leaf_spanning_tree"] = leaves
        return finish_inequality(2 * (local_average() - 1), Fraction(leaves))

    if mode == "40":
        cover = path_cover_number(adjacency)
        bipartite = largest_induced_bipartite(adjacency)
        forest = largest_induced_forest(adjacency)
        result["path_cover_number"] = cover
        result["induced_bipartite"] = bipartite
        result["induced_forest"] = forest
        inner = Fraction(cover + bipartite + 1, 2)
        return finish_inequality(Fraction(-(-inner.numerator // inner.denominator)),
                                 Fraction(forest))

    if mode == "65":
        degrees = [degree(adjacency, v) for v in range(order)]
        low = [v for v in range(order) if degrees[v] == min(degrees)]
        high = [v for v in range(order) if degrees[v] == max(degrees)]
        near_low = set_dist_min(adjacency, low)
        near_high = set_dist_min(adjacency, high)
        forest = largest_induced_forest(adjacency)
        result["dist_min_min_degree"] = near_low
        result["dist_min_max_degree"] = near_high
        result["induced_forest"] = forest
        return finish_inequality(Fraction(near_low + -(-near_high // 3)),
                                 Fraction(forest))

    if mode == "100":
        other = complement(adjacency)
        if not connected(other):
            raise ValueError("the statement assumes a connected complement")
        squares = sum(popcount(row) ** 2 for row in other)
        maximum = max(local_values())
        bound = 0
        while True:
            slack = 4 * bound - 2 * maximum
            if slack >= 0 and squares <= slack * slack:
                break
            bound += 1
        alpha = independence_number(adjacency)
        result["independence_number"] = alpha
        result["local_independence_max"] = maximum
        result["complement_degree_square_sum"] = squares
        return finish_inequality(Fraction(alpha), Fraction(bound))

    if mode == "133":
        table = distances(adjacency)
        radius = min(max(row) for row in table)
        free = not has_four_cycle(adjacency)
        average = local_average()
        term = average.numerator // average.denominator if free else 1
        path = largest_induced_path(adjacency)
        result["radius"] = radius
        result["four_cycle_free"] = free
        result["largest_induced_path"] = path
        return finish_inequality(Fraction(radius + term), Fraction(path))

    if mode == "141":
        value = girth(adjacency)
        tree = largest_induced_tree(adjacency)
        result["girth"] = value
        result["largest_induced_tree"] = tree
        return finish_inequality(Fraction(value // 2 - 1 + max(local_values())),
                                 Fraction(tree))

    if mode in {"142", "144", "145", "146"}:
        value = girth(adjacency)
        eccs = eccentricities(adjacency)
        boundary = [v for v in range(order) if eccs[v] == max(eccs)]
        centre = [v for v in range(order) if eccs[v] == min(eccs)]
        tree = largest_induced_tree(adjacency)
        result["girth"] = value
        result["largest_induced_tree"] = tree
        if mode == "142":
            reach = set_ecc(adjacency, boundary, True)
            result["boundary_eccentricity"] = reach
            return finish_inequality(Fraction(2, 3) * value + reach,
                                     Fraction(tree))
        if mode == "144":
            reach = set_ecc(adjacency, centre, False)
            result["center_eccentricity"] = reach
            return finish_inequality(Fraction(value - 1 + reach), Fraction(tree))
        reach = set_ecc(adjacency, boundary, True)
        result["boundary_eccentricity"] = reach
        if mode == "145":
            other = complement(adjacency)
            minimum = min(local_independence(other, v) for v in range(order))
            if minimum == 0:
                raise ValueError("the statement assumes a positive lMin")
            result["complement_local_independence_min"] = minimum
            return finish_inequality(Fraction(2 * reach), Fraction(tree * minimum))
        square = graph_square_radius(adjacency)
        if square == 0:
            raise ValueError("the statement assumes a positive square radius")
        result["graph_square_radius"] = square
        return finish_inequality(Fraction(2 * reach), Fraction(tree * square))

    if mode == "160":
        maximum = max(local_values())
        triangles = max(triangles_at(adjacency, v) for v in range(order))
        free = not has_four_cycle(adjacency)
        leaves = max_leaf_spanning_tree(adjacency)
        result["local_independence_max"] = maximum
        result["max_triangles_at_vertex"] = triangles
        result["four_cycle_free"] = free
        result["max_leaf_spanning_tree"] = leaves
        return finish_inequality(
            Fraction(maximum + (triangles if free else 0)), Fraction(leaves))

    if mode == "194":
        alpha = independence_number(adjacency)
        result["independence_number"] = alpha
        return finish_implication(alpha <= 1 + local_average(),
                                  has_hamiltonian_path(adjacency))

    if mode == "198a":
        bipartite = largest_induced_bipartite(adjacency)
        average = Fraction(sum(eccentricities(adjacency)), order)
        result["induced_bipartite"] = bipartite
        result["average_eccentricity"] = str(average)
        return finish_implication(bipartite <= 2 + average,
                                  has_hamiltonian_path(adjacency))

    if mode == "200":
        tree = largest_induced_tree(adjacency)
        target = 1 + local_average()
        ceiling = -(-target.numerator // target.denominator)
        result["largest_induced_tree"] = tree
        result["hypothesis_target"] = ceiling
        return finish_implication(tree == ceiling,
                                  has_hamiltonian_path(adjacency))

    if mode == "217":
        leaves = max_leaf_spanning_tree(adjacency)
        indicator = 1 if residue(adjacency) == 2 else 0
        result["max_leaf_spanning_tree"] = leaves
        result["residue"] = residue(adjacency)
        return finish_implication(leaves <= 4 * indicator + 2,
                                  has_hamiltonian_path(adjacency))

    if mode == "291":
        number, _ = total_domination(adjacency)
        step = havel_hakimi_zero_step(adjacency)
        counts = [triangles_at(adjacency, v) for v in range(order)]
        frequency = counts.count(min(counts))
        result["total_domination_number"] = number
        result["havel_hakimi_zero_step"] = step
        result["min_triangle_frequency"] = frequency
        return finish_inequality(Fraction(number), Fraction(step + frequency))

    if mode in {"314", "316", "322"}:
        _, well = total_domination(adjacency)
        result["well_totally_dominated"] = well
        if mode == "314":
            free = all(triangles_at(adjacency, v) == 0 for v in range(order))
            path = largest_induced_path(adjacency)
            result["triangle_free"] = free
            result["largest_induced_path"] = path
            return finish_implication(free and path <= 4, well)
        if mode == "316":
            other = complement(adjacency)
            average = Fraction(2 * edge_count(other), order)
            pendant = sum(1 for v in range(order) if degree(adjacency, v) == 1)
            result["complement_average_degree"] = str(average)
            result["pendant_count"] = pendant
            return finish_implication(average <= pendant, well)
        maximum = max(local_values())
        result["local_independence_max"] = maximum
        return finish_implication(order >= 5 and maximum <= 1, well)

    if mode == "58":
        average = local_average()
        bipartite = largest_induced_bipartite(adjacency)
        forest = largest_induced_forest(adjacency)
        ratio = Fraction(bipartite) / average if average else Fraction(0)
        bound = -(-ratio.numerator // ratio.denominator)
        result["local_independence_average"] = str(average)
        result["induced_bipartite"] = bipartite
        result["induced_forest"] = forest
        return finish_inequality(Fraction(bound), Fraction(forest))

    if mode == "103":
        alpha = independence_number(adjacency)
        bipartite = largest_induced_bipartite(adjacency)
        average = Fraction(sum(eccentricities(adjacency)), order)
        logarithm = ceil_natural_log(average)
        result["independence_number"] = alpha
        result["induced_bipartite"] = bipartite
        result["average_eccentricity"] = str(average)
        result["ceil_log_average_eccentricity"] = logarithm
        return finish_inequality(Fraction(alpha), Fraction(bipartite - logarithm))

    if mode == "109":
        alpha = independence_number(adjacency)
        value = residue(adjacency)
        bipartite = largest_induced_bipartite(adjacency)
        inner = Fraction(value + 2 * bipartite, 3)
        result["independence_number"] = alpha
        result["residue"] = value
        result["induced_bipartite"] = bipartite
        return finish_inequality(Fraction(alpha),
                                 Fraction(inner.numerator // inner.denominator))

    raise ValueError(f"unknown mode {mode}")


# ------------------------------------------------------------------ testing

def self_test() -> None:
    """Small hand-checkable facts, then the two published counterexamples."""
    triangle = parse_graph6("Bw")
    assert independence_number(triangle) == 1
    assert largest_induced_forest(triangle) == 2
    assert girth(triangle) == 3
    assert max_leaf_spanning_tree(triangle) == 2
    assert has_hamiltonian_path(triangle)
    assert path_cover_number(triangle) == 1

    path_four = parse_graph6("Ch")
    assert girth(path_four) == 0, girth(path_four)
    assert largest_induced_path(path_four) == 4
    assert largest_induced_tree(path_four) == 4

    cycle_five = parse_graph6("Dhc")
    assert girth(cycle_five) == 5, girth(cycle_five)
    assert independence_number(cycle_five) == 2
    assert largest_induced_forest(cycle_five) == 4
    assert largest_induced_tree(cycle_five) == 4
    assert largest_induced_path(cycle_five) == 4

    # WOWII 103's published counterexample: a triangle with four leaves on
    # each of two of its vertices. Upstream records alpha = 9, b = 10 and an
    # average eccentricity of 30/11.
    leafy = "J{aCA@?OA??"
    reading = evaluate(leafy, "103")
    assert reading["independence_number"] == 9, reading
    assert reading["induced_bipartite"] == 10, reading
    assert reading["average_eccentricity"] == "30/11", reading
    assert reading["witness"], reading

    # WOWII 109's published counterexample: an empty graph on seven joined to
    # two disjoint triangles. Upstream records alpha = 7, residue = 2, b = 9.
    joined = "L???F~~~~{^p~b"
    reading = evaluate(joined, "109")
    assert reading["independence_number"] == 7, reading
    assert reading["residue"] == 2, reading
    assert reading["induced_bipartite"] == 9, reading
    assert reading["witness"], reading

    print("oracle self-test: PASS")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("graph6", nargs="?")
    parser.add_argument("--mode", default="61")
    parser.add_argument("--self-test", action="store_true")
    parser.add_argument("--check-witness", action="store_true",
                        help="exit non-zero unless the graph refutes the mode")
    args = parser.parse_args()

    if args.self_test:
        self_test()
        return
    if args.graph6 is None:
        parser.error("graph6 is required unless --self-test is used")

    reading = evaluate(args.graph6, args.mode)
    print(json.dumps(reading, indent=2, sort_keys=True))
    if args.check_witness and not reading.get("witness"):
        print("NOT A WITNESS", file=sys.stderr)
        raise SystemExit(1)


if __name__ == "__main__":
    main()
