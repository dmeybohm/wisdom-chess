// Comments become a single space before directives are recognized, as
// "g++ -E" and "clang++ -E" show. A '#' after a comment that spans lines
// is not at the start of a line, so it does not begin a directive:
int x; /* comment
*/ #define CHECK(x) x
// And a directive goes on through a comment that spans lines, so the
// declaration after it belongs to the macro:
#define FOO /* comment
*/ int *ptr;
int after;
