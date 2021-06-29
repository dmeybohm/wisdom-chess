#include "analytics_sqlite.hpp"
#include "uuid.hpp"
#include "board.hpp"
#include "logger.hpp"

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
                throw Error { "Not in transaction" };

            char *errmsg = nullptr;
            int result = sqlite3_exec (
                    my_sqlite,
                    "COMMIT",
                    nullptr,
                    nullptr,
                    &errmsg
            );
            if (result != SQLITE_OK || errmsg != nullptr)
            {
                std::string error { errmsg };
                sqlite3_free (errmsg);
                throw Error { error };
            }
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
                throw Error (err);
            }
        }

        ~SqliteHandle ()
        {
            if (my_sqlite == nullptr)
                return;
            if (my_in_transaction)
                commit_transaction ();
            int result = sqlite3_close (my_sqlite);
            assert (result == SQLITE_OK);
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

        void exec (const char *query) // NOLINT(readability-make-member-function-const)
        {
            my_queries_in_transaction++;
            if (my_queries_in_transaction >= Max_Queries)
                commit_transaction ();

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
            {
                std::string error { errmsg };
                sqlite3_free (errmsg);
                throw Error { error };
            }
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
            {
                std::string error { errmsg };
                sqlite3_free (errmsg);
                throw Error { error };
            }
            my_in_transaction = true;
        }
    };

    static void init_schema (SqliteHandle &db)
    {
        db.exec (
                "CREATE TABLE IF NOT EXISTS iterative_searches ("
                "    id char(36) PRIMARY KEY, "
                "    fen varchar(50),  "
                "    created DATETIME "
                " )"
        );

        db.exec (
                "CREATE TABLE IF NOT EXISTS iterations ("
                "    id char(36) PRIMARY KEY, "
                "    iterative_search_id char(36), "
                "    created DATETIME "
                " )"
        );

        db.exec (
                "CREATE TABLE IF NOT EXISTS searches ("
                "    id char(36) PRIMARY KEY, "
                "    iteration_id char(36), "
                "    fen varchar(50),  "
                "    depth INT NOT NULL, "
                "    created DATETIME "
                " )"
        );

        db.exec (
                "CREATE TABLE IF NOT EXISTS decisions ("
                "    id char(36) PRIMARY KEY, "
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
                "   id char(36) PRIMARY KEY, "
                "   decision_id INT NOT NULL,"
                "   move varchar(10), "
                "   score int "
                ")"
        );
    }

    class SqlitePosition : public Position
    {
    public:
        SqlitePosition (
                SqliteHandle &handle,
                const Board &board,
                SearchId &search_id,
                DecisionId &decision_id,
                Move move) :
                my_handle { handle },
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
            my_handle.exec (insert.c_str ());
        }

        SqlitePosition (const SqlitePosition &) = delete;

        SqlitePosition &operator= (const SqlitePosition &) = delete;

        void finalize ([[maybe_unused]] const SearchResult &result) override
        {
            my_score = result.score;
        }

    private:
        SqliteHandle &my_handle;
        [[maybe_unused]] const Board &my_board;
        SearchId my_search_id;
        DecisionId my_decision_id;
        PositionId my_position_id;
        Move my_move;
        int my_score;
    };

    class SqliteDecision : public Decision
    {
    public:
        SqliteDecision (
                SqliteHandle &handle,
                const Board &board,
                const SearchId &search_id,
                const DecisionId &parent_id,
                int depth) :
                my_handle { handle },
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
            std::string query =
                    "INSERT INTO decisions (id, search_id, parent_position_id, parent_decision_id, depth, move) "
                    " VALUES ("
                    + my_decision_id.to_string() + ","
                    + my_search_id.to_string() + ", NULL, "
                    + parent_id_str + ", "
                    + std::to_string (my_depth) + ","
                    + move_str + ")";
            my_handle.exec (query.c_str ());
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

        std::unique_ptr<Decision> make_child ([[maybe_unused]] Position *position) override
        {
            return std::make_unique<SqliteDecision> (my_handle, my_board, my_search_id, my_decision_id,
                                                     my_depth + 1);
        }

        void preliminary_choice ([[maybe_unused]] Position *position) override
        {}

    private:
        SqliteHandle &my_handle;
        const Board &my_board;
        SearchId my_search_id;
        DecisionId my_decision_id;
        DecisionId my_parent_id;
        std::optional<Move> my_move;
        int my_depth;
        int my_score = Negative_Infinity;
    };

    class SqliteSearch : public Search
    {
    public:
        SqliteSearch (SqliteHandle handle, const Board &board, Color side, int depth);

        ~SqliteSearch () override
        {
            my_handle.exec ("INSERT INTO searches (id, fen, depth, created) VALUES (" +
                my_search_id.to_string() +
                "," +
                "'" + my_fen + "'" + ","
                + std::to_string (my_depth) + ","
                + "datetime('now'))"
            );
        }

        SqliteSearch (const SqliteSearch &) = delete;

        SqliteSearch &operator= (const SqliteSearch &) = delete;

        std::unique_ptr<Decision> make_decision () override
        {
            return std::make_unique<SqliteDecision> (my_handle, my_board, my_search_id, Uuid::Nil(), 0);
        }

    private:
        SqliteHandle my_handle;
        const Board &my_board;
        std::string my_fen;
        SearchId my_search_id;
        int my_depth;
    };

    class SqliteIteration : public Iteration
    {
    private:
        SqliteHandle &my_handle;
        IterationId my_iteration_id {};
        IterativeSearchId my_iterative_search_id;
        int my_depth;

    public:
        SqliteIteration (SqliteHandle &handle, const IterativeSearchId &iterative_search_id, int depth) :
            my_handle { handle },
            my_iterative_search_id { iterative_search_id },
            my_depth { depth }
        {}

        ~SqliteIteration () override
        {
            my_handle.exec ("INSERT INTO iteration (id, iterative_search_id, depth) VALUES (" +
                            my_iteration_id.to_string () + "," +
                            my_iterative_search_id.to_string () +
                            ")"
            );
        }

        std::unique_ptr<Search> make_search () override
        {
            return std::unique_ptr<SqliteSearch> ();
        }
    };

    class SqliteIterativeSearch : public IterativeSearch
    {
    private:
        SqliteHandle my_handle;
        const Board &my_board;
        Color my_turn;
        std::string my_fen;
        IterativeSearchId my_iterative_search_id;

    public:
        SqliteIterativeSearch (SqliteHandle handle, const Board &board, Color turn) :
            my_handle { std::move(handle) },
            my_board { board },
            my_turn { turn },
            my_fen { board.to_fen_string (turn) }
        {}

        ~SqliteIterativeSearch () override
        {
            my_handle.exec ("INSERT INTO iterative_searches (id, fen, depth, created) VALUES (" +
                            my_iterative_search_id.to_string() +
                            "," +
                            "'" + my_fen + "'" + ","
                            + "datetime('now'))"
            );
        }

        std::unique_ptr<Iteration> make_iteration (int depth) override
        {
            return std::make_unique<SqliteIteration> (my_handle, my_iterative_search_id, depth);
        }
    };

    class SqliteAnalytics : public Analytics
    {
    public:
        explicit SqliteAnalytics (std::string file_path, Logger &logger) :
            my_file_path { std::move (file_path) },
            my_logger { logger }
        {}

        SqliteHandle open ()
        {
            SqliteHandle handle { my_file_path, my_logger };
            init_schema (handle);
            return handle;
        }

        ~SqliteAnalytics () override = default;

        SqliteAnalytics (const SqliteAnalytics &) = delete;

        SqliteAnalytics &operator= (const SqliteAnalytics &) = delete;

        std::unique_ptr<IterativeSearch> make_iterative_search (const Board &board, Color turn) override
        {
            return std::make_unique<SqliteIterativeSearch> ( SqliteHandle { open () }, board, turn);
        }

    private:
        std::string my_file_path;
        Logger &my_logger;
    };

    SqliteSearch::SqliteSearch (SqliteHandle handle, const Board &board, Color side, int depth) :
            my_handle { std::move (handle) },
            my_board { board },
            my_fen { board.to_fen_string (side) },
            my_search_id {},
            my_depth { depth }
    {}

    std::unique_ptr<Analytics> make_sqlite_analytics (const std::string &analytics_file, Logger &logger)
    {
        return std::make_unique<SqliteAnalytics> (analytics_file, logger);
    }
}