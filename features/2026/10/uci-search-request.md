# UCI: drop the game mutex and the search ID

## Problem

`UciInterface` had a `std::mutex` around `my_game`, an atomic search ID
counter and an atomic stop flag. The mutex suggested that the search
thread and the command loop shared the game, which they do not: the
search thread searches a copy, and the command loop only touches
`my_game` after `waitForSearchThread()` has joined the thread. Every
lock was taken on the main thread with no other thread alive to contend
it, so it protected nothing.

The search ID counter existed to tell a superseded search (cancelled,
silent) from a stopped one (finishes, reports `bestmove`). Because a new
search starts only after the previous thread is joined, no two searches
ever overlap, so there is never a question of which search a flag
belongs to.

## Plan

1. Remove `my_game_mutex` and its four `lock_guard`s. The join is the
   discipline that protects the game, as it already was for the
   transposition table; say so in the header.
2. Replace `my_search_id` and `my_stop_requested` with one atomic
   `SearchRequest { Run, Stop, Cancel }`. The periodic notifier maps
   `Cancel` to `MoveTimer::setCancelled()` and `Stop` to a zero time
   limit; the thread sends `bestmove` unless the request is `Cancel`.
   `handleStop()` sets `Stop`, `waitForSearchThread()` sets `Cancel`
   unless a stop is pending, and `handleGo()` resets to `Run` before
   spawning the thread.

   A `std::jthread` with its `stop_token` as the cancel signal was tried
   first. It needs `-fexperimental-library` on the Android NDK's libc++
   18, which gates `jthread` behind that flag, so the UCI target failed
   to build in the Android CI job. The enum needs no build change, and
   the token's advantages were slight: the request has to be reset
   before each search either way, and the explicit join in the
   destructor and `handleGo()` already covers what `jthread` would
   join on its own.

What stays: the thread itself, since "stop" has to be read from standard
input while a search runs; the output lock, since the search thread
writes "info" lines while the command loop may answer "isready".

## Implementation Progress

### Session #1

- Step 1 done. The `.cpp` gained its own `#include <mutex>` for the
  output lock, which it had taken from the header.
- Step 2 done, as the enum after the `jthread` attempt failed on
  Android. The 32 `UCI: ...` tests and the UCI unit tests pass in a
  Release build, and the `UCI: ...` tests pass with the binary built
  under ThreadSanitizer (Clang 18).
