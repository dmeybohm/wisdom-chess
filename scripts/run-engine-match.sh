#!/bin/bash
# Measure playing strength by playing engines against each other. Each
# NAME=ENGINE names an engine, and fastchess plays a round-robin between
# them from a book of openings, each opening once with each colour. See
# features/2026/09/engine-match-script.md and
# features/2026/10/elo-rating.md.
#
# Usage: ./scripts/run-engine-match.sh [options] NAME=ENGINE NAME=ENGINE [NAME=ENGINE...]
#        ./scripts/run-engine-match.sh --resume RESULTS_DIR
#
# An ENGINE is one of:
#   REF                 this engine, built at a git commit, branch or tag
#   stockfish:ELO       Stockfish, limited to ELO with UCI_LimitStrength;
#                       downloaded on first use; ELO is 1320 to 3190
#   uci:PATH[@RATING]   any UCI engine on this machine, with an optional
#                       rating of its own, such as its CCRL rating
#
# An engine with a rating is an anchor. With anchors, the other engines
# play each other and every anchor, the anchors do not play each other,
# and Ordo rates every engine against the anchors, so the summary also
# gives each engine's rating on the anchors' scale. Stockfish's UCI_Elo
# scale is calibrated at 120 seconds plus 1 a move against CCRL 40/4.
# A match of anchors alone is a round-robin that checks their scale
# against each other, and no rating is computed.
#
# Examples: ./scripts/run-engine-match.sh base=main new=HEAD
#           ./scripts/run-engine-match.sh new=HEAD sf1600=stockfish:1600 sf2000=stockfish:2000
#
# Options:
#   --tc TC             time control in fastchess's format, such as 8+0.08
#                       for 8 seconds plus 0.08 a move (default: 8+0.08)
#   --rounds N          openings per pairing, each played twice (default: 250)
#   --concurrency N     games at a time (default: the number of physical
#                       cores this process may use); each game is pinned to
#                       a core of its own when there are enough
#   --overhead MS       the engines' Move Overhead (default: 30)
#   --hash MB           the engines' hash size (default: 16)
#   --seed N            seed for the order of the openings (default: 1)
#   --work-dir DIR      where builds, engines, fastchess, the book and the
#                       results go (default: $XDG_CACHE_HOME/wisdom-chess/match,
#                       or ~/.cache/wisdom-chess/match)
#   --build-only        build or fetch the engines and stop
#   --resume DIR        continue an interrupted match from its results
#                       directory; the other options are taken from it
#   -j N                parallel build jobs (default: nproc)
#   -h, --help          this message
#
# List the baseline first: the summary prints each pairing as the later
# engine against the earlier one. This engine's Depth option is set to its
# maximum, 64, so that only the clock limits a search.
#
# Engines are cached by commit, so a rerun only builds what changed. Only
# one match can use a work directory at a time. The machine is kept from
# sleeping while the match runs, but closing a laptop's lid may still
# suspend it. After an interrupt the games so far are tallied and the
# command to continue is printed.
#
# Linux only. Nothing is installed system-wide and no root is needed. When a
# required tool is missing the script prints the command to install it and
# stops; it never runs sudo itself.

set -e
set -o pipefail

FASTCHESS_REPO=https://github.com/Disservin/fastchess.git
FASTCHESS_COMMIT=60d7a7a26c6b0582a15c112fb29a1829bef2adb3

BOOK_URL=https://github.com/official-stockfish/books/raw/master/8moves_v3.pgn.zip
BOOK_SHA256=7e1e9dd118b4bb97d8a8b5b8a790c86e21f8509d59a27d2883767d94477be02e
BOOK_NAME=8moves_v3.pgn

STOCKFISH_VERSION=19
STOCKFISH_URL=https://github.com/official-stockfish/Stockfish/releases/download/sf_19/stockfish-linux-x86-64-universal.tar.gz
STOCKFISH_SHA256=9defc0d4e55d49c65a6d042f3e571a39fcea499ade6dbe741b53b8c65e03611f
STOCKFISH_MEMBER=stockfish/stockfish-linux-x86-64-universal

