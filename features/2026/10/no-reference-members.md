# No reference data members

Branch: `no-reference-members`, based on `main` after PR #378.

## Motivation

AGENTS.md said to prefer `nonnull<Type>` over a mutable `Type&` for
members and parameters alike, and to keep `const Type&` for const
references. That rule is right for parameters and wrong for members,
because the two have different lifetimes. A parameter's binding ends with
the call. A member's binding outlives the constructor that made it, and
the reference syntax hides that:

- A reference member binds a temporary silently.
  `InlineThreats { makeBoard(), who, coord }` compiles and dangles on the
  next line. A `nonnull<const Board>` member needs `&board` at the
  construction site, and a temporary has no address to take, so the same
  mistake does not compile. For a public-member struct built with
  aggregate-style initialization, as both engine cases are, this is the
  only guard there is.
- A reference member deletes copy and move assignment, so the type cannot
  be swapped, reset inside an `optional`, or kept in a container that
  assigns. It worked only because both structs were built once on the
  stack and never reused.
- The project already names a non-owning handle `nonnull` or `nullable`.
  A `const T&` member was a third spelling of the same thing with weaker
  guarantees, and one the linter's `raw-pointer` rule cannot see.

## Design

A reference may not outlive the expression or call that bound it. So:

- `const Type&` stays for parameters and locals.
- A data member that points at something it does not own is
  `nonnull<const Type>` or `nonnull<Type>`, never a reference.

The constructor that fills such a member takes the `nonnull` too, not a
reference, or the temporary guard would be lost at the one place it
matters.

The tree had three reference members:

| Member | Was | Now |
|---|---|---|
| `InlineThreats::board` (`engine/threats.hpp`) | `const Board&` | `nonnull<const Board>` |
| `MoveGeneration::board` (`engine/generate.cpp`) | `const Board&` | `nonnull<const Board>` |
| `CapturedStream::stream` (`ui/uci/test/uci_logger_test.cpp`) | `std::ostream&` | `nonnull<std::ostream>` |

`MoveGeneration` already held `nonnull<MoveList> moves` beside the
reference, so the struct is now uniform. The free functions the members
call (`validCastlingMove`, `isPawnUnmoved`, `eligibleEnPassantColumn`,
`materialDiff`) keep their `const Board&` parameter and take `*board`.
There is no `std::reference_wrapper` anywhere in the tree.

Codegen is the same: a reference member is a pointer in memory, and
`nonnull` checks for null only when constructed.

## Implementation Progress

### Session #1

- Converted the three members and their construction sites.
- Rewrote the pointer-versus-reference rule in AGENTS.md.
- Added the linter's `reference-member` rule
  (`scripts/linter/rules/reference_member.cpp`). It reports a `&` or `&&`
  that sits directly in a class, struct or union body between a type and
  a name that `;`, `=`, `{`, `,` or an attribute follows, so a reference
  that shares its declaration with other members (`int& first, second;`,
  `int plain, &last;`) or has an attribute after its name is caught too,
  as is the parenthesized declarator of a reference to an array,
  `int (&array)[3];`.
  A statement that contains parentheses, `=`, `->`, or `using`,
  `typedef`, `friend`, `operator` or `template` is not a data member and
  is skipped, so function declarations, trailing return types, alias
  declarations and default initializers do not trip it. Run over the
  tree as it was before this branch, it reported exactly the three
  members above and nothing else.

### Session #2

- Review found three declarator forms the rule missed: a name followed
  by a comma, a name followed by an attribute, and a parenthesized
  declarator. Extended the check to all three and added fixtures.

### Session #3

- Review found that an initializer on an earlier declarator hid a later
  reference, as in `int first = 0, &second;`, because the scan back to
  the statement's start gave up at `=`. The scan now steps over balanced
  brackets and initializers, and stops at `;`, an access specifier, or
  the closing brace of a body, which is told from a brace initializer's
  by its opening brace starting a line. A forward pass then finds where
  the type ends and requires the `&` to begin a declarator: the first
  one, or one after a comma at the statement's own bracket depth, so an
  `&` inside an initializer expression, `int mask = a & b, c;`, is not
  one. Fixtures cover both sides.

### Session #4

- Review found a false positive and a miss. A bit-field's width,
  `unsigned flags : Width & Mask;`, was read into the type, so its `&`
  looked like a declarator; a name that `:` follows now ends the type.
  `decltype (Width)& fifth;` was skipped as a function named `decltype`;
  the operand is now read as part of the type, and a `)` may precede the
  `&` of a declarator, since the forward pass already rejects an `&`
  that begins none.
