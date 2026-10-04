#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/expected.hpp"

namespace wisdom
{
    class Game;

    class FenParser final
    {
    public:
        // A FEN string that does not parse is a precondition failure. Text
        // from outside the program goes through parse().
        explicit FenParser (const string& input);

        [[nodiscard]] static auto
        parse (const string& input)
            -> expected<FenParser, ParseError>;

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
        using Result = expected<void, ParseError>;

        BoardBuilder my_builder;
        Color my_active_player = Color::White;

        FenParser() = default;

        [[nodiscard]] auto parseFields (const string& input) -> Result;

        [[nodiscard]] static auto parsePiece (char ch) -> expected<ColoredPiece, ParseError>;

        [[nodiscard]] auto parsePieces (string pieces_str) -> Result;

        [[nodiscard]] auto parseEnPassant (string en_passant_str) -> Result;

        [[nodiscard]] auto validateEnPassantTarget (Color vulnerable_color, Coord target) -> Result;

        [[nodiscard]] auto parseCastling (string castling_str) -> Result;

        [[nodiscard]] auto validateCastlingPieces (Color who, CastlingEligibility eligibility) -> Result;

        [[nodiscard]] auto parseHalfMove (int half_moves) -> Result;

        [[nodiscard]] auto parseFullMove (int full_moves) -> Result;

        [[nodiscard]] static auto parseActivePlayer (char ch) -> expected<Color, ParseError>;
    };
}
