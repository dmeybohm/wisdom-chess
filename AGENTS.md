# Wisdom Chess C++ Style Linter

The project includes a custom C++ style linter located in `scripts/linter/`. This linter enforces the project's idiosyncratic C++ coding style.

## Running the Linter

```bash
# Build the linter (part of the normal build)
cmake --build build --target wisdom-linter

# Run on a specific file
./build/scripts/linter/wisdom-linter <cppfile>

# Run on the entire source tree
cmake --build build --target lint
```

## Available Rules

### namespace-braces

Ensures namespace opening braces are on their own line.

**Bad:**
```cpp
namespace wisdom {
    // code
}
```

**Good:**
```cpp
namespace wisdom
{
    // code
}
```

### trailing-return-type

Functions should use trailing return type syntax with `auto`.

**Bad:**
```cpp
int getValue();
std::string getName();
```

**Good:**
```cpp
auto getValue() -> int;
auto getName() -> std::string;
```

### test-macro-spacing

Test macros (CHECK, REQUIRE, TEST_CASE, etc.) should have spaces inside parentheses.

**Bad:**
```cpp
CHECK(value == 5);
REQUIRE(result);
```

**Good:**
```cpp
CHECK( value == 5 );
REQUIRE( result );
```

### function-call-spacing

Functions with arguments need a space before the opening parenthesis. Zero-argument functions should not have a space.

**Bad:**
```cpp
getValue ();    // no args, shouldn't have space
doSomething(x); // has args, needs space
```

**Good:**
```cpp
getValue();      // no args, no space
doSomething( x ); // has args, has space
```

## Adding New Rules

Rules are defined in `scripts/linter/rules/`. To add a new rule:

1. Create a new file in `scripts/linter/rules/` (e.g., `my_new_rule.cpp`)
2. Include `"../linter.hpp"` for the `Rule` base class
3. Define your rule class in an anonymous namespace
4. Implement the factory function that returns `std::shared_ptr<Rule>`
5. Add the factory function declaration and call in `rules/init.cpp`
6. Add the source file to `CMakeLists.txt`

Example rule file structure:

```cpp
#include "../linter.hpp"

namespace wisdom_linter
{
namespace
{
    class MyNewRule : public Rule
    {
    public:
        [[nodiscard]] auto name() const -> std::string_view override
        {
            return "my-new-rule";
        }

        [[nodiscard]] auto description() const -> std::string_view override
        {
            return "Description of what the rule checks";
        }

        [[nodiscard]] auto check( const LintContext& context ) const
            -> std::vector<LintViolation> override
        {
            std::vector<LintViolation> violations;
            // Check logic here
            return violations;
        }
    };
}

auto createMyNewRule() -> std::shared_ptr<Rule>
{
    return std::make_shared<MyNewRule>();
}

} // namespace wisdom_linter
```

Then in `rules/init.cpp`, add:

```cpp
extern auto createMyNewRule() -> std::shared_ptr<Rule>;
```

And add it to the `registerAllRules()` function.

## Configuration

The linter reads configuration from `.wisdomstylerc.json` if present. Rules can be set to `"error"`, `"warning"`, or `"off"`.

```json
{
  "rules": {
    "namespace-braces": "error",
    "trailing-return-type": "warning",
    "test-macro-spacing": "error",
    "function-call-spacing": "error"
  }
}
```

## Running Tests

The linter has a test suite located in `scripts/linter/tests/`:

```bash
./scripts/linter/tests/run-tests.sh build/scripts/linter/wisdom-linter
```

## Project Structure

```
scripts/linter/
├── CMakeLists.txt
├── main.cpp              # Entry point
├── linter.hpp            # Core types and interfaces
├── linter.cpp            # Linter class and formatting
├── rules/
│   ├── init.cpp                  # Rule registration
│   ├── namespace_braces.cpp      # namespace-braces rule
│   ├── trailing_return_type.cpp  # trailing-return-type rule
│   ├── test_macro_spacing.cpp    # test-macro-spacing rule
│   └── function_call_spacing.cpp # function-call-spacing rule
└── tests/
    ├── run-tests.sh      # Test runner script
    └── *.txt             # Test case files
```
