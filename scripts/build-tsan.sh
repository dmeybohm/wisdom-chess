#!/bin/bash
# Build and run the test suite under ThreadSanitizer against a Qt that is
# itself instrumented, so that Qt's own locking is visible to TSan and the
# QML UI tests can be included.
#
# The Qt that install-qt-action and install-ci-qt.sh provide is not
# instrumented. QMutex locks through a raw futex(), which TSan cannot
# intercept, and Qt only tells TSan about those locks when Qt itself is
# built with -fsanitize=thread. Every hand-off Qt protects correctly then
# looks like a race. See features/2026/09/tsan-instrumented-deps.md.
#
# Usage: ./scripts/build-tsan.sh [options] [-- ctest arguments]
#
# Options:
#   --qt-version VERSION  Qt version to build (default: the newest release
#                         matching the version .github/workflows/cmake.yml
#                         asks for)
#   --cache-dir DIR       where sources, builds and prefixes go (default:
#                         $XDG_CACHE_HOME/wisdom-chess/qt-tsan)
#   --build-dir DIR       the application's build directory (default:
#                         build-tsan under the repository)
#   --no-download         always build Qt instead of looking for a prebuilt
#                         prefix on the release
#   --deps-only           stop once the Qt prefix is ready
#   --upload              pack the prefix and upload it to the release that
#                         holds the prebuilt ones (maintainers only)
#   -j N                  parallel jobs (default: nproc)
#   -h, --help            this message
#
# Everything after -- is passed to ctest.
#
# Nothing is installed system-wide and no root is needed. When a required
# tool or development package is missing the script prints the command to
# install it and stops; it never runs sudo itself.

set -e
set -o pipefail

# Bumped when the recipe changes in a way that invalidates existing
# prefixes without any of the hashed inputs below changing.
RECIPE_REVISION=1

# The release of this repository that holds the prebuilt prefixes.
RELEASE_TAG=tsan-deps

# qtdeclarative needs qtshadertools at build time; the rest is what the QML
# UI imports and links.
QT_MODULES=(qtbase qtshadertools qtdeclarative qtsvg)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
WORKFLOW="$REPO_DIR/.github/workflows/cmake.yml"
SUPPRESSIONS="$REPO_DIR/scripts/sanitizers/tsan.supp"

QT_VERSION="${WISDOM_CHESS_QT_VERSION:-}"
CACHE_DIR="${WISDOM_CHESS_TSAN_CACHE:-${XDG_CACHE_HOME:-$HOME/.cache}/wisdom-chess/qt-tsan}"
BUILD_DIR="$REPO_DIR/build-tsan"
DOWNLOAD=yes
DEPS_ONLY=no
UPLOAD=no
JOBS="$(nproc)"
CTEST_ARGS=()

fail() {
    echo "Error: $*" >&2
    exit 1
}

# Report a missing dependency the way this script always does: what is
# missing, the command that installs it, and nothing run on the user's
# behalf.
missing_package() {
    local what="$1"
    local command="$2"

    echo "Error: $what" >&2
    echo "Install it with:" >&2
    echo "" >&2
    echo "  $command" >&2
    exit 1
}

usage() {
    awk 'NR > 1 && /^#/ { sub(/^# ?/, ""); print; next } NR > 1 { exit }' "${BASH_SOURCE[0]}"
    exit "${1:-0}"
}

while [ $# -gt 0 ]; do
    case "$1" in
        --qt-version) QT_VERSION="$2"; shift 2 ;;
        --cache-dir) CACHE_DIR="$2"; shift 2 ;;
        --build-dir) BUILD_DIR="$2"; shift 2 ;;
        --no-download) DOWNLOAD=no; shift ;;
        --deps-only) DEPS_ONLY=yes; shift ;;
        --upload) UPLOAD=yes; shift ;;
        -j) JOBS="$2"; shift 2 ;;
        -j*) JOBS="${1#-j}"; shift ;;
        -h|--help) usage ;;
        --) shift; CTEST_ARGS=("$@"); break ;;
        *) echo "Unknown option: $1" >&2; usage 1 ;;
    esac
done

[ "$(uname -s)" = Linux ] \
    || fail "only Linux is supported; see the Out of scope section of" \
            "features/2026/09/tsan-instrumented-deps.md"

#
# Tools and development packages.
#

# The newest clang++ in PATH, or $CXX when it is set. TSan's runtime and
# the instrumented Qt must come from the same major version.
find_clang() {
    local candidate

    if [ -n "${CXX:-}" ]; then
        command -v "$CXX" > /dev/null || fail "CXX is set to $CXX, which is not in PATH"
        echo "$CXX"
        return
    fi

    for candidate in $(compgen -c 'clang++-' | sort -t- -k2 -rn -u) clang++; do
        if command -v "$candidate" > /dev/null; then
            echo "$candidate"
            return
        fi
    done

    missing_package "no clang++ was found in PATH." "sudo apt-get install clang-18"
}

