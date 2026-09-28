#include "wisdom-chess/ui/viewmodel/game_viewmodel_base.hpp"

namespace wisdom::ui
{
    class ViewModelStatusUpdate : public GameStatusUpdate
    {
    private:
        nonnull<GameViewModelBase> my_parent;

    public:
        explicit ViewModelStatusUpdate (nonnull<GameViewModelBase> parent)
            : my_parent { parent }
        {}

    protected:
        void onGameEnded (GameStatus status) override
        {
            auto game = my_parent->getGame();

            switch (status)
            {
                using enum GameStatus;
                case Checkmate:
                {
                    auto opponent = colorInvert (game->getCurrentTurn());
                    my_parent->setGameOverStatus (
                        my_parent->formatBold ("Checkmate") + " - "
                        + asString (opponent) + " wins the game."
                    );
                    break;
                }
                case Stalemate:
                {
                    auto who = game->getCurrentTurn();
                    my_parent->setGameOverStatus (
                        my_parent->formatBold ("Stalemate") + " - No legal moves for "
                        + my_parent->formatBold (asString (who))
                    );
                    break;
                }
                case InsufficientMaterialDraw:
                    my_parent->setGameOverStatus (
                        my_parent->formatBold ("Draw") + " - Insufficient material to checkmate."
                    );
                    break;
                case ThreefoldRepetitionAccepted:
                    my_parent->setGameOverStatus (
                        my_parent->formatBold ("Draw") + " - Threefold repetition rule."
                    );
                    break;
                case FivefoldRepetitionDraw:
                    my_parent->setGameOverStatus (
                        my_parent->formatBold ("Draw") + " - Fivefold repetition rule."
                    );
                    break;
                case FiftyMovesWithoutProgressAccepted:
                    my_parent->setGameOverStatus (
                        my_parent->formatBold ("Draw") + " - Fifty moves without progress."
                    );
                    break;
                case SeventyFiveMovesWithoutProgressDraw:
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
                using enum ProposedDrawType;
                case ThreeFoldRepetition:
                    my_parent->setThirdRepetitionDrawStatus (DrawByRepetitionStatus::Proposed);
                    break;
                case FiftyMovesWithoutProgress:
                    my_parent->setFiftyMovesDrawStatus (DrawByRepetitionStatus::Proposed);
                    break;
            }
        }
    };

    auto
    GameViewModelBase::needsPawnPromotion (int src_row, int src_col, int dst_row, int dst_col) const
        -> bool
    {
        auto game = getGame();
        auto game_src = makeCoord (src_row, src_col);
        auto game_dst = makeCoord (dst_row, dst_col);

        auto optional_move = game->mapCoordinatesToMove (game_src, game_dst, Piece::Queen);
        if (!optional_move.has_value())
        {
            return false;
        }
        return optional_move->isPromoting();
    }

    auto
    GameViewModelBase::canMoveFrom (int row, int col) const
        -> bool
    {
        auto game = getGame();

        if (game->getStatus() != GameStatus::Playing || game->getCurrentPlayer() != Player::Human)
        {
            return false;
        }

        auto piece = game->getBoard().pieceAt (row, col);
        return piece != Piece_And_Color_None && pieceColor (piece) == game->getCurrentTurn();
    }

    auto
    GameViewModelBase::isLegalMove (Move selected_move) const
        -> bool
    {
        auto game = getGame();

        if (game->getCurrentPlayer() != Player::Human)
        {
            return false;
        }

        auto who = game->getCurrentTurn();
        auto legal_moves = generateLegalMoves (game->getBoard(), who);

        return std::any_of (
            legal_moves.cbegin(),
            legal_moves.cend(),
            [selected_move] (const auto& move)
            {
                return move == selected_move;
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
        setInCheck (false);
        setMoveStatus ("");
        setGameOverStatus ("");
        setCurrentTurn (wisdom::Color::White);
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

        ViewModelStatusUpdate status_observer { this };
        status_observer.update (game->getStatus());

        if (isKingThreatened (board, who, board.getKingPosition (who)))
        {
            setInCheck (true);
        }

        onDisplayedGameStateUpdated();
    }

    void GameViewModelBase::setProposedDrawStatus (
        wisdom::ProposedDrawType draw_type,
        DrawByRepetitionStatus status
    )
    {
        auto game = getGame();
        auto optional_color = getFirstHumanPlayerColor (game->getPlayers());

        expects (optional_color.has_value());
        auto who = *optional_color;
        auto opponent_color = colorInvert (who);

        bool accepted = (status == DrawByRepetitionStatus::Accepted);
        game->setProposedDrawStatus (draw_type, who, accepted);
        if (game->getPlayer (opponent_color) == Player::Human)
        {
            game->setProposedDrawStatus (draw_type, opponent_color, accepted);
        }

        updateDisplayedGameState();
    }
}
