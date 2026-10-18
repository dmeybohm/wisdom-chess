#include "wisdom-chess/engine/move_list.hpp"

#include "wisdom-chess/lib/test/fatal_test.hpp"

using namespace wisdom;

namespace
{
    FATAL_CASE(
        "append-overflow",
        R"re(Precondition failed at .*move_list\.hpp:[0-9]+: my_size < Max_Move_List_Size)re"
    )
    {
        MoveList list;
        Move move = toMove ("e2 e4", Color::White);

        for (std::ptrdiff_t i = 0; i <= Max_Move_List_Size; i++)
            list.append (move);
    }

    FATAL_CASE(
        "ordered-append-overflow",
        R"re(Precondition failed at .*move_list\.hpp:[0-9]+: my_size < Max_Move_List_Size)re"
    )
    {
        auto list = std::make_unique<OrderedMoveList>();
        Move move = toMove ("e2 e4", Color::White);

        for (std::ptrdiff_t i = 0; i <= Max_Move_List_Size; i++)
            list->append (0, move);
    }

    FATAL_CASE( "remove-from-empty", R"re(Precondition failed at .*move_list\.hpp)re" )
    {
        MoveList list;
        list.removeLast();
    }

    FATAL_CASE( "front-of-empty", R"re(Precondition failed at .*move_list\.hpp)re" )
    {
        MoveList list;
        [[maybe_unused]] Move move = list.front();
    }

    FATAL_CASE( "back-of-empty", R"re(Precondition failed at .*move_list\.hpp)re" )
    {
        MoveList list;
        [[maybe_unused]] Move move = list.back();
    }
}