ORDO_COMMIT=17eec774f2e4b9fdd2b1b38739f55ea221fb851a
ORDO_URL="https://github.com/michiguel/Ordo/archive/$ORDO_COMMIT.tar.gz"
ORDO_SHA256=eae9aa565cc9c555f5f7cc90f3871563011ee24dede1e3333401e8e3494fbb32
ORDO_SIMULATIONS=1000

MAX_DEPTH=64

# fastchess warns for every move of this engine, which prints no scores,
# and for every move of a Stockfish playing below full strength, whose
# best move is on purpose not the one its last PV began with; it follows
# that warning with the PV, the position and the moves.
NOISE_PATTERN="^(Warning; (No info line available|Bestmove does not match beginning of last PV)|Info; |Position; |Moves; )"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

TC=8+0.08
ROUNDS=250
CONCURRENCY=
OVERHEAD=30
HASH=16
SEED=1
WORK_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/wisdom-chess/match"
BUILD_ONLY=no
RESUME_DIR=
JOBS="$(nproc)"
ENGINE_SPECS=()

fail() {
    echo "Error: $*" >&2
    exit 1
}

usage() {
    sed -n '2,/^$/{s/^# \{0,1\}//;p}' "${BASH_SOURCE[0]}"
}

missing_package() {
    local what="$1"
    local command="$2"

    echo "Error: $what" >&2
    echo "Install it with:" >&2
    echo "" >&2
    echo "  $command" >&2
    exit 1
}

require_tools() {
    local tool
    for tool in git cmake make c++ curl unzip tar sha256sum timeout flock python3; do
        command -v "$tool" > /dev/null \
            || missing_package "$tool is required." \
                "sudo apt install git cmake build-essential curl unzip tar coreutils util-linux python3"
    done
}

# An option's value must be there, and must not be the next option.
need_value() {
    local option="$1"
    local remaining="$2"
    local value="$3"

    [ "$remaining" -ge 2 ] || fail "$option needs a value (see --help)"
    case "$value" in
        ''|-*) fail "$option needs a value, got '$value' (see --help)" ;;
    esac
}

need_whole_number() {
    local option="$1"
    local value="$2"
    local least="$3"

    [[ "$value" =~ ^[0-9]+$ ]] && [ "$value" -ge "$least" ] \
        || fail "$option must be a whole number of at least $least, got '$value'"
}

while [ $# -gt 0 ]; do
    case "$1" in
        --tc|--rounds|--concurrency|--overhead|--hash|--seed|--work-dir|--resume|-j)
            need_value "$1" "$#" "${2-}"
            case "$1" in
                --tc) TC="$2" ;;
                --rounds) ROUNDS="$2" ;;
                --concurrency) CONCURRENCY="$2" ;;
                --overhead) OVERHEAD="$2" ;;
                --hash) HASH="$2" ;;
                --seed) SEED="$2" ;;
                --work-dir) WORK_DIR="$2" ;;
                --resume) RESUME_DIR="$2" ;;
                -j) JOBS="$2" ;;
            esac
            shift 2
            ;;
        --build-only) BUILD_ONLY=yes; shift ;;
        -h|--help) usage; exit 0 ;;
        -*) fail "unknown option $1 (see --help)" ;;
        *=*) ENGINE_SPECS+=("$1"); shift ;;
        *) fail "expected NAME=ENGINE, got $1 (see --help)" ;;
    esac
done

[ "$(uname -s)" = Linux ] || fail "this script only runs on Linux"

[[ "$TC" =~ ^([0-9]+/)?([0-9]+:)?[0-9]+(\.[0-9]+)?(\+[0-9]+(\.[0-9]+)?)?$ ]] \
    || fail "--tc must look like 8+0.08 or 40/60, got '$TC'"
need_whole_number --rounds "$ROUNDS" 1
[ -z "$CONCURRENCY" ] || need_whole_number --concurrency "$CONCURRENCY" 1
need_whole_number --overhead "$OVERHEAD" 0
need_whole_number --hash "$HASH" 1
need_whole_number --seed "$SEED" 0
need_whole_number -j "$JOBS" 1

