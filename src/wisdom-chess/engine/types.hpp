#pragma once

#include <array>
#include <chrono>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// The standard library names used without std:: inside the namespace.
namespace wisdom
{
    using zstring = char*; // lint-allow(raw-pointer): defines the C string types
    using czstring = const char*; // lint-allow(raw-pointer): defines the C string types
    using std::array;
    using std::make_shared;
    using std::make_unique;
    using std::nullopt;
    using std::optional;
    using std::pair;
    using std::string;
    using std::unique_ptr;
    using std::shared_ptr;
    using std::vector;
    using std::string_view;
    using std::span;

    namespace chrono = std::chrono;
}
