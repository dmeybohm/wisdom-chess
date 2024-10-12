#pragma once

#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/move.hpp"

#include "wisdom-chess/wasm/web_types.hpp"

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
            auto color = mapColor (who);
            auto result = new WebMove (wisdom::moveParse (tmp, color) );
            return result;
        }

        [[nodiscard]] auto getMove() const -> Move
        {
            return my_move;
        }

        [[nodiscard]] auto asString() const -> char*
        {
            std::string str = wisdom::asString (my_move);
            return strdup (str.c_str());
        }
    };
}

