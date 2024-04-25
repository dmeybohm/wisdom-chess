#pragma once

#include "board_builder.hpp"
#include "global.hpp"

namespace wisdom
{
    class Game;

    class FenParser final
    {
    public:
        explicit FenParser (const string& input)
            : active_player { Color::White }
        {
            parse (input);
        }

        [[nodiscard]] auto getActivePlayer() const -> Color
        {
            return active_player;
        }

        // Build the game:
        auto build() -> Game;

        auto buildBoard() -> Board;

    private:
        BoardBuilder builder;
        Color active_player;

        void parse (const string& input);

        static auto parsePiece (char ch) -> ColoredPiece;

        void parsePieces (string pieces_str);

        void parseEnPassant (string en_passant_str);

        void parseCastling (string castling_str);

        void parseHalfMove (int half_moves);

        void parseFullMove (int full_moves);

        static auto parseActivePlayer (char ch) -> Color;
    };

    class FenParserError : public Error
    {
    public:
        explicit FenParserError (const string& message)
            : Error (message)
        {
        }
    };
}
