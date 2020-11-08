#ifndef WIZDUMB_PARSE_SIMPLE_MOVE_HPP
#define WIZDUMB_PARSE_SIMPLE_MOVE_HPP

extern "C"
{
#include "move.h"
};

#include <vector>

class parse_simple_move_exception : public std::exception
{
    const char *message;

public:
    explicit parse_simple_move_exception (const char *message) : message { message } {}
    const char *what() const noexcept override { return this->message; }
};

// Simplified version of parse move that ignores the color argument of move_parse
// Throws parse_move_exception if the move requires a color (promotion/castling)
move_t parse_simple_move (const char *str);

#endif //WIZDUMB_PARSE_SIMPLE_MOVE_HPP
