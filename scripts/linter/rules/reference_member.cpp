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

    // The start of the declaration statement that code[last] is in, or
    // nothing when the tokens back to it show it is not a plain data member
    // declaration.
    auto statementStart (const std::vector<Token>& code, size_t last) -> std::optional<size_t>
    {
        size_t start = last + 1;
        for (size_t i = last + 1; i-- > 0; )
        {
            const auto& token = code[i];
            if (isPunctuator (token, ";") || isPunctuator (token, "{") || isPunctuator (token, "}")
                || isPunctuator (token, ":"))
            {
                break;
            }
            if (isPunctuator (token, "(") || isPunctuator (token, ")") || isPunctuator (token, "=")
                || isPunctuator (token, "->") || isNonMemberKeyword (token))
            {
                return std::nullopt;
            }
            start = i;
        }
        return start;
    }

    // The type the declaration at code[start] declares: its tokens up to
    // the first declarator, without storage specifiers and attributes.
    auto declaredTypeText (const std::vector<Token>& code, size_t start) -> std::string
    {
        std::string text;
        for (size_t i = start; i + 1 < code.size(); ++i)
        {
            const auto& token = code[i];
            if (isPunctuator (token, "&") || isPunctuator (token, "&&") || isPunctuator (token, "(")
                || (isIdentifier (token) && endsDeclarator (code[i + 1])))
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
            if (!text.empty() && isIdentifier (token) && isIdentifier (code[i - 1]))
            {
                text += ' ';
            }
            text += token.text;
            if (isPunctuator (token, ","))
            {
                text += ' ';
            }
        }
        return text;
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
                        || isPunctuator (before, ">>") || isPunctuator (before, ","))
                    && endsDeclarator (after);

                // "T (&name)[N];"
                bool parenthesized = parens == 1 && i >= 2 && isPunctuator (before, "(")
                    && (isIdentifier (code[i - 2]) || isPunctuator (code[i - 2], ">")
                        || isPunctuator (code[i - 2], ">>"))
                    && isPunctuator (after, ")");
                if (!plain && !parenthesized)
                {
                    continue;
                }

                auto start = statementStart (code, parenthesized ? i - 2 : i - 1);
                if (!start)
                {
                    continue;
                }
                auto type = declaredTypeText (code, *start);
                if (type.empty())
                {
                    continue;
                }
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
