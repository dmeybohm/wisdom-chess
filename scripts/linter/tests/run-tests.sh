#!/bin/bash
# Test runner for wisdom-chess C++ style linter
# Usage: ./run-tests.sh <linter-command>
# Example: ./run-tests.sh "node ../dist/index.js"
# Example: ./run-tests.sh "../../../build/scripts/linter/wisdom-linter"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

LINTER="$1"
PASS=0
FAIL=0

if [ -z "$LINTER" ]; then
    echo "Usage: $0 <linter-command>"
    echo "Example: $0 'node ../dist/index.js'"
    exit 1
fi

for test_dir in */; do
    test_name="${test_dir%/}"
    # Skip non-test directories
    if [ "$test_name" = "fixtures" ]; then
        continue
    fi
    for input in "$test_dir"*.cpp; do
        [ -f "$input" ] || continue
        expected="${input%.cpp}.expected"
        if [ -f "$expected" ]; then
            actual=$($LINTER --format=simple "$input" 2>&1 || true)
            expected_content=$(cat "$expected")
            if [ "$actual" = "$expected_content" ]; then
                echo "PASS: $input"
                PASS=$((PASS + 1))
            else
                echo "FAIL: $input"
                echo "  Expected:"
                if [ -z "$expected_content" ]; then
                    echo "    (empty)"
                else
                    echo "$expected_content" | sed 's/^/    /'
                fi
                echo "  Actual:"
                if [ -z "$actual" ]; then
                    echo "    (empty)"
                else
                    echo "$actual" | sed 's/^/    /'
                fi
                FAIL=$((FAIL + 1))
            fi
        fi
    done
done

echo ""
echo "Results: $PASS passed, $FAIL failed"
if [ "$FAIL" -gt 0 ]; then
    exit 1
fi
exit 0
