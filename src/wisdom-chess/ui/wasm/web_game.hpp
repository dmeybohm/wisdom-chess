#pragma once

#include "wisdom-chess/ui/wasm/web_types.hpp"
#include "wisdom-chess/ui/wasm/web_move.hpp"

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
        int moveNumber {};
        int gameId = ++our_game_id;

        WebGame()
            : WebGame (static_cast<int> (Human), static_cast<int> (ChessEngine))
        {}

        WebGame (int white_player, int black_player);

        [[nodiscard]] static auto 
        newFromSettings (const GameSettings& settings) 
            -> WebGame*;

        void setSettings (const GameSettings& settings);

        [[nodiscard]] auto 
        needsPawnPromotion (
            const WebCoord* src, 
            const WebCoord* dst
        ) const 
            -> bool
        {
            auto game_src = makeCoord (src->row, src->col);
            auto game_dst = makeCoord (dst->row, dst->col);

            auto optionalMove = my_game.mapCoordinatesToMove (game_src, game_dst,
                                                              Piece::Queen);
            if (!optionalMove.has_value())
                return false;
            return optionalMove->isPromoting();
        }

        [[nodiscard]] auto 
        createMoveFromCoordinatesAndPromotedPiece (
            const WebCoord* src, 
            const WebCoord* dst, 
            int promoted_piece_type
        )
            -> WebMove*;

        [[nodiscard]] auto makeMove (
            const WebMove *move_param
        ) 
            -> bool;

        [[nodiscard]] auto 
        isLegalMove (const WebMove* selectedMovePtr) 
            -> bool;

        void setMaxDepth (int max_depth)
        {
            my_game.setMaxDepth (max_depth);
        }

        void setThinkingTime (std::chrono::seconds thinkingTime)
        {
            my_game.setSearchTimeout (thinkingTime);
        }

        [[nodiscard]] auto 
        getMaxDepth() const 
            -> int
        {
            auto result = my_game.getMaxDepth();
            return result;
        }

        [[nodiscard]] auto 
        getPieceList() 
            -> WebColoredPieceList&
        {
            return my_pieces;
        }

        [[nodiscard]] auto 
        getCurrentTurn() const 
            -> WebColor
        {
            return mapColor (my_game.getCurrentTurn());
        }

        [[nodiscard]] auto 
        getMoveStatus() const 
            -> const char*
        {
            return my_move_status.c_str();
        }

        [[nodiscard]] auto 
        getGameOverStatus() const 
            -> const char*
        {
            return my_game_over_status.c_str();
        }

        [[nodiscard]] auto 
        getGameStatus() const 
            -> WebGameStatus
        {
            return mapGameStatus (my_game.status());
        }

        [[nodiscard]] auto 
        getPlayerOfColor (int color) const 
            -> WebPlayer
        {
            Color mapped_color = mapColor (color);
            return mapPlayer (my_game.getPlayer (mapped_color));
        }

        void setComputerDrawStatus (int type, int who, bool accepted);

        void setHumanDrawStatus (int type, int who, bool accepted)
        {
            ProposedDrawType proposed_draw_type = mapDrawByRepetitionType (type);
            Color color = mapColor (who);
            my_game.setProposedDrawStatus (proposed_draw_type, color, accepted);
            updateDisplayedGameState();
        }

    private:

        [[nodiscard]] auto findAndRemoveId (
            std::unordered_map<int, 
            WebColoredPiece>& old_list, 
            Coord coord_to_find, 
            ColoredPiece piece_to_find
        ) -> int;

        void updatePieceList (ColoredPiece promoted_piece);

        void updateDisplayedGameState();

        void setGameOverStatus (std::string new_status)
        {
            my_game_over_status = std::move (new_status);
        }

        void setInCheck (bool new_in_check)
        {
            inCheck = new_in_check;
        }

        void setMoveNumber (size_t size)
        {
            moveNumber = narrow<int> (size);
        }

        void setMoveStatus (std::string new_move_status)
        {
            my_move_status = std::move (new_move_status);
        }

        friend class WebGameStatusUpdate;
    };
};

