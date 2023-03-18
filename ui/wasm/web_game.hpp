#ifndef WISDOMCHESS_WEB_GAME_HPP
#define WISDOMCHESS_WEB_GAME_HPP

#include "web_types.hpp"
#include "web_move.hpp"

namespace wisdom
{
    class GameSettings;

    class WebGame
    {
    private:
        Game my_game;
        WebColoredPieceList my_pieces;
        std::string my_move_status;
        std::string my_game_over_status;
        static int our_game_id;

    public:
        bool inCheck = false;
        char* moveStatus{};
        char* gameOverStatus{};
        int moveNumber{};
        int gameId = ++our_game_id;

        WebGame ()
            : WebGame (static_cast<int> (Human), static_cast<int> (ChessEngine))
        {}

        WebGame (int white_player, int black_player);

        [[nodiscard]] static auto new_from_settings (const GameSettings& settings) -> WebGame*;

        void setSettings (const GameSettings& settings);

        auto needsPawnPromotion (const WebCoord* src, const WebCoord* dst) -> bool
        {
            auto game_src = make_coord (src->row, src->col);
            auto game_dst = make_coord (dst->row, dst->col);

            auto optionalMove = my_game.map_coordinates_to_move (game_src, game_dst,
                                                                 Piece::Queen);
            if (!optionalMove.has_value())
                return false;
            return optionalMove->is_promoting();
        }

        [[nodiscard]] auto createMoveFromCoordinatesAndPromotedPiece (const WebCoord* src,
                                                                      const WebCoord* dst,
                                                                      int promoted_piece_type)
            -> WebMove*;

        [[nodiscard]] auto makeMove (const WebMove *move_param) -> bool;

        [[nodiscard]] auto isLegalMove (const WebMove* selectedMovePtr) -> bool;

        void setMaxDepth (int max_depth)
        {
            my_game.set_max_depth (max_depth);
        }

        void setThinkingTime (std::chrono::seconds thinkingTime)
        {
            my_game.set_search_timeout (thinkingTime);
        }

        auto getMaxDepth() -> int
        {
            auto result = my_game.get_max_depth();
            return result;
        }

        auto getPieceList() -> WebColoredPieceList&
        {
            return my_pieces;
        }

        auto getCurrentTurn() -> WebColor
        {
            return map_color (my_game.get_current_turn());
        }

    private:

        [[nodiscard]] auto find_and_remove_id (std::unordered_map<int, WebColoredPiece>& old_list,
                                               Coord coord_to_find,
                                               ColoredPiece piece_to_find) -> int;

        void update_piece_list (ColoredPiece promoted_piece);

        void update_displayed_game_state();

        void set_game_over_status (std::string new_status)
        {
            my_game_over_status = std::move (new_status);
            gameOverStatus = const_cast<char*> (my_game_over_status.c_str());
        }

        void set_in_check (bool new_in_check)
        {
            inCheck = new_in_check;
        }

        void set_move_number (size_t size)
        {
            moveNumber = gsl::narrow<int> (size);
        }

        void set_move_status (std::string new_move_status)
        {
            my_move_status = std::move (new_move_status);
            moveStatus = const_cast<char*> (my_move_status.c_str());
        }
    };
};
#endif // WISDOMCHESS_WEB_GAME_HPP
