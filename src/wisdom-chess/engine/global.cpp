#include "wisdom-chess/engine/global.hpp"

#include <cstdio>

namespace wisdom
{
    namespace
    {
        auto
        describeFailure (const char* kind, const std::source_location& location)
            -> string
        {
            return string { kind } + " failed at " + location.file_name() + ":"
                + std::to_string (location.line());
        }
    }

    void
    throwPreconditionError (const std::source_location& location)
    {
        throw PreconditionError {
            describeFailure ("Precondition", location),
            location.function_name()
        };
    }

    void
    terminateOnPreconditionFailure (const std::source_location& location) noexcept
    {
        std::fprintf (
            stderr, "Precondition failed at %s:%u in %s\n",
            location.file_name(), static_cast<unsigned> (location.line()),
            location.function_name()
        );
        std::fflush (stderr);
        std::terminate();
    }

    void
    throwPostconditionError (const std::source_location& location)
    {
        throw PostconditionError {
            describeFailure ("Postcondition", location),
            location.function_name()
        };
    }
}
