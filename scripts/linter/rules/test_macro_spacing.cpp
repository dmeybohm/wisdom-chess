#include <unordered_set>

#include "../linter.hpp"

namespace wisdom_linter
{
namespace
{
    class TestMacroSpacingRule : public Rule
    {
    public:
        [[nodiscard]] auto name() const -> std::string_view override
        {
            return "test-macro-spacing";
        }

        [[nodiscard]] auto description() const -> std::string_view override
        {
            return "Test and contract macros should have spaces inside parentheses: MACRO( content )";
        }

        [[nodiscard]] auto check (const LintContext& context) const
            -> std::vector<LintViolation> override
        {
            std::vector<LintViolation> violations;

            static const std::unordered_set<std::string> test_macros = {
                "EXPECTS", "NOEXCEPT_EXPECTS", "ENSURES", "ASSERT",
                "TEST_CASE", "SUBCASE", "CHECK", "CHECK_FALSE", "REQUIRE", "REQUIRE_FALSE",
                "WARN", "WARN_FALSE", "INFO", "CAPTURE", "GENERATE", "SECTION",
                "CHECK_EQ", "CHECK_NE", "CHECK_GT", "CHECK_LT", "CHECK_GE", "CHECK_LE",
                "REQUIRE_EQ", "REQUIRE_NE", "REQUIRE_GT", "REQUIRE_LT", "REQUIRE_GE", "REQUIRE_LE",
                "WARN_EQ", "WARN_NE", "WARN_GT", "WARN_LT", "WARN_GE", "WARN_LE",
                "CHECK_UNARY", "CHECK_UNARY_FALSE",
                "REQUIRE_UNARY", "REQUIRE_UNARY_FALSE",
                "WARN_UNARY", "WARN_UNARY_FALSE",
                "REQUIRE_THROWS", "REQUIRE_THROWS_AS", "REQUIRE_THROWS_WITH",
                "REQUIRE_THROWS_WITH_AS", "REQUIRE_NOTHROW",
                "CHECK_THROWS", "CHECK_THROWS_AS", "CHECK_THROWS_WITH",
                "CHECK_THROWS_WITH_AS", "CHECK_NOTHROW",
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
            };

            auto code = codeTokens (context.tokens);

            for (size_t i = 0; i + 1 < code.size(); ++i)
            {
                if (!isIdentifier (code[i]) || test_macros.count (code[i].text) == 0
                    || !isPunctuator (code[i + 1], "(") || code[i + 1].line != code[i].line)
                {
                    continue;
                }

                const auto& macro = code[i].text;
                const auto& open = code[i + 1];
                if (open.spaced_before)
                {
                    violations.push_back (LintViolation {
                        std::string { name() },
                        "Unexpected space before '(' in '" + macro + "'",
                        open.line,
                        open.column,
                        Severity::Error,
                    });
                }

                if (i + 2 < code.size())
                {
                    const auto& first = code[i + 2];
                    if (first.line == open.line && !isPunctuator (first, ")") && !first.spaced_before)
                    {
                        violations.push_back (LintViolation {
                            std::string { name() },
                            "Missing space after '(' in '" + macro + "'",
                            first.line,
                            first.column,
                            Severity::Error,
                        });
                    }
                }

                // The closing parenthesis is checked only when it is on the
                // line that opens the call.
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
                if (close < code.size() && code[close].line == open.line && !code[close].spaced_before)
                {
                    violations.push_back (LintViolation {
                        std::string { name() },
                        "Missing space before ')' in '" + macro + "'",
                        code[close].line,
                        code[close].column,
                        Severity::Error,
                    });
                }
            }

            return violations;
        }
    };
} // namespace

auto createTestMacroSpacingRule() -> std::shared_ptr<Rule>
{
    return std::make_shared<TestMacroSpacingRule>();
}

} // namespace wisdom_linter