# Each engine has a name, a kind (git, stockfish or uci), a value (the
# commit, the UCI_Elo or the path) and a rating, empty unless it is an
# anchor. engines.txt in the results directory holds the same, one
# tab-separated line per engine, so that a match can be resumed.
NAMES=()
KINDS=()
VALUES=()
RATINGS=()

add_engine() {
    local name="$1"
    local kind="$2"
    local value="$3"
    local rating="$4"
    local existing

    [[ "$name" =~ ^[A-Za-z0-9_-]+$ ]] || fail "engine names may only use letters, digits, - and _: $name"
    for existing in "${NAMES[@]}"; do
        [ "$existing" != "$name" ] || fail "engine name $name is given twice"
    done
    NAMES+=("$name")
    KINDS+=("$kind")
    VALUES+=("$value")
    RATINGS+=("$rating")
}

parse_spec() {
    local spec="$1"
    local name="${spec%%=*}"
    local engine="${spec#*=}"
    local commit rating

    case "$engine" in
        stockfish:*)
            rating="${engine#stockfish:}"
            [[ "$rating" =~ ^[0-9]+$ ]] || fail "stockfish: needs a whole-number Elo, got '$rating'"
            add_engine "$name" stockfish "$rating" "$rating"
            ;;
        uci:*)
            engine="${engine#uci:}"
            rating=
            if [[ "$engine" == *@* ]]; then
                rating="${engine##*@}"
                engine="${engine%@*}"
                [[ "$rating" =~ ^[0-9]+$ ]] || fail "the rating after @ must be a whole number, got '$rating'"
            fi
            [ -n "$engine" ] || fail "uci: needs the path of an engine"
            [ -x "$engine" ] || fail "$engine is not an executable file"
            add_engine "$name" uci "$(cd "$(dirname "$engine")" && pwd)/$(basename "$engine")" "$rating"
            ;;
        *)
            commit="$(git -C "$REPO_DIR" rev-parse --verify --quiet "$engine^{commit}")" \
                || fail "$engine is not a commit, branch or tag in $REPO_DIR"
            add_engine "$name" git "$commit" ""
            ;;
    esac
}

read_engines_file() {
    local file="$1"
    local name kind value rating

    while IFS=$'\t' read -r name kind value rating; do
        # Before anchors, the file held "name commit" separated by a space.
        if [ -z "$kind" ] && [[ "$name" =~ ^([A-Za-z0-9_-]+)\ ([0-9a-f]{40})$ ]]; then
            name="${BASH_REMATCH[1]}"
            kind=git
            value="${BASH_REMATCH[2]}"
        fi
        case "$kind" in
            git|stockfish|uci) [ -n "$value" ] ;;
            *) false ;;
        esac || fail "$file should list each engine's name, kind and value"
        add_engine "$name" "$kind" "$value" "$rating"
    done < "$file"
}

if [ -n "$RESUME_DIR" ]; then
    [ "${#ENGINE_SPECS[@]}" -eq 0 ] || fail "--resume takes its engines from the results directory"
    [ -f "$RESUME_DIR/config.json" ] && [ -f "$RESUME_DIR/engines.txt" ] \
        || fail "$RESUME_DIR is not the results directory of a match that can be resumed"
    RUN_DIR="$(cd "$RESUME_DIR" && pwd)"
    WORK_DIR="$(cd "$RUN_DIR/../.." && pwd)"
    read_engines_file "$RUN_DIR/engines.txt"
else
    [ "${#ENGINE_SPECS[@]}" -ge 2 ] || fail "give at least two engines as NAME=ENGINE (see --help)"
    for spec in "${ENGINE_SPECS[@]}"; do
        parse_spec "$spec"
    done
fi
require_tools

mkdir -p "$WORK_DIR"/{engines,src,build,bin,books,cpm,results}
WORK_DIR="$(cd "$WORK_DIR" && pwd)"

