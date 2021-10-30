#ifndef WISDOM_CHESS_GAME_H_
#define WISDOM_CHESS_GAME_H_

#include "global.hpp"
#include "piece.hpp"
#include "move.hpp"
#include "board.hpp"
#include "analytics.hpp"
#include "history.hpp"

namespace wisdom::analysis
{
    class Analytics;
}

namespace wisdom
{
    class Board;
    class History;
    class BoardBuilder;
    class Logger;
    class TranspositionTable;

    class Game
    {
    public:
        Game (Color current_turn, Color computer_player);

        Game (Color current_turn, Color computer_player, BoardBuilder builder);

        static auto load (const string &filename, Color player) -> optional<Game>;

        void save (const string &filename) const;

        auto find_best_move (Logger &logger, Color whom = Color::None) const -> optional<Move>;

        void move (Move move);

        [[nodiscard]] auto get_computer_player () const -> Color;

        void set_computer_player (Color player);

        [[nodiscard]] auto get_current_turn () const -> Color;

        void set_current_turn (Color);

        [[nodiscard]] auto get_board() const& -> Board&;
        void get_board() const&& = delete;

        [[nodiscard]] auto get_history () const& -> History&;
        void get_history () const&& = delete;

        [[nodiscard]] auto get_transposition_table () const& -> TranspositionTable&;
        void get_transposition_table () const&& = delete;

        void set_analytics (unique_ptr<analysis::Analytics> new_analytics);

        [[nodiscard]] bool is_computer_turn () const;

    private:
        unique_ptr<Board> my_board = make_unique<Board> ();
        unique_ptr<History> my_history = make_unique<History> ();
        unique_ptr<analysis::Analytics> my_analytics = make_unique<analysis::Analytics> ();
        unique_ptr<TranspositionTable> my_transposition_table = make_unique<TranspositionTable> ();

        Color my_current_turn;        // whose turn it is
        Color my_computer_player;     // side the computer is playing as
    };
}

#endif // WISDOM_CHESS_GAME_H_
