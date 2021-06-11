#ifndef WISDOM_ANALYTICS_HPP
#define WISDOM_ANALYTICS_HPP

#include "global.hpp"
#include "move.hpp"
#include "search_result.hpp"

namespace wisdom
{
    class Analytics;

    class AnalyzedDecision;

    class AnalyzedPosition;

    struct Game;

    class VariationGlimpse;

    class AnalyzedDecision
    {
    public:
        virtual ~AnalyzedDecision () = 0;

        virtual std::unique_ptr<AnalyzedDecision> make_child (AnalyzedPosition *position) = 0;

        virtual std::unique_ptr<AnalyzedPosition> make_position (Move move) = 0;

        virtual void finalize (const SearchResult &result) = 0;

        virtual void preliminary_choice (AnalyzedPosition *position) = 0;
    };

    class AnalyzedPosition
    {
    public:
        virtual ~AnalyzedPosition () = 0;

        virtual void set_data ([[maybe_unused]] Move move, 
                               [[maybe_unused]] int score,
                               [[maybe_unused]] const VariationGlimpse &variation_glimpse,
                               [[maybe_unused]] int depth,
                               [[maybe_unused]] int alpha,
                               [[maybe_unused]] int beta) = 0;
    };

    class Analytics
    {
    public:
        virtual ~Analytics () = 0;

        virtual std::unique_ptr<AnalyzedDecision> make_decision (const Board &board) = 0;
    };

    class DummyPosition : public AnalyzedPosition
    {
    public:
        ~DummyPosition () override = default;

        void set_data ([[maybe_unused]] Move move,
                       [[maybe_unused]] int score,
                       [[maybe_unused]] const VariationGlimpse &variation_glimpse,
                       [[maybe_unused]] int depth,
                       [[maybe_unused]] int alpha,
                       [[maybe_unused]] int beta) override
        {
        }
    };

    class DummyDecision : public AnalyzedDecision
    {
    public:
        ~DummyDecision () override = default;

        std::unique_ptr<AnalyzedPosition> make_position ([[maybe_unused]] Move move) override
        {
            return std::make_unique<DummyPosition> ();
        }

        void finalize ([[maybe_unused]] const SearchResult &result) override
        {

        }

        std::unique_ptr<AnalyzedDecision> make_child ([[maybe_unused]] AnalyzedPosition *position) override
        {
            return std::make_unique<DummyDecision> ();
        }

        void preliminary_choice ([[maybe_unused]] AnalyzedPosition *position) override
        {}
    };

    class DummyAnalytics : public Analytics
    {
    public:
        ~DummyAnalytics () override = default;

        std::unique_ptr<AnalyzedDecision> make_decision ([[maybe_unused]] const Board &board) override
        {
            return std::make_unique<DummyDecision> ();
        }
    };

    std::unique_ptr<Analytics> make_dummy_analytics ();

    //    std::unique_ptr<Analytics> make_sqlite_analytics (Game &game, const std::string &analytics_file);
}

#endif //WISDOM_ANALYTICS_HPP
