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
