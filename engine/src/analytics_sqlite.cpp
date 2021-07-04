#include "analytics_sqlite.hpp"
#include "uuid.hpp"
#include "board.hpp"
#include "logger.hpp"
#include "transposition_table.hpp"

#include <sqlite3.h>
#include <utility>
#include <iostream>

namespace wisdom::analysis
{
    using SearchId = Uuid;
    using DecisionId = Uuid;
    using PositionId = Uuid;
    using IterativeSearchId = Uuid;
    using IterationId = Uuid;

    class SqliteAnalytics;

    constexpr int Max_Queries = 10000;

    class SqliteHandle
    {
    private:
        sqlite3 *my_sqlite;
        Logger &my_logger;
        bool my_in_transaction = false;
        int my_queries_in_transaction = 0;

    public:
        void commit_transaction ()
        {
            if (!my_in_transaction)
                do_abort ("Not in transaction");

            char *errmsg = nullptr;
            int result = sqlite3_exec (
                    my_sqlite,
                    "COMMIT",
                    nullptr,
                    nullptr,
                    &errmsg
            );
            if (result != SQLITE_OK || errmsg != nullptr)
                do_abort (errmsg);

            my_in_transaction = false;
            my_logger.println ("Commit " + std::to_string (my_queries_in_transaction) + " queries");
            my_queries_in_transaction = 0;
        }

        explicit SqliteHandle (const std::string &path, Logger &logger) :
            my_sqlite { nullptr },
            my_logger { logger }
        {
            int result = sqlite3_open (path.c_str (), &my_sqlite);
            if (result != SQLITE_OK)
            {
                std::string err { "Error opening sqlite: " };
                err += sqlite3_errmsg (my_sqlite);
                do_abort (err.c_str());
            }
        }

        ~SqliteHandle ()
        {
            if (my_sqlite == nullptr)
                return;
            if (my_in_transaction)
                commit_transaction ();
            int result = sqlite3_close (my_sqlite);
            if (result != SQLITE_OK)
                do_abort ("sqlite3_close failed");
        }

        SqliteHandle (const SqliteHandle &) = delete;

        SqliteHandle &operator= (const SqliteHandle &) = delete;

        SqliteHandle (SqliteHandle &&other) noexcept :
            my_logger { other.my_logger }
        {
            this->my_sqlite = other.my_sqlite;
            this->my_in_transaction = other.my_sqlite;
            this->my_queries_in_transaction = other.my_queries_in_transaction;

            other.my_sqlite = nullptr;
            other.my_in_transaction = false;
            other.my_queries_in_transaction = 0;
        }

        void exec (const std::string &str)
        {
            exec (str.c_str());
        }

        [[noreturn]] static void do_abort (const char *errmsg)
        {
            std::cerr << errmsg << "\n";
            std::terminate ();
        }

        void exec (const char *query) // NOLINT(readability-make-member-function-const)
        {
            if (my_queries_in_transaction >= Max_Queries)
                commit_transaction ();

            my_queries_in_transaction++;
            if (!my_in_transaction)
                start_transaction ();

            char *errmsg = nullptr;
            sqlite3_exec (
                    my_sqlite,
                    query,
                    nullptr,
                    nullptr,
                    &errmsg
            );
            if (errmsg != nullptr)
                do_abort (errmsg);
        }

        void start_transaction ()
        {
            char *errmsg = nullptr;
            if (my_in_transaction)
                return;
            int result = sqlite3_exec (
                    my_sqlite,
                    "BEGIN TRANSACTION",
                    nullptr,
                    nullptr,
                    &errmsg
            );
            if (result != SQLITE_OK || errmsg != nullptr)
                do_abort (errmsg);
            my_in_transaction = true;
        }
    };

    static void init_schema (SqliteHandle &db)
    {
        db.exec (
                "CREATE TABLE IF NOT EXISTS iterative_searches ("
                "    id INT PRIMARY KEY, "
                "    color INT NOT NULL, "
                "    fen varchar(50),  "
                "    created DATETIME "
                " )"
        );

        db.exec (
                "CREATE TABLE IF NOT EXISTS iterations ("
                "    id INT PRIMARY KEY, "
                "    iterative_search_id INT, "
                "    depth INT NOT NULL, "
                "    created DATETIME "
                " )"
        );

        db.exec (
                "CREATE TABLE IF NOT EXISTS searches ("
                "    id INT PRIMARY KEY, "
                "    iteration_id INT, "
                "    depth INT NOT NULL, "
                "    created DATETIME "
                " )"
        );

        db.exec (
                "CREATE TABLE IF NOT EXISTS decisions ("
                "    id INT PRIMARY KEY, "
                "    depth int NOT NULL, "
                "    move varchar(10),   "
                "    search_id INT NOT NULL, "
                "    chosen_position_id INT, "
                "    parent_position_id INT, "
                "    parent_decision_id INT "
                " )"
        );
        db.exec (
                "CREATE TABLE IF NOT EXISTS positions ( "
                "   id INT PRIMARY KEY, "
                "   decision_id INT NOT NULL,"
                "   move varchar(10), "
                "   score int "
                ")"
        );
        db.exec(
                "CREATE TABLE IF NOT EXISTS transposition_hits ( "
                "  id INT PRIMARY KEY, "
                "  position_id INT NOT NULL, "
                "  decision_id INT NOT NULL, "
                "  variation VARCHAR(70) "
                ")"
        );
    }

