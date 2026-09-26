#!/bin/bash
# Measure playing strength by playing versions of the engine against each
# other. Each NAME=REF builds the UCI engine at a git commit, branch or tag,
# and fastchess plays a round-robin between them from a book of openings,
# each opening once with each colour. See
# features/2026/09/engine-match-script.md.
#
# Usage: ./scripts/run-engine-match.sh [options] NAME=REF NAME=REF [NAME=REF...]
#        ./scripts/run-engine-match.sh --resume RESULTS_DIR
#
# Example: ./scripts/run-engine-match.sh base=main new=HEAD
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
#   --build-only        build the engines and stop
#   --resume DIR        continue an interrupted match from its results
#                       directory; the other options are taken from it
#   -j N                parallel build jobs (default: nproc)
#   -h, --help          this message
#
# List the baseline first: the summary prints each pairing as the later
# engine against the earlier one. The engines' Depth option is set to its
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

MAX_DEPTH=64
NO_SCORE_WARNING="No info line available"

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
        *) fail "expected NAME=REF, got $1 (see --help)" ;;
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

if [ -n "$RESUME_DIR" ]; then
    [ "${#ENGINE_SPECS[@]}" -eq 0 ] || fail "--resume takes its engines from the results directory"
    [ -f "$RESUME_DIR/config.json" ] && [ -f "$RESUME_DIR/engines.txt" ] \
        || fail "$RESUME_DIR is not the results directory of a match that can be resumed"
    RUN_DIR="$(cd "$RESUME_DIR" && pwd)"
    WORK_DIR="$(cd "$RUN_DIR/../.." && pwd)"
    mapfile -t NAMES < "$RUN_DIR/engines.txt"
else
    [ "${#ENGINE_SPECS[@]}" -ge 2 ] || fail "give at least two engines as NAME=REF (see --help)"
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
    grep -q "^option name Move Overhead" <<< "$ENGINE_OPTIONS" \
        || fail "the engine at ${commit:0:12} has no Move Overhead option, so it predates" \
            "the search timer fixes of 3c48792 and would lose on time; use a later commit"
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
    # it runs from the results. The engines print no scores, which fastchess
    # warns about for every move.
    trap 'interrupted=yes' INT
    set +e
    (cd "$RUN_DIR" && exec "${INHIBIT[@]}" "$FASTCHESS" "$@") 2>&1 \
        | (trap '' INT; exec grep --line-buffered -v "$NO_SCORE_WARNING") \
        | (trap '' INT; exec tee -a "$RUN_DIR/fastchess.out")
    FASTCHESS_STATUS="${PIPESTATUS[0]}"
    set -e
    trap - INT

    [ "$interrupted" = no ] || FASTCHESS_STATUS=130

    if [ -f "$RUN_DIR/fastchess.log" ]; then
        grep -v "$NO_SCORE_WARNING" "$RUN_DIR/fastchess.log" > "$RUN_DIR/fastchess.log.tmp" || true
        mv "$RUN_DIR/fastchess.log.tmp" "$RUN_DIR/fastchess.log"
    fi
}

tally() {
    echo "" | tee -a "$RUN_DIR/summary.txt"
    if [ -s "$RUN_DIR/games.pgn" ]; then
        python3 "$SCRIPT_DIR/engine-match-tally.py" "$RUN_DIR/games.pgn" "${NAMES[@]}" 2>&1 \
            | tee -a "$RUN_DIR/summary.txt" || true
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
}

if [ -n "$RESUME_DIR" ]; then
    build_fastchess
    echo "Resumed: $(date '+%Y-%m-%d %H:%M:%S')" | tee -a "$RUN_DIR/summary.txt"
    echo "Results: $RUN_DIR"
    run_fastchess -config "file=$RUN_DIR/config.json"
    tally
    exit 0
fi

NAMES=()
COMMITS=()
for spec in "${ENGINE_SPECS[@]}"; do
    name="${spec%%=*}"
    ref="${spec#*=}"
    [[ "$name" =~ ^[A-Za-z0-9_-]+$ ]] || fail "engine names may only use letters, digits, - and _: $name"
    for existing in "${NAMES[@]}"; do
        [ "$existing" != "$name" ] || fail "engine name $name is given twice"
    done
    commit="$(git -C "$REPO_DIR" rev-parse --verify --quiet "$ref^{commit}")" \
        || fail "$ref is not a commit, branch or tag in $REPO_DIR"
    NAMES+=("$name")
    COMMITS+=("$commit")
done

for commit in "${COMMITS[@]}"; do
    build_engine "$commit"
done

if [ "$BUILD_ONLY" = yes ]; then
    for i in "${!NAMES[@]}"; do
        echo "${NAMES[$i]}: $WORK_DIR/engines/wisdom-chess-uci-${COMMITS[$i]}"
    done
    exit 0
fi

build_fastchess
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
printf '%s\n' "${NAMES[@]}" > "$RUN_DIR/engines.txt"

ENGINE_ARGS=()
for i in "${!NAMES[@]}"; do
    ENGINE_ARGS+=(-engine "cmd=$WORK_DIR/engines/wisdom-chess-uci-${COMMITS[$i]}" "name=${NAMES[$i]}")
done

{
    echo "Engines:"
    for i in "${!NAMES[@]}"; do
        echo "  ${NAMES[$i]}: ${COMMITS[$i]} (${ENGINE_SPECS[$i]#*=})"
    done
    echo "Time control: $TC, rounds: $ROUNDS, concurrency: $CONCURRENCY, seed: $SEED"
    echo "Move Overhead: $OVERHEAD ms, hash: $HASH MB, depth limit: $MAX_DEPTH, book: $BOOK_NAME"
    echo "fastchess: $FASTCHESS_COMMIT"
} | tee "$RUN_DIR/summary.txt"
echo "Results: $RUN_DIR"

run_fastchess \
    "${ENGINE_ARGS[@]}" \
    -each "tc=$TC" "option.Move Overhead=$OVERHEAD" "option.Hash=$HASH" "option.Depth=$MAX_DEPTH" \
    -openings "file=$BOOK" format=pgn order=random \
    -srand "$SEED" \
    -rounds "$ROUNDS" -repeat \
    -concurrency "$CONCURRENCY" "${AFFINITY[@]}" \
    -tournament roundrobin \
    -recover \
    -ratinginterval 50 \
    -pgnout "file=$RUN_DIR/games.pgn" \
    -log "file=$RUN_DIR/fastchess.log" level=warn

tally
