struct Board;

class Search
{
public:
    void run (Board* board);

    [[nodiscard]] auto
    table()
        -> TranspositionTable*;

private:
    Game* my_game;
    std::vector<Board*> my_boards;
    nonnull<Game>* my_indirect;
};
