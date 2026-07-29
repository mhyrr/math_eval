#!/usr/bin/env python3

from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

import structured_search as search


class StructuredSearchTests(unittest.TestCase):
    def test_pair_index_is_bijection(self) -> None:
        indices = {
            search.PAIR_INDEX[left][right]
            for left in range(search.V)
            for right in range(left + 1, search.V)
        }
        self.assertEqual(indices, set(range(search.PAIR_COUNT)))

    def test_group_seed_shape_and_pruning(self) -> None:
        rows = search.arithmetic_rows(42, 12345)
        self.assertTrue(
            all(sorted(map(len, row)) == [1, 1, 1, 1, 2, 2] for row in rows)
        )
        pruned = search.deterministic_prune(search.group_blocks(), rows, 46)
        blocks = search.group_blocks() + [
            search.cells_to_block(row) for row in pruned
        ]
        self.assertEqual(len(blocks), 46)
        self.assertTrue(all(len(block) == 8 for block in blocks))
        self.assertEqual(len(set(blocks)), 46)

    def test_gf8_affine_primaries_are_pairwise_orthogonal(self) -> None:
        rows = search.arithmetic_rows(40, 444)
        for left_group in range(6):
            for right_group in range(left_group + 1, 6):
                primary_pairs = {
                    (row[left_group][0], row[right_group][0]) for row in rows
                }
                self.assertEqual(len(primary_pairs), 40)
        nonzero = set(range(1, 8))
        products = {search.gf8_multiply(3, value) for value in nonzero}
        self.assertEqual(products, nonzero)

    def test_exact_score_on_complete_and_incomplete_inputs(self) -> None:
        # Repeating all 48 cyclic translates of this interval is incomplete:
        # score must use the exact 1,128-pair predicate rather than structure.
        incomplete = search.develop_cyclic(tuple(range(8)))
        score = search.exact_score(incomplete)
        self.assertEqual(score.block_count, 48)
        self.assertGreater(score.uncovered, 0)
        self.assertFalse(score.is_valid)

    def test_cyclic_development_and_difference_cover(self) -> None:
        base, _ = search.find_cyclic_base(seed=9, evaluations=4000)
        orbit = search.develop_cyclic(base)
        self.assertEqual(len(base), 8)
        self.assertEqual(len(orbit), 48)
        self.assertEqual(search.cyclic_base_rank(base)[0], 0)
        self.assertTrue(search.exact_score(orbit).is_valid)

    def test_cyclic_evaluation_budget_is_literal(self) -> None:
        _, used = search.find_cyclic_base(seed=91, evaluations=17)
        self.assertEqual(used, 17)

    def test_incremental_replacement_matches_full_rescore(self) -> None:
        rows = search.arithmetic_rows(40, 91)
        state = search.GDDState(rows)
        missing_index = state.missing_indices()[0]
        move = search.best_forced_replacement(state, missing_index)
        self.assertIsNotNone(move)
        assert move is not None
        row_index, variant, uncovered_delta, square_delta, _ = move
        old_uncovered = state.uncovered
        old_square = state.square_sum
        state.replace(row_index, variant)
        counts = search.pair_counts(state.blocks)
        self.assertEqual(state.uncovered, old_uncovered + uncovered_delta)
        self.assertEqual(state.uncovered, sum(count == 0 for count in counts))
        self.assertEqual(state.square_sum, old_square + square_delta)
        self.assertEqual(state.square_sum, sum(count * count for count in counts))

    def test_checkpoint_is_canonical_and_deduplicated(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory)
            checkpointer = search.Checkpointer(output, "test", 48, 7)
            blocks = search.develop_cyclic(
                search.find_cyclic_base(7, 4000)[0]
            )
            score = search.exact_score(blocks)
            first = checkpointer.consider(
                list(reversed(blocks)),
                score,
                evaluations=1,
                restart=0,
                elapsed=0.1,
            )
            second = checkpointer.consider(
                blocks,
                score,
                evaluations=2,
                restart=1,
                elapsed=0.2,
            )
            self.assertIsNotNone(first)
            self.assertIsNone(second)
            assert first is not None
            text = first.read_text(encoding="utf-8")
            self.assertEqual(text, search.canonical_text(blocks))
            self.assertTrue(first.with_suffix(".json").exists())


if __name__ == "__main__":
    unittest.main()
