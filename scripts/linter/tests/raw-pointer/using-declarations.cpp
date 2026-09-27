// A using declaration split across lines, with only a comment after the
// keyword, is skipped rather than read past the end of the line.
using /* alias follows */
Count = int;

// "using " inside a longer name is not a using declaration, so it does not
// make "count" a type name.
auto pausing (Pausing count, int total) -> int
{
    return count*total;
}
