#include "uci_interface.hpp"
#include "wisdom-chess/engine/logger.hpp"

auto
main()
    -> int
{
    wisdom::setEmergencyLogger (wisdom::makeUciLogger (false));
    wisdom::installEmergencyTerminateHandler();

    wisdom::UciInterface uci;
    uci.run();
    return 0;
}
