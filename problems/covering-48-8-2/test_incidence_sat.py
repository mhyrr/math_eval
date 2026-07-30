#!/usr/bin/env python3

from __future__ import annotations

import subprocess
import tempfile
import unittest
from itertools import combinations
from pathlib import Path

import incidence_sat
import verifier


Z3 = "/opt/homebrew/bin/z3"


def solve(cnf: incidence_sat.CNF) -> tuple[str, str]:
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / "test.cnf"
        cnf.write_dimacs(path)
        completed = subprocess.run(
            [Z3, "-dimacs", "-model", str(path)],
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )
    return incidence_sat.solver_status(completed.stdout), completed.stdout


class IncidenceSatTests(unittest.TestCase):
    def test_guaranteed_pair_multiplicity_for_campaign_targets(self) -> None:
        self.assertEqual(
            incidence_sat.guaranteed_pair_multiplicity(v=48, k=8, blocks=44),
            4,
        )
        self.assertEqual(
            incidence_sat.guaranteed_pair_multiplicity(v=48, k=8, blocks=45),
            3,
        )
        self.assertEqual(
            incidence_sat.guaranteed_pair_multiplicity(v=48, k=8, blocks=46),
            3,
        )

    def test_exact_cardinality_counter(self) -> None:
        for assignment, expected in (
            ((True, True, False, False, False), "sat"),
            ((True, False, False, False, False), "unsat"),
            ((True, True, True, False, False), "unsat"),
        ):
            cnf = incidence_sat.CNF()
            variables = [cnf.new_variable() for _ in assignment]
            cnf.exactly(variables, 2)
            for variable, value in zip(variables, assignment, strict=True):
                cnf.add(variable if value else -variable)
            status, _ = solve(cnf)
            self.assertEqual(status, expected)

    def test_strict_lex_greater(self) -> None:
        for left, right, expected in (
            ((1, 0, 1), (1, 0, 0), "sat"),
            ((1, 0, 0), (1, 0, 1), "unsat"),
            ((1, 0, 1), (1, 0, 1), "unsat"),
        ):
            cnf = incidence_sat.CNF()
            left_variables = [cnf.new_variable() for _ in left]
            right_variables = [cnf.new_variable() for _ in right]
            cnf.strict_lex_greater(left_variables, right_variables)
            for variables, values in (
                (left_variables, left),
                (right_variables, right),
            ):
                for variable, value in zip(variables, values, strict=True):
                    cnf.add(variable if value else -variable)
            status, _ = solve(cnf)
            self.assertEqual(status, expected)

    def test_lex_greater_or_equal(self) -> None:
        for left, right, expected in (
            ((1, 0, 1), (1, 0, 0), "sat"),
            ((1, 0, 1), (1, 0, 1), "sat"),
            ((1, 0, 0), (1, 0, 1), "unsat"),
        ):
            cnf = incidence_sat.CNF()
            left_variables = [cnf.new_variable() for _ in left]
            right_variables = [cnf.new_variable() for _ in right]
            cnf.lex_greater_or_equal(left_variables, right_variables)
            for variables, values in (
                (left_variables, left),
                (right_variables, right),
            ):
                for variable, value in zip(variables, values, strict=True):
                    cnf.add(variable if value else -variable)
            status, _ = solve(cnf)
            self.assertEqual(status, expected)

    def test_small_complete_pair_cover_is_sat_and_decodes(self) -> None:
        encoding = incidence_sat.build_encoding(v=4, k=2, blocks=6)
        status, output = solve(encoding.cnf)
        self.assertEqual(status, "sat")
        blocks = incidence_sat.parse_dimacs_model(output, encoding.x)
        self.assertTrue(verifier.verify(blocks, v=4, k=2).valid)
        self.assertEqual(
            set(blocks),
            set(combinations(range(1, 5), 2)),
        )

    def test_five_pairs_cannot_cover_all_six_pairs(self) -> None:
        encoding = incidence_sat.build_encoding(v=4, k=2, blocks=5)
        status, _ = solve(encoding.cnf)
        self.assertEqual(status, "unsat")

    def test_candidate_normalization_preserves_cover(self) -> None:
        blocks = list(combinations(range(1, 5), 2))
        normalized = incidence_sat.normalize_candidate(
            blocks, v=4, k=2, anchor_index=3
        )
        self.assertEqual(normalized[0], (1, 2))
        self.assertTrue(verifier.verify(normalized, v=4, k=2).valid)

    def test_pinned_small_cover_round_trip(self) -> None:
        candidate = list(combinations(range(1, 5), 2))
        normalized = incidence_sat.normalize_candidate(
            candidate, v=4, k=2
        )
        encoding = incidence_sat.build_encoding(
            v=4,
            k=2,
            blocks=6,
            fixed_candidate=normalized,
        )
        status, output = solve(encoding.cnf)
        self.assertEqual(status, "sat")
        decoded = incidence_sat.parse_dimacs_model(output, encoding.x)
        self.assertEqual(decoded, normalized)
        self.assertEqual(
            encoding.symmetry,
            "fixed_first_block_and_strict_descending_rows",
        )

    def test_fixed_core_requires_anchor_row_frozen(self) -> None:
        with self.assertRaisesRegex(ValueError, "anchor block"):
            incidence_sat.build_encoding(
                v=4,
                k=2,
                blocks=6,
                fixed_candidate=list(combinations(range(1, 5), 2)),
                free_indices={0},
                row_lex=False,
            )

    def test_constant_folded_fixed_core_round_trip(self) -> None:
        candidate = list(combinations(range(1, 5), 2))
        normalized = incidence_sat.normalize_candidate(
            candidate, v=4, k=2
        )
        encoding = incidence_sat.build_encoding(
            v=4,
            k=2,
            blocks=6,
            fixed_candidate=normalized,
            free_indices={5},
            row_lex=False,
        )
        status, output = solve(encoding.cnf)
        self.assertEqual(status, "sat")
        decoded = incidence_sat.parse_dimacs_model(output, encoding.x)
        self.assertTrue(verifier.verify(decoded, v=4, k=2).valid)

    def test_constant_folded_core_preserves_impossibility(self) -> None:
        candidate = list(combinations(range(1, 5), 2))[:5]
        normalized = incidence_sat.normalize_candidate(
            candidate, v=4, k=2
        )
        encoding = incidence_sat.build_encoding(
            v=4,
            k=2,
            blocks=5,
            fixed_candidate=normalized,
            free_indices={3, 4},
            row_lex=False,
        )
        status, _ = solve(encoding.cnf)
        self.assertEqual(status, "unsat")

    def test_grouped_anchor_counts_fix_the_requested_rows(self) -> None:
        encoding = incidence_sat.build_encoding(
            v=4,
            k=3,
            blocks=4,
            anchor_points=1,
            anchor_counts=(2,),
        )
        status, output = solve(encoding.cnf)
        self.assertEqual(status, "sat")
        blocks = incidence_sat.parse_dimacs_model(output, encoding.x)
        self.assertTrue(verifier.verify(blocks, v=4, k=3).valid)
        self.assertEqual(encoding.anchor_counts, (2,))
        self.assertIn((encoding.x[1][0],), encoding.cnf.clauses)
        self.assertIn((encoding.x[2][0],), encoding.cnf.clauses)
        self.assertIn((-encoding.x[3][0],), encoding.cnf.clauses)

    def test_grouped_anchor_points_validate_the_anchor_bound(self) -> None:
        with self.assertRaisesRegex(ValueError, "anchor point count"):
            incidence_sat.build_encoding(
                v=7,
                k=3,
                blocks=7,
                anchor_points=3,
                anchor_counts=(1, 1, 1),
            )

    def test_free_index_selection_is_seeded_and_excludes_anchor(self) -> None:
        blocks = list(combinations(range(1, 6), 2))
        first = incidence_sat.select_free_indices(
            blocks, count=3, mode="random", seed=19
        )
        second = incidence_sat.select_free_indices(
            blocks, count=3, mode="random", seed=19
        )
        self.assertEqual(first, second)
        self.assertNotIn(0, first)


if __name__ == "__main__":
    unittest.main()
