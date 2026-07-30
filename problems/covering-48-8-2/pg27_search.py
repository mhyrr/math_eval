#!/usr/bin/env python3
"""Search size-eight supersets of truncated PG(2, 7) line traces.

This is a clean-room structured lane for C(48, 8, 2).  It constructs the
projective plane directly over F_7, deletes an eight-point conic plus one
off-conic point, and asks Z3 to retain ``--blocks`` line traces while filling
each selected trace back to size eight.

The optional restrictions ``--force-full`` and ``--balanced`` define a finite
subproblem.  An UNSAT result with either option says nothing about the full
covering-design instance.

The first bounded run was:

    python3 pg27_search.py --blocks 46 --hole 0 --seed 0 \
        --timeout 180 --force-full --balanced \
        --output best/pg27-46-hole0-seed0.txt

It returned ``unknown`` at the timeout and therefore wrote no candidate.
"""
import argparse
from collections import Counter
from itertools import combinations

import z3

Q = 7


def norm(v):
    for a in v:
        if a % Q:
            inv = pow(a, -1, Q)
            return tuple((x * inv) % Q for x in v)
    raise ValueError(v)


def projective_plane():
    vectors = {
        norm((a, b, c))
        for a in range(Q)
        for b in range(Q)
        for c in range(Q)
        if (a, b, c) != (0, 0, 0)
    }
    points = sorted(vectors)
    lines = sorted(vectors)
    incidence = [
        {
            i
            for i, p in enumerate(points)
            if sum(x * y for x, y in zip(line, p)) % Q == 0
        }
        for line in lines
    ]
    assert len(points) == len(lines) == 57
    assert all(len(line) == 8 for line in incidence)
    return points, lines, incidence


def conic_hole(points, external_index):
    conic = {
        i
        for i, (x, y, z) in enumerate(points)
        if (x * z - y * y) % Q == 0
    }
    extras = [i for i in range(57) if i not in conic]
    return conic | {extras[external_index % len(extras)]}


def solve(args):
    points, _, plane_lines = projective_plane()
    holes = conic_hole(points, args.hole)
    kept_points = [i for i in range(57) if i not in holes]
    local = {p: i for i, p in enumerate(kept_points)}
    traces = [{local[p] for p in line if p in local} for line in plane_lines]
    hole_counts = [8 - len(trace) for trace in traces]
    print("hole histogram", Counter(hole_counts), flush=True)
    assert max(hole_counts) <= 3

    solver = z3.Solver()
    solver.set(timeout=args.timeout * 1000)
    solver.set(random_seed=args.seed)
    selected = [z3.Bool(f"s_{line}") for line in range(57)]
    added = {}
    for line, trace in enumerate(traces):
        h = hole_counts[line]
        for x in range(48):
            if x not in trace:
                added[line, x] = z3.Bool(f"a_{line}_{x}")
        terms = [z3.If(added[line, x], 1, 0) for x in range(48) if x not in trace]
        solver.add(z3.Sum(terms) == h * z3.If(selected[line], 1, 0))

    solver.add(z3.PbEq([(s, 1) for s in selected], args.blocks))
    if args.force_full:
        for line, h in enumerate(hole_counts):
            if h == 0:
                solver.add(selected[line])

    for x in range(48):
        incidence = []
        for line, trace in enumerate(traces):
            if x in trace:
                incidence.append(z3.If(selected[line], 1, 0))
            else:
                incidence.append(z3.If(added[line, x], 1, 0))
        solver.add(z3.Sum(incidence) >= 7)
        if args.balanced:
            solver.add(z3.Sum(incidence) <= 8)

    for x, y in combinations(range(48), 2):
        witnesses = []
        for line, trace in enumerate(traces):
            x0, y0 = x in trace, y in trace
            if x0 and y0:
                witnesses.append(selected[line])
            elif x0:
                witnesses.append(added[line, y])
            elif y0:
                witnesses.append(added[line, x])
            else:
                witnesses.append(z3.And(added[line, x], added[line, y]))
        solver.add(z3.Or(witnesses))

    print(
        "checking",
        {
            "blocks": args.blocks,
            "hole": args.hole,
            "seed": args.seed,
            "timeout": args.timeout,
            "force_full": args.force_full,
            "balanced": args.balanced,
        },
        flush=True,
    )
    verdict = solver.check()
    print(verdict, solver.statistics(), flush=True)
    if verdict != z3.sat:
        return 1
    model = solver.model()
    blocks = []
    for line, trace in enumerate(traces):
        if z3.is_true(model.eval(selected[line])):
            block = set(trace)
            block.update(
                x
                for x in range(48)
                if x not in trace and z3.is_true(model.eval(added[line, x]))
            )
            assert len(block) == 8
            blocks.append(tuple(sorted(x + 1 for x in block)))
    blocks.sort()
    with open(args.output, "w", encoding="utf-8") as handle:
        for block in blocks:
            handle.write(" ".join(map(str, block)) + "\n")
    covered = Counter()
    for block in blocks:
        covered.update(combinations(block, 2))
    print(
        "wrote",
        args.output,
        "blocks",
        len(blocks),
        "uncovered",
        1128 - len(covered),
        "multiplicities",
        Counter(covered.values()),
        flush=True,
    )
    return 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--blocks", type=int, default=46)
    parser.add_argument("--hole", type=int, default=0)
    parser.add_argument("--timeout", type=int, default=300)
    parser.add_argument("--seed", type=int, default=0)
    parser.add_argument("--force-full", action="store_true")
    parser.add_argument("--balanced", action="store_true")
    parser.add_argument("--output", default="/private/tmp/pg46.txt")
    raise SystemExit(solve(parser.parse_args()))
