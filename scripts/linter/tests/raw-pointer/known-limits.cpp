// A PascalCase value times a variable reads like a declaration, since the
// rule has only names to go on. Such a line needs lint-allow(raw-pointer).
auto material (int count) -> int
{
    return WeightPawn * count;
}
