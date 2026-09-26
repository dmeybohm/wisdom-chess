# Declarative QML enum registration

## Motivation

Finding 3 of [qml-cleanups.md](qml-cleanups.md). `ui/qml/main/ui_types.cpp`
registers the `wisdom::ui` namespace's meta-object by hand, four times
under four names, in `registerQmlTypes()`. `main()` and each UI test call
it before loading QML. Because the registration is not declared in the
sources, the module's generated `qmltypes` file does not list the enums,
so `qmllint` reports every `Color.White`, `Player.Human`, `PieceType.Queen`
and `DrawByRepetitionStatus.Proposed` as unqualified access (31 of the
105 that remain after `qml-cleanups`), and qmlcachegen cannot compile
bindings that use them.

## Plan

1. **Declare the registration in `ui_types.hpp`.** Qt's
   `QML_FOREIGN_NAMESPACE` lets a namespace with `Q_NAMESPACE` register
   another namespace's meta-object under its own name. Four small
   namespaces, one per QML name, next to the enums:

   ```cpp
   namespace wisdom::ui::qml_color
   {
       Q_NAMESPACE
       QML_FOREIGN_NAMESPACE (wisdom::ui)
       QML_NAMED_ELEMENT (Color)
   }
   ```

   and the same for `Player`, `PieceType` and `DrawByRepetitionStatus`.
   The QML names and the values do not change, so no QML file changes.
   `qt_add_qml_module` already runs qmltyperegistrar over the
   executable's sources, so the module's generated registration file
   picks them up and its `qmltypes` lists them.
2. **Delete `registerQmlTypes()`** and `ui_types.cpp` with it, and the
   call in `main.cpp`.
3. **Register the types in the UI tests.** The tests load QML from their
   own executables and today call `registerQmlTypes()` themselves. The
   module's generated `wisdomchessqml_qmltyperegistrations.cpp` is
   self-contained: it defines `qml_register_types_WisdomChess()` and a
   static `QQmlModuleRegistration` that the engine calls when the
   `WisdomChess` module is imported, which the embedded qmldir makes
   happen. Compile that generated file into each UI test executable, in
   `wisdom_chess_add_qml_ui_test()`, which already depends on the
   application target and already takes the generated qmldir files from
   the same directory. Remove the three `registerQmlTypes()` calls. The
   settings test does not load QML and needs nothing.
4. **Check.** The `qmltypes` file names the four types; `all_qmllint`
   no longer reports the enums (expect 74 unqualified left, all
   `topWindow`, `root` and the two context properties); `ctest` passes;
   `.qmllint.ini`'s comment is updated to name only the context
   properties.

Out of scope: the context properties (`qml-singletons`) and putting the
QML module into a library the tests could link, which would replace the
tests' re-embedding of the module's resources. That is a larger build
change and is noted in `qml-cleanups.md` as a possible follow-up.

## Implementation Progress

### Session #1

Done as planned, with one addition.

- `ui_types.hpp` ends with the four namespaces; `ui_types.cpp`,
  `registerQmlTypes()` and its four call sites are gone. The generated
  registration calls `qmlRegisterNamespaceAndRevisions` once per name
  with `wisdom::ui`'s meta-object as the foreign one, which is the
  hand-written registration expressed declaratively.
- **Include path.** qmltyperegistrar includes the header as
  `<ui_types.hpp>`, guarded by `__has_include`, so with `main/` off the
  include path the generated file compiled to a registration of nothing
  and the build then failed on the unknown namespace. The application
  target and the UI test executables now have `main/` on their include
  path, with a comment in each CMake file saying why.
- The UI tests compile the generated file as planned; they needed no
  other change beyond dropping `initTestCase()`.
- `qmllint`: 105 unqualified before, 89 after (the plan's estimate of 31
  enum accesses counted lines, not names). What is left: `topWindow` 43,
  `_myGameModel` 39, `root` 6, `_myPiecesModel` 1; nothing else. The module's `qmltypes` lists `Color`,
  `Player`, `PieceType` and `DrawByRepetitionStatus`.
- `.qmllint.ini` now names only the context properties as the reason for
  the info downgrade. `AGENTS.md` is unchanged: the comment above the
  namespaces in `ui_types.hpp` says where the QML names come from.
- Verified: full `ctest` passes; the QML UI tests pass under `Basic` and
  `Fusion`; the C++ `lint` target is clean. Not run on CI.
