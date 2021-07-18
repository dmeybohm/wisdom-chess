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
    class PositionImpl
    {
    public:
       virtual ~PositionImpl() = default;

       virtual void finalize ([[maybe_unused]] const SearchResult &result) = 0;

       virtual void store_transposition_hit (const RelativeTransposition &relative_transposition) = 0;
    };

    class Position
    {
    protected:
        std::unique_ptr<PositionImpl> impl = nullptr;

    public:
        Position () = default;
        explicit Position (std::unique_ptr<PositionImpl> impl_) : impl { std::move(impl_) }
        {}
        virtual ~Position () = default;

        void finalize ([[maybe_unused]] const SearchResult &result)
        {
            if (impl)
                return impl->finalize (result);
        }

        void store_transposition_hit (const RelativeTransposition &relative_transposition)
        {
            if (impl)
                impl->store_transposition_hit (relative_transposition);
        }

        [[nodiscard]] PositionImpl *get_impl_ptr () const
        {
            return impl.get();
        }
    };

    class DecisionImpl;

    class Decision
    {
    protected:
        std::unique_ptr<DecisionImpl> impl = nullptr;

    public:
        Decision () = default;
        explicit Decision (std::unique_ptr<DecisionImpl> impl_) : impl { std::move(impl_) }
        {}
        virtual ~Decision () = default;

        Decision make_child (Position &position);

        Position make_position (Move move);

        void finalize (const SearchResult &result);

        void preliminary_choice (Position &position);
    };

    class DecisionImpl
    {
    public:
        virtual ~DecisionImpl() = default;

        virtual Decision make_child (Position &position) = 0;

        virtual Position make_position (Move move) = 0;

        virtual void finalize (const SearchResult &result) = 0;

        virtual void preliminary_choice (Position &position) = 0;
    };

    class SearchImpl
    {
    public:
        virtual ~SearchImpl() = default;

        virtual Decision make_decision() = 0;
    };

    class Search
    {
    protected:
        std::unique_ptr<SearchImpl> impl = nullptr;

    public:
        Search () = default;
        explicit Search (std::unique_ptr<SearchImpl> impl_) :
                impl { std::move(impl_) }
        {}
        virtual ~Search() = default;

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
        virtual ~IterationImpl() = default;
        virtual Search make_search() = 0;
    };

    class Iteration
    {
    protected:
        std::unique_ptr<IterationImpl> impl = nullptr;

    public:
        Iteration () = default;
        explicit Iteration (std::unique_ptr<IterationImpl> impl_) : impl { std::move(impl_) }
        {}
        virtual ~Iteration() = default;

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
        virtual ~IterativeSearchImpl() = default;
        virtual Iteration make_iteration (int depth) = 0;
    };

    class IterativeSearch
    {
    protected:
        std::unique_ptr<IterativeSearchImpl> impl = nullptr;

    public:
        IterativeSearch () = default;
        explicit IterativeSearch (std::unique_ptr<IterativeSearchImpl> impl_ ) : impl { std::move(impl_) }
        {}

        virtual ~IterativeSearch() = default;

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
        virtual ~AnalyticsImpl() = default;

        virtual IterativeSearch make_iterative_search (const Board &board, Color turn) = 0;
    };

    class Analytics
    {
    protected:
        std::unique_ptr<AnalyticsImpl> impl = nullptr;

    public:
        Analytics () = default;
        explicit Analytics (std::unique_ptr<AnalyticsImpl> impl_) : impl { std::move(impl_) }
        {}

        virtual ~Analytics () = default;

        IterativeSearch make_iterative_search (const Board &board, Color turn)
        {
            if (impl)
                return impl->make_iterative_search (board, turn);
            else
                return {};
        }
    };

    Analytics &make_dummy_analytics ();
}

#endif //WISDOM_ANALYTICS_HPP
