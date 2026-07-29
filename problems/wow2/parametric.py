#!/usr/bin/env python3
"""Parametric blueprint families for the WOWII sweep.

The exhaustive lane stops at order 9 and the path-of-gadgets catalogue builds
chains of blocks. Neither reaches where the known WOWII counterexamples
actually live:

    conjecture 103   order 11   a triangle carrying four leaves on each of two
                                of its vertices
    conjecture 109   order 13   an empty graph on 7 joined to two disjoint
                                triangles
    conjecture  58   order 79   K(3,3) with one vertex coned over K(73)

All three are algebraic families -- a join, a corona, a cone -- with an integer
parameter pushed until an inequality tips. This module builds families of that
shape and evaluates them along their parameters.

The second job matters more than the first. Conjecture 58's family crosses at
exactly c = 73, so no instantiation this representation can hold (63 vertices)
ever shows it. What a parameter sweep *can* see is that the unrounded bound is
still climbing when the vertex budget runs out. `--trend` reports precisely
that, and extrapolates where the crossing would land, so a family that escapes
the representable range is flagged rather than silently recorded as a survivor.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from dataclasses import dataclass
from fractions import Fraction
from typing import Callable, Iterator

# graph6's short form encodes orders 0 through 62 in a single leading byte, so
# 62 -- not the sweeper's 63-vertex representation limit -- is what a record can
# carry.
MAX_VERTICES = 62

# Instantiation budget, set from the command line. The exact induced-forest and
# induced-bipartite searches cost far more on a dense graph than a sparse one --
# a 60-vertex clique is the worst case in the catalogue -- so a sweep across
# every mode needs a smaller ceiling than a single targeted family does. It is
# always reported, never assumed.
ORDER_BUDGET = MAX_VERTICES

Graph = list[set[int]]


# ------------------------------------------------------------ constructions

def empty(order: int) -> Graph:
    return [set() for _ in range(order)]


def clique(order: int) -> Graph:
    return [{other for other in range(order) if other != v} for v in range(order)]


def path(order: int) -> Graph:
    graph = empty(order)
    for v in range(order - 1):
        graph[v].add(v + 1)
        graph[v + 1].add(v)
    return graph


def cycle(order: int) -> Graph:
    if order < 3:
        return path(order)
    graph = path(order)
    graph[0].add(order - 1)
    graph[order - 1].add(0)
    return graph


def biclique(left: int, right: int) -> Graph:
    graph = empty(left + right)
    for a in range(left):
        for b in range(left, left + right):
            graph[a].add(b)
            graph[b].add(a)
    return graph


def union(*parts: Graph) -> Graph:
    combined: Graph = []
    for part in parts:
        offset = len(combined)
        combined.extend({neighbor + offset for neighbor in row} for row in part)
    return combined


def join(left: Graph, right: Graph) -> Graph:
    """Every vertex of `left` adjacent to every vertex of `right`."""
    combined = union(left, right)
    offset = len(left)
    for a in range(offset):
        for b in range(offset, len(combined)):
            combined[a].add(b)
            combined[b].add(a)
    return combined


def cone(base: Graph, apex: int, target: Graph) -> Graph:
    """Add `target` disjointly, joined only to vertex `apex` of `base`."""
    combined = union(base, target)
    offset = len(base)
    for b in range(offset, len(combined)):
        combined[apex].add(b)
        combined[b].add(apex)
    return combined


def pendants(base: Graph, vertex: int, count: int) -> Graph:
    """Attach `count` fresh leaves to `vertex`."""
    combined = union(base, empty(count))
    for leaf in range(len(base), len(combined)):
        combined[vertex].add(leaf)
        combined[leaf].add(vertex)
    return combined


# ----------------------------------------------------------------- graph6 io

def to_graph6(graph: Graph) -> str:
    order = len(graph)
    if order > MAX_VERTICES:
        raise ValueError(f"order {order} exceeds the sweeper's {MAX_VERTICES}")
    bits: list[int] = []
    for right in range(1, order):
        for left in range(right):
            bits.append(1 if right in graph[left] else 0)
    text = chr(order + 63)
    for start in range(0, len(bits), 6):
        chunk = bits[start:start + 6]
        chunk += [0] * (6 - len(chunk))
        value = 0
        for bit in chunk:
            value = value * 2 + bit
        text += chr(value + 63)
    return text


def connected(graph: Graph) -> bool:
    if not graph:
        return False
    seen = {0}
    stack = [0]
    while stack:
        vertex = stack.pop()
        for neighbor in graph[vertex]:
            if neighbor not in seen:
                seen.add(neighbor)
                stack.append(neighbor)
    return len(seen) == len(graph)


# --------------------------------------------------------------- blueprints

@dataclass(frozen=True)
class Blueprint:
    name: str
    build: Callable[..., Graph]
    ranges: tuple[range, ...]
    note: str = ""

    def instances(self) -> Iterator[tuple[tuple[int, ...], Graph]]:
        for params in _product(self.ranges):
            try:
                graph = self.build(*params)
            except (ValueError, IndexError):
                continue
            if len(graph) < 2 or len(graph) > min(ORDER_BUDGET, MAX_VERTICES):
                continue
            if not connected(graph):
                continue
            yield params, graph


def _product(ranges: tuple[range, ...]) -> Iterator[tuple[int, ...]]:
    if not ranges:
        yield ()
        return
    for head in ranges[0]:
        for tail in _product(ranges[1:]):
            yield (head,) + tail


def catalogue() -> list[Blueprint]:
    """Families first, then the three that already have known answers."""
    return [
        Blueprint(
            "join_empty_cliques",
            lambda e, k, s: join(empty(e), union(*[clique(s)] * k)),
            (range(1, 16), range(1, 5), range(1, 7)),
            "K-bar(e) joined to k disjoint copies of K(s); WOWII 109 at (7,2,3)",
        ),
        Blueprint(
            "triangle_pendants",
            lambda p, q: pendants(pendants(clique(3), 0, p), 1, q),
            (range(0, 16), range(0, 16)),
            "triangle with p and q leaves on two vertices; WOWII 103 at (4,4)",
        ),
        Blueprint(
            "cone_biclique_clique",
            lambda a, b, c: cone(biclique(a, b), 0, clique(c)),
            (range(1, 7), range(1, 7), range(1, 58)),
            "K(a,b) with one vertex coned over K(c); WOWII 58 at (3,3,73)",
        ),
        Blueprint(
            "join_empty_clique",
            lambda e, c: join(empty(e), clique(c)),
            (range(1, 32), range(1, 32)),
            "complete split graphs",
        ),
        Blueprint(
            "join_cliques",
            lambda a, b: join(clique(a), clique(b)),
            (range(1, 32), range(1, 32)),
            "",
        ),
        Blueprint(
            "join_path_empty",
            lambda p, e: join(path(p), empty(e)),
            (range(1, 32), range(1, 32)),
            "",
        ),
        Blueprint(
            "join_cycle_empty",
            lambda c, e: join(cycle(c), empty(e)),
            (range(3, 32), range(1, 32)),
            "",
        ),
        Blueprint(
            "cone_clique_clique",
            lambda a, c: cone(clique(a), 0, clique(c)),
            (range(2, 32), range(1, 32)),
            "two cliques sharing a single cut vertex's edges",
        ),
        Blueprint(
            "cone_cycle_clique",
            lambda a, c: cone(cycle(a), 0, clique(c)),
            (range(3, 32), range(1, 32)),
            "",
        ),
        Blueprint(
            "clique_pendants",
            lambda k, p: pendants(clique(k), 0, p),
            (range(2, 32), range(1, 32)),
            "",
        ),
        Blueprint(
            "cycle_pendants",
            lambda c, p: pendants(cycle(c), 0, p),
            (range(3, 32), range(1, 32)),
            "",
        ),
        Blueprint(
            "biclique_pendants",
            lambda a, b, p: pendants(biclique(a, b), 0, p),
            (range(1, 12), range(1, 12), range(1, 20)),
            "",
        ),
        Blueprint(
            "union_cliques_joined_path",
            lambda k, s, p: join(path(p), union(*[clique(s)] * k)),
            (range(1, 5), range(1, 7), range(1, 8)),
            "",
        ),
        Blueprint(
            "double_cone",
            lambda a, c: cone(cone(clique(a), 0, clique(c)), 1, clique(c)),
            (range(2, 16), range(1, 16)),
            "",
        ),
    ]


# -------------------------------------------------------------------- trend

def parse_record(line: str) -> dict[str, str]:
    fields: dict[str, str] = {}
    for token in line.split():
        if "=" in token:
            key, _, value = token.partition("=")
            fields[key] = value
    return fields


def as_fraction(text: str) -> Fraction:
    return Fraction(text)


def _is_witness(fields: dict[str, str]) -> bool:
    """Counterexample test for both statement shapes.

    An inequality reports phi and is refuted when it is positive. An
    implication reports no phi at all -- it is refuted by satisfying the
    hypothesis and failing the conclusion. Reading phi unconditionally is what
    made every implication mode crash on a KeyError.
    """
    if "phi" in fields:
        return as_fraction(fields["phi"]) > 0
    return fields.get("hypothesis") == "1" and fields.get("conclusion") == "0"


def run_trend(sweeper: str, mode: str, blueprints: list[Blueprint]) -> int:
    records: dict[str, list[tuple[str, tuple[int, ...]]]] = {}
    order: list[str] = []
    for blueprint in blueprints:
        for params, graph in blueprint.instances():
            code = to_graph6(graph)
            records.setdefault(code, []).append((blueprint.name, params))
            order.append(code)

    if not order:
        print("no instantiable blueprints", file=sys.stderr)
        return 1

    completed = subprocess.run(
        [sweeper, "--mode", mode, "--scan", "--report"],
        input="\n".join(order) + "\n",
        capture_output=True,
        text=True,
        check=True,
    )

    measured: dict[str, dict[str, str]] = {}
    for line in completed.stdout.splitlines():
        if not line.startswith("RECORD "):
            continue
        fields = parse_record(line)
        measured[fields["graph6"]] = fields

    families: dict[str, list[tuple[tuple[int, ...], dict[str, str]]]] = {}
    for code, owners in records.items():
        fields = measured.get(code)
        if fields is None or "skipped" in fields:
            continue
        for name, params in owners:
            families.setdefault(name, []).append((params, fields))

    witnesses = 0
    flagged = 0
    print(f"mode={mode} families={len(families)} instances={len(measured)} "
          f"max_order={min(ORDER_BUDGET, MAX_VERTICES)}")
    for blueprint in blueprints:
        rows = families.get(blueprint.name)
        if not rows:
            continue
        hits = [(p, f) for p, f in rows if _is_witness(f)]
        if hits:
            witnesses += len(hits)
            params, fields = hits[0]
            detail = (f"phi={fields['phi']}" if "phi" in fields
                      else "hypothesis=1 conclusion=0")
            print(f"  WITNESS family={blueprint.name} params={params} "
                  f"graph6={fields['graph6']} n={fields['n']} {detail}")
            continue

        report = _climbing(blueprint, rows)
        if report is not None:
            flagged += 1
            print(f"  CLIMBING family={blueprint.name} {report}")

    print(f"SUMMARY mode={mode} witnesses={witnesses} climbing={flagged}")
    return 0


def _climbing(blueprint: Blueprint, rows) -> str | None:
    """Flag a family whose gradient still rises where the vertex budget ends.

    A family is only interesting if it is climbing along its *last* parameter at
    the largest values reachable, with every earlier parameter held fixed. That
    is the situation conjecture 58 is in: the ratio approaches the bound from
    below for every representable c and only tips past it at 73.
    """
    best: tuple[Fraction, str] | None = None
    grouped: dict[tuple[int, ...], list[tuple[int, Fraction]]] = {}
    for params, fields in rows:
        if "pressure" not in fields:
            return None
        if "phi" not in fields:
            # An implication's pressure carries a flat penalty for satisfying
            # the conclusion, so it is a step function, not a bound creeping up
            # on a threshold. Extrapolating through that step is meaningless.
            return None
        grouped.setdefault(params[:-1], []).append(
            (params[-1], as_fraction(fields["pressure"]))
        )

    for prefix, series in grouped.items():
        series.sort()
        if len(series) < 4:
            continue
        tail = series[-4:]
        values = [value for _, value in tail]
        if not all(b > a for a, b in zip(values, values[1:])):
            continue
        if values[-1] >= 0:
            continue
        # Linear extrapolation on the final step; the true crossing is later
        # when the approach is concave, so this is a lower bound on where to
        # look, never a claim that a counterexample exists.
        (t0, v0), (t1, v1) = tail[-2], tail[-1]
        slope = (v1 - v0) / (t1 - t0)
        if slope <= 0:
            continue
        crossing = t1 + (-v1) / slope
        if best is None or values[-1] > best[0]:
            best = (values[-1], f"fixed={prefix} last={t1} pressure={v1} "
                                f"predicted_crossing>={float(crossing):.1f}")
    return best[1] if best else None


# --------------------------------------------------------------------- main

def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--emit", action="store_true",
                        help="write graph6 records for every instantiation")
    parser.add_argument("--trend", action="store_true",
                        help="evaluate each family along its parameters")
    parser.add_argument("--mode", default="61", help="conjecture name")
    parser.add_argument("--sweeper", default="/tmp/math-eval-wow2/wow2-sweep")
    parser.add_argument("--family", action="append",
                        help="restrict to this blueprint (repeatable)")
    parser.add_argument("--max-order", type=int, default=MAX_VERTICES,
                        help="largest order to instantiate (default 62)")
    parser.add_argument("--list", action="store_true")
    args = parser.parse_args()

    global ORDER_BUDGET
    ORDER_BUDGET = args.max_order

    blueprints = catalogue()
    if args.family:
        wanted = set(args.family)
        blueprints = [b for b in blueprints if b.name in wanted]
        if not blueprints:
            parser.error("no blueprint matched --family")

    if args.list:
        for blueprint in blueprints:
            count = sum(1 for _ in blueprint.instances())
            print(f"{blueprint.name:26s} instances={count:6d}  {blueprint.note}")
        return

    if args.trend:
        raise SystemExit(run_trend(args.sweeper, args.mode, blueprints))

    if args.emit:
        seen: set[str] = set()
        for blueprint in blueprints:
            for _, graph in blueprint.instances():
                code = to_graph6(graph)
                if code not in seen:
                    seen.add(code)
                    print(code)
        return

    parser.error("choose --emit, --trend, or --list")


if __name__ == "__main__":
    main()
