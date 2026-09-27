auto multiply (int a, int b) -> int
{
    return a*b;
}

auto scaled (int col, nonnull<int> weight) -> int
{
    int total = Num_Rows*2 + Num_Rows*col + col * Num_Rows + col*col;
    total *= *weight;
    total = total * *weight;
    if (col > 0)
        return *weight;
    else
        return total *
            Num_Columns;
}

inline constexpr int Max_Score
    = Num_Squares * Weight_Queen *
    Scale;
