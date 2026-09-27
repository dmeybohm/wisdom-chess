// Cases the rule gets right by reading tokens rather than lines.
constexpr int Limit = 100'000; // Board* in a comment after a digit separator
auto text = R"(
Board* inside a raw string
)";

auto first()
    -> Board
    *;

auto collect (vector<
    Board>* boards) -> void;

auto names (const char* const* argv) -> void;

// A macro body is skipped, as the rest of a directive is.
#define MAKE_POINTER(name) \
    Board* name = nullptr