# Two matches in one work directory would delete each other's builds and
# pin their games to the same cores. The lock is released when the script
# and everything it started have exited.
exec 9> "$WORK_DIR/lock"
flock -n 9 || fail "another match is using $WORK_DIR; wait for it to finish or pass another --work-dir"

# Sets ENGINE_OPTIONS to what the engine prints for "uci", or fails.
engine_answers() {
    ENGINE_OPTIONS="$(printf 'uci\nquit\n' | timeout 10 "$1" 2> /dev/null)" \
        && grep -q "^uciok" <<< "$ENGINE_OPTIONS"
}

# Whether ENGINE_OPTIONS, from the last engine_answers, names the option.
engine_has_option() {
    grep -q "^option name $1 " <<< "$ENGINE_OPTIONS"
}

# Build the UCI engine from a clean export of the commit, so that local
# changes in the working tree never end up in a match.
build_engine() {
    local commit="$1"
    local engine="$WORK_DIR/engines/wisdom-chess-uci-$commit"

    if [ -x "$engine" ] && ! engine_answers "$engine"; then
        echo "The cached engine at ${commit:0:12} does not answer; rebuilding it."
        rm -f "$engine"
    fi

    if [ ! -x "$engine" ]; then
        local src="$WORK_DIR/src/$commit"
        local build="$WORK_DIR/build/$commit"
        local log="$WORK_DIR/build/$commit.log"

        echo "Building the engine at ${commit:0:12}..."
        rm -rf "$src" "$build"
        mkdir -p "$src"
        git -C "$REPO_DIR" archive "$commit" | tar -x -C "$src"

        if ! {
            cmake -S "$src" -B "$build" \
                -DCMAKE_BUILD_TYPE=Release \
                -DWISDOM_CHESS_QML_UI=OFF \
                -DWISDOM_CHESS_REACT_UI=OFF \
                -DWISDOM_CHESS_FAST_TESTS=OFF \
                -DWISDOM_CHESS_SLOW_TESTS=OFF \
                -DWISDOM_CHESS_BUILD_LINTER=OFF \
                -DCPM_SOURCE_CACHE="$WORK_DIR/cpm" \
            && cmake --build "$build" --target wisdom-chess-uci -j "$JOBS"
        } > "$log" 2>&1; then
            tail -20 "$log" >&2
            fail "the build at ${commit:0:12} failed; the full log is $log"
        fi

        cp "$build/src/wisdom-chess/ui/uci/wisdom-chess-uci" "$engine.tmp"
        mv "$engine.tmp" "$engine"
        rm -rf "$src" "$build"

        engine_answers "$engine" \
            || fail "the engine built at ${commit:0:12} does not answer the uci command"
    fi

    # Engines from before 3c48792 have no Move Overhead, and a timer that
    # overshoots short budgets by tens of milliseconds, so they lose on time
    # at short clocks.
    engine_has_option "Move Overhead" \
        || fail "the engine at ${commit:0:12} has no Move Overhead option, so it predates" \
            "the search timer fixes of 3c48792 and would lose on time; use a later commit"
}

# Download a pinned release of Stockfish, checked against its SHA-256.
fetch_stockfish() {
    STOCKFISH="$WORK_DIR/engines/stockfish-$STOCKFISH_VERSION"

    if [ ! -x "$STOCKFISH" ]; then
        local tarball="$WORK_DIR/engines/stockfish-$STOCKFISH_VERSION.tar.gz"
        local unpacked="$WORK_DIR/engines/unpacked"

        echo "Downloading Stockfish $STOCKFISH_VERSION..."
        curl -fsSL -o "$tarball" "$STOCKFISH_URL"
        echo "$STOCKFISH_SHA256  $tarball" | sha256sum --check --quiet \
            || fail "the downloaded Stockfish does not match its checksum"
        rm -rf "$unpacked"
        mkdir -p "$unpacked"
        tar -xzf "$tarball" -C "$unpacked" "$STOCKFISH_MEMBER"
        mv "$unpacked/$STOCKFISH_MEMBER" "$STOCKFISH"
        rm -rf "$unpacked" "$tarball"
    fi

    engine_answers "$STOCKFISH" || fail "$STOCKFISH does not answer the uci command"
    engine_has_option UCI_Elo || fail "$STOCKFISH has no UCI_Elo option"
}

