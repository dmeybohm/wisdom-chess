#include <cstdio>

// A Board* in a comment is not code.
class Model
{
public:
    void setGame (nonnull<Game> game);
    [[nodiscard]] auto parent() const -> nullable<Model>;
    auto operator*() const -> Model&;
    static auto create (QQmlEngine* engine, QJSEngine* js) -> Model*; // lint-allow(raw-pointer): QML singleton factory

private:
    QList<QQuickItem*> my_items;
    QObject* my_owner;
};

auto area (int width, int height) -> int
{
    auto* item = findItem();
    int total = width * height;
    total *= 2;
    auto text = "a Board* in a string";
    char quote = '*';
    return total * 2 + 0x1'000;
}
