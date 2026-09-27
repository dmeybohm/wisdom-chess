// A raw string in a directive is read whole, like one in code: a newline
// inside it does not end the directive, nor does a quote or "//".
#define FOO R"(first
int *ptr;)"
#define BAR R"(a " // not a comment)"
#define BAZ u8R"x(a )" too)x"
int after;
