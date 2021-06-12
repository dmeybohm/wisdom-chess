#ifndef WISDOM_ANALYTICS_HPP
#define WISDOM_ANALYTICS_HPP

#include "global.hpp"
#include "move.hpp"
#include "search_result.hpp"

namespace wisdom::analysis
{
    class Analytics;

    class Decision;

    class Position;

    class Search;

    class Search
    {
    public:
        virtual ~Search() = 0;

        virtual std::unique_ptr<Decision> make_decision ([[maybe_unused]] const Board &board) = 0;
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
    };

    class Analytics
    {
    public:
        virtual ~Analytics () = 0;

        virtual std::unique_ptr<Search> make_search (const Board &board) = 0;
    };

    class DummyPosition : public Position
    {
    public:
        ~DummyPosition () override = default;

        void finalize ([[maybe_unused]] const SearchResult &result) override
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
        std::unique_ptr<Decision> make_decision ([[maybe_unused]] const Board &board) override
        {
            return std::make_unique<DummyDecision> ();
        }
    };

    class DummyAnalytics : public Analytics
    {
    public:
        ~DummyAnalytics () override = default;

        std::unique_ptr<Search> make_search ([[maybe_unused]] const Board &board) override
        {
            return std::make_unique<DummySearch> ();
        }
    };

    std::unique_ptr<Analytics> make_dummy_analytics ();

    //    std::unique_ptr<Analytics> make_sqlite_analytics (const std::string &analytics_file);
}

#endif //WISDOM_ANALYTICS_HPP
