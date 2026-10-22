#include "wisdom-chess/lib/test/fatal_test.hpp"
#include "wisdom-chess/ui/wasm/web_game.hpp"

using namespace wisdom;

namespace
{
    FATAL_CASE(
        "web-computer-move-unparseable",
        R"re(Precondition failed at .*web_game\.cpp:[0-9]+: move\.has_value\(\))re"
    )
    {
        WebGame game { WebPlayer::ChessEngine, WebPlayer::Human, 1 };
        game.makeComputerMove ("not a move");
    }

    // Well formed, but a3 is empty.
    FATAL_CASE(
        "web-computer-move-illegal",
        R"re(Precondition failed at .*web_game\.cpp:[0-9]+: my_game\.isLegalMove)re"
    )
    {
        WebGame game { WebPlayer::ChessEngine, WebPlayer::Human, 1 };
        game.makeComputerMove ("a3 a4");
    }
}
