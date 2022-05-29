#ifndef WISDOM_CHESS_HISTORY_HPP
#define WISDOM_CHESS_HISTORY_HPP

#include "global.hpp"

#include "move_list.hpp"
#include "piece.hpp"
#include "board.hpp"
#include "move.hpp"

namespace wisdom
{
    enum class ThreeFoldRepetitionStatus
    {
        NOT_REACHED,
        BOTH_DECLINED,
        WHITE_DECLARED,
        BLACK_DECLARED,
        BOTH_DECLARED,
    };

    class History
    {
    private:
        MoveList my_move_history;

        // Board codes and undo positions sorted by move number:
        vector<BoardCode> my_board_codes;
        vector<UndoMove> my_undo_moves;

        ThreeFoldRepetitionStatus my_threefold_repetition_status = ThreeFoldRepetitionStatus::NOT_REACHED;

    public:
        History()
            : my_move_history { MoveList::uncached() }
        {
            my_board_codes.reserve (64);
            my_undo_moves.reserve (64);
        }

        static bool is_fifty_move_repetition (const Board& board)
        {
            return board.get_half_move_clock () >= 100;
        }

        [[nodiscard]] bool is_third_repetition (const Board& board) const;

        [[nodiscard]] bool is_fifth_repetition (const Board& board) const;

        [[nodiscard]] bool is_nth_repetition (const Board& board, int repetition_count) const
        {
            auto& find_code = board.get_code ();
            auto repetitions = std::count_if (my_board_codes.begin (), my_board_codes.end (),
                    [find_code](const BoardCode& code){
                        return (code == find_code);
                    });
            return repetitions >= repetition_count;
        }
        void add_position_and_move (const Board& board, Move move, const UndoMove& undo_state)
        {
            my_board_codes.push_back (board.get_code ());
            my_undo_moves.push_back (undo_state);
            my_move_history.push_back (move);
        }

        void remove_position_and_last_move (const Board& board)
        {
            my_board_codes.pop_back ();
            my_undo_moves.pop_back ();
            my_move_history.pop_back ();
        }

        [[nodiscard]] auto get_move_history () const& -> const MoveList&
        {
            return my_move_history;
        }
        void get_move_history () const&& = delete;

        [[nodiscard]] auto get_threefold_repetition_status () const -> ThreeFoldRepetitionStatus
        {
            return my_threefold_repetition_status;
        }

        void set_threefold_repetition_status (ThreeFoldRepetitionStatus status)
        {
            my_threefold_repetition_status = status;
        }
    };

}

#endif //WISDOM_CHESS_HISTORY_HPP
