#ifndef CHESSGAME_H
#define CHESSGAME_H

#include <memory>
#include <chrono>

#include "move.hpp"
#include "game.hpp"

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
    explicit MaxDepth(int userDepth)
        : myUserDepth { userDepth }
    {
        if (userDepth <= 0) {
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

class ChessGame
{
public:
    // The configuration of the chess engine.
    struct Config
    {
        wisdom::Players players;
        MaxDepth maxDepth;
        std::chrono::seconds maxTime;
        wisdom::MoveTimer::PeriodicFunction periodicFunction;

        static auto defaultConfig() -> Config
        {
            return {
                { wisdom::Player::Human, wisdom::Player::ChessEngine },
                MaxDepth { 4 },
                std::chrono::seconds { 4 },
                [](gsl::not_null<wisdom::MoveTimer*> timer){}
            };
        }
    };

    explicit ChessGame(std::unique_ptr<wisdom::Game> game, Config config)
        : myEngine { std::move(game) }
        , myConfig { config }
    {
        setConfig(config);
    }

    static auto fromPlayers(wisdom::Player whitePlayer,
                            wisdom::Player blackPlayer, Config config)
        -> std::unique_ptr<ChessGame>;

    static auto fromFen(const std::string& input, Config config)
        -> std::unique_ptr<ChessGame>;

    static auto fromEngine(std::unique_ptr<wisdom::Game> game, Config config)
        -> std::unique_ptr<ChessGame>;

    [[nodiscard]] auto state() -> gsl::not_null<wisdom::Game*>
    {
        return myEngine.get();
    }

    [[nodiscard]] auto state() const -> gsl::not_null<const wisdom::Game*>
    {
        return myEngine.get();
    }

  // Clone the game state
    [[nodiscard]] auto clone() const -> std::unique_ptr<ChessGame>;

    [[nodiscard]] auto isLegalMove(wisdom::Move selectedMove) const -> bool;

    [[nodiscard]] auto moveGenerator() const
        -> gsl::not_null<wisdom::MoveGenerator*>
    {
       return myMoveGenerator.get();
    }

    void setConfig(Config config);
    void setPlayers(wisdom::Player whitePLayer, wisdom::Player blackPlayer);

    [[nodiscard]] auto moveFromCoordinates(int srcRow, int srcColumn,
         int dstRow, int dstColumn,
         std::optional<wisdom::Piece> promoted) const
        -> std::pair<std::optional<wisdom::Move>, wisdom::Color>;

private:
    std::unique_ptr<wisdom::Game> myEngine;
    std::unique_ptr<wisdom::MoveGenerator> myMoveGenerator =
            std::make_unique<wisdom::MoveGenerator>();
    Config myConfig;
};



#endif // CHESSGAME_H
