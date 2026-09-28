#include <array>
#include <cctype>

#include "lexer.hpp"

namespace wisdom_linter
{
namespace
{
    auto isIdentifierStart (char c) -> bool
    {
        return std::isalpha (static_cast<unsigned char> (c)) || c == '_';
    }

    auto isIdentifierChar (char c) -> bool
    {
        return std::isalnum (static_cast<unsigned char> (c)) || c == '_';
    }

    auto isDigit (char c) -> bool
    {
        return std::isdigit (static_cast<unsigned char> (c)) != 0;
    }

    // Longest first, so that the first match is the longest one.
    constexpr std::array<std::string_view, 27> Multi_Char_Punctuators {
        "<=>", "->*", "...", "<<=", ">>=",
        "::", "->", ".*", "++", "--", "<<", ">>", "<=", ">=", "==", "!=",
        "&&", "||", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "##",
    };

    constexpr std::string_view Single_Char_Punctuators = "{}[]()<>;:,.?+-*/%^&|~!=#";

    class Lexer
    {
    public:
        explicit Lexer (std::string_view source)
            : my_source { source }
        {
        }

        auto run() -> std::vector<Token>
        {
            std::vector<Token> tokens;
            bool spaced = true;
            bool line_start = true;

            while (my_pos < my_source.size())
            {
                char c = peek();
                if (c == '\n')
                {
                    advance();
                    spaced = true;
                    line_start = true;
                    continue;
                }
                if (c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\v')
                {
                    advance();
                    spaced = true;
                    continue;
                }
                if (c == '\\' && peek (1) == '\n')
                {
                    advance (2);
                    spaced = true;
                    continue;
                }

                Token token { TokenKind::Other, {}, my_line, my_column, spaced };
                size_t start = my_pos;
                token.kind = scan (line_start);
                token.text = std::string { my_source.substr (start, my_pos - start) };

                // A comment separates the tokens around it, and a directive
                // can still follow one at the start of a line.
                spaced = token.kind == TokenKind::Comment;
                if (token.kind != TokenKind::Comment)
                {
                    line_start = false;
                }
                tokens.push_back (std::move (token));
            }

            return tokens;
        }

    private:
        std::string_view my_source;
        size_t my_pos = 0;
        int my_line = 1;
        int my_column = 1;

        [[nodiscard]] auto peek (size_t offset = 0) const -> char
        {
            return my_pos + offset < my_source.size() ? my_source[my_pos + offset] : '\0';
        }

        [[nodiscard]] auto atEnd() const -> bool
        {
            return my_pos >= my_source.size();
        }

        void advance (size_t count = 1)
        {
            for (size_t i = 0; i < count && !atEnd(); ++i)
            {
                if (my_source[my_pos] == '\n')
                {
                    ++my_line;
                    my_column = 1;
                }
                else
                {
                    ++my_column;
                }
                ++my_pos;
            }
        }

        auto scan (bool line_start) -> TokenKind
        {
            char c = peek();
            if (c == '/' && peek (1) == '/')
            {
                scanLineComment();
                return TokenKind::Comment;
            }
            if (c == '/' && peek (1) == '*')
            {
                scanBlockComment();
                return TokenKind::Comment;
            }
            if (c == '#' && line_start)
            {
                scanDirective();
                return TokenKind::Preprocessor;
            }
            if (isIdentifierStart (c))
            {
                return scanIdentifierOrPrefixedLiteral();
            }
            if (isDigit (c) || (c == '.' && isDigit (peek (1))))
            {
                scanNumber();
                return TokenKind::Number;
            }
            if (c == '"')
            {
                scanQuoted ('"');
                scanSuffix();
                return TokenKind::String;
            }
            if (c == '\'')
            {
                scanQuoted ('\'');
                scanSuffix();
                return TokenKind::Character;
            }
            if (scanPunctuator())
            {
                return TokenKind::Punctuator;
            }

            advance();
            return TokenKind::Other;
        }

        // To the end of the line, which a backslash before it continues.
        void scanLineComment()
        {
            while (!atEnd() && peek() != '\n')
            {
                advance (peek() == '\\' && peek (1) == '\n' ? 2 : 1);
            }
        }

        void scanBlockComment()
        {
            advance (2);
            while (!atEnd() && !(peek() == '*' && peek (1) == '/'))
            {
                advance();
            }
            advance (2);
        }

        // To the end of the line, past continuation lines, literals and block
        // comments. A line comment is left as a token of its own.
        void scanDirective()
        {
            while (!atEnd())
            {
                char c = peek();
                if (c == '\n')
                {
                    return;
                }
                if (c == '\\' && peek (1) == '\n')
                {
                    advance (2);
                }
                else if (c == '/' && peek (1) == '/')
                {
                    trimTrailingSpace();
                    return;
                }
                else if (c == '/' && peek (1) == '*')
                {
                    scanBlockComment();
                }
                else if (isIdentifierStart (c) && !isIdentifierChar (previous()))
                {
                    // A word as in code, so that a prefixed or raw string,
                    // which can hold newlines, is read whole.
                    scanIdentifierOrPrefixedLiteral();
                }
                else if (c == '"' || (c == '\'' && !isIdentifierChar (previous())))
                {
                    scanQuoted (c);
                }
                else
                {
                    advance();
                }
            }
        }

        [[nodiscard]] auto previous() const -> char
        {
            return my_pos > 0 ? my_source[my_pos - 1] : '\0';
        }

        // Steps back over the spaces a directive ends with before a comment,
        // so that they separate the two tokens instead.
        void trimTrailingSpace()
        {
            while (my_pos > 0 && (previous() == ' ' || previous() == '\t'))
            {
                --my_pos;
                --my_column;
            }
        }

        auto scanIdentifierOrPrefixedLiteral() -> TokenKind
        {
            size_t start = my_pos;
            while (isIdentifierChar (peek()))
            {
                advance();
            }
            auto word = my_source.substr (start, my_pos - start);

            char next = peek();
            if (next == '"'
                && (word == "R" || word == "LR" || word == "uR" || word == "UR" || word == "u8R"))
            {
                scanRawString();
                scanSuffix();
                return TokenKind::String;
            }
            if ((next == '"' || next == '\'')
                && (word == "L" || word == "u" || word == "U" || word == "u8"))
            {
                scanQuoted (next);
                scanSuffix();
                return next == '"' ? TokenKind::String : TokenKind::Character;
            }
            return TokenKind::Identifier;
        }

        // A preprocessing number: digits, letters, '.', digit separators and
        // the sign of an exponent.
        void scanNumber()
        {
            advance();
            while (!atEnd())
            {
                char c = peek();
                char next = peek (1);
                if ((c == 'e' || c == 'E' || c == 'p' || c == 'P') && (next == '+' || next == '-'))
                {
                    advance (2);
                }
                else if (isIdentifierChar (c) || c == '.' || (c == '\'' && isIdentifierChar (next)))
                {
                    advance();
                }
                else
                {
                    return;
                }
            }
        }

        // An ordinary literal cannot hold a newline, so an unterminated one
        // ends with its line. A backslash before a newline splices the next
        // line on, and before anything else escapes it.
        void scanQuoted (char quote)
        {
            advance();
            while (!atEnd() && peek() != '\n')
            {
                char c = peek();
                if (c == '\\')
                {
                    advance (2);
                    continue;
                }
                advance();
                if (c == quote)
                {
                    return;
                }
            }
        }

        // R"delimiter( ... )delimiter". A delimiter that is not valid makes
        // it an ordinary string.
        void scanRawString()
        {
            size_t delimiter_start = my_pos + 1;
            size_t open = delimiter_start;
            while (open < my_source.size() && open - delimiter_start <= 16
                   && my_source[open] != '(' && my_source[open] != ')' && my_source[open] != '\\'
                   && my_source[open] != '"' && !std::isspace (static_cast<unsigned char> (my_source[open])))
            {
                ++open;
            }
            if (open >= my_source.size() || my_source[open] != '(' || open - delimiter_start > 16)
            {
                scanQuoted ('"');
                return;
            }

            std::string closing = ")" + std::string { my_source.substr (delimiter_start, open - delimiter_start) } + "\"";
            size_t close = my_source.find (closing, open + 1);
            size_t end = close == std::string_view::npos ? my_source.size() : close + closing.size();
            advance (end - my_pos);
        }

        // A user-defined literal's suffix belongs to its literal.
        void scanSuffix()
        {
            if (isIdentifierStart (peek()))
            {
                while (isIdentifierChar (peek()))
                {
                    advance();
                }
            }
        }

        auto scanPunctuator() -> bool
        {
            auto rest = my_source.substr (my_pos);
            for (auto punctuator : Multi_Char_Punctuators)
            {
                if (rest.starts_with (punctuator))
                {
                    advance (punctuator.size());
                    return true;
                }
            }
            if (Single_Char_Punctuators.find (peek()) != std::string_view::npos)
            {
                advance();
                return true;
            }
            return false;
        }
    };
} // namespace

auto lex (std::string_view source) -> std::vector<Token>
{
    return Lexer { source }.run();
}

auto codeTokens (const std::vector<Token>& tokens) -> std::vector<Token>
{
    std::vector<Token> code;
    for (const auto& token : tokens)
    {
        if (token.kind != TokenKind::Comment && token.kind != TokenKind::Preprocessor)
        {
            code.push_back (token);
        }
    }
    return code;
}

auto tokenKindName (TokenKind kind) -> std::string_view
{
    switch (kind)
    {
    case TokenKind::Identifier:
        return "identifier";
    case TokenKind::Number:
        return "number";
    case TokenKind::String:
        return "string";
    case TokenKind::Character:
        return "character";
    case TokenKind::Punctuator:
        return "punctuator";
    case TokenKind::Comment:
        return "comment";
    case TokenKind::Preprocessor:
        return "preprocessor";
    case TokenKind::Other:
        return "other";
    }
    return "other";
}

} // namespace wisdom_linter
