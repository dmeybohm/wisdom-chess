[[nodiscard]] char getChar()
{
    return 'a';
}

[[nodiscard]] [[gnu::pure]] static CustomType createCustom();

[[nodiscard]] auto getOther() -> char
{
    return 'b';
}

[[noreturn]] void fail()
{
}

[[nodiscard]]
auto getThird() -> char;

[[nodiscard]] static constexpr auto
getFourth()
    -> char;

auto useAttribute (int value) -> int
{
    switch (value)
    {
        case 1:
            ++value;
            [[fallthrough]];
        default:
            return value;
    }
}
