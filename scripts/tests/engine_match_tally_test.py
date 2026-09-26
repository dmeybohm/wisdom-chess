#!/usr/bin/env python3
"""Tests for scripts/engine-match-tally.py, run as a program on made-up PGNs."""

import os
import subprocess
import sys
import tempfile
import unittest

TALLY = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "engine-match-tally.py")


def game(white, black, result, termination="normal"):
    return (
        f'[Event "?"]\n[White "{white}"]\n[Black "{black}"]\n[Result "{result}"]\n'
        f'[Termination "{termination}"]\n\n1. e4 e5 {result}\n\n'
    )


class TallyTest(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.pgn = os.path.join(self.directory.name, "games.pgn")

    def tearDown(self):
        self.directory.cleanup()

    def tally(self, games, *names):
        with open(self.pgn, "w", encoding="utf-8") as pgn:
            pgn.write("".join(games))
        return subprocess.run(
            [sys.executable, TALLY, self.pgn, *names], capture_output=True, text=True
        )

    def row(self, games, *names):
        result = self.tally(games, *names)
        self.assertEqual(result.returncode, 0, result.stderr)
        return result.stdout.splitlines()[1]

    def test_counts_from_the_later_engine_in_either_colour(self):
        games = [game("new", "base", "1-0"), game("base", "new", "1-0"), game("base", "new", "1/2-1/2")]
        self.assertIn("new vs base", self.row(games, "base", "new"))
        self.assertIn("1 / 1 / 1", self.row(games, "base", "new"))

    def test_an_even_score_is_zero_not_minus_zero(self):
        games = [game("new", "base", "1-0"), game("base", "new", "1-0")] * 5
        row = self.row(games, "base", "new")
        self.assertIn(" 0  ", row)
        self.assertNotIn("-0", row)

    def test_a_score_of_all_wins_has_no_estimate(self):
        row = self.row([game("new", "base", "1-0")] * 10, "base", "new")
        self.assertIn("n/a (no losses)", row)
        self.assertNotIn("+", row)

    def test_a_single_win_has_no_estimate(self):
        self.assertIn("n/a (no losses)", self.row([game("new", "base", "1-0")], "base", "new"))

    def test_all_draws_have_no_range(self):
        self.assertIn("n/a (all draws)", self.row([game("new", "base", "1/2-1/2")] * 4, "base", "new"))

    def test_few_games_have_no_range(self):
        games = [game("new", "base", "1-0"), game("new", "base", "0-1"), game("new", "base", "1-0")]
        row = self.row(games, "base", "new")
        self.assertIn("+120", row)
        self.assertIn("n/a (fewer than 10 games)", row)

    def test_enough_games_have_a_range_around_the_estimate(self):
        games = [game("new", "base", "1-0")] * 6 + [game("new", "base", "0-1")] * 4
        row = self.row(games, "base", "new")
        self.assertIn("+70", row)
        self.assertRegex(row, r"-\d+ to \+\d+$")

    def test_abnormal_endings_are_reported(self):
        games = [game("new", "base", "1-0", "time forfeit"), game("new", "base", "0-1", "illegal move")]
        output = self.tally(games, "base", "new").stdout
        self.assertIn("1 game(s) between new and base ended by time forfeit", output)
        self.assertIn("1 game(s) between new and base ended by illegal move", output)

    def test_a_missing_pgn_is_an_error(self):
        result = subprocess.run(
            [sys.executable, TALLY, os.path.join(self.directory.name, "none.pgn"), "a", "b"],
            capture_output=True, text=True,
        )
        self.assertEqual(result.returncode, 1)
        self.assertIn("Error: cannot read", result.stderr)
        self.assertNotIn("Traceback", result.stderr)

    def test_a_name_without_games_is_an_error(self):
        result = self.tally([game("new", "base", "1-0")], "base", "typo")
        self.assertEqual(result.returncode, 1)
        self.assertIn("no games for typo", result.stderr)

    def test_one_name_prints_the_usage(self):
        result = self.tally([game("new", "base", "1-0")], "base")
        self.assertEqual(result.returncode, 1)
        self.assertIn("NAME NAME", result.stderr)


if __name__ == "__main__":
    unittest.main()
