#pragma once

#include "wisdom-chess/engine/global.hpp"

#include "wisdom-chess/engine/move_list.hpp"
#include "wisdom-chess/engine/piece.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/move.hpp"

namespace wisdom
{
    enum class DrawStatus
    {
        NotReached = 0,
        Accepted,
        Declined
    };

    using BothPlayersDrawStatus = pair<DrawStatus, DrawStatus>;

    // The occurrences of a position, and the halfmoves without progress,
    // at which a position counts as a draw.
    struct DrawLimits
    {
        int repetitions;
        int half_moves_without_progress;

        [[nodiscard]] friend auto
        operator== (const DrawLimits&, const DrawLimits&)
            -> bool = default;
    };

    // The limits at which a player may claim a draw.
    inline constexpr DrawLimits Claimable_Draw_Limits {
        .repetitions = 3,
        .half_moves_without_progress = 100,
    };

    // The limits at which a game is drawn without a claim.
    inline constexpr DrawLimits Automatic_Draw_Limits {
        .repetitions = 5,
        .half_moves_without_progress = 150,
    };

    [[nodiscard]] constexpr auto
    updateDrawStatus (BothPlayersDrawStatus initial, Color player, DrawStatus new_status) noexcept
        -> BothPlayersDrawStatus
    {
        ASSERT( player == Color::White || player == Color::Black );
        if (player == Color::White)
            return { new_status, initial.second };
        else
            return { initial.first, new_status };
    }

    [[nodiscard]] constexpr auto
    drawStatusIsReplied (DrawStatus draw_status) noexcept
        -> bool
    {
        return draw_status == DrawStatus::Accepted || draw_status == DrawStatus::Declined;
    }

    [[nodiscard]] constexpr auto
    bothPlayersReplied (BothPlayersDrawStatus both_players_status) noexcept
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

        void addTentativePosition (const Board& board)
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

        [[nodiscard]] auto
        getThreefoldRepetitionStatus() const noexcept
            -> DrawStatus
        {
            return my_threefold_repetition_status;
        }

        void setThreefoldRepetitionStatus (DrawStatus status) noexcept
        {
            my_threefold_repetition_status = status;
        }

        [[nodiscard]] auto
        getFiftyMovesWithoutProgressStatus() const noexcept
            -> DrawStatus
        {
            return my_fifty_moves_without_progress_status;
        }

        void setFiftyMovesWithoutProgressStatus (DrawStatus status) noexcept
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
