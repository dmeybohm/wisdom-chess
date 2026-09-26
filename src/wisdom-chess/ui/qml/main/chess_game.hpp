#pragma once

#include <chrono>
#include <memory>

#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/ui/viewmodel/game_settings.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/move_timer.hpp"
#include "wisdom-chess/ui/viewmodel/viewmodel_types.hpp"

namespace wisdom::ui::qml
{
    class GameSettings;

    class ChessGame
    {
    public:
        // The configuration of the chess engine.
        using Config = wisdom::ui::GameSettings;

        explicit ChessGame (
            std::unique_ptr<wisdom::Game> game, 
            const Config& config
        ) 
            : my_engine { std::move (game) } 
            , my_config { config }
        {
            setConfig (config);
        }

        static auto fromPlayers (
            wisdom::Player whitePlayer, 
            wisdom::Player blackPlayer, 
            const Config& config
        ) 
            -> std::unique_ptr<ChessGame>;

        static auto fromFen (
            const std::string& input, 
            const Config& config
        )
            -> std::unique_ptr<ChessGame>;

        static auto fromEngine (
            std::unique_ptr<wisdom::Game> game, 
            const Config& config
        )
            -> std::unique_ptr<ChessGame>;

        [[nodiscard]] auto 
        state() 
            -> gsl::not_null<wisdom::Game*>
        {
            return my_engine.get();
        }

        [[nodiscard]] auto 
        state() const 
            -> gsl::not_null<const wisdom::Game*>
        {
            return my_engine.get();
        }

        [[nodiscard]] auto 
        config() const 
            -> const Config&
        {
            return my_config;
        }

        // Clone the current position. The move history is not copied, so this
        // is only equivalent to the original at the start of a game.
        [[nodiscard]] auto 
        clone() const 
            -> std::unique_ptr<ChessGame>;

        void setConfig (const Config& config);
        void setPeriodicFunction (
            const wisdom::MoveTimer::PeriodicFunction& func
        );
        void setPlayers (
            wisdom::Player whitePLayer, 
            wisdom::Player blackPlayer
        );

        [[nodiscard]] auto 
        moveFromCoordinates (
            int srcRow, 
            int srcColumn, 
            int dstRow, 
            int dstColumn, 
            std::optional<wisdom::Piece> promoted
        ) const
            -> std::pair<std::optional<wisdom::Move>, wisdom::Color>;

    private:
        std::unique_ptr<wisdom::Game> my_engine;
        Config my_config;
    };
}
