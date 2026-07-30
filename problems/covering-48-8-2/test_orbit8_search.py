#!/usr/bin/env python3
"""Tests for the independent six-layer Z_8 structured search."""

from __future__ import annotations

import subprocess
import tempfile
import unittest
from pathlib import Path

from verifier import parse_file, verify


HERE = Path(__file__).resolve().parent
SOURCE = HERE / "orbit8_search.cpp"


class Orbit8SearchTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.temporary = tempfile.TemporaryDirectory(prefix="orbit8-test-")
        cls.binary = Path(cls.temporary.name) / "orbit8_search"
        subprocess.run(
            [
                "c++",
                "-std=c++20",
                "-O2",
                "-DNDEBUG",
                str(SOURCE),
                "-o",
                str(cls.binary),
            ],
            check=True,
            cwd=HERE,
        )

    @classmethod
    def tearDownClass(cls) -> None:
        cls.temporary.cleanup()

    def test_internal_orbit_and_scoring_cross_checks(self) -> None:
        completed = subprocess.run(
            [str(self.binary), "--self-test"],
            check=True,
            cwd=HERE,
            text=True,
            capture_output=True,
        )
        self.assertIn("self-test ok", completed.stdout)
        self.assertIn("pair_classes=144", completed.stdout)
        self.assertIn("short4_orbits=2640", completed.stdout)
        self.assertIn("short2_orbits=30", completed.stdout)

    def test_seeded_run_writes_structurally_valid_46_block_checkpoint(self) -> None:
        work = Path(self.temporary.name)
        candidate = work / "candidate.txt"
        checkpoint = work / "checkpoint.txt"
        ledger = work / "ledger.csv"
        completed = subprocess.run(
            [
                str(self.binary),
                "--seed",
                "9981",
                "--steps",
                "200",
                "--restarts",
                "1",
                "--output",
                str(candidate),
                "--checkpoint",
                str(checkpoint),
                "--log",
                str(ledger),
            ],
            check=False,
            cwd=HERE,
            text=True,
            capture_output=True,
        )
        # Exit 1 means the bounded search produced a near-miss, not a tool error.
        self.assertIn(completed.returncode, (0, 1), completed.stderr)
        blocks = parse_file(candidate)
        report = verify(blocks)
        self.assertEqual(report.block_count, 46)
        self.assertFalse(
            any("entries" in error for error in report.errors), report.errors
        )
        self.assertFalse(
            any("duplicate point" in error for error in report.errors), report.errors
        )
        self.assertFalse(
            any("duplicates block" in error for error in report.errors), report.errors
        )
        self.assertTrue(checkpoint.read_text().startswith("ORBIT8_CHECKPOINT 1\n"))
        self.assertIn("lane,seed,restarts", ledger.read_text())

    def test_same_seed_is_reproducible(self) -> None:
        work = Path(self.temporary.name)
        candidates: list[str] = []
        summaries: list[str] = []
        for suffix in ("a", "b"):
            candidate = work / f"repro-{suffix}.txt"
            completed = subprocess.run(
                [
                    str(self.binary),
                    "--seed",
                    "411",
                    "--steps",
                    "300",
                    "--restarts",
                    "1",
                    "--output",
                    str(candidate),
                ],
                check=False,
                cwd=HERE,
                text=True,
                capture_output=True,
            )
            self.assertIn(completed.returncode, (0, 1), completed.stderr)
            candidates.append(candidate.read_text())
            summary_fields = [
                field
                for field in completed.stdout.split()
                if not field.startswith("seconds=")
            ]
            summaries.append(" ".join(summary_fields))
        self.assertEqual(candidates[0], candidates[1])
        self.assertEqual(summaries[0], summaries[1])


if __name__ == "__main__":
    unittest.main()
