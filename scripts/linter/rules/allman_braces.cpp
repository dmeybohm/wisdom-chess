#include "../linter.hpp"

#include <unordered_map>
#include <unordered_set>

namespace wisdom_linter
{
namespace
{
    // The index of the bracket that pairs with code[index], searching in
    // the direction of step, or code.size() when there is none.
    auto matchingBracket (const std::vector<Token>& code, size_t index, int step) -> size_t
    {
        static const std::unordered_map<std::string, std::string> pairs {
            { "(", ")" }, { ")", "(" }, { "[", "]" }, { "]", "[" }, { "{", "}" }, { "}", "{" },
        };
        const std::string& from = code[index].text;
        const std::string& to = pairs.at (from);
        int depth = 0;
        for (auto i = static_cast<std::ptrdiff_t> (index); i >= 0
             && i < static_cast<std::ptrdiff_t> (code.size()); i += step)
        {
            const auto& token = code[static_cast<size_t> (i)];
            if (isPunctuator (token, from))
            {
                ++depth;
            }
            else if (isPunctuator (token, to) && --depth == 0)
            {
                return static_cast<size_t> (i);
            }
        }
        return code.size();
    }

    enum class Brackets
    {
        Attribute,
        Capture,
        Subscript,
    };

    // What the square brackets opening at code[open] are: an attribute
    // such as [[likely]], a lambda's capture list, or a subscript, which
    // includes an array's bound and operator[].
    auto classifyBrackets (const std::vector<Token>& code, size_t open) -> Brackets
    {
        static const std::unordered_set<std::string> expression_keywords {
            "return", "throw", "co_return", "co_yield", "co_await", "case",
        };

        if (open + 1 < code.size() && isPunctuator (code[open + 1], "["))
        {
            return Brackets::Attribute;
        }
        if (open == 0)
        {
            return Brackets::Capture;
        }
        const auto& before = code[open - 1];
        bool follows_operand = (isIdentifier (before) && expression_keywords.count (before.text) == 0)
            || isPunctuator (before, ")") || isPunctuator (before, "]");
        return follows_operand ? Brackets::Subscript : Brackets::Capture;
    }

    // Whether the "{" at code[brace] opens a block rather than an
    // initializer, judged by the tokens before it. A lambda's body counts
    // as a block here.
    auto opensBlock (const std::vector<Token>& code, size_t brace) -> bool
    {
        static const std::unordered_set<std::string> block_keywords {
            "else", "do", "try",
        };
        static const std::unordered_set<std::string> specifiers {
            "const", "noexcept", "override", "final", "volatile", "mutable", "static",
            "constexpr", "consteval",
        };
        static const std::unordered_set<std::string> type_keywords {
            "class", "struct", "union", "enum",
        };

        if (brace == 0)
        {
            return true;
        }
        const auto& prev = code[brace - 1];

        if (isPunctuator (prev, ")"))
        {
            size_t open = matchingBracket (code, brace - 1, -1);
            bool requires_expression = open > 0 && open < code.size()
                && isIdentifier (code[open - 1], "requires");
            return !requires_expression;
        }
        if (isPunctuator (prev, "}"))
        {
            // The end of a constructor's initializer list.
            return true;
        }
        if (isPunctuator (prev, "]"))
        {
            size_t open = matchingBracket (code, brace - 1, -1);
            return open < code.size() && classifyBrackets (code, open) != Brackets::Subscript;
        }
        if (isIdentifier (prev) && (block_keywords.count (prev.text) > 0
                                    || specifiers.count (prev.text) > 0))
        {
            return true;
        }
        if (!isIdentifier (prev) && !isPunctuator (prev, ">") && !isPunctuator (prev, ">>")
            && !isPunctuator (prev, "&") && !isPunctuator (prev, "&&") && !isPunctuator (prev, "*"))
        {
            return false;
        }

        // A name comes before it: a type's body, a function with a trailing
        // return type or a requires clause, or an initializer. Look back to
        // the start of the statement.
        for (size_t i = brace; i-- > 0; )
        {
            const auto& token = code[i];
            if (isPunctuator (token, ";") || isPunctuator (token, "{")
                || isPunctuator (token, "}") || isPunctuator (token, "=")
                || isIdentifier (token, "return") || isIdentifier (token, "throw"))
            {
                return false;
            }
            if (isPunctuator (token, ")") || isPunctuator (token, "]"))
            {
                size_t open = matchingBracket (code, i, -1);
                if (open >= code.size())
                {
                    return false;
                }
                i = open;
                continue;
            }
            if (isPunctuator (token, "->"))
            {
                return true;
            }
            if (isIdentifier (token, "requires") && i > 0
                && (isPunctuator (code[i - 1], ")") || isPunctuator (code[i - 1], ">")))
            {
                return true;
            }
            if (isIdentifier (token) && type_keywords.count (token.text) > 0)
            {
                return true;
            }
            if (isPunctuator (token, "(") || isPunctuator (token, ","))
            {
                return false;
            }
        }
        return false;
    }

    // Whether the block opening at code[brace] is a lambda's body: looking
    // back to the start of the statement, past its parameters, template
    // parameters, specifiers and return type, there is a capture list.
    auto opensLambdaBody (const std::vector<Token>& code, size_t brace) -> bool
    {
        for (size_t i = brace; i-- > 0; )
        {
            const auto& token = code[i];
            if (isPunctuator (token, ";") || isPunctuator (token, "{") || isPunctuator (token, "}"))
            {
                return false;
            }
            if (!isPunctuator (token, ")") && !isPunctuator (token, "]"))
            {
                continue;
            }
            size_t open = matchingBracket (code, i, -1);
            if (open >= code.size())
            {
                return false;
            }
            if (isPunctuator (token, "]") && classifyBrackets (code, open) == Brackets::Capture)
            {
                return true;
            }
            i = open;
        }
        return false;
    }

    class AllmanBracesRule : public Rule
    {
    public:
        [[nodiscard]] auto name() const -> std::string_view override
        {
            return "allman-braces";
        }

        [[nodiscard]] auto description() const -> std::string_view override
        {
            return "The brace opening a function, control statement or type body "
                   "should be on its own line, unless the block is empty";
        }

        [[nodiscard]] auto check (const LintContext& context) const
            -> std::vector<LintViolation> override
        {
            std::vector<LintViolation> violations;
            auto code = codeTokens (context.tokens);

            for (size_t i = 0; i < code.size(); ++i)
            {
                if (!isPunctuator (code[i], "{") || startsLine (code, i))
                {
                    continue;
                }
                if (i + 1 < code.size() && isPunctuator (code[i + 1], "}")
                    && code[i + 1].line == code[i].line)
                {
                    continue;
                }
                if (!opensBlock (code, i) || opensLambdaBody (code, i))
                {
                    continue;
                }
                // Namespaces are the namespace-braces rule's.
                bool is_namespace = false;
                for (size_t j = i; j-- > 0 && code[j].line == code[i].line; )
                {
                    is_namespace = is_namespace || isIdentifier (code[j], "namespace");
                }
                if (is_namespace)
                {
                    continue;
                }

                violations.push_back (LintViolation {
                    std::string { name() },
                    "Opening brace should be on its own line",
                    code[i].line,
                    code[i].column,
                    Severity::Error,
                });
            }

            return violations;
        }
    };
} // namespace

auto createAllmanBracesRule() -> std::shared_ptr<Rule>
{
    return std::make_shared<AllmanBracesRule>();
}

} // namespace wisdom_linter
