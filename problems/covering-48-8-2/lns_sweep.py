#!/usr/bin/env python3
"""Sweep exact fixed-core SAT neighborhoods around a larger construction."""

from __future__ import annotations

import argparse
import hashlib
import itertools
import json
import subprocess
import sys
from pathlib import Path

import incidence_sat
import verifier


Block = tuple[int, ...]


def block_text(blocks: list[Block]) -> str:
    return verifier.canonical_text(verifier.canonicalize(blocks))


def core_digest(
    candidate: list[Block], free_indices: set[int]
) -> str:
    frozen = [
        block for index, block in enumerate(candidate)
        if index not in free_indices
    ]
    return hashlib.sha256(block_text(frozen).encode("utf-8")).hexdigest()


def deletion_sets(
    source_count: int,
    target_count: int,
    required: set[int],
) -> list[tuple[int, ...]]:
    delete_count = source_count - target_count
    if delete_count < 1:
        raise ValueError("source must contain more blocks than the target")
    if len(required) > delete_count:
        raise ValueError("too many required deletions")
    if any(index < 0 or index >= source_count for index in required):
        raise ValueError("required deletion outside source")
    choices = [index for index in range(source_count) if index not in required]
    return [
        tuple(sorted((*required, *extra)))
        for extra in itertools.combinations(choices, delete_count - len(required))
    ]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source")
    parser.add_argument("--target-blocks", type=int, choices=(44, 45), required=True)
    parser.add_argument("--must-delete", help="comma-separated 1-based source rows")
    parser.add_argument("--free-count", type=int, default=3)
    parser.add_argument("--modes", default="weak,deficit")
    parser.add_argument("--timeout", type=int, default=120)
    parser.add_argument("--seed-base", type=int, default=7000)
    parser.add_argument("--exact-pair-witnesses", action="store_true")
    parser.add_argument(
        "--solver",
        choices=("z3", "cadical", "kissat"),
        default="z3",
    )
    parser.add_argument("--solver-path")
    parser.add_argument(
        "--solver-param",
        action="append",
        default=[],
    )
    parser.add_argument("--max-jobs", type=int)
    parser.add_argument("--output-dir", default="best/overnight")
    parser.add_argument(
        "--work-dir", default="/tmp/math-eval-covering-48-8-2/lns-sweep"
    )
    parser.add_argument("--summary", default="best/overnight/summary.json")
    args = parser.parse_args()

    source = [tuple(block) for block in verifier.parse_file(args.source)]
    source_report = verifier.verify(source, v=48, k=8)
    if not source_report.valid:
        raise ValueError("source construction is not a valid 48-point covering")
    required = {
        int(field.strip()) - 1
        for field in (args.must_delete or "").split(",")
        if field.strip()
    }
    modes = [field.strip() for field in args.modes.split(",") if field.strip()]
    if not modes or any(
        mode not in {"weak", "strong", "deficit", "random"} for mode in modes
    ):
        raise ValueError("unknown or empty free-row selection mode")

    ranked: list[tuple[int, tuple[int, ...], list[Block]]] = []
    for deleted in deletion_sets(len(source), args.target_blocks, required):
        candidate = [
            block for index, block in enumerate(source)
            if index not in deleted
        ]
        uncovered = len(verifier.verify(candidate, v=48, k=8).uncovered_pairs)
        ranked.append((uncovered, deleted, candidate))
    ranked.sort(key=lambda item: (item[0], item[1]))

    output_dir = Path(args.output_dir)
    work_dir = Path(args.work_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    work_dir.mkdir(parents=True, exist_ok=True)
    seen_cores: set[str] = set()
    results: list[dict[str, object]] = []
    job_number = 0

    for uncovered, deleted, raw_candidate in ranked:
        candidate = incidence_sat.normalize_candidate(
            raw_candidate, v=48, k=8, anchor_index=0
        )
        candidate_path = work_dir / (
            "candidate-minus-" + "-".join(f"{index + 1:02d}" for index in deleted)
            + ".txt"
        )
        candidate_path.write_text(block_text(candidate), encoding="utf-8")
        for mode in modes:
            seed = args.seed_base + job_number
            free = incidence_sat.select_free_indices(
                candidate,
                count=args.free_count,
                mode=mode,
                seed=seed,
            )
            digest = core_digest(candidate, free)
            if digest in seen_cores:
                continue
            seen_cores.add(digest)
            if args.max_jobs is not None and job_number >= args.max_jobs:
                break

            tag = (
                f"b{args.target_blocks}-minus-"
                + "-".join(f"{index + 1:02d}" for index in deleted)
                + f"-r{args.free_count}-{mode}-seed{seed}"
            )
            report_path = output_dir / f"{tag}.json"
            candidate_output = output_dir / f"{tag}.txt"
            cnf_path = work_dir / f"{tag}.cnf"
            metadata_path = work_dir / f"{tag}.meta.json"
            command = [
                sys.executable,
                str(Path(__file__).with_name("incidence_sat.py")),
                "--blocks",
                str(args.target_blocks),
                "--candidate",
                str(candidate_path),
                "--free-indices",
                ",".join(str(index + 1) for index in sorted(free)),
                "--timeout",
                str(args.timeout),
                "--seed",
                str(seed),
                "--threads",
                "1",
                "--cnf-output",
                str(cnf_path),
                "--metadata-output",
                str(metadata_path),
                "--report-output",
                str(report_path),
                "--output",
                str(candidate_output),
            ]
            if args.exact_pair_witnesses:
                command.append("--exact-pair-witnesses")
            command.extend(("--solver", args.solver))
            if args.solver_path:
                command.extend(("--solver-path", args.solver_path))
            for parameter in args.solver_param:
                command.extend(("--solver-param", parameter))
            completed = subprocess.run(
                command,
                text=True,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.PIPE,
                check=False,
            )
            report = json.loads(report_path.read_text(encoding="utf-8"))
            result = {
                "deleted_source_rows_1_based": [index + 1 for index in deleted],
                "seed_uncovered_pairs": uncovered,
                "mode": mode,
                "free_indices_1_based": sorted(index + 1 for index in free),
                "frozen_core_sha256": digest,
                "seed": seed,
                "exact_pair_witnesses": args.exact_pair_witnesses,
                "solver": args.solver,
                "solver_status": report["solver_status"],
                "valid": report["valid"],
                "elapsed_seconds": report["elapsed_seconds"],
                "report": str(report_path),
                "candidate": report["output"],
                "return_code": completed.returncode,
                "stderr": completed.stderr.strip(),
            }
            results.append(result)
            Path(args.summary).parent.mkdir(parents=True, exist_ok=True)
            Path(args.summary).write_text(
                json.dumps(results, indent=2, sort_keys=True) + "\n",
                encoding="utf-8",
            )
            job_number += 1
            print(json.dumps(result, sort_keys=True), flush=True)
            if report["valid"]:
                return 0
        if args.max_jobs is not None and job_number >= args.max_jobs:
            break

    Path(args.summary).parent.mkdir(parents=True, exist_ok=True)
    Path(args.summary).write_text(
        json.dumps(results, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
