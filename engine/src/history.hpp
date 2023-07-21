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

    [[nodiscard]] constexpr auto updateDrawStatus (BothPlayersDrawStatus initial, Color player,
                                                   DrawStatus new_status) -> BothPlayersDrawStatus
    {
        assert (player == Color::White || player == Color::Black);
        if (player == Color::White)
            return { new_status, initial.second };
        else
            return { initial.first, new_status };
    }

    [[nodiscard]] constexpr auto drawStatusIsReplied (DrawStatus draw_status)
        -> bool
    {
        return draw_status == DrawStatus::Accepted || draw_status == DrawStatus::Declined;
    }

    [[nodiscard]] constexpr auto bothPlayersReplied (BothPlayersDrawStatus both_players_status)
        -> bool
    {
        return drawStatusIsReplied (both_players_status.first) && drawStatusIsReplied (both_players_status.second);
    }

    class History
    {
    public:
        History()
            : my_move_history { MoveList::uncached() }
        {
            my_board_codes.reserve (64);
            my_previous_boards.reserve (64);
        }

        [[nodiscard]] static auto hasBeenXHalfMovesWithoutProgress (const Board& board, int x)
            -> bool
        {
            return board.getHalfMoveClock () >= x;
        }

        [[nodiscard]] static auto hasBeenSeventyFiveMovesWithoutProgress (
            const Board& board) -> bool
        {
            return hasBeenXHalfMovesWithoutProgress (board, 150);
        }

        [[nodiscard]] static auto hasBeenFiftyMovesWithoutProgress (const Board& board)
            -> bool
        {
            return hasBeenXHalfMovesWithoutProgress (board, 100);
        }

        [[nodiscard]] bool isThirdRepetition (const Board& board) const;

        [[nodiscard]] bool isFifthRepetition (const Board& board) const;

        [[nodiscard]] bool isNthRepetition (const Board& board, int repetition_count) const
        {
            auto& find_code = board.getCode ();
            auto repetitions = std::count_if (my_board_codes.begin (), my_board_codes.end (),
                    [find_code](const BoardCode& code){
                        return (code == find_code);
                    });
            return repetitions >= repetition_count;
        }

        void addPositionAndMove (observer_ptr<Board> board, Move move)
        {
            my_board_codes.emplace_back (board->getCode ());
            my_previous_boards.emplace_back (board);
            my_move_history.pushBack (move);
        }

        void addPosition (observer_ptr<Board> board)
        {
            my_board_codes.emplace_back (board->getCode ());
            my_previous_boards.emplace_back (board);
        }

        void removeLastPosition()
        {
            my_board_codes.pop_back ();
            my_previous_boards.pop_back ();
            my_move_history.popBack();
        }

        [[nodiscard]] auto getMoveHistory() const& -> const MoveList&
        {
            return my_move_history;
        }
        void getMoveHistory() const&& = delete;

        [[nodiscard]] auto getThreefoldRepetitionStatus() const -> DrawStatus
        {
            return my_threefold_repetition_status;
        }

        void setThreefoldRepetitionStatus (DrawStatus status)
        {
            my_threefold_repetition_status = status;
        }

        [[nodiscard]] auto getFiftyMovesWithoutProgressStatus() const
            -> DrawStatus
        {
            return my_fifty_moves_without_progress_status;
        }

        void setFiftyMovesWithoutProgressStatus (DrawStatus status)
        {
            my_fifty_moves_without_progress_status = status;
        }

        friend auto operator<< (std::ostream& os, const History& code)
            -> std::ostream&;

    private:
        MoveList my_move_history;

        // Board codes and undo positions sorted by move number:
        vector<BoardCode> my_board_codes;
        vector<observer_ptr<Board>> my_previous_boards {};

        DrawStatus my_threefold_repetition_status = DrawStatus::NotReached;
        DrawStatus my_fifty_moves_without_progress_status
            = DrawStatus::NotReached;
    };
}

#endif //WISDOM_CHESS_HISTORY_HPP
