// A using declaration inside a string literal is not one, so it does not
// make "count" a type name.
std::string note = "using count = int;";

auto total_of (int count, int total) -> int
{
    return count*total;
}
