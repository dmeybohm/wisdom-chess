#ifndef WISDOM_CHESS_GAME_HPP
#define WISDOM_CHESS_GAME_HPP

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

    enum class GameStatus
    {
        Playing,
        Checkmate,
        Stalemate,
        ThreefoldRepetitionReached,
        ThreefoldRepetitionAccepted,
        FivefoldRepetitionDraw,
        FiftyMovesWithoutProgressReached,
        FiftyMovesWithoutProgressAccepted,
        SeventyFiveMovesWithoutProgressDraw,
        InsufficientMaterialDraw,
    };

    enum class ProposedDrawType
    {
        ThreeFoldRepetition,
        FiftyMovesWithoutProgress,
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
        [[nodiscard]] auto get_board() const&& -> Board& = delete;

        [[nodiscard]] auto get_history () const& -> History&;
        [[nodiscard]] auto get_history () const&& -> History& = delete;

        [[nodiscard]] auto get_move_generator () const& -> not_null<MoveGenerator*>;
        [[nodiscard]] auto get_move_generator () const&& -> not_null<MoveGenerator*> = delete;

        [[nodiscard]] auto get_current_player () const -> Player
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

        [[nodiscard]] auto get_player (Color color) const -> Player
        {
            auto index = color_index (color);
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

        [[nodiscard]] auto get_max_depth () const -> int
        {
            return my_max_depth;
        }

        void set_max_depth (int max_depth)
        {
            my_max_depth = max_depth;
        }

        [[nodiscard]] auto get_search_timeout () const -> std::chrono::seconds
        {
            return my_search_timeout;
        }

        void set_search_timeout (std::chrono::seconds seconds)
        {
            my_search_timeout = seconds;
        }

        void set_analytics (unique_ptr<analysis::Analytics> new_analytics);

        [[nodiscard]] auto map_coordinates_to_move (Coord src, Coord dst, optional<Piece> promoted) const
            -> optional<Move>
        {
            return ::wisdom::map_coordinates_to_move (*my_board, get_current_turn (),
                                                      src, dst, promoted);
        }

        void set_periodic_function (const MoveTimer::PeriodicFunction& periodic_function)
        {
            std::cout << "periodic function address: " << &periodic_function << "\n";
            my_periodic_function = periodic_function;
        }

        [[nodiscard]] auto status () const -> GameStatus;

        [[nodiscard]] auto computer_wants_draw (Color who) const -> bool;

        void set_proposed_draw_status (ProposedDrawType draw_type, Color who,
                                       bool accepted);

        void set_proposed_draw_status (ProposedDrawType draw_type,
                                       std::pair<bool, bool> accepted);

    private:
        unique_ptr<Board> my_board = make_unique<Board> ();
        unique_ptr<MoveGenerator> my_move_generator = make_unique<MoveGenerator> ();
        unique_ptr<History> my_history = make_unique<History> ();
        unique_ptr<analysis::Analytics> my_analytics = make_unique<analysis::Analytics> ();
        optional<MoveTimer::PeriodicFunction> my_periodic_function {};
        int my_max_depth { Default_Max_Depth };

        Players my_players = { Player::Human, Player::ChessEngine };
        chrono::seconds my_search_timeout { Default_Max_Search_Seconds };

        DrawAccepted my_third_repetition_draw;
        DrawAccepted my_fifty_moves_without_progress_draw;

        void update_threefold_repetition_draw_status ();
        void update_fifty_moves_without_progress_draw_status ();
    };
}

#endif // WISDOM_CHESS_GAME_HPP
