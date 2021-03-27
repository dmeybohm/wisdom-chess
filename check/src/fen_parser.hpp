
#ifndef WISDOM_FEN_PARSER_HPP
#define WISDOM_FEN_PARSER_HPP

#include "global.hpp"
#include "board_builder.hpp"

#include <string>
#include <sstream>
#include <memory>

namespace wisdom
{
    struct Game;

    class FenParser final
    {
    public:
        explicit FenParser (const std::string &input) :
            active_player { Color::White }
        {
            parse (input);
        }

        // Build the game:
        Game build ();

    private:
        BoardBuilder builder;
        Color active_player;

        void parse (const std::string &input);

        static ColoredPiece parse_piece (char ch);

        void parse_pieces (std::string pieces_str);

        void parse_en_passant (std::string en_passant_str);

        void parse_castling (std::string castling_str);

        void parse_halfmove (int half_moves);

        void parse_fullmove (int full_moves);

        static Color parse_active_player (char ch);
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
