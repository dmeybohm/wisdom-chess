#include <cctype>
#include <optional>
#include <unordered_set>

#include "../linter.hpp"

namespace wisdom_linter
{
namespace
{
    // The index past the template argument list that opens at code[open],
    // or open when it is not closed.
    auto skipTemplateArguments (const std::vector<Token>& code, size_t open) -> size_t
    {
        int depth = 0;
        for (size_t i = open; i < code.size(); ++i)
        {
            if (isPunctuator (code[i], "<"))
            {
                ++depth;
            }
            else if (isPunctuator (code[i], ">"))
            {
                --depth;
            }
            else if (isPunctuator (code[i], ">>"))
            {
                depth -= 2;
            }
            if (depth <= 0)
            {
                return i + 1;
            }
        }
        return open;
    }

    struct TypeName
    {
        // The index past the name.
        size_t end;
        bool qualified;
    };

    // The type name that starts at code[start], with its qualifying names
    // and template arguments, as in std::vector<int>::iterator. Nothing when
    // a template argument list is not closed before end.
    auto readTypeName (const std::vector<Token>& code, size_t start, size_t end)
        -> std::optional<TypeName>
    {
        TypeName name { start + 1, false };
        while (name.end < end)
        {
            if (isPunctuator (code[name.end], "<"))
            {
                size_t close = skipTemplateArguments (code, name.end);
                if (close == name.end || close > end)
                {
                    return std::nullopt;
                }
                name.end = close;
            }
            else if (name.end + 1 < end && isPunctuator (code[name.end], "::")
                     && isIdentifier (code[name.end + 1]))
            {
                name.end += 2;
                name.qualified = true;
            }
            else
            {
                break;
            }
        }
        return name;
    }

    auto isDeclarator (const Token& token) -> bool
    {
        return isPunctuator (token, "*") || isPunctuator (token, "&") || isPunctuator (token, "&&");
    }

    // The index of the ")" that closes the "(" at code[open], or
    // code.size() when there is none.
    auto closingParenthesis (const std::vector<Token>& code, size_t open) -> size_t
    {
        int depth = 0;
        for (size_t i = open; i < code.size(); ++i)
        {
            if (isPunctuator (code[i], "("))
            {
                ++depth;
            }
            else if (isPunctuator (code[i], ")") && --depth == 0)
            {
                return i;
            }
        }
        return code.size();
    }

    // Whether the parentheses opening at code[open] hold a function's
    // parameters and not a variable's initializer, judged by the first one:
    // they are empty, or start with a type keyword or with a type and a
    // name. A single name is taken for an initializer, and so is a cast
    // from a type keyword, int { 5 } or int (5). Parentheses after the
    // keyword are a declarator instead when a parameter list or an array
    // bound follows them, int (*callback) (int), or when they hold only a
    // name and are alone, int (name), as they are to the compiler.
    auto holdsParameters (const std::vector<Token>& code, size_t open) -> bool
    {
        static const std::unordered_set<std::string> type_keywords {
            "const", "volatile", "unsigned", "signed", "void", "bool", "char", "short", "int",
            "long", "float", "double", "auto", "typename", "struct", "class", "enum", "union",
        };
        static const std::unordered_set<std::string> expression_keywords {
            "new", "delete", "sizeof", "alignof", "typeid", "throw", "co_await", "not", "compl",
            "and", "or", "xor", "not_eq", "bitand", "bitor", "and_eq", "or_eq", "xor_eq",
        };

        size_t close = closingParenthesis (code, open);
        size_t first = open + 1;
        if (first >= close)
        {
            return close < code.size();
        }
        if (!isIdentifier (code[first]) || expression_keywords.count (code[first].text) > 0)
        {
            return false;
        }
        if (type_keywords.count (code[first].text) > 0)
        {
            size_t after = first + 1;
            while (after < close && isIdentifier (code[after])
                   && type_keywords.count (code[after].text) > 0)
            {
                ++after;
            }
            if (after < close && isPunctuator (code[after], "{"))
            {
                return false;
            }
            if (after < close && isPunctuator (code[after], "("))
            {
                size_t inner_close = closingParenthesis (code, after);
                if (inner_close + 1 < close)
                {
                    return isPunctuator (code[inner_close + 1], "(")
                        || isPunctuator (code[inner_close + 1], "[");
                }
                size_t name = after + 1;
                while (name < inner_close && isDeclarator (code[name]))
                {
                    ++name;
                }
                return name + 1 == inner_close && isIdentifier (code[name]);
            }
            return true;
        }

        auto type = readTypeName (code, first, close);
        if (!type)
        {
            return false;
        }
        size_t next = type->end;
        while (next < close && isDeclarator (code[next]) && !code[next].spaced_before)
        {
            ++next;
        }
        if (next < close && isIdentifier (code[next]))
        {
            return code[next].spaced_before && expression_keywords.count (code[next].text) == 0;
        }
        bool has_declarator = next > type->end;
        return has_declarator && (next == close || isPunctuator (code[next], ","));
    }

