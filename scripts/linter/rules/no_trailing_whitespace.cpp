#include <algorithm>
#include <unordered_set>

#include "../linter.hpp"

namespace wisdom_linter
{
namespace
{
    // The lines that end inside a string literal, where whitespace before
    // the line break is part of the string. Only a raw string spans lines.
    [[nodiscard]] auto linesEndingInsideString (const std::vector<Token>& tokens)
        -> std::unordered_set<int>
    {
        std::unordered_set<int> result;

        for (const auto& token : tokens)
        {
            if (token.kind != TokenKind::String)
            {
                continue;
            }

            auto line_breaks = std::count (token.text.begin(), token.text.end(), '\n');
            for (int i = 0; i < line_breaks; ++i)
            {
                result.insert (token.line + i);
            }
        }

        return result;
    }

    class NoTrailingWhitespaceRule : public Rule
    {
    public:
        [[nodiscard]] auto name() const -> std::string_view override
        {
            return "no-trailing-whitespace";
        }

        [[nodiscard]] auto description() const -> std::string_view override
        {
            return "Lines should not end in whitespace";
        }

        [[nodiscard]] auto check (const LintContext& context) const
            -> std::vector<LintViolation> override
        {
            std::vector<LintViolation> violations;
            auto inside_string = linesEndingInsideString (context.tokens);

            for (size_t i = 0; i < context.lines.size(); ++i)
            {
                std::string_view line = context.lines[i];
                int line_number = static_cast<int> (i + 1);

                // The line break of a file with CRLF line endings.
                if (!line.empty() && line.back() == '\r')
                {
                    line.remove_suffix (1);
                }

                auto last = line.find_last_not_of (" \t");
                size_t trailing_start = (last == std::string_view::npos) ? 0 : last + 1;
                if (trailing_start == line.size() || inside_string.contains (line_number))
                {
                    continue;
                }

                violations.push_back (LintViolation {
                    std::string { name() },
                    "Trailing whitespace",
                    line_number,
                    static_cast<int> (trailing_start + 1),
                    Severity::Error,
                });
            }

            return violations;
        }
    };
} // namespace

auto createNoTrailingWhitespaceRule() -> std::shared_ptr<Rule>
{
    return std::make_shared<NoTrailingWhitespaceRule>();
}

} // namespace wisdom_linter
