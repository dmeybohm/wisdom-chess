#ifndef CHESSGAME_H
#define CHESSGAME_H

#include <memory>

#include "game.hpp"

class ChessGame
{
public:
    explicit ChessGame(std::unique_ptr<wisdom::Game> game)
        : myGame { std::move(game) }
    {
    }

    auto access() -> gsl::not_null<wisdom::Game*>
    {
        return myGame.get();
    }

    auto moveGenerator() -> gsl::not_null<wisdom::MoveGenerator*>
    {
       return myMoveGenerator.get();
    }

private:
    std::unique_ptr<wisdom::Game> myGame;
    std::unique_ptr<wisdom::MoveGenerator> myMoveGenerator =
            std::make_unique<wisdom::MoveGenerator>();
};

#endif // CHESSGAME_H
