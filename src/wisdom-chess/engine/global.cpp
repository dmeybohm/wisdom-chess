#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/logger.hpp"

#include <cstdlib>

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
        try
        {
            logEmergency (
                describeFailure ("Precondition", location) + " in " + location.function_name()
            );
        }
        catch (...)
        {
        }

        // Already reported, so skip the terminate handler's less specific message.
        std::abort();
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
