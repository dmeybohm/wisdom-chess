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
