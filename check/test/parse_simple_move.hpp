#ifndef WIZDUMB_PARSE_SIMPLE_MOVE_HPP
#define WIZDUMB_PARSE_SIMPLE_MOVE_HPP

#include "move.h"
#include "move_tree.h"

#include <vector>
#include <algorithm>
#include <iostream>

// Parse a move
move_t parse_move (const char *str, enum color color = COLOR_NONE);

class my_move_list
{
    move_list_t moves;

public:
    my_move_list (enum color color, std::initializer_list<const char *> list)
    {
        for (auto it : list)
        {
            moves.push_back(parse_move(it, color));
            color = color_invert(color);
        }
    }

    explicit my_move_list (const move_tree_t *tree)
    {
        while (tree != nullptr)
        {
            moves.push_back (tree->move);
            tree = tree->parent;
        }
    }

    friend bool operator==(const my_move_list &first, const my_move_list &second)
    {
        return first.moves.size() == second.moves.size() && std::equal (
                first.moves.begin(),
                first.moves.end(),
                second.moves.begin(),
                move_equals
        );
    }

    [[nodiscard]] std::string to_string() const
    {
        std::string result;
        for (auto move : moves)
            result += ::to_string(move) + " ";
        return result;
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
