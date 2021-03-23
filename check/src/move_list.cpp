#include "move_list.hpp"

MoveList::MoveList (Color color, std::initializer_list<const char*> list)
{
	for (auto it : list)
	{
		my_moves.push_back (parse_move(it, color));
		color = color_invert(color);
	}
}

std::string MoveList::to_string () const
{
	std::string result;
	for (auto move : my_moves)
		result += ::to_string(move) + " ";
	return result;
}

std::string to_string (const MoveList &list)
{
    std::string result = "{ ";
    for (auto move : list)
        result += to_string(move) + " ";
    result += " }";
    return result;
}
