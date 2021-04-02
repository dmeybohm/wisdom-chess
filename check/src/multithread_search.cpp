#include "multithread_search.hpp"
#include "history.hpp"

namespace wisdom
{
    constexpr int Max_Depth = 16;

    using std::chrono::seconds;
    using wisdom::Logger;

    struct thread_params
    {
        Board board;
        Color side;
        Logger &output;
        History history;
        int depth;
        MoveTimer timer;

        thread_params (const Board &board_, Color side_, Logger &output_,
                       History history_, MoveTimer timer_, int depth_) :
                board { board_ }, side { side_ }, output { output_ },
                history { std::move (history_) }, depth { depth_ },
                timer { timer_ }
        {
        }

    };

    class MultithreadSearchHandler
    {
    public:
        MultithreadSearchHandler (Board &board_, Color side_, Logger &output_,
                                  const History &history_, const MoveTimer &timer_) :
                board { board_ }, side { side_ }, output { output_ },
                history { history_ }, timer { timer_ }
        {}

        SearchResult result ()
        {
            // If already searched, return result.
            if (search_result.move.has_value())
                return search_result;

            search_result = do_multithread_search ();
            return search_result;
        }

    private:
        Board board;
        Color side;
        Logger &output;
        const History &history; // reference here, and copied into thread params.
        struct MoveTimer timer;
        SearchResult search_result;

        // Mutex to protect next_depth
        std::mutex mutex;
        int next_depth = -1;

        // Per thread variables;
        std::vector<thread_params> all_thread_params;
        std::vector<std::thread> threads;
        std::vector<SearchResult> result_moves;

        SearchResult do_multithread_search ();

        int get_next_depth ();

        void do_thread (unsigned index);

        void add_result (const SearchResult& result);
    };

    SearchResult MultithreadSearchHandler::do_multithread_search ()
    {
        unsigned int max_nr_threads = std::thread::hardware_concurrency ();

        next_depth = -1;

        // Create all the thread parameters first, to ensure they get allocated:
        for (unsigned i = 0; i < max_nr_threads; i++)
        {
            thread_params params { board, side, output, history, timer, this->get_next_depth () };
            all_thread_params.push_back (params);
        }

        // Now create the threads:
        for (unsigned i = 0; i < max_nr_threads; i++)
        {
            std::thread thr = std::thread (&MultithreadSearchHandler::do_thread, this, i);
            threads.push_back (std::move (thr));
        }

        // kill all the threads:
        std::this_thread::sleep_for (timer.seconds ());
        for (auto &thr: threads)
        {
            thr.join ();
        }

        // todo handle empty case
        assert (!result_moves.empty ());
        SearchResult result = result_moves.front ();

        for (auto &result_move : result_moves)
        {
            if (result_move.depth > result.depth)
                result = result_move;
        }

        return result;
    }

    int MultithreadSearchHandler::get_next_depth ()
    {
        std::lock_guard guard { mutex };

        int d = next_depth;
        next_depth = d == 0 ? d + 1 : d + 2;
        return next_depth;
    }

    void MultithreadSearchHandler::add_result (const SearchResult& result)
    {
        std::lock_guard guard { mutex };

        result_moves.push_back (result);
    }

    void MultithreadSearchHandler::do_thread (unsigned index)
    {
        thread_params &params = all_thread_params[index];
        std::stringstream messages;

        if (params.depth >= Max_Depth)
            return;

        SearchResult result = iterate (params.board, params.side, params.output,
                               params.history, params.timer, params.depth);
        messages << "Finished thread " << std::this_thread::get_id () << " with depth " << params.depth << "\n";
        params.output.println (messages.str ());

        if (result.timed_out)
        {
            messages << "<Timed out>" << "\n";
            params.output.println (messages.str ());
            return;
        }

        if (result.move.has_value ())
            messages << "Move: " << to_string (*result.move) << "\n";
        else
            messages << "<No move available>" << "\n";

        params.output.println (messages.str ());

        add_result (result);

        // Continue with more depth:
        params.depth = get_next_depth ();
        do_thread (index);
    }

    MultithreadSearch::MultithreadSearch (Board &board, Color side, Logger &output,
                                          const History &history, const MoveTimer &timer)
    {
        handler = std::make_unique<MultithreadSearchHandler> (board, side, output, history, timer);
    }

    SearchResult MultithreadSearch::search ()
    {
        if (result.move.has_value ())
            result = handler->result ();

        return result;
    }

    MultithreadSearch::~MultithreadSearch () = default;
}