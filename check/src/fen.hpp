
#ifndef WIZDUMB_FEN_HPP
#define WIZDUMB_FEN_HPP

#include "global.h"
//#include "board_builder.hpp"

#include <string>

struct board;

class fen
{
    std::string input;
//    board_builder builder;

public:
    fen(std::string input) { validate(input); }

    struct board *build();

    void validate(std::string input);
};

class fen_exception : public chess_exception
{};

#endif //WIZDUMB_FEN_HPP
