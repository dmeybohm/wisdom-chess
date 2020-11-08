#include "catch.hpp"

extern "C"
{
#include "move_list.h"
}

TEST_CASE( "Copying move lists works", "[move-list]" )
{
    move_list_t *dst = nullptr;
    move_list_t *src = nullptr;

    const move_t e2e4 = move_parse ("e2e4", COLOR_WHITE);

    dst = move_list_append_move (dst, e2e4);
    dst = move_list_append_move (dst, move_parse ("a2a4", COLOR_WHITE));

    src = move_list_append_move (src, move_parse ("h7h8 (Q)", COLOR_WHITE));
    src = move_list_append_move (src, move_parse ("o-o-o", COLOR_WHITE));
    const move_t d7d5 = move_parse ("d7d5", COLOR_BLACK);

    src = move_list_append_move (src, d7d5);
    dst = move_list_append_list (dst, src);
    
    CHECK( dst->len == 5 );
    
    CHECK(move_equals (dst->move_array[0], e2e4) != 0);
    CHECK(move_equals (dst->move_array[4], d7d5) != 0);
}
