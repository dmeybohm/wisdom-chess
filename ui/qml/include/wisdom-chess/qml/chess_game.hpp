#pragma once

#include <chrono>
#include <memory>

#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/move.hpp"

// The valid internal representations of depth in the wisdom Game object is different
// from the semantics in the UI:
//
// 1 half move = Internal Depth 0
// 2 half moves = Internal Depth 1
// 4 half moves = 2 full moves = Internal Depth 3
// 6 half moves = 3 full moves = Internal Depth 5
//
// The UI specifies only full moves, and this class maps to the internal representation.
//
class MaxDepth
{
public:
    explicit MaxDepth (int userDepth) : myUserDepth { userDepth }
    {
        if (userDepth <= 0)
        {
            throw wisdom::Error { "Invalid depth" };
        }
    }

    [[nodiscard]] auto internalDepth() const -> int
    {
        return myUserDepth * 2 - 1;
    }

    [[nodiscard]] auto userDepth() const -> int
    {
        return myUserDepth;
    }

private:
    int myUserDepth;
};

class GameSettings;

class ChessGame
{
public:
    // The configuration of the chess engine.
    struct Config
    {
        wisdom::Players players;
        MaxDepth maxDepth;
        std::chrono::seconds maxTime;

        static auto fromGameSettings (const GameSettings& gameSettings) -> Config;
    };

    explicit ChessGame (std::unique_ptr<wisdom::Game> game, const Config& config) :
            my_engine { std::move (game) }, my_config { config }
    {
        setConfig (config);
    }

    static auto fromPlayers (wisdom::Player whitePlayer, wisdom::Player blackPlayer,
                             const Config& config) -> std::unique_ptr<ChessGame>;

    static auto fromFen (const std::string& input, const Config& config)
        -> std::unique_ptr<ChessGame>;

    static auto fromEngine (std::unique_ptr<wisdom::Game> game, const Config& config)
        -> std::unique_ptr<ChessGame>;

    [[nodiscard]] auto state() -> gsl::not_null<wisdom::Game*>
    {
        return my_engine.get();
    }

    [[nodiscard]] auto state() const -> gsl::not_null<const wisdom::Game*>
    {
        return my_engine.get();
    }

    // Clone the game state
    [[nodiscard]] auto clone() const -> std::unique_ptr<ChessGame>;

    [[nodiscard]] auto isLegalMove (wisdom::Move selectedMove) const -> bool;

    void setConfig (const Config& config);
    void setPeriodicFunction (const wisdom::MoveTimer::PeriodicFunction& func);
    void setPlayers (wisdom::Player whitePLayer, wisdom::Player blackPlayer);

    [[nodiscard]] auto moveFromCoordinates (int srcRow, int srcColumn, int dstRow, int dstColumn,
                                            std::optional<wisdom::Piece> promoted) const
        -> std::pair<std::optional<wisdom::Move>, wisdom::Color>;

private:
    std::unique_ptr<wisdom::Game> my_engine;
    Config my_config;
};

