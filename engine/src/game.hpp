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
    struct PeriodicNotified;

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

        Game (Color current_turn, const BoardBuilder& builder);

        static auto load (const string& filename, const Players& players) -> optional<Game>;

        void save (const string& filename) const;

        [[nodiscard]] auto find_best_move (const Logger& logger, Color whom = Color::None) const
            -> optional<Move>;

        void move (Move move);

        [[nodiscard]] auto get_current_turn () const -> Color;

        void set_current_turn (Color);

        [[nodiscard]] auto get_board() const& -> Board&;
        void get_board() const&& = delete;

        [[nodiscard]] auto get_history () const& -> History&;
        void get_history () const&& = delete;

        auto get_current_player () -> Player
        {
            auto index = color_index (my_current_turn);
            return my_players[index];
        }

        void set_white_player (Player player)
        {
            my_players[color_index(Color::White)] = player;
        }

        void set_black_player (Player player)
        {
            my_players[color_index(Color::Black)] = player;
        }

        void set_players (const array<Player, Num_Players>& players)
        {
            my_players = players;
        }

        [[nodiscard]] auto get_players () const -> array<Player, Num_Players>
        {
            return my_players;
        }

        void set_analytics (unique_ptr<analysis::Analytics> new_analytics);

        [[nodiscard]] auto map_coordinates_to_move (Coord src, Coord dst, optional<Piece> promoted)
            -> optional<Move>
        {
            return ::wisdom::map_coordinates_to_move (*my_board, my_current_turn,
                                                      src, dst, promoted);
        }

        void set_periodic_notified (PeriodicNotified* notified)
        {
            my_periodic_notified = notified;
        }

    private:
        unique_ptr<Board> my_board = make_unique<Board> ();
        unique_ptr<History> my_history = make_unique<History> ();
        unique_ptr<analysis::Analytics> my_analytics = make_unique<analysis::Analytics> ();
        PeriodicNotified* my_periodic_notified = nullptr;

        array<Player, Num_Players> my_players = { Player::Human, Player::ChessEngine };

        Color my_current_turn;        // whose turn it is
    };
}

#endif // WISDOM_CHESS_GAME_H_