# The Elo an engine's UCI_Elo option accepts, as "min max".
uci_elo_range() {
    sed -n 's/^option name UCI_Elo type spin default [0-9]* min \([0-9]*\) max \([0-9]*\).*/\1 \2/p' \
        <<< "$ENGINE_OPTIONS"
}

check_stockfish_elo() {
    local elo="$1"
    local least most

    read -r least most <<< "$(uci_elo_range)"
    [ -n "$least" ] && [ -n "$most" ] || fail "$STOCKFISH does not say what UCI_Elo it accepts"
    [ "$elo" -ge "$least" ] && [ "$elo" -le "$most" ] \
        || fail "Stockfish $STOCKFISH_VERSION accepts a UCI_Elo from $least to $most, not $elo"
}

check_uci_engine() {
    local engine="$1"

    engine_answers "$engine" || fail "$engine does not answer the uci command"
}

# Sets ENGINE_COMMAND to the engine's executable and ENGINE_ARGS to its
# fastchess options, and checks that it answers.
prepare_engine() {
    local kind="$1"
    local value="$2"

    ENGINE_ARGS=()
    case "$kind" in
        git)
            build_engine "$value"
            ENGINE_COMMAND="$WORK_DIR/engines/wisdom-chess-uci-$value"
            ;;
        stockfish)
            fetch_stockfish
            check_stockfish_elo "$value"
            ENGINE_COMMAND="$STOCKFISH"
            ENGINE_ARGS+=("option.UCI_LimitStrength=true" "option.UCI_Elo=$value")
            ;;
        uci)
            check_uci_engine "$value"
            ENGINE_COMMAND="$value"
            ;;
        *) fail "unknown engine kind $kind" ;;
    esac

    # Options go only to the engines that have them, since fastchess passes
    # every option on and an engine may not accept one it does not know.
    ! engine_has_option "Move Overhead" || ENGINE_ARGS+=("option.Move Overhead=$OVERHEAD")
    ! engine_has_option Hash || ENGINE_ARGS+=("option.Hash=$HASH")
    ! engine_has_option Depth || ENGINE_ARGS+=("option.Depth=$MAX_DEPTH")
}

describe_engine() {
    local kind="$1"
    local value="$2"
    local rating="$3"
    local spec="$4"

    case "$kind" in
        git) echo "$value ($spec)" ;;
        stockfish) echo "Stockfish $STOCKFISH_VERSION at UCI_Elo $value (anchor at $rating)" ;;
        uci) echo "$value${rating:+ (anchor at $rating)}" ;;
    esac
}

build_fastchess() {
    FASTCHESS="$WORK_DIR/bin/fastchess-${FASTCHESS_COMMIT:0:12}"
    [ -x "$FASTCHESS" ] && return

    local src="$WORK_DIR/src/fastchess"
    echo "Building fastchess at ${FASTCHESS_COMMIT:0:12}..."
    rm -rf "$src"
    git clone --quiet --filter=blob:none "$FASTCHESS_REPO" "$src"
    git -C "$src" checkout --quiet "$FASTCHESS_COMMIT"
    make -C "$src" -j "$JOBS" > "$WORK_DIR/bin/fastchess-build.log" 2>&1 \
        || fail "the fastchess build failed; see $WORK_DIR/bin/fastchess-build.log"
    cp "$src/fastchess" "$FASTCHESS.tmp"
    mv "$FASTCHESS.tmp" "$FASTCHESS"
    rm -rf "$src"
}

