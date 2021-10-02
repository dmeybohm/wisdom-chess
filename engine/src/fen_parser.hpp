#ifndef WISDOM_FEN_PARSER_HPP
#define WISDOM_FEN_PARSER_HPP

#include "global.hpp"
#include "board_builder.hpp"

namespace wisdom
{
    class Game;

    class FenParser final
    {
    public:
        explicit FenParser (const std::string &input) :
            active_player { Color::White }
        {
            parse (input);
        }

        [[nodiscard]] auto get_active_player () const -> Color
        {
            return active_player;
        }

        // Build the game:
        auto build () -> Game;

        auto build_board () -> Board;

    private:
        BoardBuilder builder;
        Color active_player;

        void parse (const std::string &input);

        static auto parse_piece (char ch) -> ColoredPiece;

        void parse_pieces (std::string pieces_str);

        void parse_en_passant (std::string en_passant_str);

        void parse_castling (std::string castling_str);

        void parse_half_move (int half_moves);

        void parse_full_move (int full_moves);

        static auto parse_active_player (char ch) -> Color;
    };

    class FenParserError : public Error
    {
    public:
        explicit FenParserError (const std::string &message) :
            Error (message)
        {}
    };
}

#endif //WISDOM_FEN_PARSER_HPP
