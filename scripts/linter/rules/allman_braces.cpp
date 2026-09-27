#include "../linter.hpp"

#include <unordered_set>

namespace wisdom_linter
{
namespace
{
    enum class BraceKind
    {
        Block,
        Lambda,
        Initializer,
    };

    // The index of the bracket that pairs with code[index], searching in
    // the direction of step, or code.size() when there is none.
    auto matchingBracket (const std::vector<Token>& code, size_t index, int step) -> size_t
    {
        const std::string& from = code[index].text;
        std::string to = from == "(" ? ")" : from == ")" ? "(" : from == "{" ? "}" : "{";
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

    // Whether the parenthesized list closing at code[close] belongs to a
    // lambda: its "(" follows the capture list's "]".
    auto closesLambdaParameters (const std::vector<Token>& code, size_t close) -> bool
    {
        size_t open = matchingBracket (code, close, -1);
        return open > 0 && open < code.size() && isPunctuator (code[open - 1], "]");
    }

    // What the "{" at code[brace] opens, judged by the tokens before it.
    auto classify (const std::vector<Token>& code, size_t brace) -> BraceKind
    {
        static const std::unordered_set<std::string> block_keywords {
            "else", "do", "try",
        };
        static const std::unordered_set<std::string> function_specifiers {
            "const", "noexcept", "override", "final", "volatile",
        };
        static const std::unordered_set<std::string> type_keywords {
            "class", "struct", "union", "enum",
        };

        if (brace == 0)
        {
            return BraceKind::Block;
        }
        const auto& prev = code[brace - 1];

        if (isPunctuator (prev, "]") || isIdentifier (prev, "mutable"))
        {
            return BraceKind::Lambda;
        }
        if (isPunctuator (prev, ")"))
        {
            return closesLambdaParameters (code, brace - 1) ? BraceKind::Lambda : BraceKind::Block;
        }
        if (isPunctuator (prev, "}"))
        {
            // The end of a constructor's initializer list.
            return BraceKind::Block;
        }
        if (isIdentifier (prev) && (block_keywords.count (prev.text) > 0
                                    || function_specifiers.count (prev.text) > 0))
        {
            return BraceKind::Block;
        }
        if (!isIdentifier (prev) && !isPunctuator (prev, ">") && !isPunctuator (prev, ">>")
            && !isPunctuator (prev, "&") && !isPunctuator (prev, "&&") && !isPunctuator (prev, "*"))
        {
            return BraceKind::Initializer;
        }

        // A name comes before it: a type's body, a function with a trailing
        // return type, or an initializer. Look back to the start of the
        // statement.
        for (size_t i = brace; i-- > 0; )
        {
            const auto& token = code[i];
            if (isPunctuator (token, ";") || isPunctuator (token, "{")
                || isPunctuator (token, "}") || isPunctuator (token, "=")
                || isIdentifier (token, "return") || isIdentifier (token, "throw"))
            {
                return BraceKind::Initializer;
            }
            if (isPunctuator (token, ")") || isPunctuator (token, "]"))
            {
                size_t open = matchingBracket (code, i, -1);
                if (open >= code.size())
                {
                    return BraceKind::Initializer;
                }
                i = open;
                continue;
            }
            if (isPunctuator (token, "->"))
            {
                return i > 0 && isPunctuator (code[i - 1], ")") && closesLambdaParameters (code, i - 1)
                    ? BraceKind::Lambda
                    : BraceKind::Block;
            }
            if (isIdentifier (token) && type_keywords.count (token.text) > 0)
            {
                return BraceKind::Block;
            }
            if (isPunctuator (token, "(") || isPunctuator (token, ","))
            {
                return BraceKind::Initializer;
            }
        }
        return BraceKind::Initializer;
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
                   "should be on its own line, unless the block closes on the same line";
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
                size_t close = matchingBracket (code, i, 1);
                if (close < code.size() && code[close].line == code[i].line)
                {
                    continue;
                }
                if (classify (code, i) != BraceKind::Block)
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
