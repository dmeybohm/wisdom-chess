
#ifndef WIZDUMB_FEN_HPP
#define WIZDUMB_FEN_HPP

#include "global.h"
#include "board_builder.hpp"

#include <string>
#include <sstream>
#include <memory>

struct game;

class fen final
{
public:
    explicit fen(const std::string &input) : active_player { Color::White }
    {
        parse(input);
    }

    // Build the game:
    game build();

private:
    board_builder builder;
    Color active_player;

    void parse(const std::string &input);
    static piece_t parse_piece (char ch);

    void parse_pieces (std::string_view pieces_str);
    void parse_en_passant (std::string_view en_passant_str);
    void parse_castling (std::string_view castling_str);

    void parse_halfmove (int half_moves);
    void parse_fullmove (int full_moves);
    static Color parse_active_player (char ch);
};

class fen_exception : public std::exception
{
    const char *message;

public:
    explicit fen_exception (const char *message) : message { message }
    {}

    [[nodiscard]] const char *what () const noexcept override
    { return this->message; }
};

#endif //WIZDUMB_FEN_HPP
