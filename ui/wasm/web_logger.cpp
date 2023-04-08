#include "web_logger.hpp"

#include <string>

extern "C"
{
    EM_JS (void, console_log, (const char* str), { console.log (UTF8ToString (str)) })
};

void wisdom::worker::WebLogger::console_log (const char* message)
{
   ::console_log (message);
}

void wisdom::worker::WebLogger::debug (const std::string& output) const
{
    wisdom::worker::WebLogger::console_log (output.c_str());
}

void wisdom::worker::WebLogger::info (const std::string& output) const
{
    wisdom::worker::WebLogger::console_log (output.c_str());
}