# Ordo rates the engines against the anchors. It is built from a pinned
# source tarball, checked against its SHA-256.
build_ordo() {
    ORDO="$WORK_DIR/bin/ordo-${ORDO_COMMIT:0:12}"
    [ -x "$ORDO" ] && return

    local tarball="$WORK_DIR/src/ordo.tar.gz"
    local src="$WORK_DIR/src/ordo"
    local log="$WORK_DIR/bin/ordo-build.log"

    echo "Building Ordo at ${ORDO_COMMIT:0:12}..."
    curl -fsSL -o "$tarball" "$ORDO_URL"
    echo "$ORDO_SHA256  $tarball" | sha256sum --check --quiet \
        || fail "the downloaded Ordo source does not match its checksum"
    rm -rf "$src"
    mkdir -p "$src"
    tar -xzf "$tarball" -C "$src" --strip-components=1
    make -C "$src" -j "$JOBS" ordo > "$log" 2>&1 \
        || fail "the Ordo build failed; see $log"
    cp "$src/ordo" "$ORDO.tmp"
    mv "$ORDO.tmp" "$ORDO"
    rm -rf "$src" "$tarball"
}

fetch_book() {
    BOOK="$WORK_DIR/books/$BOOK_NAME"
    [ -f "$BOOK" ] && return

    local zip="$WORK_DIR/books/$BOOK_NAME.zip"
    local unpacked="$WORK_DIR/books/unpacked"
    echo "Downloading the opening book..."
    curl -fsSL -o "$zip" "$BOOK_URL"
    echo "$BOOK_SHA256  $zip" | sha256sum --check --quiet \
        || fail "the downloaded book does not match its checksum"
    rm -rf "$unpacked"
    unzip -q "$zip" -d "$unpacked"
    mv "$unpacked/$BOOK_NAME" "$BOOK"
    rm -rf "$unpacked"
}

# The CPUs this process may run on, one per line.
allowed_cpus() {
    local first last
    awk '/^Cpus_allowed_list:/ { print $2 }' /proc/self/status | tr ',' '\n' \
        | while IFS=- read -r first last; do
            seq "$first" "${last:-$first}"
        done
}

# One allowed logical CPU from each physical core, so that no two games
# share a core through its hyperthreads.
physical_cores() {
    local cpu siblings
    local -A seen=()
    for cpu in $(allowed_cpus); do
        siblings="$(cat "/sys/devices/system/cpu/cpu$cpu/topology/thread_siblings_list" 2> /dev/null)" \
            || continue
        [ -z "${seen[$siblings]:-}" ] || continue
        seen[$siblings]=1
        echo "$cpu"
    done
}

run_fastchess() {
    local interrupted=no

    INHIBIT=()
    if command -v systemd-inhibit > /dev/null \
        && systemd-inhibit --what=sleep:idle --mode=block true 2> /dev/null; then
        INHIBIT=(systemd-inhibit --what=sleep:idle --mode=block
                 "--who=wisdom-chess engine match" "--why=Engine match in progress")
    else
        echo "Warning: cannot keep the machine from sleeping; a suspend will lose games on time." >&2
    fi

    # Ctrl-C reaches every process in the pipeline. fastchess stops cleanly
    # and says how to resume, so the filter and tee ignore the interrupt and
    # pass that on, and this script goes on to tally the games so far.
    # fastchess saves its state as config.json in the current directory, so
    # it runs from the results.
    trap 'interrupted=yes' INT
    set +e
    (cd "$RUN_DIR" && exec "${INHIBIT[@]}" "$FASTCHESS" "$@") 2>&1 \
        | (trap '' INT; exec grep --line-buffered -E -v "$NOISE_PATTERN") \
        | (trap '' INT; exec tee -a "$RUN_DIR/fastchess.out")
    FASTCHESS_STATUS="${PIPESTATUS[0]}"
    set -e
    trap - INT

    [ "$interrupted" = no ] || FASTCHESS_STATUS=130

    if [ -f "$RUN_DIR/fastchess.log" ]; then
        grep -E -v "$NOISE_PATTERN" "$RUN_DIR/fastchess.log" > "$RUN_DIR/fastchess.log.tmp" || true
        mv "$RUN_DIR/fastchess.log.tmp" "$RUN_DIR/fastchess.log"
    fi
}

