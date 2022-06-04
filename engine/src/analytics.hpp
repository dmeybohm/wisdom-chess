#ifndef WISDOM_CHESS_ANALYTICS_HPP
#define WISDOM_CHESS_ANALYTICS_HPP

#include "global.hpp"
#include "move.hpp"
#include "search_result.hpp"

namespace wisdom
{
    struct RelativeTransposition;
}

namespace wisdom::analysis
{
    class PositionImpl
    {
    public:
        virtual ~PositionImpl () = default;

        virtual void finalize ([[maybe_unused]] int score) = 0;
    };

    class Position
    {
    protected:
        wisdom::unique_ptr<PositionImpl> impl = nullptr;

    public:
        Position () = default;
        explicit Position (wisdom::unique_ptr<PositionImpl> impl_) : impl { std::move (impl_) } { }

        void finalize ([[maybe_unused]] int score)
        {
            if (impl)
                return impl->finalize (score);
        }

        [[nodiscard]] PositionImpl* get_impl_ptr () const { return impl.get (); }
    };

    class DecisionImpl;

    class Decision
    {
    protected:
        wisdom::unique_ptr<DecisionImpl> impl = nullptr;

    public:
        Decision () = default;
        explicit Decision (wisdom::unique_ptr<DecisionImpl> impl_) : impl { std::move (impl_) } { }

        Decision make_child (Position& position);

        Position make_position (Move move);

        void finalize (const SearchResult& result);

        void select_position (Position& position);
    };

    class DecisionImpl
    {
    public:
        virtual ~DecisionImpl () = default;

        virtual Decision make_child (Position& position) = 0;

        virtual Position make_position (Move move) = 0;

        virtual void finalize (const SearchResult& result) = 0;

        virtual void select_position (Position& position) = 0;
    };

    class SearchImpl
    {
    public:
        virtual ~SearchImpl () = default;

        virtual Decision make_decision () = 0;
    };

    class Search
    {
    protected:
        wisdom::unique_ptr<SearchImpl> impl = nullptr;

    public:
        Search () = default;
        explicit Search (wisdom::unique_ptr<SearchImpl> impl_) : impl { std::move (impl_) } { }

        Decision make_decision ()
        {
            if (impl)
                return impl->make_decision ();
            return {};
        }
    };

    class IterationImpl
    {
    public:
        virtual ~IterationImpl () = default;
        virtual Search make_search () = 0;
    };

    class Iteration
    {
    protected:
        wisdom::unique_ptr<IterationImpl> impl = nullptr;

    public:
        Iteration () = default;
        explicit Iteration (wisdom::unique_ptr<IterationImpl> impl_) :
                impl { std::move (impl_) } { }

        Search make_search ()
        {
            if (impl)
                return impl->make_search ();
            else
                return {};
        };
    };

    class IterativeSearchImpl
    {
    public:
        virtual ~IterativeSearchImpl () = default;
        virtual Iteration make_iteration (int depth) = 0;
    };

    class IterativeSearch
    {
    protected:
        wisdom::unique_ptr<IterativeSearchImpl> impl = nullptr;

    public:
        IterativeSearch () = default;
        explicit IterativeSearch (wisdom::unique_ptr<IterativeSearchImpl> impl_) :
                impl { std::move (impl_) }
        {
        }

        virtual ~IterativeSearch () = default;

        Iteration make_iteration (int depth)
        {
            if (impl)
                return impl->make_iteration (depth);
            else
                return {};
        }
    };

    class AnalyticsImpl
    {
    public:
        virtual ~AnalyticsImpl () = default;

        virtual IterativeSearch make_iterative_search (const Board& board, Color turn) = 0;
    };

    class Analytics
    {
    protected:
        wisdom::unique_ptr<AnalyticsImpl> impl = nullptr;

    public:
        Analytics () = default;
        explicit Analytics (wisdom::unique_ptr<AnalyticsImpl> impl_) :
                impl { std::move (impl_) } { }

        IterativeSearch make_iterative_search (const Board& board, Color turn)
        {
            if (impl)
                return impl->make_iterative_search (board, turn);
            else
                return {};
        }
    };

    Analytics& make_dummy_analytics ();
}

#endif // WISDOM_CHESS_ANALYTICS_HPP
