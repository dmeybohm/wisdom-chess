#include "wisdom-chess/ui/viewmodel/game_viewmodel_base.hpp"

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

    protected:
        void onGameEnded (GameStatus status) override
        {
            auto game = my_parent->getGame();

            switch (status)
            {
                case GameStatus::Checkmate:
                {
                    auto opponent = colorInvert (game->getCurrentTurn());
                    my_parent->setGameOverStatus (
                        my_parent->formatBold ("Checkmate") + " - "
                        + asString (opponent) + " wins the game."
                    );
                    break;
                }
                case GameStatus::Stalemate:
                {
                    auto who = game->getCurrentTurn();
                    my_parent->setGameOverStatus (
                        my_parent->formatBold ("Stalemate") + " - No legal moves for "
                        + my_parent->formatBold (asString (who))
                    );
                    break;
                }
                case GameStatus::InsufficientMaterialDraw:
                    my_parent->setGameOverStatus (
                        my_parent->formatBold ("Draw") + " - Insufficient material to checkmate."
                    );
                    break;
                case GameStatus::ThreefoldRepetitionAccepted:
                    my_parent->setGameOverStatus (
                        my_parent->formatBold ("Draw") + " - Threefold repetition rule."
                    );
                    break;
                case GameStatus::FivefoldRepetitionDraw:
                    my_parent->setGameOverStatus (
                        my_parent->formatBold ("Draw") + " - Fivefold repetition rule."
                    );
                    break;
                case GameStatus::FiftyMovesWithoutProgressAccepted:
                    my_parent->setGameOverStatus (
                        my_parent->formatBold ("Draw") + " - Fifty moves without progress."
                    );
                    break;
                case GameStatus::SeventyFiveMovesWithoutProgressDraw:
                    my_parent->setGameOverStatus (
                        my_parent->formatBold ("Draw") + " - Seventy-five moves without progress."
                    );
                    break;
                default:
                    break;
            }
        }

        void onDrawProposed (ProposedDrawType type) override
        {
            auto game = my_parent->getGame();
            if (!getFirstHumanPlayerColor (game->getPlayers()).has_value())
                return;

            switch (type)
            {
                case ProposedDrawType::ThreeFoldRepetition:
                    my_parent->setThirdRepetitionDrawStatus (DrawByRepetitionStatus::Proposed);
                    break;
                case ProposedDrawType::FiftyMovesWithoutProgress:
                    my_parent->setFiftyMovesDrawStatus (DrawByRepetitionStatus::Proposed);
                    break;
            }
        }
    };

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

    auto
    GameViewModelBase::formatBold (const std::string& text) const
        -> std::string
    {
        return "<strong>" + text + "</strong>";
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

        ViewModelStatusUpdate statusObserver { this };
        statusObserver.update (game->status());

        if (isKingThreatened (board, who, board.getKingPosition (who)))
        {
            setInCheck (true);
        }

        onDisplayedGameStateUpdated();
    }

    void GameViewModelBase::setProposedDrawStatus (
        wisdom::ProposedDrawType drawType,
        DrawByRepetitionStatus status
    )
    {
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
