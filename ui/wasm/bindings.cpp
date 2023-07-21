#include "bindings.hpp"
#include "web_logger.hpp"
#include "web_types.hpp"
#include "game_settings.hpp"

#include <atomic>

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

        GameState () :
            game { std::make_unique<wisdom::Game> ()}
        {
            update_settings (settings);
        }

        [[nodiscard]] static auto get_state() -> observer_ptr<GameState>
        {
            static auto instance = std::make_unique<GameState>();
            return instance.get();
        }

        [[nodiscard]] static auto get_game() -> observer_ptr<Game>
        {
            return GameState::get_state()->game.get();
        }

        void update_settings (GameSettings new_settings)
        {
            settings = new_settings;
            new_settings.apply_to_game (game.get());
        }

        auto status_transition() -> wisdom::GameStatus
        {
            WebEngineGameStatusUpdate status_manager { this };
            status_manager.update (game->status());
            return game->status ();
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
                my_parent->handle_potential_draw_position (ProposedDrawType::ThreeFoldRepetition,
                                                           who);
            }

            void fiftyMovesWithoutProgressReached() override
            {
                auto who = my_parent->game->getCurrentTurn();
                my_parent->handle_potential_draw_position ( ProposedDrawType::FiftyMovesWithoutProgress,
                                                           who);
            }
        };

        void update_draw_status (wisdom::ProposedDrawType draw_type,
                                 wisdom::Color who,
                                 bool accepts_draw)
        {
            game->setProposedDrawStatus (
                draw_type,
                who,
                accepts_draw
            );

            emscripten_wasm_worker_post_function_sig (
                EMSCRIPTEN_WASM_WORKER_ID_PARENT,
                (void*)main_thread_receive_draw_status,
                "iiii",
                game_id,
                static_cast<int> (map_draw_by_repetition_type (draw_type)),
                static_cast<int> (map_color (who)),
                static_cast<int> (accepts_draw)
            );
        }

        void handle_potential_draw_position (wisdom::ProposedDrawType proposedDrawType,
                                             wisdom::Color who)
        {
            auto current_player_accept_draw = game->computerWantsDraw (who);

            update_draw_status (proposedDrawType,
                                who,
                                current_player_accept_draw);

            auto opponent = colorInvert (who);
            auto opponent_player = game->getPlayer (opponent);
            if (opponent_player == Player::ChessEngine) {
                auto opponent_wants_draw = game->computerWantsDraw (opponent);
                update_draw_status (proposedDrawType,
                                    opponent,
                                    opponent_wants_draw);
            }
        }
    };
}

using namespace wisdom::worker;

EMSCRIPTEN_KEEPALIVE void worker_reinitialize_game (int new_game_id)
{
    auto state = GameState::get_state();

    state->game_id = new_game_id;

    auto new_game = std::make_unique<Game> ();
    state->game = std::move (new_game);
    state->update_settings (state->settings);

    auto periodic_func = [state](observer_ptr<MoveTimer> timer) {
        auto play_status = state->play_status.load();
        if (play_status != GameState::Playing) {
            timer->setCancelled (true);
        }
    };
    state->game->setPeriodicFunction (periodic_func);

    start_search();
}

EMSCRIPTEN_KEEPALIVE void start_search()
{
    const auto& logger = wisdom::worker::get_logger();
    auto state = GameState::get_state();
    auto game = GameState::get_game();

    if (state->game->getCurrentPlayer() != Player::ChessEngine)
        return;

    auto play_status = state->play_status.load();
    if (play_status != GameState::Playing)
        return;

    auto new_status = state->status_transition();
    if (new_status != wisdom::GameStatus::Playing)
        return;

    logger.debug("Going to find best move");
    logger.debug("Current turn: " + asString (game->getCurrentTurn()));

    auto move = game->findBestMove(
        logger,
        game->getCurrentTurn()
    );
    if (!move.has_value())
    {
        // Could happen if game is paused:
        get_logger().debug("No move found.");
        return;
    }
    game->move (*move);

    emscripten_wasm_worker_post_function_vii (
        EMSCRIPTEN_WASM_WORKER_ID_PARENT,
        main_thread_receive_move,
        state->game_id, move->toInt()
    );
}

EMSCRIPTEN_KEEPALIVE void worker_receive_move (int packed_move)
{
    auto game = GameState::get_game();
    auto unpacked_move = Move::fromInt (packed_move);
    game->move (unpacked_move);

    start_search();
}

void worker_receive_settings (int white_player, int black_player, int thinking_time,
                              int search_depth)
{
    auto state = GameState::get_state();

    state->update_settings (GameSettings {
        static_cast<WebPlayer> (white_player),
        static_cast<WebPlayer> (black_player),
        thinking_time,
        search_depth
    });

    start_search();
}

EM_JS (void, receiveMoveFromWorker, (int game_id, const char* str),
{
   receiveWorkerMessage ('computerMoved', game_id, UTF8ToString (str));
})

EMSCRIPTEN_KEEPALIVE void main_thread_receive_move (int game_id, int packed_move)
{
    auto& logger = wisdom::worker::get_logger();

    Move unpacked_move = Move::fromInt (packed_move);
    auto state = GameState::get_state();
    std::string str = asString (unpacked_move);
    receiveMoveFromWorker (state->game_id, str.c_str());
}

EMSCRIPTEN_KEEPALIVE void pause_worker ()
{
    auto* state = GameState::get_state();
    state->play_status.store (GameState::Paused);
}

EMSCRIPTEN_KEEPALIVE void unpause_worker ()
{
    auto* state = GameState::get_state();
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

EMSCRIPTEN_KEEPALIVE void main_thread_receive_draw_status (int game_id, int draw_type,
                                                           int color, int accepted_draw)
{
    receiveDrawStatusFromWorker ( game_id, draw_type, color,
        accepted_draw == 0 ? false : true
    );
}
