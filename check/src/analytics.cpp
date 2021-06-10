#include "analytics.hpp"

#include <sqlite3.h>

namespace wisdom
{
    // destructors for interfaces:
    AnalyzedPosition::~AnalyzedPosition ()
    {}
    AnalyzedDecision::~AnalyzedDecision ()
    {}
    Analytics::~Analytics ()
    {}

    class AnalyzedPositionImpl : public AnalyzedPosition
    {
    public:
        ~AnalyzedPositionImpl () override = default;

        void set_data (ScoredMove move, VariationGlimpse& variation_glimpse,
                       int depth, int alpha, int beta) override
        {
            // todo
        }
    };

    class AnalyzedDecisionImpl : public AnalyzedDecision
    {
    public:
        ~AnalyzedDecisionImpl () override = default;

        void add_possibility (AnalyzedPosition &position) override
        {
            // todo
        }

        std::unique_ptr<AnalyzedPosition> make_position (AnalyzedDecision& decision,
                                                         AnalyzedPosition* parent) override
        {
            return std::make_unique<AnalyzedPositionImpl> ();
        }

        void choose_position (AnalyzedPosition& position) override
        {
            // todo
        }
    };

    class AnalyticsImpl : public Analytics
    {
    public:
        AnalyticsImpl(Game& game, const std::string& analytics_file) :
            my_game { game },
            my_analytics_file { analytics_file }
        {
        }

        ~AnalyticsImpl () override = default;

        std::unique_ptr<AnalyzedDecision> make_decision () override
        {
            return std::make_unique<AnalyzedDecisionImpl> ();
        }

    private:
        Game &my_game;
        std::string my_analytics_file;

    };

    std::unique_ptr<Analytics> make_analytics (Game& game, const std::string& analytics_file)
    {
        return std::make_unique<AnalyticsImpl> (game, analytics_file);
    }

}