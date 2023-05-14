#ifndef WISDOM_CHESS_HISTORY_HPP
#define WISDOM_CHESS_HISTORY_HPP

#include "global.hpp"

#include "move_list.hpp"
#include "piece.hpp"
#include "board.hpp"
#include "move.hpp"

namespace wisdom
{
    enum class DrawStatus
    {
        NotReached = 0,
        Accepted,
        Declined
    };

    using BothPlayersDrawStatus = pair<DrawStatus, DrawStatus>;

    [[nodiscard]] constexpr auto update_draw_status (BothPlayersDrawStatus initial, Color player,
                                                     DrawStatus new_status) -> BothPlayersDrawStatus
    {
        assert (player == Color::White || player == Color::Black);
        if (player == Color::White)
            return { new_status, initial.second };
        else
            return { initial.first, new_status };
    }

    [[nodiscard]] constexpr auto draw_status_is_replied (DrawStatus draw_status)
        -> bool
    {
        return draw_status == DrawStatus::Accepted || draw_status == DrawStatus::Declined;
    }

    [[nodiscard]] constexpr auto both_players_replied (BothPlayersDrawStatus both_players_status)
        -> bool
    {
        return draw_status_is_replied (both_players_status.first) &&
               draw_status_is_replied (both_players_status.second);
    }

    class History
    {
    public:
        History()
            : my_move_history { MoveList::uncached() }
        {
            my_board_codes.reserve (64);
        }

        static auto from_initial_board (const Board& board)
        {
            auto result = History {};
            result.my_board_codes.emplace_back (board.get_board_code());
            result.my_stored_boards.emplace_back (board);
            return result;
        }

        [[nodiscard]] static auto has_been_n_half_moves_without_progress (const Board& board, int n)
            -> bool
        {
            return board.get_half_move_clock () >= n;
        }

        [[nodiscard]] static auto has_been_seventy_five_moves_without_progress (
            const Board& board) -> bool
        {
            return has_been_n_half_moves_without_progress (board, 150);
        }

        [[nodiscard]] static auto has_been_fifty_moves_without_progress (const Board& board)
            -> bool
        {
            return has_been_n_half_moves_without_progress (board, 100);
        }

        [[nodiscard]] bool is_third_repetition (const Board& board) const;

        [[nodiscard]] bool is_fifth_repetition (const Board& board) const;

        [[nodiscard]] bool is_nth_repetition (const Board& board, int repetition_count) const
        {
            const auto& find_code = board.get_code ();
            auto repetitions = std::count_if (my_board_codes.begin (), my_board_codes.end (),
                    [find_code](const BoardCode& code){
                        return (code == find_code);
                    });
            return repetitions >= repetition_count;
        }

        void add_tentative_position (const Board& board)
        {
            my_board_codes.emplace_back (board.get_board_code());
            my_tentative_nesting_count++;
        }

        void remove_last_tentative_position()
        {
            my_board_codes.pop_back ();
            my_tentative_nesting_count--;
        }

        void store_position (const Board& board, Move move)
        {
            Expects (my_tentative_nesting_count == 0);
            my_stored_boards.emplace_back (board);
            my_board_codes.emplace_back (board.get_board_code());
            my_move_history.push_back (move);
        }

        void pop_last_position ()
        {
            my_stored_boards.pop_back();
            my_board_codes.pop_back();
            my_move_history.pop_back();
        }

        [[nodiscard]] auto get_move_history() const& -> const MoveList&
        {
            return my_move_history;
        }
        void get_move_history() const&& = delete;

        [[nodiscard]] auto get_threefold_repetition_status() const -> DrawStatus
        {
            return my_threefold_repetition_status;
        }

        void set_threefold_repetition_status (DrawStatus status)
        {
            my_threefold_repetition_status = status;
        }

        [[nodiscard]] auto get_fifty_moves_without_progress_status() const
            -> DrawStatus
        {
            return my_fifty_moves_without_progress_status;
        }

        void set_fifty_moves_without_progress_status (DrawStatus status)
        {
            my_fifty_moves_without_progress_status = status;
        }

        friend auto operator<< (std::ostream& os, const History& code)
            -> std::ostream&;

    private:
        vector<BoardCode> my_board_codes {};
        vector<Board> my_stored_boards {};
        MoveList my_move_history;
        int my_tentative_nesting_count = 0;

        DrawStatus my_threefold_repetition_status = DrawStatus::NotReached;
        DrawStatus my_fifty_moves_without_progress_status
            = DrawStatus::NotReached;
    };
}

#endif //WISDOM_CHESS_HISTORY_HPP
