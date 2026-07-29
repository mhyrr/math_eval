#!/usr/bin/env python3
"""Finite-pool exact large-neighborhood repair for pair coverings.

The input may be a valid covering or a near-cover, but it must be structurally
well formed.  The program removes weak blocks, freezes the remaining core,
generates a deterministic seeded pool of replacement blocks, and asks the Z3
CLI to select exactly the requested number of replacements.  ``unsat`` means
only that this particular finite repair pool contains no solution.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import random
import re
import subprocess
import tempfile
import time
from collections import Counter
from itertools import combinations
from pathlib import Path
from typing import Iterable, Sequence


Block = tuple[int, ...]
Pair = tuple[int, int]
Z3_DEFAULT = "/opt/homebrew/bin/z3"


def pair(left: int, right: int) -> Pair:
    return (left, right) if left < right else (right, left)


def canonical_block(block: Iterable[int]) -> Block:
    return tuple(sorted(block))


def canonical_blocks(blocks: Iterable[Iterable[int]]) -> tuple[Block, ...]:
    return tuple(sorted(canonical_block(block) for block in blocks))


def canonical_text(blocks: Iterable[Iterable[int]]) -> str:
    return "".join(
        " ".join(map(str, block)) + "\n" for block in canonical_blocks(blocks)
    )


def parse_candidate(path: str | Path, *, v: int, k: int) -> list[Block]:
    """Read a candidate and reject structural errors before doing any search."""

    result: list[Block] = []
    source = Path(path)
    for line_number, raw in enumerate(
        source.read_text(encoding="utf-8").splitlines(), 1
    ):
        content = raw.partition("#")[0].strip()
        if not content:
            continue
        try:
            block = tuple(int(field, 10) for field in content.split())
        except ValueError as exc:
            raise ValueError(f"line {line_number}: non-integer point label") from exc
        if len(block) != k:
            raise ValueError(
                f"line {line_number}: block has {len(block)} entries, expected {k}"
            )
        if len(set(block)) != k:
            raise ValueError(f"line {line_number}: block repeats a point")
        outside = sorted(point for point in block if not 1 <= point <= v)
        if outside:
            raise ValueError(
                f"line {line_number}: points outside 1..{v}: {outside}"
            )
        result.append(canonical_block(block))

    duplicates = [block for block, count in Counter(result).items() if count > 1]
    if duplicates:
        raise ValueError(f"duplicate block: {' '.join(map(str, min(duplicates)))}")
    if not result:
        raise ValueError("candidate contains no blocks")
    return result


def all_pairs(v: int) -> tuple[Pair, ...]:
    return tuple(combinations(range(1, v + 1), 2))


def block_pairs(block: Sequence[int]) -> tuple[Pair, ...]:
    return tuple(combinations(canonical_block(block), 2))


def pair_counts(blocks: Iterable[Sequence[int]]) -> Counter[Pair]:
    counts: Counter[Pair] = Counter()
    for block in blocks:
        counts.update(block_pairs(block))
    return counts


def uncovered_pairs(blocks: Iterable[Sequence[int]], v: int) -> tuple[Pair, ...]:
    counts = pair_counts(blocks)
    return tuple(candidate for candidate in all_pairs(v) if counts[candidate] == 0)


def is_valid_cover(blocks: Sequence[Block], *, v: int, k: int) -> bool:
    """An exact local validity check used before checkpointing a model."""

    if len(set(blocks)) != len(blocks):
        return False
    if any(
        len(block) != k
        or len(set(block)) != k
        or any(point < 1 or point > v for point in block)
        for block in blocks
    ):
        return False
    return not uncovered_pairs(blocks, v)


def _greedy_removal_trial(
    blocks: Sequence[Block],
    remove_count: int,
    *,
    rng: random.Random | None,
    selection_width: int,
) -> tuple[int, ...]:
    counts = pair_counts(blocks)
    remaining = set(range(len(blocks)))
    removed: list[int] = []
    for _ in range(remove_count):
        ranked: list[tuple[int, int, int]] = []
        for index in remaining:
            private = sum(counts[item] == 1 for item in block_pairs(blocks[index]))
            singleton_pressure = sum(
                1 for item in block_pairs(blocks[index]) if counts[item] <= 2
            )
            ranked.append((private, singleton_pressure, index))
        ranked.sort()
        if rng is None:
            chosen = ranked[0][2]
        else:
            width = min(selection_width, len(ranked))
            chosen = ranked[rng.randrange(width)][2]
        remaining.remove(chosen)
        removed.append(chosen)
        for item in block_pairs(blocks[chosen]):
            counts[item] -= 1
    return tuple(sorted(removed))


def choose_weak_blocks(
    blocks: Sequence[Block],
    remove_count: int,
    *,
    v: int,
    seed: int,
    trials: int = 16,
    selection_width: int = 4,
) -> tuple[int, ...]:
    """Choose a small-deficit removal set using reproducible greedy trials."""

    if not 0 <= remove_count <= len(blocks):
        raise ValueError("remove_count must be between zero and the block count")
    if trials < 1:
        raise ValueError("selection trials must be positive")
    if selection_width < 1:
        raise ValueError("selection width must be positive")
    if remove_count == 0:
        return ()

    candidates = [
        _greedy_removal_trial(
            blocks, remove_count, rng=None, selection_width=selection_width
        )
    ]
    for trial in range(1, trials):
        trial_seed = seed ^ (0x9E3779B97F4A7C15 * trial)
        candidates.append(
            _greedy_removal_trial(
                blocks,
                remove_count,
                rng=random.Random(trial_seed),
                selection_width=selection_width,
            )
        )

    def removal_quality(indices: tuple[int, ...]) -> tuple[int, tuple[int, ...]]:
        removed_set = set(indices)
        core = [block for index, block in enumerate(blocks) if index not in removed_set]
        return (len(uncovered_pairs(core, v)), indices)

    return min(candidates, key=removal_quality)


def parse_remove_indices(text: str, *, block_count: int) -> tuple[int, ...]:
    """Parse unique one-based CLI indices and return zero-based indices."""

    if not text.strip():
        return ()
    try:
        one_based = [int(field.strip(), 10) for field in text.split(",")]
    except ValueError as exc:
        raise ValueError("remove indices must be comma-separated integers") from exc
    if len(one_based) != len(set(one_based)):
        raise ValueError("remove indices contain a duplicate")
    bad = [index for index in one_based if not 1 <= index <= block_count]
    if bad:
        raise ValueError(f"remove indices outside 1..{block_count}: {bad}")
    return tuple(sorted(index - 1 for index in one_based))


def block_deficit_score(block: Sequence[int], deficits: set[Pair]) -> int:
    return sum(item in deficits for item in block_pairs(block))


def _improve_block(
    block: Block,
    *,
    fixed: frozenset[int],
    deficits: set[Pair],
    v: int,
    rng: random.Random,
) -> Block:
    """Perform strict one-point hill climbing while retaining fixed anchors."""

    current = set(block)
    while True:
        moves: list[tuple[int, int, int]] = []
        best_delta = 0
        for old in sorted(current - fixed):
            rest = current - {old}
            lost = sum(pair(old, other) in deficits for other in rest)
            for new in range(1, v + 1):
                if new in current:
                    continue
                gained = sum(pair(new, other) in deficits for other in rest)
                delta = gained - lost
                if delta > best_delta:
                    best_delta = delta
                    moves = [(old, new, delta)]
                elif delta == best_delta and delta > 0:
                    moves.append((old, new, delta))
        if best_delta <= 0:
            return canonical_block(current)
        old, new, _ = moves[rng.randrange(len(moves))]
        current.remove(old)
        current.add(new)


def _anchored_block(
    anchor: Pair,
    *,
    deficits: set[Pair],
    deficit_degree: Counter[int],
    v: int,
    k: int,
    rng: random.Random,
) -> Block:
    current = set(anchor)
    while len(current) < k:
        ranked: list[tuple[int, int, int]] = []
        for point_value in range(1, v + 1):
            if point_value in current:
                continue
            immediate = sum(
                pair(point_value, other) in deficits for other in current
            )
            ranked.append((-immediate, -deficit_degree[point_value], point_value))
        ranked.sort()
        breadth = min(5, len(ranked))
        current.add(ranked[rng.randrange(breadth)][2])
    return _improve_block(
        canonical_block(current),
        fixed=frozenset(anchor),
        deficits=deficits,
        v=v,
        rng=rng,
    )


def _random_deficit_block(
    *,
    ordered_deficits: Sequence[Pair],
    deficits: set[Pair],
    deficit_degree: Counter[int],
    v: int,
    k: int,
    rng: random.Random,
) -> Block:
    if ordered_deficits:
        anchor = ordered_deficits[rng.randrange(len(ordered_deficits))]
        return _anchored_block(
            anchor,
            deficits=deficits,
            deficit_degree=deficit_degree,
            v=v,
            k=k,
            rng=rng,
        )
    return canonical_block(rng.sample(range(1, v + 1), k))


def generate_candidate_pool(
    *,
    core: Sequence[Block],
    removed: Sequence[Block],
    deficits: Sequence[Pair],
    v: int,
    k: int,
    seed: int,
    pool_size: int,
    anchor_restarts: int,
    random_candidates: int,
    include_removed: bool = True,
) -> tuple[Block, ...]:
    """Generate and down-select a finite, canonical, duplicate-free pool."""

    if pool_size < 1:
        raise ValueError("pool_size must be positive")
    if anchor_restarts < 0 or random_candidates < 0:
        raise ValueError("generation counts must be nonnegative")

    rng = random.Random(seed)
    core_set = set(core)
    deficit_set = set(deficits)
    ordered_deficits = tuple(sorted(deficit_set))
    degree: Counter[int] = Counter()
    for left, right in ordered_deficits:
        degree[left] += 1
        degree[right] += 1

    raw: set[Block] = set()

    def add(block: Iterable[int]) -> None:
        candidate = canonical_block(block)
        if (
            len(candidate) == k
            and len(set(candidate)) == k
            and candidate not in core_set
        ):
            raw.add(candidate)

    if include_removed:
        for block in removed:
            add(block)

    # One anchored construction per deficit makes unsupported hard constraints
    # unlikely; restarts supply alternative ways to pack deficits together.
    for restart in range(anchor_restarts):
        for anchor in ordered_deficits:
            local_seed = (
                seed
                ^ ((restart + 1) * 0xD1B54A32D192ED03)
                ^ (anchor[0] * 0x94D049BB133111EB)
                ^ anchor[1]
            )
            add(
                _anchored_block(
                    anchor,
                    deficits=deficit_set,
                    deficit_degree=degree,
                    v=v,
                    k=k,
                    rng=random.Random(local_seed),
                )
            )

    # Explore the immediate neighborhood of the removed blocks.  This is often
    # more useful than unrelated random blocks when the input is already close.
    for old_block in removed:
        old_set = set(old_block)
        for old_point in old_block:
            for new_point in range(1, v + 1):
                if new_point not in old_set:
                    add((old_set - {old_point}) | {new_point})

    for _ in range(random_candidates):
        add(
            _random_deficit_block(
                ordered_deficits=ordered_deficits,
                deficits=deficit_set,
                deficit_degree=degree,
                v=v,
                k=k,
                rng=rng,
            )
        )

    # Tiny instances are useful for tests and can be completed exhaustively.
    if math.comb(v, k) <= 100_000:
        for candidate in combinations(range(1, v + 1), k):
            add(candidate)

    # If generation collided heavily, fill reproducibly with ordinary blocks.
    attempts = 0
    while len(raw) < pool_size and attempts < max(1_000, 20 * pool_size):
        attempts += 1
        add(rng.sample(range(1, v + 1), k))

    coverage = {
        block: frozenset(item for item in block_pairs(block) if item in deficit_set)
        for block in raw
    }
    selected: list[Block] = []
    selected_set: set[Block] = set()

    def select(block: Block) -> None:
        if block not in selected_set and len(selected) < pool_size:
            selected.append(block)
            selected_set.add(block)

    # Retaining removed blocks makes same-cardinality repair a useful pipeline
    # sanity check.  Z3 is not required to choose them.
    if include_removed:
        for block in sorted(set(removed)):
            if block in raw:
                select(block)

    covered: set[Pair] = set()
    for block in selected:
        covered.update(coverage[block])

    # Greedy set cover reserves pool capacity for every hard deficit before
    # score-based filling.  A small pool can still leave unsupported pairs.
    available = set(raw) - selected_set
    while covered != deficit_set and available and len(selected) < pool_size:
        best = min(
            available,
            key=lambda block: (
                -len(coverage[block] - covered),
                -len(coverage[block]),
                block,
            ),
        )
        if not (coverage[best] - covered):
            break
        select(best)
        covered.update(coverage[best])
        available.remove(best)

    ranked_remainder = sorted(
        raw - selected_set,
        key=lambda block: (-len(coverage[block]), block),
    )
    for block in ranked_remainder:
        select(block)
        if len(selected) == pool_size:
            break

    return tuple(sorted(selected))


def emit_smt2(
    pool: Sequence[Block],
    deficits: Sequence[Pair],
    replacement_count: int,
    *,
    timeout_ms: int,
) -> str:
    """Encode exact cardinality and every remaining pair deficit in SMT-LIB 2."""

    if replacement_count < 0:
        raise ValueError("replacement_count must be nonnegative")
    if timeout_ms < 1:
        raise ValueError("timeout_ms must be positive")

    lines = [
        "; finite-pool exact repair for a pair covering",
        f"; pool_size={len(pool)} replacement_count={replacement_count}",
        "(set-logic QF_LIA)",
        "(set-option :produce-models true)",
        f"(set-option :timeout {timeout_ms})",
    ]
    for index, block in enumerate(pool):
        lines.append(f"; x{index} = {' '.join(map(str, block))}")
        lines.append(f"(declare-fun x{index} () Bool)")

    if pool:
        terms = " ".join(f"(ite x{index} 1 0)" for index in range(len(pool)))
        lines.append(f"(assert (= (+ {terms}) {replacement_count}))")
    else:
        lines.append(f"(assert (= 0 {replacement_count}))")

    pool_pair_sets = [set(block_pairs(block)) for block in pool]
    for left, right in sorted(deficits):
        supporters = [
            f"x{index}"
            for index, candidate_pairs in enumerate(pool_pair_sets)
            if (left, right) in candidate_pairs
        ]
        lines.append(f"; deficit {left} {right}")
        if supporters:
            lines.append(f"(assert (or {' '.join(supporters)}))")
        else:
            lines.append("(assert false)")
    lines.append("(check-sat)")
    if pool:
        lines.append(
            "(get-value (" + " ".join(f"x{i}" for i in range(len(pool))) + "))"
        )
    return "\n".join(lines) + "\n"


def run_z3(
    smt2_path: str | Path,
    *,
    z3_path: str,
    timeout_ms: int,
) -> tuple[str, tuple[int, ...], str, float]:
    """Run Z3 without a shell and parse the selected Boolean variables."""

    started = time.monotonic()
    try:
        process = subprocess.run(
            [z3_path, "-smt2", str(smt2_path)],
            text=True,
            capture_output=True,
            check=False,
            timeout=timeout_ms / 1000.0 + 10.0,
        )
    except FileNotFoundError as exc:
        raise RuntimeError(f"Z3 binary not found: {z3_path}") from exc
    except subprocess.TimeoutExpired as exc:
        elapsed = time.monotonic() - started
        return ("unknown", (), f"process timeout after {elapsed:.3f}s", elapsed)
    elapsed = time.monotonic() - started

    status_match = re.search(r"(?m)^(sat|unsat|unknown)\s*$", process.stdout)
    if status_match is None:
        detail = (process.stderr or process.stdout).strip()
        raise RuntimeError(
            f"Z3 returned no status (exit {process.returncode}): {detail[:500]}"
        )
    status = status_match.group(1)
    chosen: tuple[int, ...] = ()
    if status == "sat":
        assignments = re.findall(r"\(x(\d+)\s+(true|false)\)", process.stdout)
        chosen = tuple(
            sorted(int(index) for index, value in assignments if value == "true")
        )
    detail = process.stderr.strip()
    if status == "unknown" and not detail:
        detail = "Z3 returned unknown (normally the configured finite timeout)"
    return (status, chosen, detail, elapsed)


def _write_without_overwrite(path: Path, text: str) -> Path:
    """Write a durable artifact, preserving different existing content."""

    if path.exists():
        existing = path.read_text(encoding="utf-8")
        if existing == text:
            return path
        digest = hashlib.sha256(text.encode("utf-8")).hexdigest()[:12]
        path = path.with_name(f"{path.stem}_{digest}{path.suffix}")
        if path.exists() and path.read_text(encoding="utf-8") != text:
            raise FileExistsError(f"refusing to overwrite different artifact: {path}")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")
    return path


def checkpoint_cover(
    blocks: Sequence[Block],
    *,
    requested_path: str | None,
    input_path: Path,
    seed: int,
) -> tuple[Path, str]:
    text = canonical_text(blocks)
    digest = hashlib.sha256(text.encode("utf-8")).hexdigest()
    if requested_path is None:
        path = (
            input_path.parent
            / "candidates"
            / f"exact_repair_b{len(blocks)}_seed{seed}_{digest[:12]}.txt"
        )
    else:
        path = Path(requested_path)
    return (_write_without_overwrite(path, text), digest)


def repair(
    *,
    input_path: str | Path,
    v: int,
    k: int,
    seed: int,
    remove_count: int,
    replacement_count: int,
    explicit_remove_indices: str | None,
    selection_trials: int,
    selection_width: int,
    pool_size: int,
    anchor_restarts: int,
    random_candidates: int,
    include_removed: bool,
    timeout_ms: int,
    z3_path: str,
    output_path: str | None,
    smt2_output: str | None,
) -> dict[str, object]:
    """Run one reproducible bounded repair and return its complete run record."""

    started = time.monotonic()
    source = Path(input_path)
    blocks = parse_candidate(source, v=v, k=k)
    if explicit_remove_indices is None:
        removed_indices = choose_weak_blocks(
            blocks,
            remove_count,
            v=v,
            seed=seed,
            trials=selection_trials,
            selection_width=selection_width,
        )
    else:
        removed_indices = parse_remove_indices(
            explicit_remove_indices, block_count=len(blocks)
        )
        if len(removed_indices) != remove_count:
            raise ValueError(
                f"remove_count is {remove_count}, but "
                f"{len(removed_indices)} explicit indices were supplied"
            )

    removed_set = set(removed_indices)
    core = tuple(
        block for index, block in enumerate(blocks) if index not in removed_set
    )
    removed = tuple(blocks[index] for index in removed_indices)
    deficits = uncovered_pairs(core, v)
    pool = generate_candidate_pool(
        core=core,
        removed=removed,
        deficits=deficits,
        v=v,
        k=k,
        seed=seed,
        pool_size=pool_size,
        anchor_restarts=anchor_restarts,
        random_candidates=random_candidates,
        include_removed=include_removed,
    )
    smt2 = emit_smt2(
        pool, deficits, replacement_count, timeout_ms=timeout_ms
    )

    temporary: tempfile.TemporaryDirectory[str] | None = None
    if smt2_output is None:
        temporary = tempfile.TemporaryDirectory(prefix="covering-exact-repair-")
        smt_path = Path(temporary.name) / "repair.smt2"
        smt_path.write_text(smt2, encoding="utf-8")
        reported_smt_path: str | None = None
    else:
        smt_path = _write_without_overwrite(Path(smt2_output), smt2)
        reported_smt_path = str(smt_path)

    try:
        z3_status, chosen_indices, z3_detail, z3_seconds = run_z3(
            smt_path, z3_path=z3_path, timeout_ms=timeout_ms
        )
    finally:
        if temporary is not None:
            temporary.cleanup()

    chosen = tuple(pool[index] for index in chosen_indices)
    result = canonical_blocks((*core, *chosen))
    result_uncovered = uncovered_pairs(result, v) if z3_status == "sat" else ()
    structurally_valid_model = (
        z3_status == "sat"
        and len(chosen) == replacement_count
        and len(set(result)) == len(result)
    )
    valid = structurally_valid_model and is_valid_cover(result, v=v, k=k)

    checkpoint: str | None = None
    result_sha256: str | None = None
    if valid:
        saved, result_sha256 = checkpoint_cover(
            result,
            requested_path=output_path,
            input_path=source,
            seed=seed,
        )
        checkpoint = str(saved)

    if valid:
        outcome = "valid"
    elif z3_status == "unsat":
        outcome = "unsat_finite_pool"
    elif z3_status == "unknown":
        outcome = "unknown_finite_timeout"
    elif z3_status == "sat":
        outcome = "invalid_model"
    else:
        outcome = "solver_error"

    input_text = canonical_text(blocks)
    pool_text = canonical_text(pool)
    deficit_set = set(deficits)
    supported_deficits = {
        item
        for block in pool
        for item in block_pairs(block)
        if item in deficit_set
    }
    report: dict[str, object] = {
        "lane": "exact_large_neighborhood_repair",
        "outcome": outcome,
        "valid": valid,
        "bounded_claim": (
            "unsat and unknown apply only to this recorded finite candidate pool"
        ),
        "input": str(source),
        "input_sha256": hashlib.sha256(input_text.encode("utf-8")).hexdigest(),
        "v": v,
        "k": k,
        "seed": seed,
        "input_block_count": len(blocks),
        "remove_count": remove_count,
        "replacement_count": replacement_count,
        "result_block_count": len(result) if z3_status == "sat" else None,
        "removed_indices_1_based": [index + 1 for index in removed_indices],
        "core_block_count": len(core),
        "core_uncovered_pair_count": len(deficits),
        "pool_size_requested": pool_size,
        "pool_size_actual": len(pool),
        "pool_sha256": hashlib.sha256(pool_text.encode("utf-8")).hexdigest(),
        "pool_supported_deficit_count": len(supported_deficits),
        "anchor_restarts": anchor_restarts,
        "random_candidates": random_candidates,
        "include_removed_blocks": include_removed,
        "selection_trials": selection_trials,
        "selection_width": selection_width,
        "timeout_ms": timeout_ms,
        "z3_path": z3_path,
        "z3_status": z3_status,
        "z3_seconds": round(z3_seconds, 6),
        "z3_detail": z3_detail,
        "chosen_pool_indices": list(chosen_indices),
        "chosen_replacement_blocks": [list(block) for block in chosen],
        "result_uncovered_pair_count": (
            len(result_uncovered) if z3_status == "sat" else None
        ),
        "checkpoint": checkpoint,
        "result_sha256": result_sha256,
        "smt2_output": reported_smt_path,
        "elapsed_seconds": round(time.monotonic() - started, 6),
    }
    return report


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("candidate", help="near-cover candidate, one block per line")
    parser.add_argument("--v", type=int, default=48)
    parser.add_argument("--k", type=int, default=8)
    parser.add_argument("--seed", type=int, default=1)
    parser.add_argument("--remove-count", type=int, default=6)
    parser.add_argument(
        "--replacement-count",
        type=int,
        help="exact selected block count (default: remove-count)",
    )
    parser.add_argument(
        "--remove-indices",
        help="comma-separated one-based indices; otherwise weak blocks are selected",
    )
    parser.add_argument("--selection-trials", type=int, default=16)
    parser.add_argument("--selection-width", type=int, default=4)
    parser.add_argument("--pool-size", type=int, default=2500)
    parser.add_argument("--anchor-restarts", type=int, default=2)
    parser.add_argument("--random-candidates", type=int, default=2500)
    parser.add_argument(
        "--exclude-removed-blocks",
        action="store_true",
        help="do not retain the removed originals in the finite pool",
    )
    parser.add_argument("--timeout-ms", type=int, default=60_000)
    parser.add_argument("--z3", default=Z3_DEFAULT, help="path to the Z3 CLI")
    parser.add_argument(
        "--output",
        help="valid checkpoint path (default: candidates/content-addressed name)",
    )
    parser.add_argument("--smt2-output", help="preserve the exact SMT-LIB instance")
    parser.add_argument("--report-output", help="also write the JSON run record")
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()
    replacement_count = (
        args.remove_count
        if args.replacement_count is None
        else args.replacement_count
    )
    if args.v < 2 or not 2 <= args.k <= args.v:
        parser.error("require v >= 2 and 2 <= k <= v")
    if args.remove_count < 0:
        parser.error("remove-count must be nonnegative")
    if replacement_count < 0:
        parser.error("replacement-count must be nonnegative")

    try:
        report = repair(
            input_path=args.candidate,
            v=args.v,
            k=args.k,
            seed=args.seed,
            remove_count=args.remove_count,
            replacement_count=replacement_count,
            explicit_remove_indices=args.remove_indices,
            selection_trials=args.selection_trials,
            selection_width=args.selection_width,
            pool_size=args.pool_size,
            anchor_restarts=args.anchor_restarts,
            random_candidates=args.random_candidates,
            include_removed=not args.exclude_removed_blocks,
            timeout_ms=args.timeout_ms,
            z3_path=args.z3,
            output_path=args.output,
            smt2_output=args.smt2_output,
        )
    except (OSError, ValueError, RuntimeError) as exc:
        print(json.dumps({"lane": "exact_large_neighborhood_repair", "error": str(exc)}))
        return 2

    rendered = json.dumps(report, indent=2, sort_keys=True) + "\n"
    print(rendered, end="")
    if args.report_output:
        _write_without_overwrite(Path(args.report_output), rendered)
    return 0 if report["valid"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