check_tools() {
    local missing=()
    local tool

    for tool in cmake ninja curl zstd tar nm pkg-config python3; do
        command -v "$tool" > /dev/null || missing+=("$tool")
    done
    python3 -c 'import venv' 2> /dev/null || missing+=(python3-venv)

    [ ${#missing[@]} -eq 0 ] || missing_package \
        "missing tools: ${missing[*]}" \
        "sudo apt-get install cmake ninja-build curl zstd binutils pkg-config python3-venv"

    # Qt's fontconfig feature needs the system freetype next to it. Both
    # are used from the GUI thread only, so they stay uninstrumented.
    pkg-config --exists fontconfig freetype2 2> /dev/null || missing_package \
        "the fontconfig and freetype development files are missing." \
        "sudo apt-get install libfontconfig1-dev libfreetype-dev"
}

#
# Qt version, configure arguments and the cache key.
#

# The version CI asks install-qt-action for, such as 6.9.*.
ci_version_spec() {
    local matches
    matches="$(sed -n "s/^[[:space:]]*version:[[:space:]]*['\"]\\(.*\\)['\"][[:space:]]*$/\\1/p" "$WORKFLOW")"
    [ -n "$matches" ] || fail "no Qt version found in $WORKFLOW"
    echo "${matches%%$'\n'*}"
}

# aqtinstall, in its own virtual environment, is used for both the version
# lookup and the source download.
setup_aqt() {
    AQT="$CACHE_DIR/aqt-venv/bin/aqt"

    if [ ! -x "$AQT" ]; then
        echo "Creating the aqtinstall environment in $CACHE_DIR/aqt-venv"
        python3 -m venv "$CACHE_DIR/aqt-venv"
        "$CACHE_DIR/aqt-venv/bin/pip" install --quiet --upgrade pip
        "$CACHE_DIR/aqt-venv/bin/pip" install --quiet aqtinstall
    fi
}

resolve_version() {
    local spec

    [ -n "$QT_VERSION" ] && return 0

    spec="$(ci_version_spec)"
    echo "CI asks for Qt $spec; looking up the newest matching release"

    # install-qt-action resolves 6.9.* to the newest 6.9; --spec takes the
    # same range without the wildcard. aqt writes aqtinstall.log to the
    # current directory.
    QT_VERSION="$(cd "$CACHE_DIR" && "$AQT" list-qt linux desktop --spec "${spec%.\*}" --latest-version)"
    [ -n "$QT_VERSION" ] || fail "no Qt release matching $spec"
}

# Everything but -prefix, which holds the key this list is hashed into.
#
# -no-glib keeps Qt on its own event dispatcher rather than glib's, which
# is uninstrumented and wakes threads through an eventfd TSan reports on.
# The bundled zlib, libpng and the rest are built with Qt's flags, so they
# are instrumented too. D-Bus, ICU, OpenSSL and the SQL drivers are not
# used by the application. -no-opengl is enough because the QML tests run
# with QT_QPA_PLATFORM=offscreen and QT_QUICK_BACKEND=software.
qt_configure_args() {
    printf '%s\n' \
        -release \
        -force-debug-info \
        -sanitize thread \
        -nomake examples \
        -nomake tests \
        -no-glib \
        -no-dbus \
        -no-icu \
        -no-openssl \
        -no-opengl \
        -fontconfig \
        -system-freetype \
        -qt-zlib \
        -qt-libpng \
        -qt-libjpeg \
        -qt-pcre \
        -qt-harfbuzz \
        -qt-doubleconversion
}

# Line tables only: a report needs file and line, not a multi-gigabyte
# prefix full of full debug info.
qt_cmake_args() {
    printf '%s\n' \
        "-DCMAKE_C_FLAGS_RELWITHDEBINFO=-O2 -gline-tables-only -DNDEBUG" \
        "-DCMAKE_CXX_FLAGS_RELWITHDEBINFO=-O2 -gline-tables-only -DNDEBUG"
}

# Names the prefix and the release asset, so that changing any input
# builds a new Qt rather than reusing a stale one.
cache_key() {
    {
        echo "revision $RECIPE_REVISION"
        echo "qt $QT_VERSION"
        echo "modules ${QT_MODULES[*]}"
        echo "arch $(uname -m)"
        echo "clang $CLANG_MAJOR"
        qt_configure_args
        qt_cmake_args
    } | sha256sum | cut -c1-16
}

#
# Building Qt.
#

sources_present() {
    local module

    for module in "${QT_MODULES[@]}"; do
        [ -d "$SRC_DIR/$module" ] || return 1
    done
}

fetch_sources() {
    local module

    sources_present && return 0

    echo "=== Downloading the Qt $QT_VERSION sources ==="
    "$AQT" install-src linux desktop "$QT_VERSION" \
        --archives "${QT_MODULES[@]}" --outputdir "$CACHE_DIR/src"

    sources_present || fail "the sources are incomplete under $SRC_DIR"
}

# Qt's host tools (moc, qmlcachegen, ...) come out instrumented as well,
# and TSan exits 66 once it has reported anything, which would fail the
# build on a report inside a build tool.
quietly_instrumented() {
    TSAN_OPTIONS=report_bugs=0 "$@"
}

configure_qt_module() {
    local module="$1"
    local cmake_args=()

    mapfile -t cmake_args < <(qt_cmake_args)
    cmake_args+=("-DCMAKE_C_COMPILER=$CC" "-DCMAKE_CXX_COMPILER=$CXX")

    if [ "$module" != qtbase ]; then
        quietly_instrumented "$PREFIX/bin/qt-configure-module" "$SRC_DIR/$module" \
            -- "${cmake_args[@]}"
        return
    fi

    local configure_args=()
    mapfile -t configure_args < <(qt_configure_args)

    quietly_instrumented "$SRC_DIR/qtbase/configure" \
        -prefix "$PREFIX" \
        -cmake-generator Ninja \
        "${configure_args[@]}" \
        -- "${cmake_args[@]}"
}

build_qt_module() {
    local module="$1"
    local module_build="$CACHE_DIR/build-$KEY/$module"

    echo "=== Building $module ==="
    mkdir -p "$module_build"

    # Both configure and qt-configure-module work in the current directory.
    [ -f "$module_build/CMakeCache.txt" ] \
        || (cd "$module_build" && configure_qt_module "$module")

    quietly_instrumented cmake --build "$module_build" --parallel "$JOBS"
    quietly_instrumented cmake --install "$module_build"
}

# An instrumented object refers to TSan's entry points. Without them the
# prefix would link and run, and report nothing that goes through Qt.
verify_instrumented() {
    local library="$PREFIX/lib/$1"
    local symbols

    [ -e "$library" ] || fail "no $1 in $PREFIX/lib"

    # Read the symbols first rather than piping into grep: grep -q stops at
    # the first match, and the SIGPIPE that leaves nm with would fail the
    # pipeline under pipefail.
    symbols="$(nm -D --undefined-only "$library")"
    grep -q __tsan_func_entry <<< "$symbols" \
        || fail "$library is not ThreadSanitizer-instrumented; check that" \
                "configure accepted -sanitize thread"
}

record_build_host() {
    {
        echo "qt $QT_VERSION"
        echo "key $KEY"
        echo "clang $("$CXX" --version | head -1)"
        echo "host $(uname -srm)"
        echo "libc $(ldd --version | head -1)"
        echo "built $(date -u +%Y-%m-%dT%H:%M:%SZ)"
    } > "$PREFIX/BUILD-HOST"
}

build_qt() {
    local module

    fetch_sources

    for module in "${QT_MODULES[@]}"; do
        build_qt_module "$module"
    done

    verify_instrumented libQt6Core.so
    verify_instrumented libQt6Quick.so
    record_build_host
}

# BUILD-HOST is written last, so a prefix left behind by an interrupted
# build does not count as one. A prefix built against a newer glibc than
# the one here cannot load, which is what the smoke run catches.
prefix_is_usable() {
    [ -e "$PREFIX/BUILD-HOST" ] || return 1
    "$PREFIX/bin/qmake" -query QT_VERSION > /dev/null 2>&1
}

#
# Sharing the prefix through a release of this repository.
#

repo_slug() {
    local url
    url="$(git -C "$REPO_DIR" remote get-url origin 2> /dev/null || true)"
    url="${url#*github.com[:/]}"
    echo "${url%.git}"
}

download_qt() {
    local base tarball
    local slug

    slug="$(repo_slug)"
    [ -n "$slug" ] || return 1
    base="https://github.com/$slug/releases/download/$RELEASE_TAG"
    tarball="$CACHE_DIR/$ARCHIVE"

    echo "Looking for a prebuilt prefix at $base/$ARCHIVE"
    curl -fsSL -o "$tarball.sha256" "$base/$ARCHIVE.sha256" 2> /dev/null || return 1
    curl -fL --progress-bar -o "$tarball" "$base/$ARCHIVE" || return 1

    (cd "$CACHE_DIR" && sha256sum --check --status "$ARCHIVE.sha256") \
        || fail "$ARCHIVE does not match its checksum; remove $tarball and try again"

    echo "Extracting into $PREFIX"
    mkdir -p "$PREFIX"
    tar --use-compress-program=unzstd -xf "$tarball" -C "$PREFIX"
    rm -f "$tarball" "$tarball.sha256"

    if ! prefix_is_usable; then
        echo "The downloaded prefix does not run here; building instead" >&2
        rm -rf "$PREFIX"
        return 1
    fi
}

pack_qt() {
    local size

    echo "=== Packing $ARCHIVE ==="
    tar --use-compress-program='zstd -19 -T0' -cf "$CACHE_DIR/$ARCHIVE" -C "$PREFIX" .
    (cd "$CACHE_DIR" && sha256sum "$ARCHIVE" > "$ARCHIVE.sha256")

    size="$(stat -c %s "$CACHE_DIR/$ARCHIVE")"
    echo "$ARCHIVE is $((size / 1024 / 1024)) MiB"
    [ "$size" -lt $((2 * 1024 * 1024 * 1024)) ] \
        || fail "$ARCHIVE is over GitHub's 2 GB per-asset limit; drop the debug" \
                "info to -g0 or strip the QML tooling"
}

upload_qt() {
    local slug
    slug="$(repo_slug)"

    command -v gh > /dev/null || missing_package \
        "gh is not installed." "sudo apt-get install gh"

    if ! gh auth status > /dev/null 2>&1; then
        echo "Error: gh is not authenticated. Run:" >&2
        echo "" >&2
        echo "  gh auth login" >&2
        exit 1
    fi

    pack_qt

    gh release view "$RELEASE_TAG" --repo "$slug" > /dev/null 2>&1 \
        || gh release create "$RELEASE_TAG" --repo "$slug" \
            --title "ThreadSanitizer dependency builds" \
            --notes "Instrumented Qt prefixes for scripts/build-tsan.sh, named by the key the script computes."

    echo "=== Uploading to the $RELEASE_TAG release ==="
    gh release upload "$RELEASE_TAG" --repo "$slug" --clobber \
        "$CACHE_DIR/$ARCHIVE" "$CACHE_DIR/$ARCHIVE.sha256"
}

#
# The application.
#

build_app() {
    echo "=== Building the application against $PREFIX ==="
    cmake -S "$REPO_DIR" -B "$BUILD_DIR" -G Ninja \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCMAKE_C_COMPILER="$CC" \
        -DCMAKE_CXX_COMPILER="$CXX" \
        -DWISDOM_CHESS_QT_DIR="$PREFIX" \
        -DWISDOM_CHESS_QML_UI=ON \
        -DWISDOM_CHESS_TSAN=On \
        -DWISDOM_CHESS_FAST_TESTS=On \
        -DWISDOM_CHESS_BUILD_LINTER=Off
    cmake --build "$BUILD_DIR" --parallel "$JOBS"
}

run_tests() {
    echo "=== Running the tests under ThreadSanitizer ==="
    TSAN_OPTIONS="suppressions=$SUPPRESSIONS:halt_on_error=1:second_deadlock_stack=1" \
        ctest --test-dir "$BUILD_DIR" --output-on-failure -j 4 "${CTEST_ARGS[@]}"
}

#
# Main.
#

check_tools

CXX="$(find_clang)"
CC="${CXX/clang++/clang}"
command -v "$CC" > /dev/null || fail "found $CXX but no C compiler named $CC"

CLANG_MAJOR="$("$CXX" -dumpversion | cut -d. -f1)"
[ "$CLANG_MAJOR" -ge 18 ] 2> /dev/null \
    || fail "clang 18 or newer is needed; $CXX reports $CLANG_MAJOR"

mkdir -p "$CACHE_DIR"
setup_aqt
resolve_version

SRC_DIR="$CACHE_DIR/src/$QT_VERSION/Src"
KEY="$(cache_key)"
PREFIX="$CACHE_DIR/$KEY"
ARCHIVE="qt-tsan-$KEY.tar.zst"

echo "Qt $QT_VERSION, $CXX $CLANG_MAJOR, key $KEY"
echo "Prefix: $PREFIX"

if prefix_is_usable; then
    echo "Reusing the prefix already in the cache"
elif [ "$DOWNLOAD" = yes ] && download_qt; then
    echo "Using the prebuilt prefix from the $RELEASE_TAG release"
else
    build_qt
fi

if [ "$UPLOAD" = yes ]; then
    upload_qt
fi

if [ "$DEPS_ONLY" = yes ]; then
    echo ""
    echo "The Qt prefix is ready. Configure against it with:"
    echo ""
    echo "  cmake -S . -B build-tsan -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \\"
    echo "      -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX \\"
    echo "      -DWISDOM_CHESS_QT_DIR=$PREFIX \\"
    echo "      -DWISDOM_CHESS_QML_UI=ON -DWISDOM_CHESS_TSAN=On"
    echo ""
    exit 0
fi

build_app
run_tests
