struct InlineThreats
{
    const Board& board;
    Color opponent;

    InlineThreats (const Board& board, Color opponent)
        : board { board }
        , opponent { opponent }
    {
    }
};

class Model
{
public:
    explicit Model (std::ostream& stream);

private:
    std::ostream& my_stream;
    vector<Board>& my_boards;
    Board const& my_board;
    Game&& my_game;
    static const Settings& Defaults;
    mutable Logger& my_logger;
};

auto make() -> int
{
    struct Local
    {
        History& history;
    };
    return 0;
}

// Declarators the rule reads past the name of.
struct Declarators
{
    int& first, second;
    int plain, &last;
    int& attributed [[maybe_unused]];
    int (&array)[3];
    [[maybe_unused]] const Board& marked;
    span<const Move> (&lists)[8];
};
