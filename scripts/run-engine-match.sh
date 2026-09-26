#!/bin/bash
# Measure playing strength by playing versions of the engine against each
# other. Each NAME=REF builds the UCI engine at a git commit, branch or tag,
# and fastchess plays a round-robin between them from a book of openings,
# each opening once with each colour. See
# features/2026/09/engine-match-script.md.
#
# Usage: ./scripts/run-engine-match.sh [options] NAME=REF NAME=REF [NAME=REF...]
#
# Example: ./scripts/run-engine-match.sh base=main new=HEAD
#
# Options:
#   --tc TC             time control, as seconds+increment (default: 8+0.08)
#   --rounds N          openings per pairing, each played twice (default: 250)
#   --concurrency N     games at a time (default: the number of physical
#                       cores); each game is pinned to its own core
#   --overhead MS       the engines' Move Overhead (default: 30)
#   --hash MB           the engines' hash size (default: 16)
#   --seed N            seed for the order of the openings (default: 1)
#   --work-dir DIR      where builds, engines, fastchess, the book and the
#                       results go (default: $XDG_CACHE_HOME/wisdom-chess/match)
#   --build-only        build the engines and stop
#   -j N                parallel build jobs (default: nproc)
#   -h, --help          this message
#
# List the baseline first: the summary prints each pairing as the later
# engine against the earlier one.
#
# Engines are cached by commit, so a rerun only builds what changed. The
# machine is kept from sleeping while the match runs, but closing a laptop's
# lid may still suspend it.
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
    for tool in git cmake make c++ curl unzip sha256sum python3; do
        command -v "$tool" > /dev/null \
            || missing_package "$tool is required." \
                "sudo apt install git cmake build-essential curl unzip coreutils python3"
    done
}

while [ $# -gt 0 ]; do
    case "$1" in
        --tc) TC="$2"; shift 2 ;;
        --rounds) ROUNDS="$2"; shift 2 ;;
        --concurrency) CONCURRENCY="$2"; shift 2 ;;
        --overhead) OVERHEAD="$2"; shift 2 ;;
        --hash) HASH="$2"; shift 2 ;;
        --seed) SEED="$2"; shift 2 ;;
        --work-dir) WORK_DIR="$2"; shift 2 ;;
        --build-only) BUILD_ONLY=yes; shift ;;
        -j) JOBS="$2"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        -*) fail "unknown option $1 (see --help)" ;;
        *=*) ENGINE_SPECS+=("$1"); shift ;;
        *) fail "expected NAME=REF, got $1 (see --help)" ;;
    esac
done

[ "$(uname -s)" = Linux ] || fail "this script only runs on Linux"
[ "${#ENGINE_SPECS[@]}" -ge 2 ] || fail "give at least two engines as NAME=REF (see --help)"
require_tools

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

mkdir -p "$WORK_DIR"/{engines,src,build,bin,books,cpm,results}
WORK_DIR="$(cd "$WORK_DIR" && pwd)"

# Build the UCI engine from a clean export of the commit, so that local
# changes in the working tree never end up in a match.
build_engine() {
    local commit="$1"
    local engine="$WORK_DIR/engines/wisdom-chess-uci-$commit"

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
        rm -rf "$src"
    fi

    # Engines from before the millisecond timing spend at least a second on
    # every move and lose on time at short clocks.
    local options
    options="$(printf 'uci\nquit\n' | "$engine")"
    if ! grep -q "^option name Move Overhead" <<< "$options"; then
        fail "the engine at ${commit:0:12} has no Move Overhead option, so it predates" \
            "millisecond search timing and would lose on time; use a later commit"
    fi
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
    cp "$src/fastchess" "$FASTCHESS"
    rm -rf "$src"
}

