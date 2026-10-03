#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/board_builder.hpp"

namespace wisdom
{
    class Game;

    class FenParser final
    {
    public:
        explicit FenParser (const string& input)
            : my_active_player { Color::White }
        {
            parse (input);
        }

        [[nodiscard]] auto
        getActivePlayer() const noexcept
            -> Color
        {
            return my_active_player;
        }

        // Build the game:
        [[nodiscard]] auto build() -> Game;

        [[nodiscard]] auto buildBoard() -> Board;

    private:
        BoardBuilder my_builder;
        Color my_active_player;

        void parse (const string& input);

        [[nodiscard]] static auto parsePiece (char ch) -> ColoredPiece;

        void parsePieces (string pieces_str);

        void parseEnPassant (string en_passant_str);

        void validateEnPassantTarget (Color vulnerable_color, Coord target);

        void parseCastling (string castling_str);

        void validateCastlingPieces (Color who, CastlingEligibility eligibility);

        void parseHalfMove (int half_moves);

        void parseFullMove (int full_moves);

        [[nodiscard]] static auto parseActivePlayer (char ch) -> Color;
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
