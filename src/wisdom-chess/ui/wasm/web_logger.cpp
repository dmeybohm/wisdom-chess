#include <string>

#include "wisdom-chess/ui/wasm/web_logger.hpp"

extern "C"
{
    EM_JS (void, consoleLog, (const char* str), { console.log (UTF8ToString (str)) }) // lint-allow(raw-pointer): EM_JS signature
    EM_JS (void, consoleError, (const char* str), { console.error (UTF8ToString (str)) }) // lint-allow(raw-pointer): EM_JS signature
    EM_JS (void, consoleErrorBytes, (const char* str, int length), { console.error (UTF8ToString (str, length)) }) // lint-allow(raw-pointer): EM_JS signature
};

void wisdom::worker::WebLogger::consoleLog (czstring message)
{
   ::consoleLog (message);
}

void wisdom::worker::WebLogger::consoleError (czstring message)
{
    ::consoleError (message);
}

void wisdom::worker::WebLogger::debug (const std::string& output) noexcept
{
   wisdom::worker::WebLogger::consoleLog (output.c_str());
}

void wisdom::worker::WebLogger::info (const std::string& output) noexcept
{
    wisdom::worker::WebLogger::consoleLog (output.c_str());
}

void wisdom::worker::WebLogger::emergency (std::string_view output) noexcept
{
    ::consoleErrorBytes (output.data(), wisdom::narrow_cast<int> (output.size()));
}
