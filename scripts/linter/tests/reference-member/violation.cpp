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

// An earlier declarator's initializer does not hide a later reference.
struct Initializers
{
    int first = 0, &second;
    int third { 0 }, &fourth;
    int fifth = max (1, 2), &sixth;
    int seventh = Row_Mask & Column_Mask, &eighth;
    int ninth = 0, (&tenth)[2];
};

// A decltype is a type, and a bit-field's width does not hide a later
// declarator.
struct Decltype
{
    decltype (Width)& fifth;
    decltype (Width) const& sixth;
    unsigned flags : Width & Mask, &seventh;
};
