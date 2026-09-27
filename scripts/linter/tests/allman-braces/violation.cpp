struct Point {
    int x;
};

enum Kind {
    One,
};

class Widget : public Base {
};

auto Widget::run (
    int first,
    int second
) const noexcept {
    if (first) {
        step();
    } else {
        other();
    }
    for (int i = 0; i < second; ++i) {
        while (true) {
            switch (i) {
            }
        }
    }
    try {
        step();
    } catch (...) {
    }
    do {
        step();
    } while (false);
}

auto Widget::size() const -> int {
    return my_size;
}

Widget::Widget()
    : my_size { 0 } {
}

class Counter
{
    [[nodiscard]] auto count() const -> int { return my_count; }
    void reset() override { my_count = 0; }
    [[nodiscard]] static auto limit() -> int { return 10; }
};

void check (bool ok)
{
    if (ok) { return; }
}
