#!/usr/bin/env python3
"""Structured search for C(48, 8, 2).

This clean-room lane uses two arithmetic sources:

* cyclic developments in Z_48; and
* a six-groups-of-eight decomposition.

The group-divisible model reserves six blocks for the groups themselves.  Each
remaining block has occupancy (2, 2, 1, 1, 1, 1) across the six groups, so it
contains eight points and is useful almost entirely on cross-group pairs.
Arithmetic formulae seed the entries; exact pair counts drive all subsequent
replacement decisions.

The search objective is graded, but ``is_valid`` is deliberately the literal
covering predicate.  No verifier code is imported.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import random
import signal
import sys
import time
from dataclasses import asdict, dataclass
from itertools import combinations, product
from pathlib import Path
from typing import Iterable, Sequence


V = 48
K = 8
GROUP_COUNT = 6
GROUP_SIZE = 8
PAIR_COUNT = V * (V - 1) // 2

Block = tuple[int, ...]  # zero-based internally
Pair = tuple[int, int]
Cells = list[list[int]]

ALL_PAIRS: tuple[Pair, ...] = tuple(combinations(range(V), 2))
PAIR_INDEX = [[-1] * V for _ in range(V)]
for _pair_index, (_left, _right) in enumerate(ALL_PAIRS):
    PAIR_INDEX[_left][_right] = _pair_index
    PAIR_INDEX[_right][_left] = _pair_index


@dataclass(frozen=True)
class Score:
    """Exact integer statistics for a candidate."""

    block_count: int
    uncovered: int
    weighted_deficit: int
    pair_square_sum: int
    repeated_incidences: int
    removable_blocks: int
    duplicate_blocks: int
    used_points: int

    @property
    def is_valid(self) -> bool:
        return (
            self.block_count > 0
            and self.uncovered == 0
            and self.duplicate_blocks == 0
            and self.used_points == V
        )

    def rank(self) -> tuple[int, int, int, int]:
        """Lexicographic search rank; validity itself remains exact."""

        return (
            self.duplicate_blocks,
            self.uncovered,
            self.weighted_deficit,
            self.pair_square_sum,
        )


@dataclass
class SearchResult:
    lane: str
    seed: int
    target_blocks: int
    evaluations: int
    restarts_completed: int
    elapsed_seconds: float
    blocks: list[Block]
    score: Score
    artifact: str | None = None


def canonicalize(blocks: Iterable[Sequence[int]]) -> tuple[Block, ...]:
    return tuple(sorted(tuple(sorted(block)) for block in blocks))


def canonical_text(blocks: Iterable[Sequence[int]]) -> str:
    return "".join(
        " ".join(str(point + 1) for point in block) + "\n"
        for block in canonicalize(blocks)
    )


def block_pair_indices(block: Sequence[int]) -> tuple[int, ...]:
    return tuple(PAIR_INDEX[left][right] for left, right in combinations(block, 2))


def pair_counts(blocks: Sequence[Sequence[int]]) -> list[int]:
    counts = [0] * PAIR_COUNT
    for block in blocks:
        if len(block) != K or len(set(block)) != K:
            raise ValueError("every search block must have eight distinct points")
        if any(point < 0 or point >= V for point in block):
            raise ValueError("search point outside 0..47")
        for index in block_pair_indices(block):
            counts[index] += 1
    return counts


def exact_score(
    blocks: Sequence[Sequence[int]],
    *,
    persistence: Sequence[int] | None = None,
) -> Score:
    """Compute all score fields directly from blocks."""

    counts = pair_counts(blocks)
    uncovered_indices = [index for index, count in enumerate(counts) if count == 0]
    if persistence is None:
        weighted = len(uncovered_indices)
    else:
        weighted = sum(1 + persistence[index] for index in uncovered_indices)
    repeated = sum(max(0, count - 1) for count in counts)
    removable = 0
    for block in blocks:
        if all(counts[index] >= 2 for index in block_pair_indices(block)):
            removable += 1
    normalized = [tuple(sorted(block)) for block in blocks]
    return Score(
        block_count=len(blocks),
        uncovered=len(uncovered_indices),
        weighted_deficit=weighted,
        pair_square_sum=sum(count * count for count in counts),
        repeated_incidences=repeated,
        removable_blocks=removable,
        duplicate_blocks=len(normalized) - len(set(normalized)),
        used_points=len({point for block in blocks for point in block}),
    )


def group_point(group: int, value: int) -> int:
    return group * GROUP_SIZE + value


def group_blocks() -> list[Block]:
    return [
        tuple(group_point(group, value) for value in range(GROUP_SIZE))
        for group in range(GROUP_COUNT)
    ]


def cells_to_block(cells: Sequence[Sequence[int]]) -> Block:
    block = tuple(
        sorted(
            group_point(group, value)
            for group, values in enumerate(cells)
            for value in values
        )
    )
    if len(block) != K or len(set(block)) != K:
        raise ValueError("invalid group-divisible row")
    return block


def block_to_cells(block: Sequence[int]) -> Cells:
    cells: Cells = [[] for _ in range(GROUP_COUNT)]
    for point in block:
        cells[point // GROUP_SIZE].append(point % GROUP_SIZE)
    for values in cells:
        values.sort()
    return cells


DOUBLE_GROUP_PAIRS: tuple[tuple[int, int], ...] = tuple(
    combinations(range(GROUP_COUNT), 2)
)


def balanced_double_schedule(row_count: int, offset: int = 0) -> list[tuple[int, int]]:
    """A deterministic near-regular schedule of the two doubled groups."""

    if row_count < 0:
        raise ValueError("row_count must be nonnegative")
    schedule: list[tuple[int, int]] = []
    # Rotating the 15 pairs alone creates prefix imbalance.  Greedily take the
    # least-used admissible pair, with the arithmetic rotation only breaking
    # ties.
    degrees = [0] * GROUP_COUNT
    pair_uses = {pair: 0 for pair in DOUBLE_GROUP_PAIRS}
    for row in range(row_count):
        rotated = [
            DOUBLE_GROUP_PAIRS[(index + offset + 7 * row) % len(DOUBLE_GROUP_PAIRS)]
            for index in range(len(DOUBLE_GROUP_PAIRS))
        ]
        pair = min(
            rotated,
            key=lambda item: (
                degrees[item[0]] + degrees[item[1]],
                max(degrees[item[0]], degrees[item[1]]),
                pair_uses[item],
                rotated.index(item),
            ),
        )
        schedule.append(pair)
        degrees[pair[0]] += 1
        degrees[pair[1]] += 1
        pair_uses[pair] += 1
    return schedule


def gf8_multiply(left: int, right: int) -> int:
    """Multiply in GF(8) = GF(2)[x] / (x^3 + x + 1)."""

    if not 0 <= left < GROUP_SIZE or not 0 <= right < GROUP_SIZE:
        raise ValueError("GF(8) elements are encoded by integers 0..7")
    product_value = 0
    multiplicand = left
    multiplier = right
    while multiplier:
        if multiplier & 1:
            product_value ^= multiplicand
        multiplier >>= 1
        multiplicand <<= 1
        if multiplicand & 0b1000:
            multiplicand ^= 0b1011
    return product_value


def arithmetic_rows(
    row_count: int,
    seed: int,
    *,
    schedule_offset: int = 0,
) -> list[Cells]:
    """Generate group-divisible rows from affine formulae over Z_8.

    The primary entries are evaluations of affine functions ``a*x+b`` over
    GF(8), at six distinct field elements.  Consequently any two group
    columns determine ``(a,b)`` uniquely: primary-primary pairs never repeat
    while the selected affine lines remain distinct.
    """

    rng = random.Random(seed)
    affine_lines = [
        (slope, intercept)
        for slope in range(GROUP_SIZE)
        for intercept in range(GROUP_SIZE)
    ]
    rng.shuffle(affine_lines)
    if row_count > len(affine_lines):
        raise ValueError("at most 64 distinct GF(8) affine rows are available")
    schedule = balanced_double_schedule(row_count, schedule_offset)
    rows: list[Cells] = []
    for row, doubled in enumerate(schedule):
        slope, intercept = affine_lines[row]
        cells: Cells = []
        for group in range(GROUP_COUNT):
            first = gf8_multiply(slope, group) ^ intercept
            values = [first]
            if group in doubled:
                # XOR by a nonzero field element always changes the value.
                delta = 1 + ((row + 3 * group + seed) % (GROUP_SIZE - 1))
                values.append(first ^ delta)
            # Keep the affine primary first.  Blocks themselves are
            # canonicalized by ``cells_to_block``.
            cells.append(values)
        rows.append(cells)
    return rows


def cyclic_difference_histogram(base: Sequence[int]) -> tuple[int, ...]:
    histogram = [0] * (V // 2 + 1)
    for left, right in combinations(base, 2):
        distance = (right - left) % V
        distance = min(distance, V - distance)
        histogram[distance] += 1
    return tuple(histogram[1:])


def cyclic_base_rank(base: Sequence[int]) -> tuple[int, int, int, Block]:
    histogram = cyclic_difference_histogram(base)
    missing = sum(count == 0 for count in histogram)
    # Distance 24 has only 24 distinct pairs; all others have 48.  Squared
    # multiplicities still gives the desired preference for a flat spectrum.
    return (
        missing,
        sum((count - 1) * (count - 1) for count in histogram),
        max(histogram),
        tuple(sorted(base)),
    )


def find_cyclic_base(seed: int, evaluations: int) -> tuple[Block, int]:
    """Hill-climb an 8-set whose Z_48 differences cover all distances."""

    if evaluations < 1:
        raise ValueError("evaluations must be positive")
    rng = random.Random(seed)
    best: Block = tuple(range(K))
    best_rank = cyclic_base_rank(best)
    used = 0
    while used < evaluations:
        base = tuple(sorted(rng.sample(range(V), K)))
        rank = cyclic_base_rank(base)
        used += 1
        while used < evaluations:
            move_best = base
            move_rank = rank
            occupied = set(base)
            for position in range(K):
                for replacement in range(V):
                    if replacement in occupied:
                        continue
                    candidate = tuple(
                        sorted(base[:position] + base[position + 1 :] + (replacement,))
                    )
                    candidate_rank = cyclic_base_rank(candidate)
                    used += 1
                    if candidate_rank < move_rank:
                        move_best, move_rank = candidate, candidate_rank
                    if used >= evaluations:
                        break
                if used >= evaluations:
                    break
            if move_rank < rank:
                base, rank = move_best, move_rank
            else:
                # Reproducible restart rather than an annealing move.
                break
            if rank < best_rank:
                best, best_rank = base, rank
            if best_rank[0] == 0:
                return best, used
    return best, used


def develop_cyclic(base: Sequence[int], step: int = 1) -> list[Block]:
    """Develop a block under translation by ``step`` in Z_48."""

    if not base:
        return []
    developed: list[Block] = []
    seen: set[Block] = set()
    shift = 0
    while True:
        block = tuple(sorted((point + shift) % V for point in base))
        if block in seen:
            break
        seen.add(block)
        developed.append(block)
        shift = (shift + step) % V
    return developed


def deterministic_prune(
    fixed: Sequence[Block],
    mutable: list[Cells],
    target_blocks: int,
) -> list[Cells]:
    """Delete rows one at a time, minimizing exact unique-pair damage."""

    rows = list(mutable)
    while len(fixed) + len(rows) > target_blocks:
        blocks = list(fixed) + [cells_to_block(row) for row in rows]
        counts = pair_counts(blocks)
        choice = min(
            range(len(rows)),
            key=lambda index: (
                sum(
                    counts[pair_index] == 1
                    for pair_index in block_pair_indices(cells_to_block(rows[index]))
                ),
                -sum(
                    counts[pair_index] - 1
                    for pair_index in block_pair_indices(cells_to_block(rows[index]))
                ),
                cells_to_block(rows[index]),
            ),
        )
        del rows[choice]
    return rows


class GDDState:
    """Incrementally scored group-divisible construction."""

    def __init__(self, rows: list[Cells]):
        self.fixed = group_blocks()
        self.rows = [[list(values) for values in row] for row in rows]
        self.blocks = self.fixed + [cells_to_block(row) for row in self.rows]
        self.counts = pair_counts(self.blocks)
        self.square_sum = sum(count * count for count in self.counts)
        self.uncovered = sum(count == 0 for count in self.counts)
        self.block_set = set(self.blocks)

    def score(self, persistence: Sequence[int] | None = None) -> Score:
        return exact_score(self.blocks, persistence=persistence)

    def missing_indices(self) -> list[int]:
        return [index for index, count in enumerate(self.counts) if count == 0]

    def replacement_delta(
        self, row_index: int, new_cells: Cells
    ) -> tuple[int, int, Block] | None:
        old_block = self.blocks[GROUP_COUNT + row_index]
        new_block = cells_to_block(new_cells)
        if new_block == old_block:
            return (0, 0, new_block)
        if new_block in self.block_set:
            return None
        old_pairs = set(block_pair_indices(old_block))
        new_pairs = set(block_pair_indices(new_block))
        removed = old_pairs - new_pairs
        added = new_pairs - old_pairs
        uncovered_delta = sum(self.counts[index] == 1 for index in removed)
        uncovered_delta -= sum(self.counts[index] == 0 for index in added)
        square_delta = sum(
            (self.counts[index] - 1) ** 2 - self.counts[index] ** 2
            for index in removed
        )
        square_delta += sum(
            (self.counts[index] + 1) ** 2 - self.counts[index] ** 2
            for index in added
        )
        return uncovered_delta, square_delta, new_block

    def replace(self, row_index: int, new_cells: Cells) -> None:
        old_block = self.blocks[GROUP_COUNT + row_index]
        new_block = cells_to_block(new_cells)
        if new_block != old_block and new_block in self.block_set:
            raise ValueError("duplicate replacement")
        old_pairs = set(block_pair_indices(old_block))
        new_pairs = set(block_pair_indices(new_block))
        self.block_set.remove(old_block)
        for index in old_pairs - new_pairs:
            count = self.counts[index]
            if count == 1:
                self.uncovered += 1
            self.square_sum += (count - 1) ** 2 - count**2
            self.counts[index] -= 1
        for index in new_pairs - old_pairs:
            count = self.counts[index]
            if count == 0:
                self.uncovered -= 1
            self.square_sum += (count + 1) ** 2 - count**2
            self.counts[index] += 1
        self.rows[row_index] = [list(values) for values in new_cells]
        self.blocks[GROUP_COUNT + row_index] = new_block
        self.block_set.add(new_block)


def forced_pair_variants(cells: Cells, pair: Pair) -> Iterable[Cells]:
    """All ways to make one row contain a requested cross-group pair."""

    left, right = pair
    left_group, left_value = divmod(left, GROUP_SIZE)
    right_group, right_value = divmod(right, GROUP_SIZE)
    if left_group == right_group:
        return

    choices: list[list[int | None]] = []
    for group, value in ((left_group, left_value), (right_group, right_value)):
        if value in cells[group]:
            choices.append([None])
        else:
            choices.append(list(range(len(cells[group]))))

    seen: set[tuple[tuple[int, ...], ...]] = set()
    for left_slot, right_slot in product(*choices):
        variant = [list(values) for values in cells]
        if left_slot is not None:
            variant[left_group][left_slot] = left_value
            variant[left_group].sort()
        if right_slot is not None:
            variant[right_group][right_slot] = right_value
            variant[right_group].sort()
        key = tuple(tuple(values) for values in variant)
        if key not in seen:
            seen.add(key)
            yield variant


def single_value_variants(cells: Cells) -> Iterable[Cells]:
    for group, values in enumerate(cells):
        for slot in range(len(values)):
            for replacement in range(GROUP_SIZE):
                if replacement in values:
                    continue
                variant = [list(cell) for cell in cells]
                variant[group][slot] = replacement
                variant[group].sort()
                yield variant


def best_forced_replacement(
    state: GDDState,
    missing_index: int,
    *,
    tabu_blocks: set[Block] | None = None,
) -> tuple[int, Cells, int, int, Block] | None:
    pair = ALL_PAIRS[missing_index]
    best: tuple[tuple[int, int, Block, int], int, Cells, int, int, Block] | None = None
    for row_index, cells in enumerate(state.rows):
        for variant in forced_pair_variants(cells, pair):
            delta = state.replacement_delta(row_index, variant)
            if delta is None:
                continue
            uncovered_delta, square_delta, new_block = delta
            if tabu_blocks and new_block in tabu_blocks:
                continue
            key = (
                state.uncovered + uncovered_delta,
                state.square_sum + square_delta,
                new_block,
                row_index,
            )
            if best is None or key < best[0]:
                best = (
                    key,
                    row_index,
                    variant,
                    uncovered_delta,
                    square_delta,
                    new_block,
                )
    if best is None:
        return None
    _, row_index, variant, uncovered_delta, square_delta, new_block = best
    return row_index, variant, uncovered_delta, square_delta, new_block


def deterministic_descent(state: GDDState, evaluation_budget: int) -> int:
    """Best-improvement one-value descent; useful after pruning and at the end."""

    evaluations = 0
    while evaluations < evaluation_budget:
        best: tuple[tuple[int, int, Block, int], int, Cells] | None = None
        for row_index, cells in enumerate(state.rows):
            for variant in single_value_variants(cells):
                evaluations += 1
                delta = state.replacement_delta(row_index, variant)
                if delta is None:
                    continue
                uncovered_delta, square_delta, new_block = delta
                if uncovered_delta > 0:
                    continue
                if uncovered_delta == 0 and square_delta >= 0:
                    continue
                key = (
                    state.uncovered + uncovered_delta,
                    state.square_sum + square_delta,
                    new_block,
                    row_index,
                )
                if best is None or key < best[0]:
                    best = (key, row_index, variant)
                if evaluations >= evaluation_budget:
                    break
            if evaluations >= evaluation_budget:
                break
        if best is None:
            break
        _, row_index, variant = best
        state.replace(row_index, variant)
        if state.uncovered == 0:
            break
    return evaluations


class Checkpointer:
    def __init__(
        self,
        output_dir: Path | None,
        lane: str,
        target: int,
        seed: int,
        *,
        parameters: dict[str, object] | None = None,
    ):
        self.output_dir = output_dir
        self.lane = lane
        self.target = target
        self.seed = seed
        self.parameters = parameters or {}
        self.best_rank: tuple[int, int, int, int] | None = None
        self.best_path: Path | None = None
        self.valid_hashes: set[str] = set()
        if output_dir is not None:
            output_dir.mkdir(parents=True, exist_ok=True)

    def consider(
        self,
        blocks: Sequence[Block],
        score: Score,
        *,
        evaluations: int,
        restart: int,
        elapsed: float,
    ) -> Path | None:
        rank = score.rank()
        if self.best_rank is not None and rank >= self.best_rank and not score.is_valid:
            return None
        text = canonical_text(blocks)
        digest = hashlib.sha256(text.encode("ascii")).hexdigest()[:16]
        if score.is_valid and digest in self.valid_hashes:
            return None
        if score.is_valid:
            self.valid_hashes.add(digest)
        self.best_rank = min(rank, self.best_rank) if self.best_rank is not None else rank
        if self.output_dir is None:
            return None
        kind = "valid" if score.is_valid else "best"
        suffix = f"_{digest}" if score.is_valid else ""
        path = self.output_dir / (
            f"structured_{self.lane}_{self.target}_seed{self.seed}_{kind}{suffix}.txt"
        )
        path.write_text(text, encoding="utf-8")
        metadata = {
            "lane": self.lane,
            "seed": self.seed,
            "target_blocks": self.target,
            "evaluations": evaluations,
            "restart": restart,
            "elapsed_seconds": round(elapsed, 6),
            "sha256_prefix": digest,
            "score": asdict(score),
            "candidate": path.name,
            "parameters": self.parameters,
        }
        path.with_suffix(".json").write_text(
            json.dumps(metadata, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
        self.best_path = path
        return path


def gdd_search(
    *,
    target_blocks: int,
    seed: int,
    steps: int,
    restarts: int,
    extra_rows: int,
    output_dir: Path | None,
    seconds: float | None = None,
) -> SearchResult:
    if target_blocks not in (45, 46):
        raise ValueError("group-divisible search supports 45 or 46 blocks")
    if steps < 1 or restarts < 1:
        raise ValueError("steps and restarts must be positive")
    if extra_rows < 0:
        raise ValueError("extra_rows must be nonnegative")
    if seconds is not None and seconds <= 0:
        raise ValueError("seconds must be positive")
    if target_blocks - GROUP_COUNT + extra_rows > GROUP_SIZE * GROUP_SIZE:
        raise ValueError("too many rows for the 64-line GF(8) seed")
    start = time.monotonic()
    deadline = start + seconds if seconds is not None else None
    master = random.Random(seed)
    checkpointer = Checkpointer(
        output_dir,
        "gdd",
        target_blocks,
        seed,
        parameters={
            "steps": steps,
            "restarts": restarts,
            "extra_rows": extra_rows,
            "seconds": seconds,
        },
    )
    best_blocks: list[Block] = []
    best_score: Score | None = None
    evaluations = 0
    completed = 0
    stop = False

    def interrupted(_signum: int, _frame: object) -> None:
        nonlocal stop
        stop = True

    previous_handler = signal.signal(signal.SIGINT, interrupted)
    try:
        target_rows = target_blocks - GROUP_COUNT
        for restart in range(restarts):
            if stop or (deadline is not None and time.monotonic() >= deadline):
                break
            restart_seed = master.randrange(1 << 63)
            rows = arithmetic_rows(
                target_rows + extra_rows,
                restart_seed,
                schedule_offset=restart,
            )
            rows = deterministic_prune(group_blocks(), rows, target_blocks)
            state = GDDState(rows)
            persistence = [0] * PAIR_COUNT
            score = state.score(persistence)
            if best_score is None or score.rank() < best_score.rank():
                best_blocks, best_score = list(state.blocks), score
                checkpointer.consider(
                    best_blocks,
                    best_score,
                    evaluations=evaluations,
                    restart=restart,
                    elapsed=time.monotonic() - start,
                )

            # Each step repairs a currently absent cross-group pair by the
            # least damaging one- or two-value row replacement.  Short tabu
            # history prevents immediate two-cycles.  Every 97th step applies
            # the deterministic one-value descent to flatten multiplicities.
            tabu: list[Block] = []
            tabu_set: set[Block] = set()
            rng = random.Random(restart_seed ^ 0x48_08_02)
            restart_budget = max(1, steps // restarts)
            for step in range(restart_budget):
                if stop or (deadline is not None and time.monotonic() >= deadline):
                    break
                missing = state.missing_indices()
                if not missing:
                    score = state.score(persistence)
                    best_blocks, best_score = list(state.blocks), score
                    checkpointer.consider(
                        best_blocks,
                        best_score,
                        evaluations=evaluations,
                        restart=restart,
                        elapsed=time.monotonic() - start,
                    )
                    return SearchResult(
                        lane="gdd",
                        seed=seed,
                        target_blocks=target_blocks,
                        evaluations=evaluations,
                        restarts_completed=restart + 1,
                        elapsed_seconds=time.monotonic() - start,
                        blocks=best_blocks,
                        score=best_score,
                        artifact=str(checkpointer.best_path)
                        if checkpointer.best_path
                        else None,
                    )
                for index in missing:
                    persistence[index] += 1
                # Persistent deficits come first; seeded selection among the
                # top few changes trajectories while remaining reproducible.
                missing.sort(key=lambda index: (-persistence[index], index))
                window = min(5, len(missing))
                missing_index = missing[rng.randrange(window)]
                move = best_forced_replacement(
                    state, missing_index, tabu_blocks=tabu_set
                )
                evaluations += len(state.rows)
                if move is None:
                    break
                row_index, variant, uncovered_delta, _, new_block = move
                # The forced move is allowed to rise slightly; structured
                # min-conflicts otherwise freezes as soon as all useful pairs
                # are singletons.  Large regressions trigger a deterministic
                # alternative from the most persistent missing pair.
                if uncovered_delta > 2:
                    alternative = best_forced_replacement(
                        state, missing[0], tabu_blocks=tabu_set
                    )
                    evaluations += len(state.rows)
                    if alternative is not None:
                        row_index, variant, uncovered_delta, _, new_block = alternative
                old_block = state.blocks[GROUP_COUNT + row_index]
                state.replace(row_index, variant)
                tabu.append(old_block)
                tabu_set.add(old_block)
                if len(tabu) > 11:
                    expired = tabu.pop(0)
                    if expired not in tabu:
                        tabu_set.discard(expired)

                if step % 97 == 96:
                    evaluations += deterministic_descent(
                        state, min(2000, restart_budget - step)
                    )
                score = state.score(persistence)
                if best_score is None or score.rank() < best_score.rank():
                    best_blocks, best_score = list(state.blocks), score
                    checkpointer.consider(
                        best_blocks,
                        best_score,
                        evaluations=evaluations,
                        restart=restart,
                        elapsed=time.monotonic() - start,
                    )
            completed += 1
    finally:
        signal.signal(signal.SIGINT, previous_handler)

    if best_score is None:
        raise RuntimeError("search stopped before constructing a seed")
    return SearchResult(
        lane="gdd",
        seed=seed,
        target_blocks=target_blocks,
        evaluations=evaluations,
        restarts_completed=completed,
        elapsed_seconds=time.monotonic() - start,
        blocks=best_blocks,
        score=best_score,
        artifact=str(checkpointer.best_path) if checkpointer.best_path else None,
    )


def cyclic_search(
    *,
    target_blocks: int,
    seed: int,
    evaluations: int,
    output_dir: Path | None,
) -> SearchResult:
    """Generate one cyclic orbit and prune it deterministically to the target."""

    if not 1 <= target_blocks <= V:
        raise ValueError("target block count must be in 1..48")
    start = time.monotonic()
    base, used = find_cyclic_base(seed, evaluations)
    blocks = develop_cyclic(base)
    while len(blocks) > target_blocks:
        counts = pair_counts(blocks)
        choice = min(
            range(len(blocks)),
            key=lambda index: (
                sum(
                    counts[pair_index] == 1
                    for pair_index in block_pair_indices(blocks[index])
                ),
                blocks[index],
            ),
        )
        del blocks[choice]
    score = exact_score(blocks)
    checkpointer = Checkpointer(
        output_dir,
        "cyclic",
        target_blocks,
        seed,
        parameters={"cyclic_evaluations": evaluations},
    )
    path = checkpointer.consider(
        blocks,
        score,
        evaluations=used,
        restart=0,
        elapsed=time.monotonic() - start,
    )
    return SearchResult(
        lane="cyclic",
        seed=seed,
        target_blocks=target_blocks,
        evaluations=used,
        restarts_completed=1,
        elapsed_seconds=time.monotonic() - start,
        blocks=blocks,
        score=score,
        artifact=str(path) if path else None,
    )


def render_result(result: SearchResult) -> str:
    status = "VALID" if result.score.is_valid else "near-miss"
    lines = [
        f"lane: {result.lane}",
        f"status: {status}",
        f"seed: {result.seed}",
        f"target blocks: {result.target_blocks}",
        f"evaluations: {result.evaluations}",
        f"restarts completed: {result.restarts_completed}",
        f"elapsed seconds: {result.elapsed_seconds:.3f}",
        f"uncovered pairs: {result.score.uncovered}",
        f"weighted deficit: {result.score.weighted_deficit}",
        f"pair square sum: {result.score.pair_square_sum}",
        f"repeated incidences: {result.score.repeated_incidences}",
        f"removable blocks: {result.score.removable_blocks}",
        f"duplicate blocks: {result.score.duplicate_blocks}",
        f"used points: {result.score.used_points}/{V}",
    ]
    if result.artifact:
        lines.append(f"artifact: {result.artifact}")
    return "\n".join(lines)


def parse_args(argv: Sequence[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mode", choices=("gdd", "cyclic", "both"), default="gdd")
    parser.add_argument("--blocks", type=int, choices=(44, 45, 46, 48), default=46)
    parser.add_argument("--seed", type=int, default=480802)
    parser.add_argument(
        "--steps",
        type=int,
        default=200_000,
        help="total structured repair steps across all restarts",
    )
    parser.add_argument("--restarts", type=int, default=20)
    parser.add_argument(
        "--extra-rows",
        type=int,
        default=2,
        help="arithmetic rows generated before deterministic pruning",
    )
    parser.add_argument(
        "--cyclic-evaluations",
        type=int,
        default=20_000,
        help="cyclic base-set hill-climb budget",
    )
    parser.add_argument(
        "--seconds",
        type=float,
        help="optional wall-clock cap for the GDD lane",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("structured_artifacts"),
        help="checkpoint directory; use '-' to disable",
    )
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args(argv)
    if str(args.output_dir) == "-":
        args.output_dir = None
    return args


def main(argv: Sequence[str] | None = None) -> int:
    args = parse_args(argv)
    results: list[SearchResult] = []
    if args.mode in ("cyclic", "both"):
        results.append(
            cyclic_search(
                target_blocks=args.blocks,
                seed=args.seed,
                evaluations=args.cyclic_evaluations,
                output_dir=args.output_dir,
            )
        )
    if args.mode in ("gdd", "both"):
        results.append(
            gdd_search(
                target_blocks=args.blocks,
                seed=args.seed,
                steps=args.steps,
                restarts=args.restarts,
                extra_rows=args.extra_rows,
                output_dir=args.output_dir,
                seconds=args.seconds,
            )
        )
    if args.json:
        print(
            json.dumps(
                [
                    {
                        **{
                            key: value
                            for key, value in asdict(result).items()
                            if key != "blocks"
                        },
                        "blocks": [list(block) for block in result.blocks],
                    }
                    for result in results
                ],
                indent=2,
                sort_keys=True,
            )
        )
    else:
        print("\n\n".join(render_result(result) for result in results))
    return 0 if any(result.score.is_valid for result in results) else 1


if __name__ == "__main__":
    sys.exit(main())
