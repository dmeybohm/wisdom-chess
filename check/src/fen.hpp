
#ifndef WIZDUMB_FEN_HPP
#define WIZDUMB_FEN_HPP

#include "global.h"
#include "board_builder.hpp"

#include <string>

struct game;

class fen final
{
public:
    explicit fen(const std::string &input) : active_player { COLOR_WHITE }
    {
        parse(input);
    }

    // Build the game:
    struct game *build();

private:
    board_builder builder;
    enum color active_player;

    void parse(const std::string &input);
    static piece_t parse_piece (char ch);
    std::string_view parse_en_passant (std::string_view str);
    std::string_view parse_castling (std::string_view str);
    std::string_view parse_halfmove (std::string_view str);
    std::string_view parse_fullmove (std::string_view str);
    static enum color parse_active_player (char ch);
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