    // The index past the attributes, such as [[nodiscard]], that start at
    // code[start], or end when one is not closed on the line.
    auto skipAttributes (const std::vector<Token>& code, size_t start, size_t end) -> size_t
    {
        while (start + 1 < end && isPunctuator (code[start], "[") && isPunctuator (code[start + 1], "["))
        {
            size_t close = start + 2;
            while (close + 1 < end
                   && !(isPunctuator (code[close], "]") && isPunctuator (code[close + 1], "]")))
            {
                ++close;
            }
            if (close + 1 >= end)
            {
                return end;
            }
            start = close + 2;
        }
        return start;
    }

    // Whether the "{" at code[brace] opens a namespace, a type or a linkage
    // block, where functions are declared, judged by the tokens back to the
    // start of the statement.
    auto opensDeclarationScope (const std::vector<Token>& code, size_t brace) -> bool
    {
        static const std::unordered_set<std::string> type_keywords {
            "class", "struct", "union", "enum",
        };

        if (brace >= 2 && code[brace - 1].kind == TokenKind::String
            && isIdentifier (code[brace - 2], "extern"))
        {
            return true;
        }

        int depth = 0;
        for (size_t i = brace; i-- > 0; )
        {
            const auto& token = code[i];
            if (isPunctuator (token, ")"))
            {
                ++depth;
            }
            else if (isPunctuator (token, "(") && --depth < 0)
            {
                return false;
            }
            if (depth > 0)
            {
                continue;
            }
            if (isPunctuator (token, ";") || isPunctuator (token, "{") || isPunctuator (token, "}"))
            {
                return false;
            }
            if (isIdentifier (token, "namespace"))
            {
                return true;
            }
            bool template_parameter = i > 0
                && (isPunctuator (code[i - 1], "<") || isPunctuator (code[i - 1], ","));
            if (isIdentifier (token) && type_keywords.count (token.text) > 0 && !template_parameter)
            {
                return true;
            }
        }
        return false;
    }

    // For each token, whether it is inside a function's body or an
    // initializer, where a name followed by parentheses usually declares a
    // variable and not a function.
    auto findTokensInBodies (const std::vector<Token>& code) -> std::vector<bool>
    {
        std::vector<bool> in_body;
        std::vector<bool> scopes;
        in_body.reserve (code.size());
        for (size_t i = 0; i < code.size(); ++i)
        {
            if (isPunctuator (code[i], "}") && !scopes.empty())
            {
                scopes.pop_back();
            }
            in_body.push_back (!scopes.empty() && scopes.back());
            if (isPunctuator (code[i], "{"))
            {
                scopes.push_back (!opensDeclarationScope (code, i));
            }
        }
        return in_body;
    }

    class TrailingReturnTypeRule : public Rule
    {
    public:
        [[nodiscard]] auto name() const -> std::string_view override
        {
            return "trailing-return-type";
        }

        [[nodiscard]] auto description() const -> std::string_view override
        {
            return "Functions should use trailing return type syntax: auto func() -> Type";
        }

        [[nodiscard]] auto check (const LintContext& context) const
            -> std::vector<LintViolation> override
        {
            std::vector<LintViolation> violations;
            auto code = codeTokens (context.tokens);
            auto in_body = findTokensInBodies (code);

            for (size_t line_start = 0; line_start < code.size(); )
            {
                size_t line_end = line_start + 1;
                while (line_end < code.size() && code[line_end].line == code[line_start].line)
                {
                    ++line_end;
                }

                auto violation = checkLine (context, code, line_start, line_end,
                                            in_body[line_start]);
                if (violation)
                {
                    violations.push_back (std::move (*violation));
                }
                line_start = line_end;
            }

            return violations;
        }

