#!/usr/bin/env python3

from __future__ import annotations

import tempfile
import unittest
from itertools import combinations
from pathlib import Path

import independent_verify
import verifier


def affine_plane_order_three() -> list[tuple[int, int, int]]:
    """Generate the 12 lines of AG(2,3), independently of either verifier."""

    label = {(x, y): 1 + 3 * x + y for x in range(3) for y in range(3)}
    lines: list[tuple[int, int, int]] = []
    for intercept in range(3):
        lines.append(tuple(label[(intercept, y)] for y in range(3)))
    for slope in range(3):
        for intercept in range(3):
            lines.append(
                tuple(label[(x, (slope * x + intercept) % 3)] for x in range(3))
            )
    return lines


class VerifierAgreementTests(unittest.TestCase):
    def assert_agree(
        self,
        blocks: list[tuple[int, ...]] | list[list[int]],
        *,
        v: int,
        k: int,
        valid: bool,
    ) -> None:
        primary = verifier.verify(blocks, v=v, k=k)
        independent = independent_verify.check(blocks, v=v, k=k)
        self.assertEqual(primary.valid, valid)
        self.assertEqual(independent["valid"], valid)
        self.assertEqual(
            primary.uncovered_pairs, independent["uncovered_pairs"]
        )
        self.assertEqual(
            primary.pair_multiplicity_histogram,
            independent["pair_multiplicity_histogram"],
        )
        self.assertEqual(
            primary.repeated_pair_incidences,
            independent["repeated_pair_incidences"],
        )
        self.assertEqual(primary.redundant_blocks, independent["redundant_blocks"])

    def test_complete_pair_cover(self) -> None:
        blocks = [tuple(pair) for pair in combinations(range(1, 6), 2)]
        self.assert_agree(blocks, v=5, k=2, valid=True)

    def test_independently_generated_affine_plane(self) -> None:
        blocks = affine_plane_order_three()
        self.assertEqual(len(blocks), 12)
        self.assert_agree(blocks, v=9, k=3, valid=True)
        report = verifier.verify(blocks, v=9, k=3)
        self.assertEqual(report.pair_multiplicity_histogram, {1: 36})
        self.assertEqual(report.redundant_blocks, [])

    def test_malformed_block_size(self) -> None:
        blocks = [(1, 2), (1, 2, 3), (1, 3), (2, 3)]
        self.assert_agree(blocks, v=3, k=2, valid=False)

    def test_duplicate_point_inside_block(self) -> None:
        blocks = [(1, 1), (1, 2), (1, 3), (2, 3)]
        self.assert_agree(blocks, v=3, k=2, valid=False)

    def test_missing_pair(self) -> None:
        blocks = [(1, 2), (1, 3)]
        self.assert_agree(blocks, v=3, k=2, valid=False)
        report = verifier.verify(blocks, v=3, k=2)
        self.assertEqual(report.uncovered_pairs, [(2, 3)])

    def test_duplicate_block_ignores_point_order(self) -> None:
        blocks = [(1, 2), (2, 1), (1, 3), (2, 3)]
        self.assert_agree(blocks, v=3, k=2, valid=False)

    def test_out_of_range_point(self) -> None:
        blocks = [(1, 2), (1, 3), (2, 4), (2, 3)]
        self.assert_agree(blocks, v=3, k=2, valid=False)

    def test_redundant_block(self) -> None:
        blocks = [(1, 2, 3), (1, 2), (1, 3), (2, 3)]
        self.assert_agree(blocks, v=3, k=2, valid=False)
        primary = verifier.verify(blocks, v=3, k=2)
        # The malformed 3-block is excluded; the valid pair blocks are essential.
        self.assertEqual(primary.redundant_blocks, [])

    def test_valid_cover_with_redundancy(self) -> None:
        blocks = [(1, 2), (1, 2), (1, 3), (2, 3)]
        # Duplicate blocks invalidate the certificate even though coverage holds.
        self.assert_agree(blocks, v=3, k=2, valid=False)

    def test_parser_comments_and_canonicalization(self) -> None:
        text = """
        # example
        3 1 2  # first block

        2 1 3
        """
        blocks = verifier.parse_text(text)
        self.assertEqual(blocks, [(3, 1, 2), (2, 1, 3)])
        self.assertEqual(verifier.canonical_text(blocks), "1 2 3\n1 2 3\n")

    def test_parser_rejects_non_integer(self) -> None:
        with self.assertRaisesRegex(ValueError, "line 1"):
            verifier.parse_text("1 two 3\n")

    def test_cli_parsers_read_same_file(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "cover.txt"
            path.write_text("1 2\n1 3\n2 3\n", encoding="utf-8")
            self.assertEqual(
                verifier.parse_file(path),
                independent_verify.read_blocks(str(path)),
            )


if __name__ == "__main__":
    unittest.main()
