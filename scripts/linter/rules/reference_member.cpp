#include <optional>
#include <unordered_set>

#include "../linter.hpp"

namespace wisdom_linter
{
namespace
{
    // Whether the "{" at code[brace] opens the body of a class, struct or
    // union, judged by the tokens back to the start of the statement.
    auto opensTypeBody (const std::vector<Token>& code, size_t brace) -> bool
    {
        static const std::unordered_set<std::string> type_keywords {
            "class", "struct", "union",
        };

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
            if (isPunctuator (token, ";") || isPunctuator (token, "{") || isPunctuator (token, "}")
                || isPunctuator (token, "="))
            {
                return false;
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

    // Specifiers that may precede a member's type without being part of it.
    auto isStorageSpecifier (const Token& token) -> bool
    {
        static const std::unordered_set<std::string> specifiers {
            "static", "inline", "mutable", "constexpr", "constinit", "thread_local", "extern",
        };
        return isIdentifier (token) && specifiers.count (token.text) > 0;
    }

    // Words that make the statement something other than a data member.
    auto isNonMemberKeyword (const Token& token) -> bool
    {
        static const std::unordered_set<std::string> keywords {
            "using", "typedef", "friend", "operator", "return", "template", "requires",
        };
        return isIdentifier (token) && keywords.count (token.text) > 0;
    }

    // What may follow a member's name: the end of the declaration, an
    // initializer, another declarator or an attribute.
    auto endsDeclarator (const Token& token) -> bool
    {
        return isPunctuator (token, ";") || isPunctuator (token, "=") || isPunctuator (token, "{")
            || isPunctuator (token, ",") || isPunctuator (token, "[");
    }

    auto isOpenBracket (const Token& token) -> bool
    {
        return isPunctuator (token, "(") || isPunctuator (token, "[") || isPunctuator (token, "{");
    }

    auto isCloseBracket (const Token& token) -> bool
    {
        return isPunctuator (token, ")") || isPunctuator (token, "]") || isPunctuator (token, "}");
    }

    // The open bracket matching the close bracket at code[close].
    auto matchingOpen (const std::vector<Token>& code, size_t close) -> std::optional<size_t>
    {
        int depth = 0;
        for (size_t i = close + 1; i-- > 0; )
        {
            if (isCloseBracket (code[i]))
            {
                ++depth;
            }
            else if (isOpenBracket (code[i]) && --depth == 0)
            {
                return i;
            }
        }
        return std::nullopt;
    }

    // Whether the ":" at code[colon] ends an access specifier.
    auto endsAccessSpecifier (const std::vector<Token>& code, size_t colon) -> bool
    {
        static const std::unordered_set<std::string> specifiers {
            "public", "private", "protected", "signals", "slots", "Q_SIGNALS", "Q_SLOTS",
        };
        return colon > 0 && isIdentifier (code[colon - 1])
            && specifiers.count (code[colon - 1].text) > 0;
    }

    // The start of the declaration statement that code[last] is in, or
    // nothing when the tokens back to it show it is not a data member
    // declaration. The initializers of earlier declarators are stepped
    // over; a body's closing brace, whose opening brace starts its line,
    // ends the search.
    auto statementStart (const std::vector<Token>& code, size_t last) -> std::optional<size_t>
    {
        size_t start = last + 1;
        for (size_t i = last + 1; i-- > 0; )
        {
            const auto& token = code[i];
            if (isCloseBracket (token))
            {
                auto open = matchingOpen (code, i);
                if (!open)
                {
                    return std::nullopt;
                }
                if (isPunctuator (token, "}") && startsLine (code, *open))
                {
                    break;
                }
                i = *open;
                start = i;
                continue;
            }
            if (isPunctuator (token, ";") || isOpenBracket (token)
                || (isPunctuator (token, ":") && endsAccessSpecifier (code, i)))
            {
                break;
            }
            if (isPunctuator (token, "->") || isNonMemberKeyword (token))
            {
                return std::nullopt;
            }
            start = i;
        }
        return start;
    }

    struct DeclaredType
    {
        // The type's text without storage specifiers and attributes.
        std::string text;

        // Index of the first declarator's first token.
        size_t end;
    };

    // The type the declaration at code[start] declares, or nothing when
    // the first declarator is a function's.
    auto declaredType (const std::vector<Token>& code, size_t start) -> std::optional<DeclaredType>
    {
        std::string text;
        size_t i = start;
        for (; i + 1 < code.size(); ++i)
        {
            const auto& token = code[i];
            if (isPunctuator (token, "&") || isPunctuator (token, "&&") || isPunctuator (token, "(")
                || isPunctuator (token, "*"))
            {
                break;
            }
            // decltype's operand is part of the type.
            if (isIdentifier (token, "decltype") && isPunctuator (code[i + 1], "("))
            {
                size_t close = i + 1;
                int depth = 0;
                while (close < code.size())
                {
                    if (isPunctuator (code[close], "("))
                    {
                        ++depth;
                    }
                    else if (isPunctuator (code[close], ")") && --depth == 0)
                    {
                        break;
                    }
                    ++close;
                }
                if (close >= code.size())
                {
                    return std::nullopt;
                }
                for (; i <= close; ++i)
                {
                    text += code[i].text;
                }
                --i;
                continue;
            }
            // A name before a parameter list is a function's; a "(" that a
            // "&" follows parenthesizes a declarator instead.
            if (isIdentifier (token) && isPunctuator (code[i + 1], "(") && i + 2 < code.size()
                && !isPunctuator (code[i + 2], "&") && !isPunctuator (code[i + 2], "&&"))
            {
                return std::nullopt;
            }
            // A name that a bit-field's width follows ends the type too.
            if (isIdentifier (token) && (endsDeclarator (code[i + 1]) || isPunctuator (code[i + 1], ":")))
            {
                break;
            }
            if (isPunctuator (token, "[") && isPunctuator (code[i + 1], "["))
            {
                while (i + 1 < code.size()
                       && !(isPunctuator (code[i], "]") && isPunctuator (code[i + 1], "]")))
                {
                    ++i;
                }
                ++i;
                continue;
            }
            if (isStorageSpecifier (token))
            {
                continue;
            }
            if (!text.empty() && isIdentifier (token)
                && (isIdentifier (code[i - 1]) || isPunctuator (code[i - 1], ")")))
            {
                text += ' ';
            }
            text += token.text;
            if (isPunctuator (token, ","))
            {
                text += ' ';
            }
        }
        if (text.empty())
        {
            return std::nullopt;
        }
        return DeclaredType { text, i };
    }

    // Whether code[index] begins a declarator of the statement whose type
    // ends at code[type_end]: it is the first, or follows a comma at the
    // statement's own bracket depth, so not one inside an initializer.
    auto beginsDeclarator (const std::vector<Token>& code, size_t type_end, size_t index) -> bool
    {
        if (index == type_end)
        {
            return true;
        }
        if (index == 0 || !isPunctuator (code[index - 1], ","))
        {
            return false;
        }
        int depth = 0;
        for (size_t j = type_end; j + 1 < index; ++j)
        {
            if (isOpenBracket (code[j]))
            {
                ++depth;
            }
            else if (isCloseBracket (code[j]))
            {
                --depth;
            }
        }
        return depth == 0;
    }

    class ReferenceMemberRule : public Rule
    {
    public:
        [[nodiscard]] auto name() const -> std::string_view override
        {
            return "reference-member";
        }

        [[nodiscard]] auto description() const -> std::string_view override
        {
            return "A data member that refers to an object it does not own should be "
                   "nonnull<T> or nullable<T>, not a reference";
        }

        [[nodiscard]] auto check (const LintContext& context) const
            -> std::vector<LintViolation> override
        {
            std::vector<LintViolation> violations;
            auto code = codeTokens (context.tokens);

            // For each open brace, whether it is a type body.
            std::vector<bool> scopes;
            int parens = 0;

            for (size_t i = 0; i < code.size(); ++i)
            {
                const auto& token = code[i];
                if (isPunctuator (token, "{"))
                {
                    scopes.push_back (opensTypeBody (code, i));
                    continue;
                }
                if (isPunctuator (token, "}"))
                {
                    if (!scopes.empty())
                    {
                        scopes.pop_back();
                    }
                    continue;
                }
                if (isPunctuator (token, "("))
                {
                    ++parens;
                    continue;
                }
                if (isPunctuator (token, ")"))
                {
                    --parens;
                    continue;
                }

                if ((!isPunctuator (token, "&") && !isPunctuator (token, "&&"))
                    || scopes.empty() || !scopes.back() || i == 0 || i + 2 >= code.size()
                    || !isIdentifier (code[i + 1]))
                {
                    continue;
                }

                // "T& name;", "T& name = ...;", "T& name { ... };", "T& name, ...;",
                // "T& name [[...]];", or "T a, &name;" with the type's
                // first declarator before it.
                const auto& before = code[i - 1];
                const auto& member = code[i + 1];
                const auto& after = code[i + 2];
                bool plain = parens == 0
                    && (isIdentifier (before) || isPunctuator (before, ">")
                        || isPunctuator (before, ">>") || isPunctuator (before, ")")
                        || isPunctuator (before, ","))
                    && endsDeclarator (after);

                // "T (&name)[N];" or "T a, (&name)[N];"
                bool parenthesized = parens == 1 && i >= 2 && isPunctuator (before, "(")
                    && (isIdentifier (code[i - 2]) || isPunctuator (code[i - 2], ">")
                        || isPunctuator (code[i - 2], ">>") || isPunctuator (code[i - 2], ","))
                    && isPunctuator (after, ")");
                if (!plain && !parenthesized)
                {
                    continue;
                }

                size_t declarator = parenthesized ? i - 1 : i;
                auto start = statementStart (code, declarator - 1);
                if (!start)
                {
                    continue;
                }
                auto declared = declaredType (code, *start);
                if (!declared || !beginsDeclarator (code, declared->end, declarator))
                {
                    continue;
                }
                auto type = declared->text;
                if (parenthesized)
                {
                    for (size_t j = i + 3; j < code.size() && !isPunctuator (code[j], ";")
                         && !isPunctuator (code[j], "=") && !isPunctuator (code[j], "{")
                         && !isPunctuator (code[j], ","); ++j)
                    {
                        type += code[j].text;
                    }
                }

                violations.push_back (LintViolation {
                    std::string { name() },
                    "Reference member '" + member.text + "': use nonnull<" + type + "> or nullable<"
                        + type + ">, which cannot bind a temporary and keep the type assignable",
                    token.line,
                    token.column,
                    Severity::Error,
                });
            }

            return violations;
        }
    };
} // namespace

auto createReferenceMemberRule() -> std::shared_ptr<Rule>
{
    return std::make_shared<ReferenceMemberRule>();
}

} // namespace wisdom_linter
