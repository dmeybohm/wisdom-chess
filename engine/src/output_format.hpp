#ifndef WISDOM_OUTPUT_FORMAT_HPP
#define WISDOM_OUTPUT_FORMAT_HPP

#include "global.hpp"
#include "piece.hpp"

namespace wisdom
{
    class History;
    class Board;

    class OutputFormat
    {
    public:
        virtual void save (const string &filename, const Board &board, const History &history, Color turn) = 0;
    };

    class FenOutputFormat : public OutputFormat
    {
    public:
        void save (const string &filename, const Board &board, const History &history, Color turn) override;
    };

    class WisdomGameOutputFormat : public OutputFormat
    {
    public:
        void save (const string &str, const Board &board, const History &history, Color turn) override;

    };

}


#endif //WISDOM_OUTPUT_FORMAT_HPP
