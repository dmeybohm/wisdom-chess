#pragma once

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

    [[nodiscard]] constexpr auto
    updateDrawStatus (BothPlayersDrawStatus initial, Color player, DrawStatus new_status)
        -> BothPlayersDrawStatus
    {
        assert (player == Color::White || player == Color::Black);
        if (player == Color::White)
            return { new_status, initial.second };
        else
            return { initial.first, new_status };
    }

    [[nodiscard]] constexpr auto 
    drawStatusIsReplied (DrawStatus draw_status)
        -> bool
    {
        return draw_status == DrawStatus::Accepted || draw_status == DrawStatus::Declined;
    }

    [[nodiscard]] constexpr auto 
    bothPlayersReplied (BothPlayersDrawStatus both_players_status)
        -> bool
    {
        return drawStatusIsReplied (both_players_status.first)
            && drawStatusIsReplied (both_players_status.second);
    }

    class History
    {
    public:
        History()
        {
            my_board_codes.reserve (64);
        }

        static auto 
        fromInitialBoard (const Board& board)
            -> History
        {
            auto result = History {};
            result.my_board_codes.emplace_back (board.getBoardCode());
            result.my_stored_boards.emplace_back (board);
            return result;
        }

        [[nodiscard]] static auto
        hasBeenXHalfMovesWithoutProgress (const Board& board, int x_half_moves) -> bool
        {
            return board.getHalfMoveClock() >= x_half_moves;
        }

        [[nodiscard]] static auto 
        hasBeenSeventyFiveMovesWithoutProgress (const Board& board)
            -> bool
        {
            return hasBeenXHalfMovesWithoutProgress (board, 150);
        }

        [[nodiscard]] static auto 
        hasBeenFiftyMovesWithoutProgress (const Board& board)
            -> bool
        {
            return hasBeenXHalfMovesWithoutProgress (board, 100);
        }

        [[nodiscard]] bool isThirdRepetition (const Board& board) const;

        [[nodiscard]] bool isFifthRepetition (const Board& board) const;

        [[nodiscard]] bool isProbablyThirdRepetition (const Board& board) const;
        [[nodiscard]] bool isCertainlyThirdRepetition (const Board& board) const;
        [[nodiscard]] bool isProbablyFifthRepetition (const Board& board) const;
        [[nodiscard]] bool isCertainlyFifthRepetition (const Board& board) const;

        [[nodiscard]] auto 
        isProbablyNthRepetition (const Board& board, int repetition_count) const
            -> bool
        {
            auto code = board.getCode();
            auto count = std::count (my_board_codes.begin(), my_board_codes.end(), code);
            return count >= repetition_count;
        }

        [[nodiscard]] auto 
        isCertainlyNthRepetition (const Board& board, int repetition_count) const
            -> bool
        {
            auto repetitions = std::count (my_stored_boards.begin(), my_stored_boards.end(), board);
            return repetitions >= repetition_count;
        }

        void addTentativePosition (const Board& board)
        {
            my_board_codes.emplace_back (board.getBoardCode());
            my_tentative_nesting_count++;
        }

        void removeLastTentativePosition()
        {
            my_board_codes.pop_back();
            my_tentative_nesting_count--;
        }

        void addPosition (const Board& board, Move move)
        {
            Expects (my_tentative_nesting_count == 0);
            my_stored_boards.emplace_back (board);
            my_board_codes.emplace_back (board.getBoardCode());
            my_move_history.push_back (move);
        }

        void removeLastPosition()
        {
            Expects (my_tentative_nesting_count == 0);
            my_stored_boards.pop_back();
            my_board_codes.pop_back();
            my_move_history.pop_back();
        }

        [[nodiscard]] auto 
        getMoveHistory() const& 
            -> const vector<Move>&
        {
            return my_move_history;
        }
        void getMoveHistory() const&& = delete;

        [[nodiscard]] auto 
        getThreefoldRepetitionStatus() const 
            -> DrawStatus
        {
            return my_threefold_repetition_status;
        }

        void setThreefoldRepetitionStatus (DrawStatus status)
        {
            my_threefold_repetition_status = status;
        }

        [[nodiscard]] auto 
        getFiftyMovesWithoutProgressStatus() const
            -> DrawStatus
        {
            return my_fifty_moves_without_progress_status;
        }

        void setFiftyMovesWithoutProgressStatus (DrawStatus status)
        {
            my_fifty_moves_without_progress_status = status;
        }

        friend auto 
        operator<< (std::ostream& os, const History& code)
            -> std::ostream&;

    private:
        vector<BoardCode> my_board_codes {};
        vector<Board> my_stored_boards {};
        vector<Move> my_move_history {};
        int my_tentative_nesting_count = 0;

        DrawStatus my_threefold_repetition_status = DrawStatus::NotReached;
        DrawStatus my_fifty_moves_without_progress_status = DrawStatus::NotReached;
    };
}
