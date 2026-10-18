// References that end with a call or a scope are fine; only a data member
// may not hold one.
struct MoveGeneration
{
    nonnull<const Board> board;
    nonnull<MoveList> moves;
    int piece_row;

    MoveGeneration (const Board& board, nonnull<MoveList> moves)
        : board { &board }
        , moves { moves }
    {
    }

    [[nodiscard]] auto
    first() const -> const Move&;

    auto operator*() const noexcept -> Board&;
    auto operator& (const MoveGeneration& other) const -> int;
    friend auto operator<< (std::ostream& out, const MoveGeneration& g) -> std::ostream&;

    using BoardRef = const Board&;
    typedef Board& LegacyRef;

    template <typename T, T& ref>
    void bind();

    int mask = Row_Mask & Column_Mask;
    int flags { Row_Mask & Column_Mask };
    std::function<void()> callback = [&] { run(); };

    void run()
    {
        const Board& local = *board;
        Board& other = local;
        for (auto& move : *moves)
            use (move);
    }
};

enum class Flags
{
    Both = 1 & 3,
};

auto count (const Board& board, Color who) -> int;

auto total (int a, int b) -> int
{
    const int& ref = a;
    return ref & b;
}

const Board& Global_Board = board;

struct Declarators
{
    int values[3], count;
    int (*callback) (int); // lint-allow(raw-pointer): fixture
    nonnull<const Board> (&boards)[2]; // lint-allow(reference-member): fixture of the marker
};

// An '&' inside an initializer expression is not a declarator.
struct Initializers
{
    int wide = Row_Mask & Column_Mask, narrow = 0;
    int total = first & second, other = third & fourth;
    int masked = count & Row_Mask;
    int chosen = pick (first & second, third & fourth);
    bool both = first && second, either = false;
};
