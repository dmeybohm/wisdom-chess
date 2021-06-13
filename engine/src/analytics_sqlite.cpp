#include "analytics_sqlite.hpp"

#include <sqlite3.h>
#include <utility>
#include <iostream>

namespace wisdom::analysis
{
    using SearchId = sqlite_int64;
    using DecisionId = sqlite_int64;
    using PositionId = sqlite_int64;

    class SqliteAnalytics;

    class SqliteHandle
    {
    public:
        sqlite3 *my_sqlite;

        explicit SqliteHandle (const std::string &path) : my_sqlite { nullptr }
        {
            int result = sqlite3_open (path.c_str (), &my_sqlite);
            if (result != SQLITE_OK)
            {
                std::cerr << "Error closing: " << sqlite3_errmsg (my_sqlite) << "\n";
                std::terminate ();
            }
        }

        ~SqliteHandle ()
        {
            if (my_sqlite == nullptr)
                return;
            int result = sqlite3_close (my_sqlite);
            if (result != SQLITE_OK)
                std::cerr << "Error closing: " << sqlite3_errmsg (my_sqlite) << "\n";
        }

        SqliteHandle (const SqliteHandle &) = delete;

        SqliteHandle &operator= (const SqliteHandle &) = delete;

        SqliteHandle (SqliteHandle &&other) noexcept
        {
            this->my_sqlite = other.my_sqlite;
            other.my_sqlite = nullptr;
        }

        void exec (const char *query) // NOLINT(readability-make-member-function-const)
        {
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

        [[nodiscard]] sqlite3_int64 last_insert_id () const
        {
            return sqlite3_last_insert_rowid (my_sqlite);
        }
    };

    static void init_schema (SqliteHandle &db)
    {
        db.exec (
                "CREATE TABLE IF NOT EXISTS searches ("
                "    id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "    fen varchar(50),  "
                "    created DATETIME "
                " )"
        );
        db.exec (
                "CREATE TABLE IF NOT EXISTS decisions ("
                "    id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "    move varchar(10),   "
                "    search_id INT NOT NULL, "
                "    chosen_position_id INT, "
                "    parent_position_id INT, "
                "    parent_decision_id INT "
                " )"
        );
        db.exec (
                "CREATE TABLE IF NOT EXISTS positions ( "
                "   id INTEGER PRIMARY KEY AUTOINCREMENT, "
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
                SearchId search_id,
                DecisionId decision_id,
                Move move) :
                my_handle { handle },
                my_board { board },
                my_search_id { search_id },
                my_decision_id { decision_id },
                my_position_id { 0 },
                my_move { move }
        {
            std::string insert = std::string("INSERT INTO positions (decision_id, move, score) VALUES (") +
                    std::to_string (my_decision_id) + "," +
                    "'" + wisdom::to_string (move) + "'," +
                    "0" +
                    ")";

            my_handle.exec (insert.c_str ());
            my_position_id = my_handle.last_insert_id ();
        }

        ~SqlitePosition () override = default;

        SqlitePosition (const SqlitePosition &) = delete;

        SqlitePosition &operator= (const SqlitePosition &) = delete;

        void finalize ([[maybe_unused]] const SearchResult &result) override
        {}

    private:
        SqliteHandle &my_handle;
        const Board &my_board;
        SearchId my_search_id;
        DecisionId my_decision_id;
        PositionId my_position_id;
        Move my_move;
    };

    class SqliteDecision : public Decision
    {
    public:
        SqliteDecision (SqliteHandle &handle, const Board &board, SearchId search_id) :
                my_handle { handle },
                my_board { board },
                my_search_id { search_id }
        {
            std::string query =
                    "INSERT INTO decisions (search_id, parent_position_id, parent_decision_id, move) "
                    " VALUES ("
                    + std::to_string(my_search_id) + ", NULL, NULL, NULL )";
            my_handle.exec (query.c_str ());
            my_decision_id = my_handle.last_insert_id ();
        }

        ~SqliteDecision () override = default;

        SqliteDecision (const SqliteDecision &) = delete;

        SqliteDecision &operator= (const SqliteDecision &) = delete;

        std::unique_ptr<Position> make_position ([[maybe_unused]] Move move) override
        {
            return std::make_unique<SqlitePosition> (my_handle, my_board, my_search_id, my_decision_id, move);
        }

        void finalize ([[maybe_unused]] const SearchResult &result) override
        {}

        std::unique_ptr<Decision> make_child ([[maybe_unused]] Position *position) override
        {
            return std::make_unique<SqliteDecision> (my_handle, my_board, my_search_id);
        }

        void preliminary_choice ([[maybe_unused]] Position *position) override
        {}

    private:
        SqliteHandle &my_handle;
        const Board &my_board;
        SearchId my_search_id;
        DecisionId my_decision_id;
    };

    class SqliteSearch : public Search
    {
    public:
        SqliteSearch (SqliteHandle handle, const Board &board);

        ~SqliteSearch () override = default;

        SqliteSearch (const SqliteSearch &) = delete;

        SqliteSearch &operator= (const SqliteSearch &) = delete;

        std::unique_ptr<Decision> make_decision () override
        {
            return std::make_unique<SqliteDecision> (my_handle, my_board, my_search_id);
        }

    private:
        SqliteHandle my_handle;
        const Board &my_board;
        SearchId my_search_id = 0;
    };

    class SqliteAnalytics : public Analytics
    {
    public:
        explicit SqliteAnalytics (std::string file_path) : my_file_path { std::move (file_path) }
        {
            auto handle = SqliteHandle { file_path };
            init_schema (handle);
        }

        SqliteHandle open ()
        {
            SqliteHandle handle { my_file_path };
            init_schema (handle);
            return std::move (handle);
        }

        ~SqliteAnalytics () override = default;

        SqliteAnalytics (const SqliteAnalytics &) = delete;

        SqliteAnalytics &operator= (const SqliteAnalytics &) = delete;

        std::unique_ptr<Search> make_search ([[maybe_unused]] const Board &board) override
        {
            return std::make_unique<SqliteSearch> (SqliteHandle { open () }, board);
        }

    private:
        std::string my_file_path;
    };

    SqliteSearch::SqliteSearch (SqliteHandle handle, const Board &board) :
            my_handle { std::move (handle) },
            my_board { board }
    {
        my_handle.exec ("INSERT INTO searches (fen, created) VALUES ('todo', datetime('now'))");
        my_search_id = my_handle.last_insert_id ();
    }

    std::unique_ptr<Analytics> make_sqlite_analytics (const std::string &analytics_file)
    {
        return std::make_unique<SqliteAnalytics> (analytics_file);
    }

}