    class SqlitePosition : public Position
    {
    private:
        std::shared_ptr<SqliteHandle> my_handle;
        [[maybe_unused]] const Board &my_board;
        SearchId my_search_id;
        DecisionId my_decision_id;
        PositionId my_position_id;
        Move my_move;
        int my_score;

    public:
        SqlitePosition (
                std::shared_ptr<SqliteHandle> handle,
                const Board &board,
                SearchId &search_id,
                DecisionId &decision_id,
                Move move
        ) :
                my_handle { std::move(handle) },
                my_board { board },
                my_search_id { search_id },
                my_decision_id { decision_id },
                my_position_id { },
                my_move { move },
                my_score { Negative_Infinity }
        {}

        ~SqlitePosition () override
        {
            std::string insert = std::string("INSERT INTO positions (id, decision_id, move, score) VALUES (") +
                    my_position_id.to_string() + "," +
                    my_decision_id.to_string () + "," +
                    "'" + wisdom::to_string (my_move) + "'," +
                    std::to_string (my_score) +
                    ")";
            my_handle->exec (insert.c_str ());
        }

        SqlitePosition (const SqlitePosition &) = delete;

        SqlitePosition &operator= (const SqlitePosition &) = delete;

        [[nodiscard]] PositionId id () const noexcept
        {
            return my_position_id;
        }

        void finalize ([[maybe_unused]] const SearchResult &result) override
        {
            my_score = result.score;
        }

        void store_transposition_hit (const RelativeTransposition &relative_transposition) override
        {
            Uuid transposition_hit_id;
            std::string query = "INSERT INTO transposition_hits (id, position_id, decision_id, variation) VALUES (";

            query += transposition_hit_id.to_string () + "," +
                    my_position_id.to_string () + "," +
                    my_decision_id.to_string () + "," +
                    "'" + relative_transposition.variation_glimpse.to_string() + "'" +
            ")";

            my_handle->exec(query);
        }
    };

    class SqliteDecision : public Decision
    {
    private:
        std::shared_ptr<SqliteHandle> my_handle;
        const Board &my_board;
        SearchId my_search_id;
        DecisionId my_decision_id;
        DecisionId my_parent_id;
        std::optional<Move> my_move;
        int my_depth;
        int my_score = Negative_Infinity;
        PositionId my_parent_position_id = Uuid::Nil();

    public:
        SqliteDecision (
                std::shared_ptr<SqliteHandle> handle,
                const Board &board,
                const SearchId &search_id,
                const DecisionId &parent_id,
                int depth
        ) :
                my_handle { std::move(handle) },
                my_board { board },
                my_search_id { search_id },
                my_decision_id {},
                my_parent_id { parent_id },
                my_depth { depth }
        {}

        ~SqliteDecision () override
        {
            std::string parent_id_str = my_parent_id == Uuid::Nil()
                    ? "NULL" : my_parent_id.to_string();
            std::string move_str = my_move.has_value() ?
                    "'" + to_string (*my_move) + "'":
                    "NULL";
            std::string parent_pos_id_str = my_parent_position_id == Uuid::Nil()
                    ? "NULL" : my_parent_position_id.to_string ();
            std::string query =
                    "INSERT INTO decisions (id, search_id, parent_position_id, parent_decision_id, depth, move) "
                    " VALUES ("
                    + my_decision_id.to_string() + ","
                    + my_search_id.to_string() + ","
                    + parent_pos_id_str + ", "
                    + parent_id_str + ", "
                    + std::to_string (my_depth) + ","
                    + move_str + ")";
            my_handle->exec (query.c_str ());
        }

        SqliteDecision (const SqliteDecision &) = delete;

        SqliteDecision &operator= (const SqliteDecision &) = delete;

        std::unique_ptr<Position> make_position ([[maybe_unused]] Move move) override
        {
            return std::make_unique<SqlitePosition> (my_handle, my_board, my_search_id, my_decision_id,
                                                     move);
        }

        void finalize ([[maybe_unused]] const SearchResult &result) override
        {
            my_move = result.move;
            my_score = result.score;
        }

        void set_parent_position_id (PositionId parent_position_id)
        {
            my_parent_position_id = parent_position_id;
        }

