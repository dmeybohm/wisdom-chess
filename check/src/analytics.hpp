#ifndef WISDOM_ANALYTICS_HPP
#define WISDOM_ANALYTICS_HPP

#include "global.hpp"
#include "move.hpp"
#include "search.hpp"
#include "variation_glimpse.hpp"

namespace wisdom
{
    class Analytics;
    class AnalyzedDecision;
    class AnalyzedPosition;
    class Game;

    class AnalyzedDecision
    {
    public:
        virtual ~AnalyzedDecision() = 0;
        virtual void add_possibility(AnalyzedPosition &position) = 0;

        virtual std::unique_ptr<AnalyzedPosition> make_position (AnalyzedDecision& decision,
                                                                 AnalyzedPosition* parent) = 0;

        virtual void choose_position (AnalyzedPosition &position) = 0;
    };

    class AnalyzedPosition
    {
    public:
        virtual ~AnalyzedPosition() = 0;
        virtual void set_data(ScoredMove move, VariationGlimpse &variation_glimpse, int depth,
                              int alpha, int beta) = 0;
    };

    class Analytics
    {
    public:
        virtual ~Analytics() = 0;
        virtual std::unique_ptr<AnalyzedDecision> make_decision () = 0;
    };

    std::unique_ptr<Analytics> make_analytics (Game &game, const std::string& analytics_file);
}

#endif //WISDOM_ANALYTICS_HPP
