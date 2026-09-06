# UTC log timestamps

## Motivation

`formatLogTimestamp()` in `logger.cpp` produced a local-time prefix. Doing
that required `std::tm`, `strftime`, and a platform split between
`localtime_s` (Windows) and `localtime_r` (everything else). The question
was whether `std::chrono` alone could replace all of it.

## Investigation

The platform split exists only for the local time zone conversion. The
pure-chrono replacement is C++20's `std::chrono::current_zone()` and
`zoned_time`, which need the standard library's time zone database.
Test-compiling that against the toolchains this project builds with:

| Toolchain | `zoned_time` |
|---|---|
| GCC 13 / libstdc++ | works |
| Emscripten 4.0.7 / libc++ | fails: `_LIBCPP_HAS_NO_TIME_ZONE_DATABASE` is defined |
| Emscripten 3.1.70 (CI) | older, also unavailable |
| Apple Clang / libc++ (CI) | tz support marked unavailable on Apple platforms |

Both WebAssembly frontends would break, so local time via chrono is not an
option for this repository.

## Decision

Log timestamps are UTC time of day. `system_clock` counts from the Unix
epoch in UTC, so the time since epoch modulo 24 hours is the UTC clock
reading. That is plain duration arithmetic: no `std::tm`, no `strftime`,
no `#ifdef`, and it compiles under every toolchain above.

Trade-off: the prefix no longer matches the wall clock on the user's
machine. For an engine debug log that is acceptable, and it makes logs
from different machines directly comparable.

## Implementation Progress

### Session #1

- Rewrote `formatLogTimestamp()` using `duration_cast` on the time since
  epoch; dropped `<ctime>`.
- Updated the header comment to say UTC and why.
- Added deterministic tests, which UTC makes possible: a known offset from
  the epoch produces a known stamp, and the clock wraps at midnight.