# Rate every engine against the anchors with Ordo, over the games the
# tally counted, so that a game replayed after a resume is not counted
# twice. Ordo's anchors file only takes effect with two or more anchors;
# a single one is given with -A and -a.
rate() {
    local i
    local anchor_args=()
    local anchors=0

    for i in "${!NAMES[@]}"; do
        [ -n "${RATINGS[$i]}" ] || continue
        anchors=$((anchors + 1))
        anchor_args=(-A "${NAMES[$i]}" -a "${RATINGS[$i]}")
        echo "\"${NAMES[$i]}\",${RATINGS[$i]}"
    done > "$RUN_DIR/anchors.csv"
    [ "$anchors" -eq 1 ] || anchor_args=(-m "$RUN_DIR/anchors.csv")

    echo ""
    echo "Ratings against the anchors (Ordo, $ORDO_SIMULATIONS simulations for the errors):"
    if ! "$ORDO" -p "$RUN_DIR/counted.pgn" "${anchor_args[@]}" \
        -s "$ORDO_SIMULATIONS" -D -W -o "$RUN_DIR/ratings.txt" > "$RUN_DIR/ordo.log" 2>&1; then
        tail -5 "$RUN_DIR/ordo.log" >&2
        echo "Error: Ordo failed to rate the games; the full output is $RUN_DIR/ordo.log" >&2
        return 1
    fi
    cat "$RUN_DIR/ratings.txt"
}

# Tally the games, and rate them when there is an engine to rate. A tally
# or rating that fails still prints how to resume an interrupted match,
# and then the script fails.
tally() {
    local status=0

    echo "" | tee -a "$RUN_DIR/summary.txt"
    if [ -s "$RUN_DIR/games.pgn" ]; then
        set +e
        {
            python3 "$SCRIPT_DIR/engine-match-tally.py" --counted "$RUN_DIR/counted.pgn" \
                "$RUN_DIR/games.pgn" "${NAMES[@]}" 2>&1 \
            && { [ "$TOURNAMENT_KIND" != gauntlet ] || rate; }
        } | tee -a "$RUN_DIR/summary.txt"
        status="${PIPESTATUS[0]}"
        set -e
    else
        echo "No games finished." | tee -a "$RUN_DIR/summary.txt"
    fi

    if [ "$FASTCHESS_STATUS" -ne 0 ]; then
        echo ""
        echo "The match did not finish. To continue it, run:"
        echo ""
        echo "  $0 --resume $RUN_DIR"
        exit "$FASTCHESS_STATUS"
    fi
    exit "$status"
}

# Build, fetch or check every engine, and collect fastchess's -engine
# arguments. Games between two anchors say nothing about the other engines'
# ratings, so with anchors the tournament is a gauntlet: the engines
# without a rating are its seeds, which fastchess takes first, and they
# play each other and every anchor.
SEEDS=()
ANCHORS=()
SEED_COUNT=0
ANCHOR_COUNT=0
COMMANDS=()
for i in "${!NAMES[@]}"; do
    prepare_engine "${KINDS[$i]}" "${VALUES[$i]}"
    COMMANDS+=("$ENGINE_COMMAND")
    if [ -z "${RATINGS[$i]}" ]; then
        SEEDS+=(-engine "cmd=$ENGINE_COMMAND" "name=${NAMES[$i]}" "${ENGINE_ARGS[@]}")
        SEED_COUNT=$((SEED_COUNT + 1))
    else
        ANCHORS+=(-engine "cmd=$ENGINE_COMMAND" "name=${NAMES[$i]}" "${ENGINE_ARGS[@]}")
        ANCHOR_COUNT=$((ANCHOR_COUNT + 1))
    fi
done
FASTCHESS_ENGINES=("${SEEDS[@]}" "${ANCHORS[@]}")
if [ "$ANCHOR_COUNT" -gt 0 ] && [ "$SEED_COUNT" -gt 0 ]; then
    TOURNAMENT_KIND=gauntlet
    TOURNAMENT=(-tournament gauntlet -seeds "$SEED_COUNT")
    TOURNAMENT_TEXT="gauntlet, $SEED_COUNT engine(s) against $ANCHOR_COUNT anchor(s)"
