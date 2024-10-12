#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/piece.hpp"

namespace wisdom
{
    class History;
    class Board;

    class OutputFormat
    {
    public:
        virtual void save (
            const string& filename, 
            const Board& board, 
            const History& history, 
            Color turn
        ) = 0;

        virtual ~OutputFormat() = default;
    };

    class FenOutputFormat : public OutputFormat
    {
    public:
        void save (
            const string& filename, 
            const Board& board, 
            const History& history, 
            Color turn
        ) override;
    };

    class WisdomGameOutputFormat : public OutputFormat
    {
    public:
        void save (
            const string& str, 
            const Board& board, 
            const History& history, 
            Color turn
        ) override;
    };

}
