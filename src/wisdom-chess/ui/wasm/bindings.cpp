#include <atomic>

#include "wisdom-chess/ui/wasm/bindings.hpp"
#include "wisdom-chess/ui/wasm/game_model.hpp"
#include "wisdom-chess/ui/wasm/web_logger.hpp"
#include "wisdom-chess/ui/wasm/web_types.hpp"
#include "wisdom-chess/ui/wasm/game_settings.hpp"

#include "wisdom-chess/engine/logger.hpp"
#include "wisdom-chess/engine/transposition_table.hpp"
#include "wisdom-chess/ui/viewmodel/viewmodel_types.hpp"

namespace ui = wisdom::ui;

using namespace wisdom;

namespace wisdom::worker
{
    struct GameState
    {
        enum PlayStatus {
            Playing = 0,
            Paused = 1,
        };

        wisdom::Game game;
        wisdom::TranspositionTable transposition_table;
        wisdom::GameSettings settings {};
        int game_id {};
        std::atomic<int> play_status = PlayStatus::Playing;

        // Retains search output while debug logging is off and replays it
        // when the setting is switched on.
        shared_ptr<BufferedLogger> logger = makeBufferedLogger (wisdom::worker::makeLogger());

        GameState()
            : game { Game::createStandardGame() }
        {
            updateSettings (settings);
        }

        [[nodiscard]] static auto 
        getState() 
            -> observer_ptr<GameState>
        {
            static auto instance = std::make_unique<GameState>();
            return instance.get();
        }

        [[nodiscard]] static auto 
        getGame() 
            -> observer_ptr<Game>
        {
            return &GameState::getState()->game;
        }

        void updateSettings (GameSettings new_settings)
        {
            settings = new_settings;
            new_settings.applyToGame (&game);
            logger->setEnabled (new_settings.debugLogging);
        }

        auto
        statusTransition()
            -> wisdom::GameStatus
        {
            WebEngineGameStatusUpdate status_manager { this };
            return ui::transitionGameStatus (status_manager, game);
        }

        friend class WebEngineGameStatusUpdate;

        class WebEngineGameStatusUpdate : public GameStatusUpdate
        {
        private:
            observer_ptr<GameState> my_parent;

        public:
            explicit WebEngineGameStatusUpdate (observer_ptr<GameState> parent)
                : my_parent { parent }
            {
            }

        protected:
            void onDrawProposed (ProposedDrawType type) override
            {
                auto who = my_parent->game.getCurrentTurn();
                my_parent->handlePotentialDrawPosition (type, who);
            }
        };

        void
        handlePotentialDrawPosition (
            wisdom::ProposedDrawType proposedDrawType,
            wisdom::Color who
        ) {
            // The answers are recorded here; the main thread hears each one.
            ui::negotiateDraw (
                &game,
                proposedDrawType,
                who,
                [this, proposedDrawType] (Color player, bool accepted)
                {
                    emscripten_wasm_worker_post_function_sig (
                        EMSCRIPTEN_WASM_WORKER_ID_PARENT, (void*)mainThreadReceiveDrawStatus,
                        "iiii",
                        game_id,
                        static_cast<int> (mapDrawByRepetitionType (proposedDrawType)),
                        static_cast<int> (mapColor (player)),
                        static_cast<int> (accepted)
                    );
                }
            );
        }
    };
}

using namespace wisdom::worker;

EMSCRIPTEN_KEEPALIVE void workerReinitializeGame (int new_game_id)
{
    auto state = GameState::getState();

    state->game_id = new_game_id;

    state->game = Game::createStandardGame();
    state->transposition_table.clear();
    state->updateSettings (state->settings);

    auto periodic_func = [state](nonnull_observer_ptr<MoveTimer> timer) {
        auto play_status = state->play_status.load();
        if (play_status != GameState::Playing) {
            timer->setCancelled (true);
        }
    };
    state->game.setPeriodicFunction (periodic_func);

    startSearch();
}

EMSCRIPTEN_KEEPALIVE void startSearch()
{
    auto state = GameState::getState();
    auto game = GameState::getGame();
    auto logger = state->logger;

    if (state->game.getCurrentPlayer() != Player::ChessEngine)
        return;

    auto play_status = state->play_status.load();
    if (play_status != GameState::Playing)
        return;

    auto new_status = state->statusTransition();
    if (new_status != wisdom::GameStatus::Playing)
        return;

    logger->debug ("Going to find best move");
    logger->debug ("Current turn: " + asString (game->getCurrentTurn()));

    auto move = game->findBestMove(
        logger,
        &state->transposition_table,
        game->getCurrentTurn()
    );
    if (!move.has_value())
    {
        // Could happen if game is paused:
        logger->debug ("No move found.");
        return;
    }
    game->move (*move);

    emscripten_wasm_worker_post_function_vii (
        EMSCRIPTEN_WASM_WORKER_ID_PARENT, 
        mainThreadReceiveMove, 
        state->game_id, 
        move->toInt()
    );
}

EMSCRIPTEN_KEEPALIVE void workerReceiveMove (int packed_move)
{
    auto game = GameState::getGame();
    auto unpacked_move = Move::fromInt (packed_move);
    game->move (unpacked_move);

    startSearch();
}

void
workerReceiveSettings (
    int white_player, 
    int black_player, 
    int thinking_time, 
    int search_depth,
    int debug_logging
) {
    auto state = GameState::getState();

    state->updateSettings (
        GameSettings { 
            static_cast<WebPlayer> (white_player),
            static_cast<WebPlayer> (black_player), 
            thinking_time, 
            search_depth,
            debug_logging != 0
        }
    );

    startSearch();
}

EM_JS (void, receiveMoveFromWorker, (int game_id, const char* str),
{
   receiveWorkerMessage ('computerMoved', game_id, UTF8ToString (str));
})

EMSCRIPTEN_KEEPALIVE void
mainThreadReceiveMove (
    int game_id,
    int packed_move
) {
    // Validate game_id against the authoritative GameModel ID.
    // Reject stale moves from old games.
    if (game_id != GameModel::currentGameId())
        return;

    Move unpacked_move = Move::fromInt (packed_move);
    std::string str = asString (unpacked_move);
    receiveMoveFromWorker (game_id, str.c_str());
}

EMSCRIPTEN_KEEPALIVE void pauseWorker()
{
    auto* state = GameState::getState();
    state->play_status.store (GameState::Paused);
}

EMSCRIPTEN_KEEPALIVE void unpauseWorker()
{
    auto* state = GameState::getState();
    state->play_status.store (GameState::Playing);
}

EM_JS (void, receiveDrawStatusFromWorker, (int game_id, int draw_type, int color, bool accepted),
{
   receiveWorkerMessage (
        'computerDrawStatusUpdated',
        game_id,
        JSON.stringify ({
            draw_type: draw_type,
            color: color,
            accepted: accepted
       })
   )
})

EMSCRIPTEN_KEEPALIVE void 
mainThreadReceiveDrawStatus (
    int game_id, 
    int draw_type, 
    int color, 
    int accepted_draw
) {
    receiveDrawStatusFromWorker (
        game_id, 
        draw_type, 
        color, 
        accepted_draw == 0 ? false : true
    );
}
