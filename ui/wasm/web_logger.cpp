#include <string>

#include "web_logger.hpp"

extern "C"
{
    EM_JS (void, consoleLog, (const char* str), { console.log (UTF8ToString (str)) })
};

void wisdom::worker::WebLogger::consoleLog (const char* message)
{
   ::consoleLog (message);
}

void wisdom::worker::WebLogger::debug (const std::string& output) const
{
   wisdom::worker::WebLogger::consoleLog (output.c_str());
}

void wisdom::worker::WebLogger::info (const std::string& output) const
{
    wisdom::worker::WebLogger::consoleLog (output.c_str());
}


