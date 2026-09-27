void sortAll()
{
    std::sort (items.begin(), items.end(), [] (const Item& a, const Item& b) {
        return a < b;
    });
    auto next = [this] () mutable {
        return ++my_count;
    };
    auto typed = [] (int x) -> int {
        return x;
    };
    std::for_each (items.begin(), items.end(), [] {
        step();
    });
}

auto templated = []<typename T>(T value) { return value; };
auto no_throw = []() noexcept -> int { return 1; };
auto changing = []() mutable -> int { return 1; };
auto conditional = [] () noexcept(true) -> int {
    return 1;
};
auto constrained = [] <typename T> (T x) requires std::integral<T> {
    return x;
};
auto compile_time = [] () constexpr {
    return 1;
};
auto stateless = [] () static {
    return 1;
};
auto container = [] () -> std::vector<int> {
    return {};
};

auto makeAdder (int x)
{
    return [x] (int y) {
        return x + y;
    };
}
