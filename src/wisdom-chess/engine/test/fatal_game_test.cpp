#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/history.hpp"

#include "wisdom-chess/lib/test/fatal_test.hpp"

using namespace wisdom;

namespace
{
    FATAL_CASE(
        "game-external-draw-arbiter-without-repetitions",
        "Precondition failed at .*game\\.cpp:[0-9]+: draw_limits\\.repetitions > 0"
    )
    {
        auto game = Game::createStandardGame();
        game.setExternalDrawArbiter ({ .repetitions = 0, .half_moves_without_progress = 100 });
    }

    FATAL_CASE(
        "game-external-draw-arbiter-without-half-moves",
        "Precondition failed at .*game\\.cpp:[0-9]+: draw_limits\\.half_moves_without_progress > 0"
    )
    {
        auto game = Game::createStandardGame();
        game.setExternalDrawArbiter ({ .repetitions = 3, .half_moves_without_progress = 0 });
    }

    FATAL_CASE(
        "game-max-depth-zero",
        "Precondition failed at .*game\\.cpp:[0-9]+: max_depth > 0"
    )
    {
        auto game = Game::createStandardGame();
        int volatile depth = 0;
        game.setMaxDepth (depth);
    }

    FATAL_CASE(
        "game-search-timeout-zero",
        "Precondition failed at .*game\\.cpp:[0-9]+: timeout > chrono::milliseconds::zero\\(\\)"
    )
    {
        auto game = Game::createStandardGame();
        game.setSearchTimeout (chrono::milliseconds { 0 });
    }

    FATAL_CASE(
        "history-add-position-while-tentative",
        "Precondition failed at .*history\\.hpp:[0-9]+: my_tentative_nesting_count == 0"
    )
    {
        Board board = Board { BoardBuilder::fromDefaultPosition() };
        History history = History::fromInitialBoard (board);
        history.addTentativePosition (board);
        history.addPosition (board, toMove ("e2 e4", Color::White));
    }
}