    private:
        // A declaration of a function with a leading return type, on the line
        // of tokens code[start, end). In a function's body, only one whose
        // parentheses hold parameters.
        [[nodiscard]] auto checkLine (const LintContext& context, const std::vector<Token>& code,
                                      size_t start, size_t end, bool in_body) const
            -> std::optional<LintViolation>
        {
            static const std::unordered_set<std::string> skip_starts {
                "class", "struct", "enum", "union", "namespace", "using", "typedef", "template",
                "return", "auto",
            };
            static const std::unordered_set<std::string> control_statements {
                "if", "for", "while", "switch", "catch",
            };
            static const std::unordered_set<std::string> specifiers {
                "static", "inline", "virtual", "explicit", "constexpr", "consteval", "friend",
                "extern",
            };
            static const std::unordered_set<std::string> common_types {
                "char", "short", "long", "float", "double", "size_t", "int8_t", "int16_t",
                "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t", "string",
                "wstring",
            };

            start = skipAttributes (code, start, end);
            if (start >= end)
            {
                return std::nullopt;
            }

            const auto& first = code[start];
            if ((isIdentifier (first) && skip_starts.count (first.text) > 0) || isPunctuator (first, "*"))
            {
                return std::nullopt;
            }

            bool has_body_or_end = false;
            for (size_t i = start; i < end; ++i)
            {
                bool followed_by_paren = i + 1 < end && isPunctuator (code[i + 1], "(");
                bool lambda = isPunctuator (code[i], "=") && i + 1 < end && isPunctuator (code[i + 1], "[");
                if ((isIdentifier (code[i]) && control_statements.count (code[i].text) > 0 && followed_by_paren)
                    || lambda || isPunctuator (code[i], "->"))
                {
                    return std::nullopt;
                }
                has_body_or_end = has_body_or_end || isPunctuator (code[i], "{") || isPunctuator (code[i], ";");
            }

            size_t type_start = start;
            while (type_start < end && isIdentifier (code[type_start])
                   && specifiers.count (code[type_start].text) > 0)
            {
                ++type_start;
            }
            if (type_start >= end || !isIdentifier (code[type_start]))
            {
                return std::nullopt;
            }

            // The return type: one of the common types, possibly a pointer or
            // reference, or a PascalCase or qualified type with any template
            // arguments.
            auto type = readTypeName (code, type_start, end);
            if (!type)
            {
                return std::nullopt;
            }
            size_t type_end = type->end;
            size_t name_index = type_end;
            const auto& type_name = code[type_start].text;
            if (type_end == type_start + 1 && common_types.count (type_name) > 0)
            {
                while (name_index < end && isDeclarator (code[name_index]))
                {
                    ++name_index;
                }
            }
            else if (type->qualified || std::isupper (static_cast<unsigned char> (type_name[0])))
            {
                if (name_index >= end || !code[name_index].spaced_before)
                {
                    return std::nullopt;
                }
            }
            else
            {
                return std::nullopt;
            }

            if (name_index + 1 >= end || !isIdentifier (code[name_index])
                || !isPunctuator (code[name_index + 1], "("))
            {
                return std::nullopt;
            }

            if (in_body && !holdsParameters (code, name_index + 1))
            {
                return std::nullopt;
            }

            const auto& func_name = code[name_index].text;
            const auto& line = context.lines[static_cast<size_t> (code[start].line - 1)];
            const auto& last_type_token = code[type_end - 1];
            auto type_begin = static_cast<size_t> (code[type_start].column - 1);
            auto type_finish = static_cast<size_t> (last_type_token.column - 1) + last_type_token.text.size();
            std::string return_type = line.substr (type_begin, type_finish - type_begin);

            if (func_name == "main" || return_type == func_name || func_name == "TEST_CASE"
                || func_name == "SUBCASE" || func_name == "SECTION")
            {
                return std::nullopt;
            }

            bool next_line_opens = end < code.size() && code[end].line == code[start].line + 1
                && isPunctuator (code[end], "{");
            if (!has_body_or_end && !next_line_opens)
            {
                return std::nullopt;
            }

            return LintViolation {
                std::string { name() },
                "Function '" + func_name + "' should use trailing return type: auto " + func_name
                    + "(...) -> " + return_type,
                code[start].line,
                1,
                Severity::Warning,
            };
        }
    };
} // namespace

auto createTrailingReturnTypeRule() -> std::shared_ptr<Rule>
{
    return std::make_shared<TrailingReturnTypeRule>();
}

} // namespace wisdom_linter
