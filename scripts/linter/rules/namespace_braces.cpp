#include "../linter.hpp"

namespace wisdom_linter
{
namespace
{
    class NamespaceBracesRule : public Rule
    {
    public:
        [[nodiscard]] auto name() const -> std::string_view override
        {
            return "namespace-braces";
        }

        [[nodiscard]] auto description() const -> std::string_view override
        {
            return "Namespace opening brace should be on its own line";
        }

        [[nodiscard]] auto check (const LintContext& context) const
            -> std::vector<LintViolation> override
        {
            std::vector<LintViolation> violations;
            auto code = codeTokens (context.tokens);

            for (size_t i = 0; i < code.size(); ++i)
            {
                if (!isIdentifier (code[i], "namespace") || !startsLine (code, i))
                {
                    continue;
                }

                // The brace that opens it, unless it is an alias ("=") or
                // declares nothing (";").
                size_t brace = i + 1;
                while (brace < code.size() && !isPunctuator (code[brace], "{")
                       && !isPunctuator (code[brace], "=") && !isPunctuator (code[brace], ";"))
                {
                    ++brace;
                }
                if (brace >= code.size() || !isPunctuator (code[brace], "{")
                    || code[brace].line != code[i].line)
                {
                    continue;
                }

                std::string namespace_name = i + 1 < brace && isIdentifier (code[i + 1])
                    ? code[i + 1].text
                    : "(anonymous)";

                violations.push_back (LintViolation {
                    std::string { name() },
                    "Namespace '" + namespace_name + "' opening brace should be on the next line",
                    code[brace].line,
                    code[brace].column,
                    Severity::Error,
                });
            }

            return violations;
        }
    };
} // namespace

auto createNamespaceBracesRule() -> std::shared_ptr<Rule>
{
    return std::make_shared<NamespaceBracesRule>();
}

} // namespace wisdom_linter