        std::unique_ptr<Decision> make_child ([[maybe_unused]] Position *position) override
        {
            auto result = std::make_unique<SqliteDecision> (my_handle, my_board, my_search_id, my_decision_id,
                                                     my_depth + 1);
            const auto &parent_position = dynamic_cast<SqlitePosition&> (*position);
            result->set_parent_position_id (parent_position.id ());
            return result;
        }

        void preliminary_choice ([[maybe_unused]] Position *position) override
        {}
    };

    class SqliteSearch : public Search
    {
    private:
        std::shared_ptr<SqliteHandle> my_handle;
        const Board &my_board;
        IterationId my_iteration_id;
        SearchId my_search_id;
        int my_depth;

    public:
        SqliteSearch (std::shared_ptr<SqliteHandle> handle, const Board &board, IterationId iteration_id, int depth);

        ~SqliteSearch () override
        {
            my_handle->exec ("INSERT INTO searches (id, iteration_id, depth, created) VALUES (" +
                my_search_id.to_string () + "," +
                my_iteration_id.to_string () + "," +
                std::to_string (my_depth) + "," +
                "datetime('now'))"
            );
        }

        SqliteSearch (const SqliteSearch &) = delete;

        SqliteSearch &operator= (const SqliteSearch &) = delete;

        std::unique_ptr<Decision> make_decision () override
        {
            return std::make_unique<SqliteDecision> (my_handle, my_board, my_search_id, Uuid::Nil(), 0);
        }
    };

    class SqliteIteration : public Iteration
    {
    private:
        std::shared_ptr<SqliteHandle> my_handle;
        const Board &my_board;
        IterationId my_iteration_id {};
        IterativeSearchId my_iterative_search_id;
        int my_depth;

    public:
        SqliteIteration (
                std::shared_ptr<SqliteHandle> handle,
                const Board &board,
                const IterativeSearchId &iterative_search_id,
                int depth
        ) :
            my_handle { std::move(handle) },
            my_board { board },
            my_iterative_search_id { iterative_search_id },
            my_depth { depth }
        {}

        ~SqliteIteration () override
        {
            my_handle->exec (
                    "INSERT INTO iterations (id, iterative_search_id, depth, created) VALUES (" +
                    my_iteration_id.to_string () + "," +
                    my_iterative_search_id.to_string () + "," +
                    std::to_string (my_depth) + "," +
                    "datetime('now'))"
            );
        }

        std::unique_ptr<Search> make_search () override
        {
            return std::make_unique<SqliteSearch> (my_handle, my_board, my_iteration_id, my_depth);
        }
    };

    class SqliteIterativeSearch : public IterativeSearch
    {
    private:
        std::shared_ptr<SqliteHandle> my_handle;
        const Board &my_board;
        Color my_turn;
        std::string my_fen;
        IterativeSearchId my_iterative_search_id;

    public:
        SqliteIterativeSearch (std::shared_ptr<SqliteHandle> handle, const Board &board, Color turn) :
            my_handle { std::move(handle) },
            my_board { board },
            my_turn { turn },
            my_fen { board.to_fen_string (turn) }
        {}

        ~SqliteIterativeSearch () override
        {
            auto index = color_index (my_turn);
            my_handle->exec (
                    "INSERT INTO iterative_searches (id, fen, color, created) VALUES (" +
                        my_iterative_search_id.to_string() + "," +
                        "'" + my_fen + "'" + "," +
                        std::to_string (index) + "," +
                        "datetime('now'))"
            );
        }

        std::unique_ptr<Iteration> make_iteration (int depth) override
        {
            return std::make_unique<SqliteIteration> (my_handle, my_board, my_iterative_search_id, depth);
        }
    };

    class SqliteAnalytics : public Analytics
    {
    private:
        std::string my_file_path;
        Logger &my_logger;

    public:
        explicit SqliteAnalytics (std::string file_path, Logger &logger) :
            my_file_path { std::move (file_path) },
            my_logger { logger }
        {}

        std::shared_ptr<SqliteHandle> open ()
        {
            auto handle = std::make_shared<SqliteHandle> (my_file_path, my_logger);
            init_schema (*handle);
            return handle;
        }

        ~SqliteAnalytics () override = default;

        SqliteAnalytics (const SqliteAnalytics &) = delete;

        SqliteAnalytics &operator= (const SqliteAnalytics &) = delete;

        std::unique_ptr<IterativeSearch> make_iterative_search (const Board &board, Color turn) override
        {
            return std::make_unique<SqliteIterativeSearch> ( open (), board, turn);
        }
    };

    SqliteSearch::SqliteSearch (std::shared_ptr<SqliteHandle> handle, const Board &board, IterationId iteration_id, int depth) :
            my_handle { std::move(handle) },
            my_board { board },
            my_iteration_id { iteration_id },
            my_search_id {},
            my_depth { depth }
    {}

    std::unique_ptr<Analytics> make_sqlite_analytics (const std::string &analytics_file, Logger &logger)
    {
        return std::make_unique<SqliteAnalytics> (analytics_file, logger);
    }
}