#include "wisdom-chess/ui/viewmodel/game_viewmodel_base.hpp"
#include "wisdom-chess/engine/evaluate.hpp"

namespace wisdom::ui
{
    class ViewModelStatusUpdate : public GameStatusUpdate
    {
    private:
        observer_ptr<GameViewModelBase> my_parent;

    public:
        explicit ViewModelStatusUpdate (observer_ptr<GameViewModelBase> parent)
            : my_parent { parent }
        {}

        void checkmate() override
        {
            auto game = my_parent->getGame();
            auto who = game->getCurrentTurn();
            auto opponent = colorInvert (who);
            auto whoString = "<b>Checkmate</b> - " + asString (opponent) + " wins the game.";
            my_parent->setGameOverStatus (whoString);
        }

        void stalemate() override
        {
            auto game = my_parent->getGame();
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
            auto game = my_parent->getGame();
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
            auto game = my_parent->getGame();
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

    GameViewModelBase::GameViewModelBase() = default;

    GameViewModelBase::GameViewModelBase (int initial_game_id)
        : my_game_id { initial_game_id }
    {
    }

    auto
    GameViewModelBase::needsPawnPromotion (int srcRow, int srcCol, int dstRow, int dstCol) const
        -> bool
    {
        auto game = getGame();
        auto gameSrc = makeCoord (srcRow, srcCol);
        auto gameDst = makeCoord (dstRow, dstCol);

        auto optionalMove = game->mapCoordinatesToMove (gameSrc, gameDst, Piece::Queen);
        if (!optionalMove.has_value())
        {
            return false;
        }
        return optionalMove->isPromoting();
    }

    auto
    GameViewModelBase::isLegalMove (Move selectedMove) const
        -> bool
    {
        auto game = getGame();

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

    auto
    GameViewModelBase::inCheck() const
        -> bool
    {
        return my_in_check;
    }

    auto
    GameViewModelBase::moveStatus() const
        -> const std::string&
    {
        return my_move_status;
    }

    auto
    GameViewModelBase::gameOverStatus() const
        -> const std::string&
    {
        return my_game_over_status;
    }

    auto
    GameViewModelBase::gameId() const
        -> int
    {
        return my_game_id.load();
    }

    auto
    GameViewModelBase::thirdRepetitionDrawStatus() const
        -> DrawByRepetitionStatus
    {
        return my_third_repetition_draw_status;
    }

    auto
    GameViewModelBase::fiftyMovesDrawStatus() const
        -> DrawByRepetitionStatus
    {
        return my_fifty_moves_draw_status;
    }

    auto
    GameViewModelBase::currentTurn() const
        -> wisdom::Color
    {
        return my_current_turn;
    }

    void GameViewModelBase::setInCheck (bool value)
    {
        if (my_in_check != value)
        {
            my_in_check = value;
            onInCheckChanged();
        }
    }

    void GameViewModelBase::setMoveStatus (std::string value)
    {
        if (my_move_status != value)
        {
            my_move_status = std::move (value);
            onMoveStatusChanged();
        }
    }

    void GameViewModelBase::setGameOverStatus (std::string value)
    {
        if (my_game_over_status != value)
        {
            my_game_over_status = std::move (value);
            onGameOverStatusChanged();
        }
    }

    void GameViewModelBase::setCurrentTurn (wisdom::Color value)
    {
        if (my_current_turn != value)
        {
            my_current_turn = value;
            onCurrentTurnChanged();
        }
    }

    void GameViewModelBase::setThirdRepetitionDrawStatus (DrawByRepetitionStatus value)
    {
        if (my_third_repetition_draw_status != value)
        {
            my_third_repetition_draw_status = value;
            onThirdRepetitionDrawStatusChanged();
        }
    }

    void GameViewModelBase::setFiftyMovesDrawStatus (DrawByRepetitionStatus value)
    {
        if (my_fifty_moves_draw_status != value)
        {
            my_fifty_moves_draw_status = value;
            onFiftyMovesDrawStatusChanged();
        }
    }

    void GameViewModelBase::incrementGameId()
    {
        my_game_id++;
    }

    void GameViewModelBase::resetStateForNewGame()
    {
        setThirdRepetitionDrawStatus (DrawByRepetitionStatus::NotReached);
        setFiftyMovesDrawStatus (DrawByRepetitionStatus::NotReached);
    }

    void GameViewModelBase::updateDisplayedGameState()
    {
        auto game = getGame();
        auto& board = game->getBoard();
        auto who = game->getCurrentTurn();

        setMoveStatus ("");
        setGameOverStatus ("");
        setInCheck (false);

        ViewModelStatusUpdate statusManager { this };
        statusManager.update (game->status());

        if (isKingThreatened (board, who, board.getKingPosition (who)))
        {
            setInCheck (true);
        }
    }

    void
    GameViewModelBase::setProposedDrawStatus (
        wisdom::ProposedDrawType drawType,
        DrawByRepetitionStatus status
    ) {
        auto game = getGame();
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
}
