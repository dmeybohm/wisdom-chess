#include "analytics_sqlite.hpp"
#include "uuid.hpp"
#include "board.hpp"

#include <sqlite3.h>
#include <utility>
#include <iostream>

namespace wisdom::analysis
{
    using SearchId = Uuid;
    using DecisionId = Uuid;
    using PositionId = Uuid;

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

        void exec (const std::string &str)
        {
            exec (str.c_str());
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
    };

    static void init_schema (SqliteHandle &db)
    {
        db.exec (
                "CREATE TABLE IF NOT EXISTS searches ("
                "    id char(36) PRIMARY KEY, "
                "    fen varchar(50),  "
                "    created DATETIME "
                " )"
        );
        db.exec (
                "CREATE TABLE IF NOT EXISTS decisions ("
                "    id char(36) PRIMARY KEY, "
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
                my_move { move }
        {}

        ~SqlitePosition () override
        {
            std::string insert = std::string("INSERT INTO positions (id, decision_id, move, score) VALUES (") +
                    my_position_id.to_string() + "," +
                    my_decision_id.to_string () + "," +
                    "'" + wisdom::to_string (my_move) + "'," +
                    "0" +
                    ")";
            my_handle.exec (insert.c_str ());
        }

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
        SqliteDecision (SqliteHandle &handle, const Board &board, SearchId &search_id) :
                my_handle { handle },
                my_board { board },
                my_search_id { search_id },
                my_decision_id {}
        {}

        ~SqliteDecision () override
        {
            std::string query =
                    "INSERT INTO decisions (id, search_id, parent_position_id, parent_decision_id, move) "
                    " VALUES ("
                    + my_decision_id.to_string() + ","
                    + my_search_id.to_string() + ", NULL, NULL, NULL )";
            my_handle.exec (query.c_str ());
        }

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
        SqliteSearch (SqliteHandle handle, const Board &board, Color side);

        ~SqliteSearch () override
        {
            my_handle.exec ("INSERT INTO searches (id, fen, created) VALUES (" +
                my_search_id.to_string() +
                "," +
                "'" + my_fen + "'" + "," + "datetime('now'))"
            );
        }

        SqliteSearch (const SqliteSearch &) = delete;

        SqliteSearch &operator= (const SqliteSearch &) = delete;

        std::unique_ptr<Decision> make_decision () override
        {
            return std::make_unique<SqliteDecision> (my_handle, my_board, my_search_id);
        }

    private:
        SqliteHandle my_handle;
        const Board &my_board;
        std::string my_fen;
        SearchId my_search_id;
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

        std::unique_ptr<Search> make_search (const Board &board, Color turn) override
        {
            return std::make_unique<SqliteSearch> (SqliteHandle { open () }, board, turn);
        }

    private:
        std::string my_file_path;
    };

    SqliteSearch::SqliteSearch (SqliteHandle handle, const Board &board, Color side) :
            my_handle { std::move (handle) },
            my_board { board },
            my_fen { board.to_fen_string (side) },
            my_search_id {}
    {}

    std::unique_ptr<Analytics> make_sqlite_analytics (const std::string &analytics_file)
    {
        return std::make_unique<SqliteAnalytics> (analytics_file);
    }

}