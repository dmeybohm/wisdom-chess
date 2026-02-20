# Refactor GameViewModelBase: CRTP to NVI (Non-Virtual Interface)

## Summary

Replaced the CRTP (Curiously Recurring Template Pattern) implementation of
`GameViewModelBase` with the NVI (Non-Virtual Interface) idiom for simpler,
more conventional code.

## Motivation

The CRTP refactoring from Session #1 successfully removed `game_id` from the
shared base and eliminated virtual dispatch. However, for UI code with only
6 callbacks and 2 derived classes, CRTP added template complexity (header-only
requirement, no `.hpp`/`.cpp` split) without meaningful performance benefit.
The NVI idiom gives proper encapsulation with a conventional class layout.

## Design

### NVI Structure

```
GameViewModelBase  (regular class, not a template)
    |-- GameModel : public QObject, public GameViewModelBase
    |-- WebGame : public GameViewModelBase
```

- **Public non-virtual** methods: `inCheck()`, `moveStatus()`, etc.
- **Protected pure virtual**: `getGame()` / `getGame() const`
- **Protected virtual no-ops**: `onInCheckChanged()`, etc.
- **Protected non-virtual**: setters, `updateDisplayedGameState()`, etc.
- **Protected constructor**: can only be constructed by derived classes
- **Virtual destructor**: for proper cleanup through base pointers
- `game_id` stays out of the base (kept from CRTP refactoring)

### Key differences from CRTP version

1. `GameViewModelBase` is a regular class, not a template
2. Method bodies moved from header to `.cpp` file
3. `StatusUpdateObserver` renamed to `ViewModelStatusUpdate`, moved to `.cpp`
4. Virtual dispatch through `getGame()` and `on*Changed()` callbacks
5. `friend Derived` replaced by standard inheritance + `override`

## Files Changed

| File | Change |
|------|--------|
| `viewmodel/game_viewmodel_base.hpp` | Template -> regular class with NVI |
| `viewmodel/game_viewmodel_base.cpp` | Re-created with method implementations |
| `viewmodel/CMakeLists.txt` | Added `.cpp` back to sources |
| `qml/main/game_model.hpp` | Base class without template param, `override` on virtuals |
| `wasm/web_game.hpp` | Base class without template param, `getGame()` moved to protected |

## Implementation Progress

### Session #1

- Converted CRTP template to regular NVI class
- All 85 fast tests pass
- Linter passes on all modified files
