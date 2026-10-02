auto outer (size_t table_size) -> void
{
    // Declarations of functions.
    char first (int);
    CustomType second (int value);
    std::string third();
    std::string fourth (const Board& board, int depth);
    CustomType fifth (Board board);
    CustomType sixth (std::vector<int> values);
    CustomType seventh (Board& board);
    CustomType eighth (Board*); // lint-allow(raw-pointer): an unnamed pointer parameter
    std::vector<int>::iterator ninth (std::vector<int>& values);

    // Variables.
    Buckets buckets (table_size);
    std::vector<size_t> counts (table_size, 0);
    std::string text (count, 'a');
    std::string copy (other.name());
    std::unique_ptr<Board> board (new Board);
    CustomType scaled (table_size * 2);
    CustomType masked (table_size & mask);
    CustomType compared (table_size < limit);
    CustomType negated (not ready);
    CustomType both (ready and willing);
    CustomType converted (static_cast<int> (table_size));
    CustomType moved (std::move (other));
    CustomType sized (sizeof table_size);
    std::thread worker ([this] (int value)
    {
        run (value);
    });
}
