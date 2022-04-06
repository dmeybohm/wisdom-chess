#ifndef CHESSGAME_H
#define CHESSGAME_H

#include <mutex>

#include "game.hpp"

class ChessGame
{
public:
    ChessGame(std::unique_ptr<wisdom::Game> game)
        : myGame { std::move(game) }
    {
    }

    class ChessGameHandle
    {
    public:
        ChessGameHandle(ChessGame& game)
            : myLockGuard(game.mutex()),
              myGame(game.myGame.get())
        {}

        auto operator->() -> gsl::not_null<wisdom::Game*>
        {
            return myGame;
        }

    private:
        std::lock_guard<std::mutex> myLockGuard;
        gsl::not_null<wisdom::Game*> myGame;
    };

    auto mutex() -> std::mutex&
    {
        return myMutex;
    }

    auto access() -> ChessGameHandle
    {
        return ChessGameHandle { *this };
    }

private:
    std::mutex myMutex;
    std::unique_ptr<wisdom::Game> myGame;
};


#endif // CHESSGAME_H
