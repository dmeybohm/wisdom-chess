# The models as QML singletons

## Motivation

Finding 6 of [qml-cleanups.md](qml-cleanups.md), the last item of that
plan. `main()` and the UI test fixture hand `GameModel` and `PiecesModel`
to QML as the context properties `_myGameModel` and `_myPiecesModel`.
Context properties are invisible to every tool: `qmllint` reports each
of the 36 uses as unqualified access, which is why
`src/wisdom-chess/ui/qml/.qmllint.ini` downgrades that category, and
qmlcachegen cannot compile a binding that touches them. Qt has
deprecated the pattern in favour of singletons since 6.0.

## Plan

1. **Declare both models as module singletons.** `QML_ELEMENT` and
   `QML_SINGLETON` on `GameModel` and `PiecesModel`, with a static
   `create (QQmlEngine*, QJSEngine*)` that returns the instance `main()`
   made, as Qt documents for exposing an existing object. The instance is
   stored by a static `setQmlInstance()` before the engine loads;
   `create()` marks it `CppOwnership` so the engine never deletes it. The
   UI tests create a new `Application`, models and engine per test, so
   `create()` checks only that an instance was set, not that the engine
   is the same one as before. The generated registration file, which the
   application and the UI tests already compile, registers them; nothing
   else changes in CMake.
2. **QML** uses `GameModel` and `PiecesModel` in place of the two
   context properties: 36 occurrences across 12 files, a rename.
3. **`main.cpp` and `application_fixture.hpp`** set the instances
   instead of the context properties.
4. **Lint.** Delete the `UnqualifiedAccess=info` override from
   `.qmllint.ini`, so any unqualified access fails the lint and CI;
   expect zero findings. Adjust the `AGENTS.md` lint bullet, which says
   the file downgrades a category meanwhile.
5. **Check.** `all_qmllint` reports nothing; `ctest` passes; the UI tests
   pass under `Basic` and `Fusion`; the C++ lint target is clean. Look
   at any new lint warning the typed singletons expose, for instance a
   comparison between the model's enum property and the QML enum.

## Risks

- A singleton is created lazily on first use from QML. Both are used by
  the first frame, and `create()` returns an object that already exists,
  so ordering does not change; the models are still connected to each
  other and started from `main()` as before.
- The engine must not outlive the models. It does not today: `main()`
  destroys the engine before the models, and the fixture's members are
  declared in the same order as before.

## Implementation Progress

### Session #1

Done, with one change to the plan's step 1 and two type fixes lint
asked for once it could see the models.

- **Wrappers instead of macros on the models.** With `QML_ELEMENT` and
  `QML_SINGLETON` on `GameModel` and `PiecesModel` themselves, the engine
  ignored `create()` and default-constructed its own instances:
  `qqmlprivate.h`'s `singletonConstructionMode()` prefers a default
  constructor over a factory whenever one exists, and both models have
  one. The UI tests showed it as a board with no pieces and engine
  replies that never came. `main/qml_singletons.hpp` now has
  `GameModelSingleton` and `PiecesModelSingleton`, `Q_GADGET` wrappers
  with `QML_FOREIGN`, `QML_NAMED_ELEMENT` and `QML_SINGLETON`, whose
  `create()` returns the instance set through `setInstance()`. A wrapper's
  factory takes precedence over the foreign type's constructor. The
  models are untouched apart from the property types below.
- **Property types.** With the singleton typed, lint could not resolve
  `UISettings` (Board.qml reads `GameModel.uiSettings.flipped`) or the
  view-model's `wisdom::ui::DrawByRepetitionStatus` on the two draw
  status properties. `UISettings` and `GameSettings` are `QML_ANONYMOUS`
  value types now, and the draw status properties are declared in the
  mirror enum `QmlDrawByRepetitionStatus`, through `qml*` getters and
  setters that cast; the view-model's enum has no meta-object, which is
  what the mirror is for.
- `main()` and the fixture call the wrappers' `setInstance()` before the
  engine loads; `QQmlContext` is no longer included anywhere.
- `.qmllint.ini` is down to `MaxWarnings=0`, so unqualified access fails
  the lint and CI; the `AGENTS.md` bullet no longer mentions a downgrade.
- `qmllint`: nothing reported at any level, from 129 warnings when the
  review started.
- Verified: full `ctest` passes; the four QML UI tests pass under `Basic`
  and `Fusion`; the C++ `lint` target is clean. Not run on CI.
