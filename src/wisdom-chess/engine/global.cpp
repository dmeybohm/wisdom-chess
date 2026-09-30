#include <cstdlib>
#include <iostream>

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/logger.hpp"

namespace wisdom
{
    namespace
    {
        auto
        describeFailure (
            string_view kind,
            string_view expression,
            const std::source_location& location
        )
            -> string
        {
            return string { kind } + " failed at " + location.file_name() + ":"
                + std::to_string (location.line()) + ": " + string { expression };
        }
    }

    void
    throwPreconditionError (string_view expression, const std::source_location& location)
    {
        throw PreconditionError {
            describeFailure ("Precondition", expression, location),
            location.function_name()
        };
    }

    void
    throwPostconditionError (string_view expression, const std::source_location& location)
    {
        throw PostconditionError {
            describeFailure ("Postcondition", expression, location),
            location.function_name()
        };
    }

    void
    terminateOnCheckFailure (
        string_view kind,
        string_view expression,
        const std::source_location& location
    ) noexcept
    {
        try
        {
            logEmergency (
                describeFailure (kind, expression, location) + " in " + location.function_name()
            );
        }
        catch (...)
        {
            // Building the message failed, most likely for lack of memory.
            try
            {
                std::cerr << kind << " failed at " << location.file_name() << ':'
                          << location.line() << '\n';
            }
            catch (...)
            {
            }
        }

        // Already reported, so skip the terminate handler's less specific message.
        std::abort();
    }
}
