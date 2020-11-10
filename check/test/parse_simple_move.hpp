#ifndef WIZDUMB_PARSE_SIMPLE_MOVE_HPP
#define WIZDUMB_PARSE_SIMPLE_MOVE_HPP

extern "C"
{
#include "move.h"
#include "move_tree.h"
};

#include <vector>
#include <algorithm>
#include <iostream>

// Parse a move
move_t parse_move (const char *str, enum color color = COLOR_NONE);

class my_move_list
{
    std::vector<move_t> moves;

public:
    my_move_list (enum color color, std::initializer_list<const char *> list)
    {
        for (auto it : list)
        {
            moves.push_back(parse_move(it, color));
            color = color_invert(color);
        }
    }

    explicit my_move_list (move_tree_t *tree)
    {
        while (tree != nullptr)
        {
            moves.push_back (tree->move);
            tree = tree->parent;
        }
    }

    friend bool operator==(const my_move_list &first, const my_move_list &second)
    {
        return std::equal (
                first.moves.begin(),
                first.moves.end(),
                second.moves.begin(),
                move_equals
        );
    }
};

class parse_move_exception : public std::exception
{
    const char *message;

public:
    explicit parse_move_exception (const char *message) : message { message } {}
    [[nodiscard]] const char *what() const noexcept override { return this->message; }
};

#endif //WIZDUMB_PARSE_SIMPLE_MOVE_HPP
