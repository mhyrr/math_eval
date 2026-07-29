#!/usr/bin/env python3

from __future__ import annotations

import tempfile
import unittest
from itertools import combinations
from pathlib import Path

import exact_repair


Z3 = "/opt/homebrew/bin/z3"


class ExactRepairTests(unittest.TestCase):
    def test_rejects_structurally_malformed_candidate(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "bad.txt"
            path.write_text("1 1 2\n", encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "repeats a point"):
                exact_repair.parse_candidate(path, v=4, k=3)

    def test_pool_is_seeded_canonical_and_duplicate_free(self) -> None:
        core = ((1, 2, 3),)
        removed = ((1, 2, 4),)
        deficits = exact_repair.uncovered_pairs(core, 4)
        keyword = dict(
            core=core,
            removed=removed,
            deficits=deficits,
            v=4,
            k=3,
            seed=271828,
            pool_size=4,
            anchor_restarts=2,
            random_candidates=20,
        )
        first = exact_repair.generate_candidate_pool(**keyword)
        second = exact_repair.generate_candidate_pool(**keyword)
        self.assertEqual(first, second)
        self.assertEqual(first, tuple(sorted(set(first))))
        self.assertNotIn(core[0], first)

    def test_smt_encoding_can_be_unsat_only_for_its_pool(self) -> None:
        smt2 = exact_repair.emit_smt2(
            pool=((1, 2),),
            deficits=((3, 4),),
            replacement_count=1,
            timeout_ms=5_000,
        )
        self.assertIn("; x0 = 1 2", smt2)
        self.assertIn("; deficit 3 4\n(assert false)", smt2)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "repair.smt2"
            path.write_text(smt2, encoding="utf-8")
            status, chosen, _, _ = exact_repair.run_z3(
                path, z3_path=Z3, timeout_ms=5_000
            )
        self.assertEqual(status, "unsat")
        self.assertEqual(chosen, ())

    def test_equal_size_repair_recovers_removed_blocks(self) -> None:
        blocks = list(combinations(range(1, 5), 2))
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "pairs.txt"
            output = root / "repaired.txt"
            source.write_text(exact_repair.canonical_text(blocks), encoding="utf-8")
            report = exact_repair.repair(
                input_path=source,
                v=4,
                k=2,
                seed=7,
                remove_count=2,
                replacement_count=2,
                explicit_remove_indices="1,6",
                selection_trials=2,
                selection_width=2,
                pool_size=6,
                anchor_restarts=1,
                random_candidates=10,
                include_removed=True,
                timeout_ms=5_000,
                z3_path=Z3,
                output_path=str(output),
                smt2_output=None,
            )
            self.assertTrue(report["valid"])
            self.assertEqual(report["result_block_count"], 6)
            self.assertTrue(output.exists())
            repaired = exact_repair.parse_candidate(output, v=4, k=2)
            self.assertTrue(exact_repair.is_valid_cover(repaired, v=4, k=2))

    def test_exact_reduction_finds_a_three_block_cover(self) -> None:
        # All four triples cover every pair twice.  Any frozen pair of triples
        # misses one pair, which a single generated replacement can cover.
        blocks = list(combinations(range(1, 5), 3))
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "triples.txt"
            output = root / "reduced.txt"
            source.write_text(exact_repair.canonical_text(blocks), encoding="utf-8")
            report = exact_repair.repair(
                input_path=source,
                v=4,
                k=3,
                seed=11,
                remove_count=2,
                replacement_count=1,
                explicit_remove_indices=None,
                selection_trials=4,
                selection_width=2,
                pool_size=4,
                anchor_restarts=2,
                random_candidates=10,
                include_removed=True,
                timeout_ms=5_000,
                z3_path=Z3,
                output_path=str(output),
                smt2_output=str(root / "repair.smt2"),
            )
            self.assertEqual(report["z3_status"], "sat")
            self.assertTrue(report["valid"])
            self.assertEqual(report["result_block_count"], 3)
            self.assertEqual(report["result_uncovered_pair_count"], 0)
            self.assertTrue((root / "repair.smt2").exists())


if __name__ == "__main__":
    unittest.main()
