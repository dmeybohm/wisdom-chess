#pragma once

#include "wisdom-chess/engine/global.hpp"

#include "wisdom-chess/engine/move_list.hpp"
#include "wisdom-chess/engine/piece.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/move.hpp"

namespace wisdom
{
    class History
    {
    public:
        History()
        {
            my_board_codes.reserve (64);
        }

        [[nodiscard]] static auto
        fromInitialBoard (const Board& board)
            -> History
        {
            auto result = History {};
            result.my_board_codes.emplace_back (board.getBoardCode());
            result.my_stored_boards.emplace_back (board);
            return result;
        }

        [[nodiscard]] static auto
        hasBeenXHalfMovesWithoutProgress (const Board& board, int x_half_moves) noexcept -> bool
        {
            return board.getHalfMoveClock() >= x_half_moves;
        }

        [[nodiscard]] static auto
        hasBeenSeventyFiveMovesWithoutProgress (const Board& board) noexcept
            -> bool
        {
            return hasBeenXHalfMovesWithoutProgress (board, 150);
        }

        [[nodiscard]] static auto
        hasBeenFiftyMovesWithoutProgress (const Board& board) noexcept
            -> bool
        {
            return hasBeenXHalfMovesWithoutProgress (board, 100);
        }

        [[nodiscard]] auto isThirdRepetition (const Board& board) const noexcept -> bool;

        [[nodiscard]] auto isFifthRepetition (const Board& board) const noexcept -> bool;

        [[nodiscard]] auto isProbablyThirdRepetition (const Board& board) const noexcept -> bool;
        [[nodiscard]] auto isCertainlyThirdRepetition (const Board& board) const noexcept -> bool;
        [[nodiscard]] auto isProbablyFifthRepetition (const Board& board) const noexcept -> bool;
        [[nodiscard]] auto isCertainlyFifthRepetition (const Board& board) const noexcept -> bool;

        [[nodiscard]] auto
        isProbablyNthRepetition (const Board& board, int repetition_count) const noexcept
            -> bool
        {
            // A position cannot recur across a capture or a pawn move.
            auto history_size = std::ssize (my_board_codes);
            auto clock = board.getHalfMoveClock();
            auto reversible_count = clock < history_size ? clock + 1 : history_size;
            if (reversible_count < repetition_count)
                return false;

            auto code = board.getBoardCode();
            auto count = std::count (my_board_codes.end() - reversible_count, my_board_codes.end(), code);
            return count >= repetition_count;
        }

        [[nodiscard]] auto
        isCertainlyNthRepetition (const Board& board, int repetition_count) const noexcept
            -> bool
        {
            auto repetitions = std::count (my_stored_boards.begin(), my_stored_boards.end(), board);
            return repetitions >= repetition_count;
        }

        // Make room for this many more tentative positions, so that adding
        // them does not allocate.
        void reserveTentativePositions (size_t count)
        {
            my_board_codes.reserve (my_board_codes.size() + count);
        }

        void addTentativePosition (const Board& board) noexcept
        {
            my_board_codes.emplace_back (board.getBoardCode());
            my_tentative_nesting_count++;
        }

        void removeLastTentativePosition() noexcept
        {
            my_board_codes.pop_back();
            my_tentative_nesting_count--;
        }

        void addPosition (const Board& board, Move move)
        {
            EXPECTS( my_tentative_nesting_count == 0 );
            my_stored_boards.emplace_back (board);
            my_board_codes.emplace_back (board.getBoardCode());
            my_move_history.push_back (move);
        }

        // Replace the position most recently added, which is the current
        // one, with a board that stands in its place, such as the same
        // board with the other side to move.
        void replaceLastPosition (const Board& board)
        {
            EXPECTS( my_tentative_nesting_count == 0 );
            my_stored_boards.back() = board;
            my_board_codes.back() = board.getBoardCode();
        }

        [[nodiscard]] auto
        getMoveHistory() const& noexcept
            -> const vector<Move>&
        {
            return my_move_history;
        }
        void getMoveHistory() const&& = delete;

        friend auto
        operator<< (std::ostream& os, const History& code)
            -> std::ostream&;

    private:
        vector<BoardCode> my_board_codes {};
        vector<Board> my_stored_boards {};
        vector<Move> my_move_history {};
        int my_tentative_nesting_count = 0;
    };
}
