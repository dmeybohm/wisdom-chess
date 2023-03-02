#ifndef WISDOMCHESS_WEB_MOVE_HPP
#define WISDOMCHESS_WEB_MOVE_HPP

#include "game.hpp"
#include "move.hpp"

namespace wisdom
{
    class WebGame;

    class WebMove
    {
    private:
        Move my_move;

    public:
        explicit WebMove (Move move) : my_move { move }
        {}

        static auto fromString (char* string, int who) -> WebMove*
        {
            std::string tmp { string };
            auto color = map_color (who);
            auto result = new WebMove ( wisdom::move_parse (tmp, color) );
            return result;
        }

        [[nodiscard]] auto get_move() const -> Move
        {
            return my_move;
        }

        [[nodiscard]] auto asString() const -> char*
        {
            std::string str = to_string (my_move);
            return strdup (str.c_str());
        }
    };
}

#endif // WISDOMCHESS_WEB_MOVE_HPP
