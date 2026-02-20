#pragma once

#include "wisdom-chess/ui/wasm/web_types.hpp"
#include "wisdom-chess/ui/wasm/web_move.hpp"
#include "wisdom-chess/ui/viewmodel/game_viewmodel_base.hpp"

namespace wisdom
{
    class GameSettings;

    class WebGame : public ui::GameViewModelBase<WebGame>
    {
    private:
        Game my_game;
        WebColoredPieceList my_pieces;
        int my_game_id;

    public:
        int moveNumber {};

        WebGame (int white_player, int black_player, int game_id);

        [[nodiscard]] auto
        getGame()
            -> observer_ptr<Game>
        {
            return &my_game;
        }

        [[nodiscard]] auto
        getGame() const
            -> observer_ptr<const Game>
        {
            return &my_game;
        }

        [[nodiscard]] static auto
        newFromSettings (const GameSettings& settings, int game_id)
            -> WebGame*;

        void setSettings (const GameSettings& settings);

        [[nodiscard]] auto
        needsPawnPromotion (
            const WebCoord* src,
            const WebCoord* dst
        ) const
            -> bool
        {
            return GameViewModelBase::needsPawnPromotion (
                src->row, src->col, dst->row, dst->col
            );
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
            return moveStatus().c_str();
        }

        [[nodiscard]] auto
        getGameOverStatus() const
            -> const char*
        {
            return gameOverStatus().c_str();
        }

        [[nodiscard]] auto
        getInCheck() const
            -> bool
        {
            return inCheck();
        }

        [[nodiscard]] auto
        getGameId() const
            -> int
        {
            return my_game_id;
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

        void updateWebDisplayedGameState();

        void setMoveNumber (size_t size)
        {
            moveNumber = narrow<int> (size);
        }

        friend class WebGameStatusUpdate;
    };
};
