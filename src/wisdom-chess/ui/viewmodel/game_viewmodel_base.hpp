#pragma once

#include <string>

#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/ui/viewmodel/viewmodel_types.hpp"
#include "wisdom-chess/ui/viewmodel/viewmodel_settings.hpp"

namespace wisdom::ui
{
    template <typename Derived>
    class GameViewModelBase
    {
        friend Derived;
        GameViewModelBase() = default;

    public:
        ~GameViewModelBase() = default;

        GameViewModelBase (const GameViewModelBase&) = delete;
        auto operator= (const GameViewModelBase&) -> GameViewModelBase& = delete;

        [[nodiscard]] auto
        needsPawnPromotion (int srcRow, int srcCol, int dstRow, int dstCol) const
            -> bool
        {
            auto game = derived().getGame();
            auto gameSrc = makeCoord (srcRow, srcCol);
            auto gameDst = makeCoord (dstRow, dstCol);

            auto optionalMove = game->mapCoordinatesToMove (gameSrc, gameDst, Piece::Queen);
            if (!optionalMove.has_value())
            {
                return false;
            }
            return optionalMove->isPromoting();
        }

        [[nodiscard]] auto
        isLegalMove (Move selectedMove) const
            -> bool
        {
            auto game = derived().getGame();

            if (game->getCurrentPlayer() != Player::Human)
            {
                return false;
            }

            auto who = game->getCurrentTurn();
            auto legalMoves = generateLegalMoves (game->getBoard(), who);

            return std::any_of (
                legalMoves.cbegin(),
                legalMoves.cend(),
                [selectedMove] (const auto& move)
                {
                    return move == selectedMove;
                }
            );
        }

        [[nodiscard]] auto
        inCheck() const
            -> bool
        {
            return my_in_check;
        }

        [[nodiscard]] auto
        moveStatus() const
            -> const std::string&
        {
            return my_move_status;
        }

        [[nodiscard]] auto
        gameOverStatus() const
            -> const std::string&
        {
            return my_game_over_status;
        }

        [[nodiscard]] auto
        thirdRepetitionDrawStatus() const
            -> DrawByRepetitionStatus
        {
            return my_third_repetition_draw_status;
        }

        [[nodiscard]] auto
        fiftyMovesDrawStatus() const
            -> DrawByRepetitionStatus
        {
            return my_fifty_moves_draw_status;
        }

        [[nodiscard]] auto
        currentTurn() const
            -> wisdom::Color
        {
            return my_current_turn;
        }

    protected:
        void onInCheckChanged() {}
        void onMoveStatusChanged() {}
        void onGameOverStatusChanged() {}
        void onCurrentTurnChanged() {}
        void onThirdRepetitionDrawStatusChanged() {}
        void onFiftyMovesDrawStatusChanged() {}

        void setInCheck (bool value)
        {
            if (my_in_check != value)
            {
                my_in_check = value;
                derived().onInCheckChanged();
            }
        }

        void setMoveStatus (std::string value)
        {
            if (my_move_status != value)
            {
                my_move_status = std::move (value);
                derived().onMoveStatusChanged();
            }
        }

        void setGameOverStatus (std::string value)
        {
            if (my_game_over_status != value)
            {
                my_game_over_status = std::move (value);
                derived().onGameOverStatusChanged();
            }
        }

        void setCurrentTurn (wisdom::Color value)
        {
            if (my_current_turn != value)
            {
                my_current_turn = value;
                derived().onCurrentTurnChanged();
            }
        }

        void setThirdRepetitionDrawStatus (DrawByRepetitionStatus value)
        {
            if (my_third_repetition_draw_status != value)
            {
                my_third_repetition_draw_status = value;
                derived().onThirdRepetitionDrawStatusChanged();
            }
        }

        void setFiftyMovesDrawStatus (DrawByRepetitionStatus value)
        {
            if (my_fifty_moves_draw_status != value)
            {
                my_fifty_moves_draw_status = value;
                derived().onFiftyMovesDrawStatusChanged();
            }
        }

        void resetStateForNewGame()
        {
            setThirdRepetitionDrawStatus (DrawByRepetitionStatus::NotReached);
            setFiftyMovesDrawStatus (DrawByRepetitionStatus::NotReached);
        }

