// Comments and literals are not calls, and a comment alone between the
// parentheses counts as an argument.
void example()
{
    int limit = 100'000; // a comment after a digit separator: call(x)
    reset (/* only a comment */);
    auto text = R"(multiply(x)
over two lines)";
    bad(x);
}
