# Retry a failed test in the Android test job

## Motivation

The `android-tests` job ([android-qml-tests.md](android-qml-tests.md))
failed on its first run on `main`, after passing twice on PR #295 with
the same tree. The test that failed, "QML: wasm", had passed on the
emulator:

```
Totals: 5 passed, 0 failed, 0 skipped, 0 blacklisted, 2871ms
Error: failed to fetch logcat of the test
"Error: failed to retrieve the test result file stdout.txt."
```

The errors are from Qt's `androidtestrunner`, which after a test reads the
device's log and the test's output file through `adb`. That step failed,
so the runner reported a failure for a test that had none. A rerun of the
job passed. That is one failure in four runs of the same code.

Why the step failed is not known. The job's log has no `adb` or emulator
error near it, and the runner comes with Qt as a binary.

## Plan

Give a failed test a second try: `ctest --repeat until-pass:2` in
`scripts/android-tests.sh`. A test that fails twice still fails the job,
and the log keeps the first failure, so a test that needs its second try
often can be found.

The cost is that a test of ours that fails now and then can hide behind
the retry, on Android only. The desktop jobs run the same tests without
one.

The retry is in the script rather than in the workflow, so a local run
behaves like CI.

## Implementation Progress

### Session #1

Added the option. A scratch project confirmed what it does: a test that
fails and then passes is a pass with its failure still printed, and a test
that fails both times is a failure.

The script then ran locally against the emulator with Qt 6.9.3, and all
266 tests passed.
