#ifndef CHESSGAME_H
#define CHESSGAME_H

#include <memory>

#include "move.hpp"
#include "game.hpp"

class ChessGame
{
public:
    explicit ChessGame(std::unique_ptr<wisdom::Game> game)
        : myEngine { std::move(game) }
    {
    }

    static auto fromPlayers(wisdom::Player whitePlayer, wisdom::Player blackPlayer)
        -> std::unique_ptr<ChessGame>;

    static auto fromFen(const std::string& input) -> std::unique_ptr<ChessGame>;

    static auto fromEngine(std::unique_ptr<wisdom::Game> game)
        -> std::unique_ptr<ChessGame>;

    auto engine() -> gsl::not_null<wisdom::Game*>
    {
        return myEngine.get();
    }

    auto clone() const -> std::unique_ptr<ChessGame>;

    auto engine() const -> gsl::not_null<const wisdom::Game*>
    {
        return myEngine.get();
    }

    auto isLegalMove(wisdom::Move selectedMove) -> bool;

    auto moveGenerator() -> gsl::not_null<wisdom::MoveGenerator*>
    {
       return myMoveGenerator.get();
    }

    void setupNotify(std::atomic<int>* gameId);

    auto moveFromCoordinates(int srcRow, int srcColumn,
                             int dstRow, int dstColumn, std::optional<wisdom::Piece> promoted)
        -> std::pair<std::optional<wisdom::Move>, wisdom::Color>;
private:
    std::unique_ptr<wisdom::Game> myEngine;
    std::unique_ptr<wisdom::MoveGenerator> myMoveGenerator =
            std::make_unique<wisdom::MoveGenerator>();
};

#endif // CHESSGAME_H
