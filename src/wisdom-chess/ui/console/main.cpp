#include <cstdlib>

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/logger.hpp"

namespace wisdom::ui::console
{
    void play();
};

int main()
{
    wisdom::setEmergencyLogger (wisdom::makeStandardLogger());
    wisdom::installEmergencyTerminateHandler();

    try
    {
        wisdom::ui::console::play();
    }
    catch (const wisdom::Error& e)
    {
        wisdom::logEmergency ("Uncaught error: " + e.message() + "\n" + e.extra_info());
        std::abort();
    }

    return 0;
}
