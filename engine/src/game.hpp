#ifndef WISDOM_CHESS_GAME_H_
#define WISDOM_CHESS_GAME_H_

#include "global.hpp"
#include "piece.hpp"
#include "move.hpp"
#include "move_list.hpp"
#include "board.hpp"
#include "analytics.hpp"
#include "history.hpp"
#include "move_timer.hpp"

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

    enum class Player
    {
        Human,
        ChessEngine
    };

    using Players = array<Player, Num_Players>;

    class Game
    {
    public:
        Game ();

        explicit Game (const Players& players);

        Game (Player white_player, Player black_player);

        explicit Game (Color current_turn);

        explicit Game (const BoardBuilder& builder);

        // Delete copy
        Game (const Game& other) = delete;
        Game& operator= (const Game& other) = delete;

        // Default move:
        Game (Game&& other) = default;
        Game& operator= (Game&& other) = default;

        static auto load (const string& filename, const Players& players) -> optional<Game>;

        void save (const string& filename) const;

        [[nodiscard]] auto find_best_move (const Logger& logger, Color whom = Color::None) const
            -> optional<Move>;

        void move (Move move);

        [[nodiscard]] auto get_current_turn () const -> Color;

        void set_current_turn (Color);

        [[nodiscard]] auto get_board() const& -> Board&;
        auto get_board() const&& -> Board& = delete;

        [[nodiscard]] auto get_history () const& -> History&;
        auto get_history () const&& -> History& = delete;

        [[nodiscard]] auto get_move_generator () const& -> gsl::not_null<MoveGenerator*>;
        auto get_move_generator () const&& -> gsl::not_null<MoveGenerator*> = delete;

        auto get_current_player () -> Player
        {
            return get_player (my_board->get_current_turn ());
        }

        void set_white_player (Player player)
        {
            my_players[color_index(Color::White)] = player;
        }

        void set_black_player (Player player)
        {
            my_players[color_index(Color::Black)] = player;
        }

        auto get_player (Color color) -> Player
        {
            auto index = color_index (my_board->get_current_turn ());
            return my_players[index];
        }

        void set_players (const Players& players)
        {
            my_players = players;
        }

        [[nodiscard]] auto get_players () const -> Players
        {
            return my_players;
        }

        void set_max_depth (int max_depth)
        {
            my_max_depth = max_depth;
        }

        void set_search_timeout (std::chrono::seconds seconds)
        {
            my_search_timeout = seconds;
        }

        void set_analytics (unique_ptr<analysis::Analytics> new_analytics);

        [[nodiscard]] auto map_coordinates_to_move (Coord src, Coord dst, optional<Piece> promoted)
            -> optional<Move>
        {
            return ::wisdom::map_coordinates_to_move (*my_board, get_current_turn (),
                                                      src, dst, promoted);
        }

        void set_periodic_function (MoveTimer::PeriodicFunction periodic_function)
        {
            my_periodic_function = std::move (periodic_function);
        }

    private:
        unique_ptr<Board> my_board = make_unique<Board> ();
        unique_ptr<MoveGenerator> my_move_generator = make_unique<MoveGenerator> ();
        unique_ptr<History> my_history = make_unique<History> ();
        unique_ptr<analysis::Analytics> my_analytics = make_unique<analysis::Analytics> ();
        optional<MoveTimer::PeriodicFunction> my_periodic_function {};
        int my_max_depth { Max_Depth };

        Players my_players = { Player::Human, Player::ChessEngine };
        chrono::seconds my_search_timeout { Max_Search_Seconds };
    };
}

#endif // WISDOM_CHESS_GAME_H_
