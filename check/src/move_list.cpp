
#include "move_list.hpp"

move_list_t::move_list_t(enum color color, std::initializer_list<const char*> list)
{
	for (auto it : list)
	{
		my_moves.push_back(parse_move(it, color));
		color = color_invert(color);
	}
}

std::string move_list_t::to_string() const
{
	std::string result;
	for (auto move : my_moves)
		result += ::to_string(move) + " ";
	return result;
}
