#include <atomic>

#include "bindings.hpp"
#include "web_logger.hpp"
#include "web_types.hpp"
#include "game_settings.hpp"

using namespace wisdom;

namespace wisdom::worker
{
    struct GameState
    {
        enum PlayStatus {
            Playing = 0,
            Paused = 1,
        };

        std::unique_ptr<wisdom::Game> game;
        wisdom::GameSettings settings {};
        int game_id {};
        bool is_game_over = false;
        std::atomic<int> play_status = PlayStatus::Playing;

        GameState() :
            game { std::make_unique<wisdom::Game>()}
        {
            updateSettings (settings);
        }

        [[nodiscard]] static auto getState() -> observer_ptr<GameState>
        {
            static auto instance = std::make_unique<GameState>();
            return instance.get();
        }

        [[nodiscard]] static auto getGame() -> observer_ptr<Game>
        {
            return GameState::getState()->game.get();
        }

        void updateSettings (GameSettings new_settings)
        {
            settings = new_settings;
            new_settings.applyToGame (game.get());
        }

        auto statusTransition() -> wisdom::GameStatus
        {
            WebEngineGameStatusUpdate status_manager { this };
            status_manager.update (game->status());
            return game->status();
        }

        friend class WebEngineGameStatusUpdate;

        class WebEngineGameStatusUpdate : public GameStatusUpdate
        {
        private:
            observer_ptr<GameState> my_parent;

        public:
            explicit WebEngineGameStatusUpdate (observer_ptr<GameState> parent) :
                    my_parent { parent }
            {
            }

            void checkmate() override
            {
            }

            void stalemate() override
            {
            }

            void insufficientMaterial() override
            {
            }

            void thirdRepetitionDrawAccepted() override
            {
            }

            void fifthRepetitionDraw() override
            {
            }

            void fiftyMovesWithoutProgressAccepted() override
            {
            }

            void seventyFiveMovesWithNoProgress() override
            {
            }

            void thirdRepetitionDrawReached() override
            {
                auto who = my_parent->game->getCurrentTurn();
                my_parent->handlePotentialDrawPosition (ProposedDrawType::ThreeFoldRepetition, who);
            }

            void fiftyMovesWithoutProgressReached() override
            {
                auto who = my_parent->game->getCurrentTurn();
                my_parent->handlePotentialDrawPosition (ProposedDrawType::FiftyMovesWithoutProgress,
                                                        who);
            }
        };

        void updateDrawStatus (wisdom::ProposedDrawType draw_type,
                               wisdom::Color who,
                               bool accepts_draw)
        {
            game->setProposedDrawStatus (
                draw_type,
                who,
                accepts_draw
            );

            emscripten_wasm_worker_post_function_sig (
                EMSCRIPTEN_WASM_WORKER_ID_PARENT, (void*)mainThreadReceiveDrawStatus,
                "iiii",
                game_id,
                static_cast<int> (mapDrawByRepetitionType (draw_type)),
                static_cast<int> (mapColor (who)),
                static_cast<int> (accepts_draw)
            );
        }

        void handlePotentialDrawPosition (wisdom::ProposedDrawType proposedDrawType,
                                          wisdom::Color who)
        {
            auto current_player_accept_draw = game->computerWantsDraw (who);

            updateDrawStatus (proposedDrawType, who, current_player_accept_draw);

            auto opponent = colorInvert (who);
            auto opponent_player = game->getPlayer (opponent);
            if (opponent_player == Player::ChessEngine) {
                auto opponent_wants_draw = game->computerWantsDraw (opponent);
                updateDrawStatus (proposedDrawType, opponent, opponent_wants_draw);
            }
        }
    };
}

using namespace wisdom::worker;

EMSCRIPTEN_KEEPALIVE void workerReinitializeGame (int new_game_id)
{
    auto state = GameState::getState();

    state->game_id = new_game_id;

    auto new_game = std::make_unique<Game>();
    state->game = std::move (new_game);
    state->updateSettings (state->settings);

    auto periodic_func = [state](observer_ptr<MoveTimer> timer) {
        auto play_status = state->play_status.load();
        if (play_status != GameState::Playing) {
            timer->setCancelled (true);
        }
    };
    state->game->setPeriodicFunction (periodic_func);

    startSearch();
}

EMSCRIPTEN_KEEPALIVE void startSearch()
{
    auto logger = wisdom::worker::makeLogger();
    auto state = GameState::getState();
    auto game = GameState::getGame();

    if (state->game->getCurrentPlayer() != Player::ChessEngine)
        return;

    auto play_status = state->play_status.load();
    if (play_status != GameState::Playing)
        return;

    auto new_status = state->statusTransition();
    if (new_status != wisdom::GameStatus::Playing)
        return;

    logger->debug("Going to find best move");
    logger->debug("Current turn: " + asString (game->getCurrentTurn()));

    auto move = game->findBestMove(
        logger,
        game->getCurrentTurn()
    );
    if (!move.has_value())
    {
        // Could happen if game is paused:
        logger->debug("No move found.");
        return;
    }
    game->move (*move);

    emscripten_wasm_worker_post_function_vii (EMSCRIPTEN_WASM_WORKER_ID_PARENT,
                                              mainThreadReceiveMove, state->game_id, move->toInt());
}

EMSCRIPTEN_KEEPALIVE void workerReceiveMove (int packed_move)
{
    auto game = GameState::getGame();
    auto unpacked_move = Move::fromInt (packed_move);
    game->move (unpacked_move);

    startSearch();
}

void workerReceiveSettings (int white_player, int black_player, int thinking_time,
                              int search_depth)
{
    auto state = GameState::getState();

    state->updateSettings (GameSettings { static_cast<WebPlayer> (white_player),
                                          static_cast<WebPlayer> (black_player), 
                                          thinking_time,
                                          search_depth });

    startSearch();
}

EM_JS (void, receiveMoveFromWorker, (int game_id, const char* str),
{
   receiveWorkerMessage ('computerMoved', game_id, UTF8ToString (str));
})

EMSCRIPTEN_KEEPALIVE void mainThreadReceiveMove (int game_id, int packed_move)
{
    Move unpacked_move = Move::fromInt (packed_move);
    auto state = GameState::getState();
    std::string str = asString (unpacked_move);
    receiveMoveFromWorker (state->game_id, str.c_str());
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

EMSCRIPTEN_KEEPALIVE void mainThreadReceiveDrawStatus (int game_id, int draw_type,
                                                       int color, int accepted_draw)
{
    receiveDrawStatusFromWorker (game_id, draw_type, color, accepted_draw == 0 ? false : true);
}
