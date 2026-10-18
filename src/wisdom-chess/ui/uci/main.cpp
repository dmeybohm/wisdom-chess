#include "wisdom-chess/ui/uci/uci_interface.hpp"
#include "wisdom-chess/lib/logger.hpp"

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
