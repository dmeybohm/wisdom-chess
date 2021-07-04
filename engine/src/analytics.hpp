#ifndef WISDOM_ANALYTICS_HPP
#define WISDOM_ANALYTICS_HPP

#include "global.hpp"
#include "move.hpp"
#include "search_result.hpp"

namespace wisdom
{
    struct RelativeTransposition;
}

namespace wisdom::analysis
{
    class Analytics;

    class IterativeSearch;

    class Iteration;

    class Decision;

    class Position;

    class Search;

    class IterativeSearch
    {
    public:
        virtual ~IterativeSearch() = 0;

        virtual std::unique_ptr<Iteration> make_iteration (int depth) = 0;
    };

    class Iteration
    {
    public:
        virtual ~Iteration() = 0;

        virtual std::unique_ptr<Search> make_search () = 0;
    };

    class Search
    {
    public:
        virtual ~Search() = 0;

        virtual std::unique_ptr<Decision> make_decision () = 0;
    };

    class Decision
    {
    public:
        virtual ~Decision () = 0;

        virtual std::unique_ptr<Decision> make_child (Position *position) = 0;

        virtual std::unique_ptr<Position> make_position (Move move) = 0;

        virtual void finalize (const SearchResult &result) = 0;

        virtual void preliminary_choice (Position *position) = 0;
    };

    class Position
    {
    public:
        virtual ~Position () = 0;

        virtual void finalize (const SearchResult &result) = 0;

        virtual void store_transposition_hit (const RelativeTransposition &relative_transposition) = 0;
    };

    class Analytics
    {
    public:
        virtual ~Analytics () = 0;

        virtual std::unique_ptr<IterativeSearch> make_iterative_search (
                [[maybe_unused]] const Board &board,
                [[maybe_unused]] Color turn
        ) = 0;
    };

    class DummyPosition : public Position
    {
    public:
        ~DummyPosition () override = default;

        void finalize ([[maybe_unused]] const SearchResult &result) override
        {}

        void store_transposition_hit ([[maybe_unused]] const RelativeTransposition &relative_transposition) override
        {}
    };

    class DummyDecision : public Decision
    {
    public:
        ~DummyDecision () override = default;

        std::unique_ptr<Position> make_position ([[maybe_unused]] Move move) override
        {
            return std::make_unique<DummyPosition> ();
        }

        void finalize ([[maybe_unused]] const SearchResult &result) override
        {}

        std::unique_ptr<Decision> make_child ([[maybe_unused]] Position *position) override
        {
            return std::make_unique<DummyDecision> ();
        }

        void preliminary_choice ([[maybe_unused]] Position *position) override
        {}
    };

    class DummySearch : public Search
    {
    public:
        ~DummySearch () override = default;

        std::unique_ptr<Decision> make_decision () override
        {
            return std::make_unique<DummyDecision> ();
        }
    };

    class DummyIteration : public Iteration
    {
    public:
        ~DummyIteration() override = default;

        std::unique_ptr<Search> make_search () override
        {
            return std::make_unique<DummySearch> ();
        }
    };

    class DummyIterativeSearch : public IterativeSearch
    {
    public:
        ~DummyIterativeSearch () override = default;

        std::unique_ptr<Iteration> make_iteration ([[maybe_unused]] int depth) override
        {
            return std::make_unique<DummyIteration> ();
        }
    };

    class DummyAnalytics : public Analytics
    {
    public:
        ~DummyAnalytics () override = default;

        static Analytics *get_analytics ()
        {
            static DummyAnalytics dummy_analytics;
            return &dummy_analytics;
        }

        std::unique_ptr<IterativeSearch> make_iterative_search (
                [[maybe_unused]] const Board &board,
                [[maybe_unused]] Color turn) override
        {
            return std::make_unique<DummyIterativeSearch> ();
        }
    };

    Analytics *make_dummy_analytics ();
}

#endif //WISDOM_ANALYTICS_HPP
