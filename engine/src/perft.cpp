#include "perft.hpp"
#include "board.hpp"
#include "move.hpp"
#include "check.hpp"
#include "piece.hpp"
#include "str.hpp"

using wisdom::Board;
using wisdom::Color;
using wisdom::perft::Stats;

void Stats::search_moves (Board &board, Color side, int depth, int max_depth)
{
    if (depth >= max_depth)
        return;

    const auto moves = generate_moves (board, side);

    for (auto move : moves)
    {
        UndoMove undo_state = board.make_move (side, move);

        if (!was_legal_move (board, side, move))
        {
            board.take_back (side, move, undo_state);
            continue;
        }

        counters.nodes++;
        if (is_any_capturing_move (move))
            counters.captures++;

        if (is_en_passant_move (move))
            counters.en_passants++;

        search_moves (board, color_invert (side), depth + 1, max_depth);

        board.take_back (side, move, undo_state);
    }
}

static auto convert_move (const wisdom::Board &board,
                          Color who, std::string move_str)
    -> wisdom::Move
{
    // 1. castling is represented by two space king moves

    // 2. en-passant is represented without (ep) suffix

    // 3. captures denoted without x

    // 4. Promotions are not in parenthesis

    // todo
    return {};
}

auto wisdom::perft::to_move_list (const wisdom::Board &board, Color who,
                                  const std::string &move_list) -> wisdom::MoveList
{
    MoveList result;

    // Make a copy of the board for modifications:
    auto board_copy = board;

    auto moves_str_list = wisdom::split (move_list, "\n");

    for (auto move_str : moves_str_list)
    {
        auto move = convert_move (board_copy, who, move_str);
        board_copy.make_move (who, move);
        result.push_back (move);
    }

    return result;
}