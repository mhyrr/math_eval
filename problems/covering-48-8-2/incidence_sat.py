#!/usr/bin/env python3
"""Generate and solve an exact incidence-CNF model for covering designs."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import random
import re
import subprocess
import time
from collections import Counter
from dataclasses import dataclass
from itertools import combinations
from pathlib import Path
from typing import Iterable, Sequence

import independent_verify
import verifier


Block = tuple[int, ...]


class CNF:
    """Small dependency-free DIMACS builder with Tseitin helpers."""

    def __init__(self) -> None:
        self.variable_count = 0
        self.clauses: list[tuple[int, ...]] = []
        self.true = self.new_variable()
        self.false = self.new_variable()
        self.add(self.true)
        self.add(-self.false)

    def new_variable(self) -> int:
        self.variable_count += 1
        return self.variable_count

    def add(self, *literals: int) -> None:
        if not literals:
            raise ValueError("empty clause")
        self.clauses.append(tuple(literals))

    def equivalent_and(self, output: int, inputs: Sequence[int]) -> None:
        if not inputs:
            self.add(output)
            return
        for literal in inputs:
            self.add(-output, literal)
        self.add(output, *(-literal for literal in inputs))

    def equivalent_or(self, output: int, inputs: Sequence[int]) -> None:
        if not inputs:
            self.add(-output)
            return
        for literal in inputs:
            self.add(-literal, output)
        self.add(-output, *inputs)

    def equivalent_xnor(self, output: int, left: int, right: int) -> None:
        self.add(-output, -left, right)
        self.add(-output, left, -right)
        self.add(output, -left, -right)
        self.add(output, left, right)

    def equivalent_xor(self, output: int, left: int, right: int) -> None:
        self.add(-output, left, right)
        self.add(-output, -left, -right)
        self.add(output, -left, right)
        self.add(output, left, -right)

    def unary_at_least(
        self, literals: Sequence[int], limit: int
    ) -> list[int]:
        """Return variables for count(literals) >= 1, ..., >= limit."""

        if not 1 <= limit <= len(literals):
            raise ValueError("unary counter limit must be within literal count")
        previous: list[int] = []
        for index, literal in enumerate(literals, start=1):
            current: list[int] = []
            for threshold in range(1, min(index, limit) + 1):
                already = (
                    previous[threshold - 1]
                    if threshold <= len(previous)
                    else self.false
                )
                lower = (
                    self.true
                    if threshold == 1
                    else previous[threshold - 2]
                    if threshold - 1 <= len(previous)
                    else self.false
                )
                total = self.new_variable()
                # total <-> already OR (literal AND lower)
                self.add(-already, total)
                self.add(-literal, -lower, total)
                self.add(-total, already, literal)
                self.add(-total, already, lower)
                current.append(total)
            previous = current
        if len(previous) != limit:
            raise AssertionError("incomplete unary counter")
        return previous

    def exactly(self, literals: Sequence[int], count: int) -> None:
        if count < 0 or count > len(literals):
            self.add(self.false)
            return
        if count == 0:
            for literal in literals:
                self.add(-literal)
            return
        if count == len(literals):
            for literal in literals:
                self.add(literal)
            return
        counter = self.unary_at_least(literals, count + 1)
        self.add(counter[count - 1])
        self.add(-counter[count])

    def at_least(self, literals: Sequence[int], count: int) -> None:
        if count <= 0:
            return
        if count > len(literals):
            self.add(self.false)
            return
        counter = self.unary_at_least(literals, count)
        self.add(counter[count - 1])

    def at_most(self, literals: Sequence[int], count: int) -> None:
        if count >= len(literals):
            return
        if count < 0:
            self.add(self.false)
            return
        if count == 0:
            for literal in literals:
                self.add(-literal)
            return
        counter = self.unary_at_least(literals, count + 1)
        self.add(-counter[count])

    def strict_lex_greater(
        self, left: Sequence[int], right: Sequence[int]
    ) -> None:
        """Require equal-length Boolean vector left > right, with 1 > 0."""

        if len(left) != len(right):
            raise ValueError("lex vectors differ in length")
        prefix_equal = self.true
        for left_bit, right_bit in zip(left, right, strict=True):
            # A first differing 0/1 pair would violate descending order.
            self.add(-prefix_equal, left_bit, -right_bit)
            next_prefix = self.new_variable()
            # next_prefix <-> prefix_equal AND (left_bit == right_bit)
            self.add(-next_prefix, prefix_equal)
            self.add(-next_prefix, -left_bit, right_bit)
            self.add(-next_prefix, left_bit, -right_bit)
            self.add(-prefix_equal, -left_bit, -right_bit, next_prefix)
            self.add(-prefix_equal, left_bit, right_bit, next_prefix)
            prefix_equal = next_prefix
        self.add(-prefix_equal)

    def lex_greater_or_equal(
        self, left: Sequence[int], right: Sequence[int]
    ) -> None:
        """Require equal-length Boolean vector left >= right, with 1 > 0."""

        if len(left) != len(right):
            raise ValueError("lex vectors differ in length")
        prefix_equal = self.true
        for left_bit, right_bit in zip(left, right, strict=True):
            # A first differing 0/1 pair would violate descending order.
            self.add(-prefix_equal, left_bit, -right_bit)
            next_prefix = self.new_variable()
            # next_prefix <-> prefix_equal AND (left_bit == right_bit)
            self.add(-next_prefix, prefix_equal)
            self.add(-next_prefix, -left_bit, right_bit)
            self.add(-next_prefix, left_bit, -right_bit)
            self.add(-prefix_equal, -left_bit, -right_bit, next_prefix)
            self.add(-prefix_equal, left_bit, right_bit, next_prefix)
            prefix_equal = next_prefix

    def vectors_differ(
        self, left: Sequence[int], right: Sequence[int]
    ) -> None:
        if len(left) != len(right):
            raise ValueError("vectors differ in length")
        differences: list[int] = []
        for left_bit, right_bit in zip(left, right, strict=True):
            difference = self.new_variable()
            self.equivalent_xor(difference, left_bit, right_bit)
            differences.append(difference)
        self.add(*differences)

    def write_dimacs(self, path: Path, comments: Iterable[str] = ()) -> str:
        digest = hashlib.sha256()
        with path.open("w", encoding="ascii") as handle:
            for comment in comments:
                line = f"c {comment}\n"
                handle.write(line)
                digest.update(line.encode("ascii"))
            header = f"p cnf {self.variable_count} {len(self.clauses)}\n"
            handle.write(header)
            digest.update(header.encode("ascii"))
            for clause in self.clauses:
                line = " ".join(map(str, clause)) + " 0\n"
                handle.write(line)
                digest.update(line.encode("ascii"))
        return digest.hexdigest()


@dataclass
class IncidenceEncoding:
    cnf: CNF
    x: list[list[int]]
    v: int
    k: int
    blocks: int
    replication_minimum: int
    max_replication: int | None
    symmetry: str
    anchor_points: int | None
    anchor_counts: tuple[int, ...] | None
    distinguished_pair_minimum: int | None


def guaranteed_pair_multiplicity(
    *, v: int, k: int, blocks: int
) -> int | None:
    """Return a globally guaranteed useful pair multiplicity, if any.

    Let m_e be pair multiplicities, E=sum(m_e-1), and
    H=sum(binomial(m_e-1, 2)).  Counting intersections of pairs of blocks
    gives H >= sum_x binomial(r_x, 2) - binomial(blocks, 2) - E.
    Convexity supplies the minimum possible replication term.
    """

    total_pair_occurrences = blocks * math.comb(k, 2)
    pair_count = math.comb(v, 2)
    excess = total_pair_occurrences - pair_count
    if excess < 0:
        return None
    quotient, remainder = divmod(blocks * k, v)
    replication_term = (
        (v - remainder) * math.comb(quotient, 2)
        + remainder * math.comb(quotient + 1, 2)
    )
    h_lower = replication_term - math.comb(blocks, 2) - excess
    if 2 * h_lower > excess:
        return 4
    if h_lower > 0:
        return 3
    return None


def normalize_candidate(
    blocks: Sequence[Sequence[int]],
    *,
    v: int,
    k: int,
    anchor_index: int = 0,
) -> list[Block]:
    report = verifier.verify(blocks, v=v, k=k)
    structural_errors = [
        error for error in report.errors if "uncovered" not in error
    ]
    if structural_errors:
        raise ValueError("; ".join(structural_errors))
    if not 0 <= anchor_index < len(blocks):
        raise ValueError("anchor index outside candidate")
    anchor = sorted(blocks[anchor_index])
    anchor_set = set(anchor)
    remainder = [point for point in range(1, v + 1) if point not in anchor_set]
    relabel = {
        **{point: index + 1 for index, point in enumerate(anchor)},
        **{
            point: k + index + 1
            for index, point in enumerate(remainder)
        },
    }
    transformed = [
        tuple(sorted(relabel[point] for point in block)) for block in blocks
    ]
    transformed.sort()
    if transformed[0] != tuple(range(1, k + 1)):
        raise AssertionError("anchor normalization failed")
    return transformed


def build_encoding(
    *,
    v: int,
    k: int,
    blocks: int,
    max_replication: int | None = None,
    fixed_candidate: Sequence[Sequence[int]] | None = None,
    free_indices: set[int] | None = None,
    row_lex: bool = True,
    anchor_points: int | None = None,
    anchor_counts: Sequence[int] | None = None,
) -> IncidenceEncoding:
    if v < 2 or not 2 <= k <= v or blocks < 1:
        raise ValueError("invalid covering parameters")
    replication_minimum = math.ceil((v - 1) / (k - 1))
    if max_replication is not None and max_replication >= blocks:
        max_replication = None
    if max_replication is not None and max_replication < replication_minimum:
        raise ValueError("max replication is below the necessary minimum")
    if fixed_candidate is not None and len(fixed_candidate) != blocks:
        raise ValueError("candidate block count differs from requested blocks")
    free = free_indices or set()
    if any(index < 0 or index >= blocks for index in free):
        raise ValueError("free block index outside candidate")
    if fixed_candidate is None and free:
        raise ValueError("free indices require a candidate")
    if 0 in free:
        raise ValueError("the normalized anchor block must remain frozen")
    if fixed_candidate is not None and free and row_lex:
        raise ValueError("row lex is unsafe for a fixed-core neighborhood")
    if anchor_points is not None and not 1 <= anchor_points < k:
        raise ValueError("anchor point count must satisfy 1 <= a < k")
    normalized_anchor_counts: tuple[int, ...] | None = None
    if anchor_counts is not None:
        if anchor_points is None:
            raise ValueError("anchor counts require anchor points")
        normalized_anchor_counts = tuple(anchor_counts)
        if len(normalized_anchor_counts) != anchor_points:
            raise ValueError("anchor count list length differs from anchor points")
        if any(count < 0 for count in normalized_anchor_counts):
            raise ValueError("anchor counts must be nonnegative")
        if tuple(sorted(normalized_anchor_counts, reverse=True)) != (
            normalized_anchor_counts
        ):
            raise ValueError("anchor counts must be nonincreasing")
        if sum(normalized_anchor_counts) > blocks - 1:
            raise ValueError("anchor counts exceed rows after the first")
        if fixed_candidate is not None:
            raise ValueError("grouped anchor models do not freeze a candidate")

    cnf = CNF()
    x = [
        [cnf.new_variable() for _ in range(v)]
        for _ in range(blocks)
    ]

    searched_rows = (
        sorted(free)
        if fixed_candidate is not None
        else list(range(blocks))
    )
    for block in searched_rows:
        cnf.exactly(x[block], k)

    for left in range(v):
        for right in range(left + 1, v):
            if fixed_candidate is not None and any(
                left + 1 in fixed_candidate[block]
                and right + 1 in fixed_candidate[block]
                for block in range(blocks)
                if block not in free
            ):
                continue
            witnesses: list[int] = []
            for block in searched_rows:
                witness = cnf.new_variable()
                # A witness may be true only when its block contains the pair.
                # The reverse implication is unnecessary for equisatisfiability.
                cnf.add(-witness, x[block][left])
                cnf.add(-witness, x[block][right])
                witnesses.append(witness)
            if witnesses:
                cnf.add(*witnesses)
            else:
                cnf.add(cnf.false)

    for point in range(v):
        if fixed_candidate is not None:
            fixed_count = sum(
                point + 1 in fixed_candidate[block]
                for block in range(blocks)
                if block not in free
            )
            incidence = [x[block][point] for block in searched_rows]
            cnf.at_least(incidence, replication_minimum - fixed_count)
            if max_replication is not None:
                cnf.at_most(incidence, max_replication - fixed_count)
        else:
            incidence = [x[block][point] for block in range(blocks)]
            limit = (
                max(replication_minimum, max_replication + 1)
                if max_replication is not None
                else replication_minimum
            )
            counter = cnf.unary_at_least(incidence, limit)
            cnf.add(counter[replication_minimum - 1])
            if max_replication is not None:
                cnf.add(-counter[max_replication])

    for point in range(k):
        cnf.add(x[0][point])
    for point in range(k, v):
        cnf.add(-x[0][point])

    distinguished_pair_minimum = (
        guaranteed_pair_multiplicity(v=v, k=k, blocks=blocks)
        if fixed_candidate is None and anchor_points is None and row_lex
        else None
    )
    if distinguished_pair_minimum is not None:
        if distinguished_pair_minimum > blocks:
            raise ValueError("guaranteed pair multiplicity exceeds block count")
        for block in range(distinguished_pair_minimum):
            cnf.add(x[block][0])
            cnf.add(x[block][1])

    if normalized_anchor_counts is not None:
        first = 1
        groups: list[range] = []
        for anchor, count in enumerate(normalized_anchor_counts):
            group = range(first, first + count)
            groups.append(group)
            for block in group:
                for point in range(anchor_points or 0):
                    cnf.add(x[block][point] if point == anchor else -x[block][point])
                # With one anchor point a grouped row could otherwise equal
                # the fixed first row.  For two or more anchors, their forced
                # anchor patterns already distinguish those rows.
                if anchor_points == 1:
                    cnf.vectors_differ(x[0], x[block])
            first += count
        anchorless = range(first, blocks)
        groups.append(anchorless)
        for block in anchorless:
            for point in range(anchor_points or 0):
                cnf.add(-x[block][point])
        for group in groups:
            rows = list(group)
            for offset in range(len(rows) - 1):
                cnf.strict_lex_greater(
                    x[rows[offset]][anchor_points:],
                    x[rows[offset + 1]][anchor_points:],
                )
        symmetry = "fixed_first_block_and_grouped_anchor_rows"
    elif row_lex:
        for block in range(blocks - 1):
            cnf.strict_lex_greater(x[block], x[block + 1])
        if fixed_candidate is None and anchor_points is None:
            # Choose the lexicographically greatest matrix in the residual
            # row/column orbit.  Points inside and outside the fixed first
            # block remain independently interchangeable.
            point_groups = (
                ((0, 2), (2, k), (k, v))
                if distinguished_pair_minimum is not None
                else ((0, k), (k, v))
            )
            for first, last in point_groups:
                for point in range(first, last - 1):
                    cnf.lex_greater_or_equal(
                        [x[block][point] for block in range(blocks)],
                        [x[block][point + 1] for block in range(blocks)],
                    )
            symmetry = "fixed_first_block_row_and_residual_column_lex"
        else:
            symmetry = "fixed_first_block_and_strict_descending_rows"
    else:
        for left in range(blocks):
            for right in range(left + 1, blocks):
                left_free = left in free
                right_free = right in free
                if not left_free and not right_free:
                    continue
                if left_free and right_free:
                    cnf.vectors_differ(x[left], x[right])
                    continue
                free_row = left if left_free else right
                fixed_row = right if left_free else left
                fixed_points = set((fixed_candidate or [])[fixed_row])
                cnf.add(
                    *(
                        -x[free_row][point - 1]
                        if point in fixed_points
                        else x[free_row][point - 1]
                        for point in range(1, v + 1)
                    )
                )
        symmetry = "fixed_first_block_and_pairwise_distinct_rows"

    if fixed_candidate is not None:
        for block, points in enumerate(fixed_candidate):
            if block in free:
                continue
            point_set = set(points)
            for point in range(1, v + 1):
                cnf.add(
                    x[block][point - 1]
                    if point in point_set
                    else -x[block][point - 1]
                )

    if anchor_points is not None and normalized_anchor_counts is None:
        rows_with_anchor: list[int] = []
        for block in range(1, blocks):
            row_anchors = x[block][:anchor_points]
            for left in range(anchor_points):
                for right in range(left + 1, anchor_points):
                    cnf.add(-row_anchors[left], -row_anchors[right])
            has_anchor = cnf.new_variable()
            cnf.equivalent_or(has_anchor, row_anchors)
            rows_with_anchor.append(has_anchor)
        cnf.exactly(rows_with_anchor, blocks - 2)

    return IncidenceEncoding(
        cnf=cnf,
        x=x,
        v=v,
        k=k,
        blocks=blocks,
        replication_minimum=replication_minimum,
        max_replication=max_replication,
        symmetry=symmetry,
        anchor_points=anchor_points,
        anchor_counts=normalized_anchor_counts,
        distinguished_pair_minimum=distinguished_pair_minimum,
    )


def parse_indices(text: str | None, block_count: int) -> set[int]:
    if not text:
        return set()
    result: set[int] = set()
    for field in text.split(","):
        try:
            index = int(field.strip())
        except ValueError as error:
            raise ValueError("free indices must be comma-separated integers") from error
        if not 1 <= index <= block_count:
            raise ValueError(f"free index {index} outside 1..{block_count}")
        result.add(index - 1)
    return result


def select_free_indices(
    blocks: Sequence[Sequence[int]],
    *,
    count: int,
    mode: str,
    seed: int,
) -> set[int]:
    if not 1 <= count < len(blocks):
        raise ValueError("free count must be between 1 and blocks-1")
    candidates = list(range(1, len(blocks)))
    if mode == "random":
        return set(random.Random(seed).sample(candidates, count))
    pair_counts: Counter[tuple[int, int]] = Counter(
        pair
        for block in blocks
        for pair in combinations(sorted(block), 2)
    )
    unique_scores = {
        index: sum(
            pair_counts[pair] == 1
            for pair in combinations(sorted(blocks[index]), 2)
        )
        for index in candidates
    }
    if mode in {"weak", "strong"}:
        reverse = mode == "strong"
        ordered = sorted(
            candidates,
            key=lambda index: (
                unique_scores[index],
                tuple(blocks[index]),
            ),
            reverse=reverse,
        )
        return set(ordered[:count])
    if mode == "deficit":
        v = max(point for block in blocks for point in block)
        points = Counter(
            point
            for left, right in combinations(range(1, v + 1), 2)
            if pair_counts[(left, right)] == 0
            for point in (left, right)
        )
        ordered = sorted(
            candidates,
            key=lambda index: (
                sum(points[point] for point in blocks[index]),
                -unique_scores[index],
                tuple(blocks[index]),
            ),
            reverse=True,
        )
        return set(ordered[:count])
    raise ValueError(f"unknown free selection mode {mode}")


def parse_dimacs_model(text: str, x: Sequence[Sequence[int]]) -> list[Block]:
    assignments: dict[int, bool] = {}
    for raw_line in text.splitlines():
        if not raw_line.startswith("v "):
            continue
        for field in raw_line.split()[1:]:
            literal = int(field)
            if literal:
                assignments[abs(literal)] = literal > 0
    needed = {variable for row in x for variable in row}
    missing = needed.difference(assignments)
    if missing:
        raise ValueError(f"solver model omitted {len(missing)} incidence variables")
    blocks = [
        tuple(
            point + 1
            for point, variable in enumerate(row)
            if assignments[variable]
        )
        for row in x
    ]
    return list(verifier.canonicalize(blocks))


def solver_status(output: str) -> str:
    for line in output.splitlines():
        if line == "s SATISFIABLE":
            return "sat"
        if line == "s UNSATISFIABLE":
            return "unsat"
        if line in {"s UNKNOWN", "unknown"}:
            return "unknown"
    return "error"


def metadata(
    encoding: IncidenceEncoding,
    *,
    cnf_path: Path,
    cnf_sha256: str,
    candidate: str | None,
    free_indices: set[int],
) -> dict[str, object]:
    return {
        "v": encoding.v,
        "k": encoding.k,
        "block_count": encoding.blocks,
        "replication_minimum": encoding.replication_minimum,
        "max_replication": encoding.max_replication,
        "symmetry": encoding.symmetry,
        "anchor_points": encoding.anchor_points,
        "anchor_counts": encoding.anchor_counts,
        "distinguished_pair_minimum": encoding.distinguished_pair_minimum,
        "candidate": candidate,
        "free_indices_1_based": sorted(index + 1 for index in free_indices),
        "cnf": str(cnf_path),
        "cnf_sha256": cnf_sha256,
        "variable_count": encoding.cnf.variable_count,
        "clause_count": len(encoding.cnf.clauses),
        "x_variables": encoding.x,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--v", type=int, default=48)
    parser.add_argument("--k", type=int, default=8)
    parser.add_argument("--blocks", type=int, choices=(44, 45, 46), required=True)
    parser.add_argument("--candidate")
    parser.add_argument(
        "--anchor-index",
        type=int,
        default=1,
        help="1-based candidate block mapped to the fixed first block",
    )
    parser.add_argument(
        "--free-indices",
        help="normalized 1-based rows left free; all other candidate rows are frozen",
    )
    parser.add_argument("--free-count", type=int)
    parser.add_argument(
        "--free-mode",
        choices=("weak", "strong", "deficit", "random"),
        default="weak",
    )
    parser.add_argument("--max-replication", type=int)
    parser.add_argument(
        "--anchor-pattern",
        type=int,
        metavar="A",
        help=(
            "among rows after the first, at most one of points 1..A per row "
            "and exactly blocks-2 total anchor incidences"
        ),
    )
    parser.add_argument(
        "--anchor-counts",
        help=(
            "nonincreasing comma-separated counts for grouped anchor rows; "
            "requires --anchor-pattern"
        ),
    )
    parser.add_argument("--seed", type=int, default=1)
    parser.add_argument("--phase", default="caching")
    parser.add_argument("--threads", type=int, default=1)
    parser.add_argument(
        "--z3-param",
        action="append",
        default=[],
        help="additional Z3 parameter assignment, repeatable",
    )
    parser.add_argument("--timeout", type=int, default=300)
    parser.add_argument("--z3", default="/opt/homebrew/bin/z3")
    parser.add_argument("--cnf-output", required=True)
    parser.add_argument("--metadata-output", required=True)
    parser.add_argument("--report-output")
    parser.add_argument("--output")
    parser.add_argument("--generate-only", action="store_true")
    args = parser.parse_args()

    started = time.monotonic()
    try:
        candidate: list[Block] | None = None
        if args.candidate:
            parsed = verifier.parse_file(args.candidate)
            candidate = normalize_candidate(
                parsed,
                v=args.v,
                k=args.k,
                anchor_index=args.anchor_index - 1,
            )
        if args.free_indices and args.free_count:
            raise ValueError("use either free indices or free count, not both")
        free_indices = parse_indices(args.free_indices, args.blocks)
        if args.free_count:
            if candidate is None:
                raise ValueError("free count requires a candidate")
            free_indices = select_free_indices(
                candidate,
                count=args.free_count,
                mode=args.free_mode,
                seed=args.seed,
            )
        anchor_counts = (
            tuple(int(field.strip()) for field in args.anchor_counts.split(","))
            if args.anchor_counts
            else None
        )
        row_lex = not free_indices
        encoding = build_encoding(
            v=args.v,
            k=args.k,
            blocks=args.blocks,
            max_replication=args.max_replication,
            fixed_candidate=candidate,
            free_indices=free_indices,
            row_lex=row_lex,
            anchor_points=args.anchor_pattern,
            anchor_counts=anchor_counts,
        )
        cnf_path = Path(args.cnf_output)
        cnf_path.parent.mkdir(parents=True, exist_ok=True)
        cnf_sha256 = encoding.cnf.write_dimacs(
            cnf_path,
            comments=(
                f"covering design v={args.v} k={args.k} blocks={args.blocks}",
                f"symmetry={encoding.symmetry}",
            ),
        )
        run_metadata = metadata(
            encoding,
            cnf_path=cnf_path,
            cnf_sha256=cnf_sha256,
            candidate=args.candidate,
            free_indices=free_indices,
        )
        Path(args.metadata_output).write_text(
            json.dumps(run_metadata, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
        if args.generate_only:
            print(json.dumps(run_metadata, indent=2, sort_keys=True))
            return 0

        command = [
            args.z3,
            "-dimacs",
            "-model",
            "-st",
            f"-T:{args.timeout}",
            f"sat.random_seed={args.seed}",
            f"sat.phase={args.phase}",
            f"sat.threads={args.threads}",
            *args.z3_param,
            str(cnf_path),
        ]
        solve_started = time.monotonic()
        completed = subprocess.run(
            command,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=args.timeout + 15,
            check=False,
        )
        solve_seconds = time.monotonic() - solve_started
        status = solver_status(completed.stdout)
        if (
            status == "error"
            and completed.returncode == 0
            and solve_seconds >= 0.95 * args.timeout
        ):
            status = "unknown"
        decoded: list[Block] | None = None
        primary: dict[str, object] | None = None
        independent: dict[str, object] | None = None
        valid = False
        if status == "sat":
            decoded = parse_dimacs_model(completed.stdout, encoding.x)
            primary_report = verifier.verify(decoded, v=args.v, k=args.k)
            independent_report = independent_verify.check(
                decoded, v=args.v, k=args.k
            )
            primary = {
                "valid": primary_report.valid,
                "uncovered_pairs": len(primary_report.uncovered_pairs),
                "pair_multiplicity_histogram": (
                    primary_report.pair_multiplicity_histogram
                ),
                "redundant_blocks": primary_report.redundant_blocks,
                "errors": primary_report.errors,
            }
            independent = {
                "valid": independent_report["valid"],
                "uncovered_pairs": len(independent_report["uncovered_pairs"]),
                "pair_multiplicity_histogram": independent_report[
                    "pair_multiplicity_histogram"
                ],
                "redundant_blocks": independent_report["redundant_blocks"],
                "errors": independent_report["errors"],
            }
            valid = primary_report.valid and bool(independent_report["valid"])
            if valid and args.output:
                Path(args.output).write_text(
                    verifier.canonical_text(decoded), encoding="utf-8"
                )

        if free_indices:
            lane = "incidence_sat_fixed_core"
            bounded_claim = (
                "UNSAT excludes only this exact frozen-core neighborhood; "
                "publishable nonexistence also requires a preserved and "
                "independently checked proof"
            )
        elif candidate:
            lane = "incidence_sat_pinned"
            bounded_claim = "UNSAT applies only to the fully pinned candidate"
        elif args.anchor_pattern:
            lane = "incidence_sat_anchor_family"
            bounded_claim = (
                "UNSAT excludes only the named anchor-pattern subfamily; "
                "it is not a global lower bound"
            )
        elif args.max_replication is not None:
            lane = "incidence_sat_replication_bounded"
            bounded_claim = (
                "UNSAT excludes only the named replication-bounded subfamily; "
                "it is not a global lower bound"
            )
        else:
            lane = "incidence_sat_unrestricted"
            bounded_claim = (
                "UNSAT is for the unrestricted fixed-block-count CNF; "
                "publishable nonexistence also requires a preserved and "
                "independently checked proof"
            )

        report = {
            **{key: value for key, value in run_metadata.items() if key != "x_variables"},
            "lane": lane,
            "seed": args.seed,
            "phase": args.phase,
            "threads": args.threads,
            "z3_params": args.z3_param,
            "timeout_seconds": args.timeout,
            "solver_command": command,
            "solver_status": status,
            "solver_exit_code": completed.returncode,
            "solver_seconds": solve_seconds,
            "solver_stderr": completed.stderr.strip(),
            "solver_stdout_tail": [
                line
                for line in completed.stdout.splitlines()
                if not line.startswith("v ")
            ][-120:],
            "solver_statistics": [
                line
                for line in completed.stdout.splitlines()
                if line.startswith("(:") or line.startswith(" ")
            ][-80:],
            "decoded_block_count": len(decoded) if decoded else None,
            "primary_verification": primary,
            "independent_verification": independent,
            "valid": valid,
            "output": args.output if valid and args.output else None,
            "elapsed_seconds": time.monotonic() - started,
            "bounded_claim": bounded_claim,
        }
        if args.report_output:
            Path(args.report_output).write_text(
                json.dumps(report, indent=2, sort_keys=True) + "\n",
                encoding="utf-8",
            )
        print(json.dumps(report, indent=2, sort_keys=True))
        if valid:
            return 0
        if status in {"unsat", "unknown"}:
            return 1
        return 2
    except (
        OSError,
        ValueError,
        subprocess.SubprocessError,
    ) as error:
        print(json.dumps({"error": str(error)}, indent=2))
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
