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