else
    TOURNAMENT_KIND=roundrobin
    TOURNAMENT=(-tournament roundrobin)
    TOURNAMENT_TEXT="round-robin"
    [ "$ANCHOR_COUNT" -eq 0 ] \
        || echo "Note: every engine is an anchor, so they play each other and no rating is computed."
fi

if [ "$BUILD_ONLY" = yes ]; then
    for i in "${!NAMES[@]}"; do
        echo "${NAMES[$i]}: ${COMMANDS[$i]}"
    done
    exit 0
fi

build_fastchess
[ "$TOURNAMENT_KIND" != gauntlet ] || build_ordo

if [ -n "$RESUME_DIR" ]; then
    # The saved config.json names the engines, which were checked above
    # since they may have gone or stopped answering since the interrupt.
    echo "Resumed: $(date '+%Y-%m-%d %H:%M:%S')" | tee -a "$RUN_DIR/summary.txt"
    echo "Results: $RUN_DIR"
    run_fastchess -config "file=$RUN_DIR/config.json"
    tally
    exit 0
fi

fetch_book

mapfile -t CORES < <(physical_cores)
AFFINITY=()
if [ "${#CORES[@]}" -eq 0 ]; then
    [ -n "$CONCURRENCY" ] || CONCURRENCY="$(nproc)"
    echo "Warning: cannot read the CPU topology, so games will not be pinned to cores." >&2
elif [ -z "$CONCURRENCY" ] || [ "$CONCURRENCY" -le "${#CORES[@]}" ]; then
    [ -n "$CONCURRENCY" ] || CONCURRENCY="${#CORES[@]}"
    AFFINITY=(-use-affinity "$(IFS=,; echo "${CORES[*]:0:$CONCURRENCY}")")
else
    echo "Warning: $CONCURRENCY games at a time is more than the ${#CORES[@]} physical" \
         "cores available, so games are not pinned and will share cores." >&2
fi

RUN_DIR="$WORK_DIR/results/$(date +%Y%m%d-%H%M%S)-$(IFS=-; echo "${NAMES[*]}")"
mkdir -p "$RUN_DIR"
for i in "${!NAMES[@]}"; do
    printf '%s\t%s\t%s\t%s\n' "${NAMES[$i]}" "${KINDS[$i]}" "${VALUES[$i]}" "${RATINGS[$i]}"
done > "$RUN_DIR/engines.txt"

{
    echo "Engines:"
    for i in "${!NAMES[@]}"; do
        echo "  ${NAMES[$i]}: $(describe_engine "${KINDS[$i]}" "${VALUES[$i]}" "${RATINGS[$i]}" "${ENGINE_SPECS[$i]#*=}")"
    done
    echo "Tournament: $TOURNAMENT_TEXT"
    echo "Time control: $TC, rounds: $ROUNDS, concurrency: $CONCURRENCY, seed: $SEED"
    echo "Move Overhead: $OVERHEAD ms, hash: $HASH MB, depth limit: $MAX_DEPTH, book: $BOOK_NAME"
    echo "fastchess: $FASTCHESS_COMMIT"
    [ "$TOURNAMENT_KIND" != gauntlet ] || echo "Ordo: $ORDO_COMMIT"
} | tee "$RUN_DIR/summary.txt"
echo "Results: $RUN_DIR"

run_fastchess \
    "${FASTCHESS_ENGINES[@]}" \
    -each "tc=$TC" \
    -openings "file=$BOOK" format=pgn order=random \
    -srand "$SEED" \
    -rounds "$ROUNDS" -repeat \
    -concurrency "$CONCURRENCY" "${AFFINITY[@]}" \
    "${TOURNAMENT[@]}" \
    -recover \
    -ratinginterval 50 \
    -pgnout "file=$RUN_DIR/games.pgn" \
    -log "file=$RUN_DIR/fastchess.log" level=warn

tally
