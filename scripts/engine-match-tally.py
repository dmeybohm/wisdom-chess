#!/usr/bin/env python3
"""Tally an engine match by pairing.

Usage: engine-match-tally.py GAMES.pgn NAME [NAME...]

Prints wins, draws and losses for each pair of the named engines, with an
Elo difference and a 95% range from a normal approximation over single
games. Each pair is printed later name against earlier name, so list a
baseline first. Games lost on time are counted and reported, since at short
clocks they usually point to a machine problem rather than a weak engine.
"""

import math
import re
import sys


def elo(score):
    return -400 * math.log10(1 / score - 1)


def main():
    if len(sys.argv) < 4:
        sys.exit(__doc__.strip())

    names = sys.argv[2:]
    text = open(sys.argv[1], encoding="utf-8").read()

    # Keyed by (earlier, later): wins, draws and losses for the later engine.
    results = {}
    time_losses = {}

    for game in re.split(r"\n(?=\[Event )", text):
        tags = dict(re.findall(r'\[(\w+) "([^"]*)"\]', game))
        white, black, result = tags.get("White"), tags.get("Black"), tags.get("Result")
        if white not in names or black not in names or result not in ("1-0", "0-1", "1/2-1/2"):
            continue

        earlier, later = sorted((white, black), key=names.index)
        score = {"1-0": 1.0, "0-1": 0.0, "1/2-1/2": 0.5}[result]
        if later == black:
            score = 1.0 - score

        counts = results.setdefault((earlier, later), [0, 0, 0])
        counts[{1.0: 0, 0.5: 1, 0.0: 2}[score]] += 1

        if "loses on time" in game:
            time_losses[(earlier, later)] = time_losses.get((earlier, later), 0) + 1

    print(f"{'Pairing':<28} {'Games':>6} {'W / D / L':>16} {'Score':>7} {'Elo':>6}  95% range")
    for (earlier, later), (wins, draws, losses) in sorted(
        results.items(), key=lambda item: (names.index(item[0][1]), names.index(item[0][0]))
    ):
        games = wins + draws + losses
        score = (wins + draws / 2) / games
        clamped = min(max(score, 0.5 / games), 1 - 0.5 / games)
        variance = (
            wins * (1 - clamped) ** 2 + draws * (0.5 - clamped) ** 2 + losses * clamped ** 2
        ) / games
        margin = 1.96 * math.sqrt(variance / games)
        low = elo(max(clamped - margin, 1e-6))
        high = elo(min(clamped + margin, 1 - 1e-6))

        pairing = f"{later} vs {earlier}"
        record = f"{wins} / {draws} / {losses}"
        print(f"{pairing:<28} {games:>6} {record:>16} {100 * score:>6.1f}% "
              f"{elo(clamped):>+6.0f}  {low:+.0f} to {high:+.0f}")

    for (earlier, later), count in sorted(time_losses.items()):
        print(f"Warning: {count} game(s) between {later} and {earlier} lost on time")


if __name__ == "__main__":
    main()
