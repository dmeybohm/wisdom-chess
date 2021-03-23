
#ifndef WIZDUMB_FEN_HPP
#define WIZDUMB_FEN_HPP

#include "global.h"
#include "board_builder.hpp"

#include <string>
#include <sstream>
#include <memory>

struct Game;

class Fen final
{
public:
    explicit Fen (const std::string &input) : active_player {Color::White }
    {
        parse(input);
    }

    // Build the game:
    Game build ();

private:
    BoardBuilder builder;
    Color active_player;

    void parse(const std::string &input);
    static ColoredPiece parse_piece (char ch);

    void parse_pieces (std::string pieces_str);
    void parse_en_passant (std::string en_passant_str);
    void parse_castling (std::string castling_str);

    void parse_halfmove (int half_moves);
    void parse_fullmove (int full_moves);
    static Color parse_active_player (char ch);
};

class FenException : public std::exception
{
    const char *message;

public:
    explicit FenException (const char *message) : message {message }
    {}

    [[nodiscard]] const char *what () const noexcept override
    { return this->message; }
};

#endif //WIZDUMB_FEN_HPP