fetch_book() {
    BOOK="$WORK_DIR/books/$BOOK_NAME"
    [ -f "$BOOK" ] && return

    local zip="$WORK_DIR/books/$BOOK_NAME.zip"
    echo "Downloading the opening book..."
    curl -fsSL -o "$zip" "$BOOK_URL"
    echo "$BOOK_SHA256  $zip" | sha256sum --check --quiet \
        || fail "the downloaded book does not match its checksum"
    unzip -o -q "$zip" -d "$WORK_DIR/books"
}

# One logical CPU from each physical core, so that no two games share a
# core through its hyperthreads.
physical_cores() {
    cat /sys/devices/system/cpu/cpu[0-9]*/topology/thread_siblings_list 2>/dev/null \
        | sed 's/[,-].*//' | sort -n -u
}

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
[ -n "$CONCURRENCY" ] || CONCURRENCY="${#CORES[@]}"
[ "$CONCURRENCY" -ge 1 ] || CONCURRENCY=1

AFFINITY=()
if [ "${#CORES[@]}" -ge "$CONCURRENCY" ]; then
    AFFINITY=(-use-affinity "$(IFS=,; echo "${CORES[*]:0:$CONCURRENCY}")")
else
    echo "Warning: $CONCURRENCY games at a time is more than the ${#CORES[@]} physical" \
         "cores, so games will share cores and the results will be noisier." >&2
fi

RUN_DIR="$WORK_DIR/results/$(date +%Y%m%d-%H%M%S)-$(IFS=-; echo "${NAMES[*]}")"
mkdir -p "$RUN_DIR"

ENGINE_ARGS=()
for i in "${!NAMES[@]}"; do
    ENGINE_ARGS+=(-engine "cmd=$WORK_DIR/engines/wisdom-chess-uci-${COMMITS[$i]}" "name=${NAMES[$i]}")
done

FASTCHESS_ARGS=(
    "${ENGINE_ARGS[@]}"
    -each "tc=$TC" "option.Move Overhead=$OVERHEAD" "option.Hash=$HASH" option.Depth=64
    -openings "file=$BOOK" format=pgn order=random
    -srand "$SEED"
    -rounds "$ROUNDS" -repeat
    -concurrency "$CONCURRENCY" "${AFFINITY[@]}"
    -tournament roundrobin
    -recover
    -ratinginterval 50
    -pgnout "file=$RUN_DIR/games.pgn"
    -log "file=$RUN_DIR/fastchess.log" level=warn
)

{
    echo "Engines:"
    for i in "${!NAMES[@]}"; do
        echo "  ${NAMES[$i]}: ${COMMITS[$i]} (${ENGINE_SPECS[$i]#*=})"
    done
    echo "Time control: $TC, rounds: $ROUNDS, concurrency: $CONCURRENCY, seed: $SEED"
    echo "Move Overhead: $OVERHEAD ms, hash: $HASH MB, book: $BOOK_NAME"
    echo "fastchess: $FASTCHESS_COMMIT"
} | tee "$RUN_DIR/summary.txt"
echo "Results: $RUN_DIR"

INHIBIT=()
if command -v systemd-inhibit > /dev/null \
    && systemd-inhibit --what=sleep:idle --mode=block true 2>/dev/null; then
    INHIBIT=(systemd-inhibit --what=sleep:idle --mode=block
             "--who=wisdom-chess engine match" "--why=Engine match in progress")
else
    echo "Warning: cannot keep the machine from sleeping; a suspend will lose games on time." >&2
fi

# fastchess saves its state as config.json in the current directory, so it
# runs from the results. The engines print no scores, which fastchess warns
# about for every move.
(cd "$RUN_DIR" && "${INHIBIT[@]}" "$FASTCHESS" "${FASTCHESS_ARGS[@]}") 2>&1 \
    | grep --line-buffered -v "No info line available" \
    | tee "$RUN_DIR/fastchess.out"

echo "" | tee -a "$RUN_DIR/summary.txt"
python3 "$SCRIPT_DIR/engine-match-tally.py" "$RUN_DIR/games.pgn" "${NAMES[@]}" \
    | tee -a "$RUN_DIR/summary.txt"