        void updateDisplayedGameState()
        {
            auto game = derived().getGame();
            auto& board = game->getBoard();
            auto who = game->getCurrentTurn();

            setMoveStatus ("");
            setGameOverStatus ("");
            setInCheck (false);

            StatusUpdateObserver statusObserver { this };
            statusObserver.update (game->status());

            if (isKingThreatened (board, who, board.getKingPosition (who)))
            {
                setInCheck (true);
            }
        }

        void setProposedDrawStatus (
            wisdom::ProposedDrawType drawType,
            DrawByRepetitionStatus status
        )
        {
            auto game = derived().getGame();
            auto optionalColor = getFirstHumanPlayerColor (game->getPlayers());

            assert (optionalColor.has_value());
            auto who = *optionalColor;
            auto opponentColor = colorInvert (who);

            bool accepted = (status == DrawByRepetitionStatus::Accepted);
            game->setProposedDrawStatus (drawType, who, accepted);
            if (game->getPlayer (opponentColor) == Player::Human)
            {
                game->setProposedDrawStatus (drawType, opponentColor, accepted);
            }

            updateDisplayedGameState();
        }

    private:
        [[nodiscard]] auto derived() -> Derived&
        {
            return static_cast<Derived&> (*this);
        }

        [[nodiscard]] auto derived() const -> const Derived&
        {
            return static_cast<const Derived&> (*this);
        }

        class StatusUpdateObserver : public GameStatusUpdate
        {
        private:
            observer_ptr<GameViewModelBase> my_parent;

        public:
            explicit StatusUpdateObserver (observer_ptr<GameViewModelBase> parent)
                : my_parent { parent }
            {}

            void checkmate() override
            {
                auto game = my_parent->derived().getGame();
                auto who = game->getCurrentTurn();
                auto opponent = colorInvert (who);
                auto whoString = "<b>Checkmate</b> - " + asString (opponent) + " wins the game.";
                my_parent->setGameOverStatus (whoString);
            }

            void stalemate() override
            {
                auto game = my_parent->derived().getGame();
                auto who = game->getCurrentTurn();
                auto stalemateStr = "<b>Stalemate</b> - No legal moves for <b>"
                    + asString (who) + "</b>";
                my_parent->setGameOverStatus (stalemateStr);
            }

            void insufficientMaterial() override
            {
                my_parent->setGameOverStatus ("<b>Draw</b> - Insufficient material to checkmate.");
            }

            void thirdRepetitionDrawReached() override
            {
                auto game = my_parent->derived().getGame();
                if (getFirstHumanPlayerColor (game->getPlayers()).has_value())
                {
                    my_parent->setThirdRepetitionDrawStatus (DrawByRepetitionStatus::Proposed);
                }
            }

            void thirdRepetitionDrawAccepted() override
            {
                my_parent->setGameOverStatus ("<b>Draw</b> - Threefold repetition rule.");
            }

            void fifthRepetitionDraw() override
            {
                my_parent->setGameOverStatus ("<b>Draw</b> - Fivefold repetition rule.");
            }

            void fiftyMovesWithoutProgressReached() override
            {
                auto game = my_parent->derived().getGame();
                if (getFirstHumanPlayerColor (game->getPlayers()).has_value())
                {
                    my_parent->setFiftyMovesDrawStatus (DrawByRepetitionStatus::Proposed);
                }
            }

            void fiftyMovesWithoutProgressAccepted() override
            {
                my_parent->setGameOverStatus ("<b>Draw</b> - Fifty moves without progress.");
            }

            void seventyFiveMovesWithNoProgress() override
            {
                my_parent->setGameOverStatus ("<b>Draw</b> - Seventy-five moves without progress.");
            }
        };

        bool my_in_check = false;
        std::string my_move_status;
        std::string my_game_over_status;
        wisdom::Color my_current_turn = wisdom::Color::White;
        DrawByRepetitionStatus my_third_repetition_draw_status = DrawByRepetitionStatus::NotReached;
        DrawByRepetitionStatus my_fifty_moves_draw_status = DrawByRepetitionStatus::NotReached;
    };
}
