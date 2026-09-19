#!/bin/bash
# TEMPORARY: diagnostics for the QML UI tests that fail on the macOS and
# Windows CI jobs but not on Linux. Remove with the workflow steps that run
# it once the failures are understood. See features/2026/09/qml-tests.md.
#
# Run from the build directory. Writes everything to qml-diagnostics/ and
# prints a summary. Always exits 0: it only reports.
#
#   1. Each test under the platform's default style, then Fusion and Basic.
#      The failing platforms default to the native-look macOS and Windows
#      styles; Linux defaults to Fusion.
#   2. Each test function in a process of its own, to see whether a
#      failure needs an earlier test to have run first.
#   3. On macOS, the full test under lldb, for a backtrace of the crash.

set -u

out=qml-diagnostics
mkdir -p "$out"

export QT_QPA_PLATFORM=offscreen
export QT_QUICK_BACKEND=software

find_test() {
    find src/wisdom-chess/ui/qml/test -type f \
        \( -name "wisdom-chess-qml-$1-test" -o -name "wisdom-chess-qml-$1-test.exe" \) \
        | head -n 1
}

# Prints the lines that say what happened, from a Qt Test log: each failure
# with the lines that follow it, and the totals.
summarize() {
    tr -d '\r' < "$1" \
        | grep -E -A3 "^(FAIL!|XPASS|QFATAL)|Received signal|^Totals" \
        | grep -v "^INFO " \
        | cut -c1-220
}

for name in dialogs mobile; do
    exe=$(find_test "$name")
    if [ -z "$exe" ]; then
        echo "::warning::No executable found for the QML $name test"
        continue
    fi

    echo "::group::QML $name: styles"
    for style in default Fusion Basic; do
        if [ "$style" = default ]; then
            unset QT_QUICK_CONTROLS_STYLE
        else
            export QT_QUICK_CONTROLS_STYLE=$style
        fi

        log="$out/$name-style-$style.txt"
        "$exe" -v2 -o "$log,txt" > "$out/$name-style-$style.stdout.txt" 2>&1
        status=$?
        echo "--- $name, style $style: exit $status"
        summarize "$log"
    done
    unset QT_QUICK_CONTROLS_STYLE
    echo "::endgroup::"

    echo "::group::QML $name: one process per test function"
    for function in $("$exe" -functions | tr -d '\r'); do
        function=${function%()}
        log="$out/$name-function-$function.txt"
        "$exe" "$function" -o "$log,txt" > "$log.stdout.txt" 2>&1
        status=$?
        totals=$(grep -h "^Totals" "$log" | tr -d '\r')
        echo "$function: exit $status ${totals:-(no totals: the process did not finish)}"
        if [ "$status" -ne 0 ]; then
            summarize "$log"
        fi
    done
    echo "::endgroup::"

    if [ "$(uname)" = Darwin ]; then
        echo "::group::QML $name: backtrace under lldb"
        lldb --batch -o run -k "thread backtrace all" -k quit -- "$exe" -nocrashhandler \
            > "$out/$name-lldb.txt" 2>&1
        grep -E "stop reason|^\s*\* thread|^\s*frame #" "$out/$name-lldb.txt" | head -n 60
        echo "::endgroup::"
    fi
done

exit 0
