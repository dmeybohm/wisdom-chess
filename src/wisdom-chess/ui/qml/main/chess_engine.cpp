#include <QDebug>
#include <QThread>
#include <mutex>

#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/logger.hpp"

#include "wisdom-chess/ui/qml/main/chess_engine.hpp"
#include "wisdom-chess/ui/viewmodel/viewmodel_types.hpp"

using namespace wisdom;
namespace ui = wisdom::ui;
using std::optional;
using std::shared_ptr;
using wisdom::GameStatus;
using wisdom::ProposedDrawType;

namespace wisdom::ui::qml
{
    // The engine's types, not the wisdom::ui mirrors QML sees.
    using wisdom::Color;
    using wisdom::Player;

    void ChessEngine::ChessEngineLogger::debug (const std::string& line) noexcept
    {
        qDebug() << line.c_str();
    }

    void ChessEngine::ChessEngineLogger::info (const std::string& line) noexcept
    {
        qDebug() << line.c_str();
    }

    void ChessEngine::ChessEngineLogger::emergency (std::string_view line) noexcept
    {
        qCritical().noquote() << QString::fromUtf8 (line.data(), narrow_debug<qsizetype> (line.size()));
    }

    ChessEngine::ChessEngine (shared_ptr<ChessGame> game, int game_id, QObject* parent)
        : QObject { parent }
        , my_game { std::move (game) }
        , my_game_id { game_id }
        , my_logger { makeBufferedLogger (make_shared<ChessEngineLogger>()) }
    {
        syncDebugLogging();
    }

    void ChessEngine::syncDebugLogging()
    {
        my_logger->setEnabled (my_game->config().debugLogging);
    }

    void ChessEngine::init() noexcept
    {
        findMove();
    }

    void ChessEngine::opponentMoved (Move move, [[maybe_unused]] Color who) noexcept
    {
        auto game = my_game->state();
        game->move (move);
        findMove();
    }

    void
    ChessEngine::receiveEngineMoved (
        [[maybe_unused]] wisdom::Move move,
        [[maybe_unused]] wisdom::Color who,
        int game_id
    ) noexcept
    {
        if (game_id == my_game_id)
        {
            // The GUI has shown the move. Do another if the engine is hooked
            // up to itself:
            my_move_awaiting_gui = false;
            init();
        }
    }

    class QmlEngineGameStatusUpdate : public GameStatusUpdate
    {
    private:
        nonnull<ChessEngine> my_parent;

    public:
        explicit QmlEngineGameStatusUpdate (
            nonnull<ChessEngine> parent
        )
            : my_parent { parent }
        {
        }

    protected:
        void onGameEnded ([[maybe_unused]] GameStatus status) override
        {
            my_parent->my_is_game_over = true;
            emit my_parent->noMovesAvailable();
        }

        void onDrawProposed (ProposedDrawType type) override
        {
            auto game_state = my_parent->my_game->state();
            auto who = game_state->getCurrentTurn();
            my_parent->handlePotentialDrawPosition (type, who);
        }
    };

    auto
    ChessEngine::gameStatusTransition()
        -> wisdom::GameStatus
    {
        QmlEngineGameStatusUpdate status_manager { this };
        return ui::transitionGameStatus (status_manager, *my_game->state());
    }

    void ChessEngine::findMove()
    {
        auto game_state = my_game->state();

        if (my_is_game_over || my_move_awaiting_gui)
        {
            return;
        }

        auto player = game_state->getCurrentPlayer();
        if (player != Player::ChessEngine)
        {
            return;
        }

        auto next_status = gameStatusTransition();
        if (next_status != GameStatus::Playing)
        {
            // The game is now over - or we're waiting for a response on a draw proposal.
            return;
        }

        auto who = game_state->getCurrentTurn();

        my_logger->debug ("Searching for move");
        auto optional_move = game_state->findBestMove (my_logger, &my_transposition_table);

        // The game was playing, so there was a legal move; the search comes
        // back empty only when it was cancelled before finishing a root move.
        if (optional_move.has_value())
        {
            game_state->move (*optional_move);
            my_move_awaiting_gui = true;
            emit engineMoved (*optional_move, who, my_game_id);
        }
        else
        {
            emit searchInterrupted();
        }
    }

    void
    ChessEngine::handlePotentialDrawPosition (
        wisdom::ProposedDrawType proposed_draw_type,
        wisdom::Color who
    )
    {
        auto game_state = my_game->state();
        bool any_accepted = false;

        ui::negotiateDraw (
            game_state,
            proposed_draw_type,
            who,
            [this, proposed_draw_type, &any_accepted] (Color player, bool accepted)
            {
                emit updateDrawStatus (proposed_draw_type, player, accepted);
                if (accepted)
                {
                    any_accepted = true;
                    my_is_game_over = true;
                    emit noMovesAvailable();
                }
            }
        );

        // When the computer is playing itself and both sides declined,
        // resume searching:
        auto opponent = colorInvert (who);
        if (!any_accepted && game_state->getPlayer (opponent) == Player::ChessEngine
            && gameStatusTransition() == GameStatus::Playing)
        {
            findMove();
        }
    }

    void
    ChessEngine::receiveDrawStatus (
        wisdom::ProposedDrawType draw_type,
        wisdom::Color player,
        bool accepted
    ) noexcept
    {
        auto game_state = my_game->state();
        game_state->setProposedDrawStatus (draw_type, player, accepted);

        auto next_status = gameStatusTransition();
        if (next_status == GameStatus::Playing)
        {
            findMove(); // resume playing.
        }
    }

    void ChessEngine::reloadGame (shared_ptr<ChessGame> new_game, int new_game_id) noexcept
    {
        my_game = std::move (new_game);
        my_game_id = new_game_id;
        my_is_game_over = false;
        my_move_awaiting_gui = false;
        my_transposition_table.clear();
        syncDebugLogging();

        // Possibly resume searching for the next move:
        findMove();
    }

    void
    ChessEngine::updateConfig (
        const ChessGame::Config& config,
        const wisdom::MoveTimer::PeriodicFunction& notifier
    ) noexcept
    {
        my_game->setConfig (config);
        syncDebugLogging();

        // Update the notifier:
        my_game->setPeriodicFunction (notifier);

        // Possibly resume searching for the next move:
        findMove();
    }

    void ChessEngine::quit() noexcept
    {
        QThread::currentThread()->quit();
    }
}
