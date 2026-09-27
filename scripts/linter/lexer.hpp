#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace wisdom_linter
{

enum class TokenKind
{
    Identifier,
    Number,
    String,
    Character,
    Punctuator,
    Comment,
    Preprocessor,
    Other,
};

struct Token
{
    TokenKind kind;
    std::string text;

    // Where the token starts, 1-based, in bytes.
    int line;
    int column;

    // Whether whitespace, a comment or the start of a line comes before it.
    bool spaced_before;
};

// Splits C++ source into tokens without parsing it. Keywords are
// identifiers, and a preprocessor directive, with its continuation lines,
// is a single token. It never fails: an unrecognized character is a token
// of its own, an unterminated string or character literal ends at the end
// of its line, and an unterminated comment or raw string at the end of the
// source.
[[nodiscard]] auto lex (std::string_view source) -> std::vector<Token>;

[[nodiscard]] auto tokenKindName (TokenKind kind) -> std::string_view;

// The tokens rules read: comments and preprocessor directives left out.
[[nodiscard]] auto codeTokens (const std::vector<Token>& tokens) -> std::vector<Token>;

[[nodiscard]] inline auto isIdentifier (const Token& token, std::string_view text = {}) -> bool
{
    return token.kind == TokenKind::Identifier && (text.empty() || token.text == text);
}

[[nodiscard]] inline auto isPunctuator (const Token& token, std::string_view text) -> bool
{
    return token.kind == TokenKind::Punctuator && token.text == text;
}

// Whether code[index] is the first token on its line.
[[nodiscard]] inline auto startsLine (const std::vector<Token>& code, size_t index) -> bool
{
    return index == 0 || code[index - 1].line != code[index].line;
}

} // namespace wisdom_linter
