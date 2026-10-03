#include <algorithm>
#include <unordered_set>
#include <utility>

#include "../linter.hpp"

namespace wisdom_linter
{
namespace
{
    // Whether a comment comes right after the token first, before last.
    auto commentBetween (const std::vector<Token>& tokens, const Token& first, const Token& last) -> bool
    {
        auto position = [] (const Token& token) { return std::pair { token.line, token.column }; };
        auto next = std::upper_bound (tokens.begin(), tokens.end(), first,
            [&] (const Token& value, const Token& element) { return position (value) < position (element); });
        return next != tokens.end() && next->kind == TokenKind::Comment && position (*next) < position (last);
    }

    class FunctionCallSpacingRule : public Rule
    {
    public:
        [[nodiscard]] auto name() const -> std::string_view override
        {
            return "function-call-spacing";
        }

        [[nodiscard]] auto description() const -> std::string_view override
        {
            return "Functions with arguments need space before (, zero-arg functions do not";
        }

        [[nodiscard]] auto check (const LintContext& context) const
            -> std::vector<LintViolation> override
        {
            std::vector<LintViolation> violations;

            static const std::unordered_set<std::string> exception_keywords = {
                "if", "for", "while", "switch", "catch", "sizeof", "alignof", "decltype",
                "typeid", "noexcept", "void", "return", "throw", "co_return", "co_yield",
                "co_await", "defined", "NOLINT", "Expects", "Ensures", "assert", "static_assert",
                "EXPECTS", "EXPECTS_NOEXCEPT", "ENSURES", "ENSURES_NOEXCEPT", "ASSERT",
                "TEST_CASE", "SUBCASE", "CHECK", "CHECK_FALSE", "REQUIRE", "REQUIRE_FALSE",
                "REQUIRE_THROWS", "REQUIRE_THROWS_AS", "REQUIRE_THROWS_WITH",
                "REQUIRE_THROWS_WITH_AS", "REQUIRE_NOTHROW", "CHECK_THROWS", "CHECK_THROWS_AS",
                "CHECK_THROWS_WITH", "CHECK_THROWS_WITH_AS", "CHECK_NOTHROW", "WARN", "WARN_FALSE",
                "INFO", "CAPTURE", "GENERATE", "SECTION",
                "CHECK_EQ", "CHECK_NE", "CHECK_GT", "CHECK_LT", "CHECK_GE", "CHECK_LE",
                "REQUIRE_EQ", "REQUIRE_NE", "REQUIRE_GT", "REQUIRE_LT", "REQUIRE_GE", "REQUIRE_LE",
                "WARN_EQ", "WARN_NE", "WARN_GT", "WARN_LT", "WARN_GE", "WARN_LE",
                "CHECK_UNARY", "CHECK_UNARY_FALSE",
                "REQUIRE_UNARY", "REQUIRE_UNARY_FALSE",
                "WARN_UNARY", "WARN_UNARY_FALSE",
                "WARN_THROWS", "WARN_THROWS_AS", "WARN_THROWS_WITH", "WARN_THROWS_WITH_AS",
                "WARN_NOTHROW",
                "CHECK_MESSAGE", "REQUIRE_MESSAGE", "WARN_MESSAGE",
                "MESSAGE", "FAIL", "FAIL_CHECK",
                "QVERIFY", "QVERIFY2", "QCOMPARE",
                "QCOMPARE_EQ", "QCOMPARE_NE", "QCOMPARE_LT", "QCOMPARE_LE", "QCOMPARE_GT",
                "QCOMPARE_GE",
                "QTRY_VERIFY", "QTRY_VERIFY2", "QTRY_COMPARE",
                "QTRY_VERIFY_WITH_TIMEOUT", "QTRY_VERIFY2_WITH_TIMEOUT",
                "QTRY_COMPARE_WITH_TIMEOUT",
                "QVERIFY_THROWS_EXCEPTION", "QVERIFY_THROWS_NO_EXCEPTION",
                "QFETCH", "QFETCH_GLOBAL", "QFAIL", "QSKIP", "QEXPECT_FAIL",
                "QTEST_MAIN", "QTEST_GUILESS_MAIN", "QTEST_APPLESS_MAIN",
                "Q_OBJECT", "Q_PROPERTY", "Q_SIGNAL",
                "Q_SLOT", "Q_EMIT", "Q_INVOKABLE", "Q_DECLARE_METATYPE", "Q_ENUM", "Q_FLAG",
                "Q_NAMESPACE", "Q_UNUSED", "emit", "signals", "slots",
                "EM_ASM", "EM_ASM_PTR", "EM_ASM_INT", "EM_ASM_DOUBLE", "EM_ASM_ARGS",
                "lengthBytesUTF8", "stringToUTF8", "UTF8ToString", "_malloc", "_free",
            };

            auto code = codeTokens (context.tokens);

            for (size_t i = 0; i + 1 < code.size(); ++i)
            {
                if (!isIdentifier (code[i]) || exception_keywords.count (code[i].text) > 0
                    || !isPunctuator (code[i + 1], "(") || code[i + 1].line != code[i].line)
                {
                    continue;
                }

                const auto& func_name = code[i].text;
                const auto& open = code[i + 1];

                int depth = 0;
                size_t close = i + 1;
                for (; close < code.size(); ++close)
                {
                    if (isPunctuator (code[close], "("))
                    {
                        ++depth;
                    }
                    else if (isPunctuator (code[close], ")") && --depth == 0)
                    {
                        break;
                    }
                }

                // A call whose arguments start on the next line is left alone.
                // A comment alone between the parentheses counts as an
                // argument, as it did before the rule read tokens.
                bool has_args = true;
                if (close < code.size() && code[close].line == open.line)
                {
                    has_args = close > i + 2 || commentBetween (context.tokens, open, code[close]);
                }
                else if (i + 2 >= code.size() || code[i + 2].line != open.line)
                {
                    continue;
                }

                bool has_space = open.spaced_before;
                if (!has_space && has_args)
                {
                    violations.push_back (LintViolation {
                        std::string { name() },
                        "Missing space before '(' in '" + func_name + "' (has arguments)",
                        open.line,
                        open.column,
                        Severity::Error,
                    });
                }
                else if (has_space && !has_args)
                {
                    violations.push_back (LintViolation {
                        std::string { name() },
                        "Unnecessary space before '(' in '" + func_name + "' (no arguments)",
                        open.line,
                        open.column,
                        Severity::Error,
                    });
                }
            }

            return violations;
        }
    };
} // namespace

auto createFunctionCallSpacingRule() -> std::shared_ptr<Rule>
{
    return std::make_shared<FunctionCallSpacingRule>();
}

} // namespace wisdom_linter
