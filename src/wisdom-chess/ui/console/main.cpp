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

    wisdom::ui::console::play();

    return 0;
}
