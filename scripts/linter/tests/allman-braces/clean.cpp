namespace wisdom
{

extern "C" {
void exported();
}

struct Empty {};

class Widget : public Base
{
public:
    void onChanged() override {}
    [[nodiscard]] auto size() const -> int
    {
        return my_size;
    }

private:
    int my_size;
};

enum class Color : int8_t
{
    White,
    Black,
};

auto Widget::run (
    int first,
    int second
) const noexcept -> std::vector<int>
{
    if (first)
    {
        return {};
    }
    if (second) {}
    for (int i = 0; i < second; ++i)
    {
        try
        {
            step();
        }
        catch (...)
        {
        }
    }
    do
    {
        step();
    }
    while (false);
    return {};
}

}
