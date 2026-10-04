#include <algorithm>
#include <array>
#include <charconv>
#include <cstdlib>

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/logger.hpp"

namespace wisdom
{
    namespace
    {
        // A message built in place, without the heap. Text that does not
        // fit is dropped.
        class FixedMessage
        {
        public:
            void
            append (string_view text) noexcept
            {
                auto count = std::min (text.size(), my_buffer.size() - my_size);
                std::copy_n (text.data(), count, my_buffer.data() + my_size);
                my_size += count;
            }

            void
            append (unsigned number) noexcept
            {
                std::array<char, 16> digits {};
                auto [end, error] = std::to_chars (digits.data(), digits.data() + digits.size(), number);
                append (string_view { digits.data(), end });
            }

            [[nodiscard]] auto
            view() const noexcept
                -> string_view
            {
                return { my_buffer.data(), my_size };
            }

        private:
            std::array<char, 1024> my_buffer {};
            size_t my_size = 0;
        };
    }

    void
    terminateOnCheckFailure (
        string_view kind,
        string_view expression,
        std::source_location location
    ) noexcept
    {
        FixedMessage message;
        message.append (kind);
        message.append (" failed at ");
        message.append (location.file_name());
        message.append (":");
        message.append (location.line());
        message.append (": ");
        message.append (expression);
        message.append (" in ");
        message.append (location.function_name());
        logEmergency (message.view());

        // Already reported, so skip the terminate handler's less specific message.
        std::abort();
    }
}
