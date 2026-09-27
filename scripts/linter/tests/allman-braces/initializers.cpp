Foo::Foo (int a)
    : my_a {
        a,
    }
    , my_b {
        [] (int x)
        {
            return x;
        },
    }
{
}

auto make() -> Record
{
    Record record {
        1,
    };
    std::array<int, 2> pair {
        1, 2,
    };
    call (Foo {
        1,
    });
    if (record.empty())
    {
        throw Error {
            "empty",
        };
    }
    return {
        1,
    };
}

template <typename P>
concept Dereferenceable = requires (P p) { *p; } || requires { P::value; };

template <typename P>
concept Multiline = requires (P p) {
    *p;
};

int values[2] { 1, 2 };
char text[] { 'a', 'b' };

void arrays()
{
    int local[2] { 1, 2 };
    int grid[2][2] {
        { 1, 2 },
        { 3, 4 },
    };
    static constexpr int sizes[Num_Players] {
        1, 2,
    };
}
