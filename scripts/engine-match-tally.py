#!/usr/bin/env python3
"""Tally an engine match by pairing.

Usage: engine-match-tally.py GAMES.pgn NAME NAME [NAME...]

Prints wins, draws and losses for each pair of the named engines, with an
Elo difference and a 95% range from a normal approximation over single
games. Each pair is printed later name against earlier name, so list a
baseline first.

The Elo difference needs both wins and losses, since a score of 0% or 100%
has no finite Elo, and the range needs at least 10 games,
below which the normal approximation means little. Games that did not end
normally, such as losses on time or crashes, are counted and reported,
since they usually point to a machine or engine problem rather than to
playing strength.
"""

import math
import re
import sys

MIN_GAMES_FOR_RANGE = 10


def elo(score):
    return -400 * math.log10(1 / score - 1)


def signed(value):
    rounded = round(value)
    return f"{rounded:+d}" if rounded != 0 else "0"


def estimate(wins, draws, losses):
    """Return the Elo difference and its 95% range as printable strings."""
    games = wins + draws + losses
    score = (wins + draws / 2) / games

    if wins == 0 and losses == 0:
        return "0", "n/a (all draws)"
    if losses == 0:
        return "n/a", "n/a (no losses)"
    if wins == 0:
        return "n/a", "n/a (no wins)"

    difference = signed(elo(score))
    if games < MIN_GAMES_FOR_RANGE:
        return difference, f"n/a (fewer than {MIN_GAMES_FOR_RANGE} games)"

    variance = (wins * (1 - score) ** 2 + draws * (0.5 - score) ** 2 + losses * score ** 2) / games
    margin = 1.96 * math.sqrt(variance / games)
    low = score - margin
    high = score + margin
    low_text = signed(elo(low)) if low > 0 else "-inf"
    high_text = signed(elo(high)) if high < 1 else "+inf"
    return difference, f"{low_text} to {high_text}"


def main():
    if len(sys.argv) < 4:
        print(__doc__.strip(), file=sys.stderr)
        sys.exit(1)

    path = sys.argv[1]
    names = sys.argv[2:]
    if len(set(names)) != len(names):
        sys.exit("Error: an engine name is given twice")

    try:
        with open(path, encoding="utf-8") as pgn:
            text = pgn.read()
    except OSError as error:
        sys.exit(f"Error: cannot read {path}: {error.strerror}")

    # Keyed by (earlier, later): wins, draws and losses for the later engine.
    results = {}
    abnormal = {}
    seen = set()

    for game in re.split(r"\n(?=\[Event )", text):
        tags = dict(re.findall(r'\[(\w+) "([^"]*)"\]', game))
        white, black, result = tags.get("White"), tags.get("Black"), tags.get("Result")
        seen.update(name for name in (white, black) if name)
        if white not in names or black not in names or result not in ("1-0", "0-1", "1/2-1/2"):
            continue

        earlier, later = sorted((white, black), key=names.index)
        score = {"1-0": 1.0, "0-1": 0.0, "1/2-1/2": 0.5}[result]
        if later == black:
            score = 1.0 - score

        counts = results.setdefault((earlier, later), [0, 0, 0])
        counts[{1.0: 0, 0.5: 1, 0.0: 2}[score]] += 1

        termination = tags.get("Termination", "normal")
        if termination != "normal":
            key = (earlier, later, termination)
            abnormal[key] = abnormal.get(key, 0) + 1

    missing = [name for name in names if name not in seen]
    if missing:
        found = ", ".join(sorted(seen)) or "none"
        sys.exit(f"Error: no games for {', '.join(missing)} in {path} (engines found: {found})")

    print(f"{'Pairing':<28} {'Games':>6} {'W / D / L':>16} {'Score':>7} {'Elo':>6}  95% range")
    for (earlier, later), (wins, draws, losses) in sorted(
        results.items(), key=lambda item: (names.index(item[0][1]), names.index(item[0][0]))
    ):
        games = wins + draws + losses
        score = (wins + draws / 2) / games
        difference, interval = estimate(wins, draws, losses)

        pairing = f"{later} vs {earlier}"
        record = f"{wins} / {draws} / {losses}"
        print(f"{pairing:<28} {games:>6} {record:>16} {100 * score:>6.1f}% {difference:>6}  {interval}")

    for (earlier, later, termination), count in sorted(abnormal.items()):
        print(f"Warning: {count} game(s) between {later} and {earlier} ended by {termination}")


if __name__ == "__main__":
    main()
