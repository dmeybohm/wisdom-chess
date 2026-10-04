#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/history.hpp"
#include "wisdom-chess/engine/move_timer.hpp"
#include "wisdom-chess/engine/search.hpp"
#include "wisdom-chess/engine/transposition_table.hpp"

#include <stdexcept>

#include "fatal_test.hpp"

using namespace wisdom;

namespace
{
    FATAL_CASE( "uncaught-exception", "Uncaught exception: boom", test::Reports_Uncaught_Errors )
    {
        throw std::runtime_error { "boom" };
    }

    // The periodic function runs inside the noexcept search, so a throw
    // from it ends the process. Depth 4 reaches the first periodic call.
    FATAL_CASE(
        "periodic-function-throws",
        "Uncaught exception: boom",
        test::Reports_Uncaught_Errors
    )
    {
        Board board { BoardBuilder::fromDefaultPosition() };
        History history;
        MoveTimer timer { 30 };
        timer.setPeriodicFunction ([] (nonnull<MoveTimer>)
        {
            throw std::runtime_error { "boom" };
        });
        auto transposition_table = TranspositionTable::fromMegabytes (1);
        auto search = IterativeSearch::create (
            board,
            history,
            std::make_shared<test::MarkedLogger>(),
            timer,
            4,
            &transposition_table,
            Claimable_Draw_Limits
        );
        (void)search.iterativelyDeepen (Color::White);
    }
}
