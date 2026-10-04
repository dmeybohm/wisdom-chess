#include "wisdom-chess/engine/move_list.hpp"

#include "fatal_test.hpp"

using namespace wisdom;

namespace
{
    FATAL_CASE(
        "append-overflow",
        "Precondition failed at .*move_list\\.hpp:[0-9]+: my_size < Max_Move_List_Size"
    )
    {
        MoveList list;
        Move move = moveParse ("e2 e4", Color::White);

        for (std::ptrdiff_t i = 0; i <= Max_Move_List_Size; i++)
            list.append (move);
    }

    FATAL_CASE( "remove-from-empty", "Precondition failed at .*move_list\\.hpp" )
    {
        MoveList list;
        list.removeLast();
    }

    FATAL_CASE( "front-of-empty", "Precondition failed at .*move_list\\.hpp" )
    {
        MoveList list;
        [[maybe_unused]] Move move = list.front();
    }

    FATAL_CASE( "back-of-empty", "Precondition failed at .*move_list\\.hpp" )
    {
        MoveList list;
        [[maybe_unused]] Move move = list.back();
    }
}
