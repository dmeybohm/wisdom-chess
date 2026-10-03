# Non-const Logger methods

## Motivation

Logging can change the logger's state. `BufferedLogger` appends to a ring
buffer when logging is disabled, and test loggers record messages. Marking
the logging methods `const` required these members to be `mutable`, which
hid those state changes in the interface.

## Design

Make `Logger::debug()`, `info()`, and `emergency()` non-const, including
their overrides. Keep their `noexcept` contract. Remove `mutable` from
state that is written by those methods. Helpers that accept a logger for
output use `nonnull<Logger>` as the project's non-owning mutable pointer
type.

## Implementation Progress

### Session #1

Created branch and worktree `logger-nonconst-methods` from `main`.
Updated the engine and frontend loggers, the two helper parameters, and
the logger test fixtures. The Release build and C++ style linter pass;
all 282 configured tests pass. Qt 6 was not available to this build, so
the QML frontend and its tests were disabled by CMake.
