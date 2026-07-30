#!/usr/bin/env python3

from __future__ import annotations

import unittest
from itertools import combinations

import cyclic_reduce_z3
import direct_repair


class DirectRepairEncodingTests(unittest.TestCase):
    def test_direct_encoding_requires_distinct_replacements(self) -> None:
        smt = direct_repair.emit_direct_smt2(
            core=[],
            deficits=set(combinations(range(1, 5), 2)),
            replacements=2,
            v=4,
            k=2,
            max_replication=3,
            timeout_ms=1000,
        )
        self.assertIn("(xor x_0_0 x_1_0)", smt)
        self.assertIn("(set-logic QF_FD)", smt)

    def test_direct_encoding_excludes_a_core_duplicate(self) -> None:
        smt = direct_repair.emit_direct_smt2(
            core=[(1, 2)],
            deficits={(3, 4)},
            replacements=1,
            v=4,
            k=2,
            max_replication=3,
            timeout_ms=1000,
        )
        self.assertIn(
            "(assert (or (not x_0_0) (not x_0_1) x_0_2 x_0_3))",
            smt,
        )

    def test_joint_encoding_excludes_kept_block_duplicates(self) -> None:
        smt = cyclic_reduce_z3.emit(
            [(1, 2, 3, 4, 5, 6, 7, 8)],
            timeout_ms=1000,
            max_replication=10,
            retained_count=1,
            replacement_count=1,
            fix_removed_zero=False,
        )
        self.assertIn("(assert (or (not keep_0)", smt)
        self.assertIn("(not x_0_0)", smt)


if __name__ == "__main__":
    unittest.main()
