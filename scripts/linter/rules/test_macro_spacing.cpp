#include "../linter.hpp"

#include <cctype>

namespace wisdom_linter
{
namespace
{
    auto isComment( std::string_view line ) -> bool
    {
        size_t start = line.find_first_not_of( " \t" );
        if ( start == std::string_view::npos )
        {
            return false;
        }
        std::string_view trimmed = line.substr( start );
        return trimmed.starts_with( "//" ) || trimmed.starts_with( "/*" ) ||
               trimmed.starts_with( "*" );
    }

    class TestMacroSpacingRule : public Rule
    {
    public:
        [[nodiscard]] auto name() const -> std::string_view override
        {
            return "test-macro-spacing";
        }

        [[nodiscard]] auto description() const -> std::string_view override
        {
            return "Test macros should have spaces inside parentheses: MACRO( content )";
        }

        [[nodiscard]] auto check( const LintContext& context ) const
            -> std::vector<LintViolation> override
        {
            std::vector<LintViolation> violations;

            static const std::vector<std::string> test_macros = {
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
            };

            for ( size_t i = 0; i < context.lines.size(); ++i )
            {
                const auto& line = context.lines[i];
                int line_number = static_cast<int>( i + 1 );

                if ( isComment( line ) )
                {
                    continue;
                }

                for ( const auto& macro : test_macros )
                {
                    size_t pos = 0;
                    while ( ( pos = line.find( macro, pos ) ) != std::string::npos )
                    {
                        if ( pos > 0 && ( std::isalnum( line[pos - 1] ) || line[pos - 1] == '_' ) )
                        {
                            ++pos;
                            continue;
                        }

                        size_t after_macro = pos + macro.size();
                        size_t open_paren = after_macro;
                        bool has_space_before_paren = false;

                        while ( open_paren < line.size() &&
                                ( line[open_paren] == ' ' || line[open_paren] == '\t' ) )
                        {
                            has_space_before_paren = true;
                            ++open_paren;
                        }

                        if ( open_paren >= line.size() || line[open_paren] != '(' )
                        {
                            ++pos;
                            continue;
                        }

                        if ( has_space_before_paren )
                        {
                            violations.push_back( LintViolation {
                                std::string { name() },
                                "Unexpected space before '(' in '" + macro + "'",
                                line_number,
                                static_cast<int>( open_paren + 1 ),
                                Severity::Error,
                            } );
                        }
                        if ( open_paren + 1 < line.size() && line[open_paren + 1] != ' ' &&
                             line[open_paren + 1] != ')' )
                        {
                            violations.push_back( LintViolation {
                                std::string { name() },
                                "Missing space after '(' in '" + macro + "'",
                                line_number,
                                static_cast<int>( open_paren + 2 ),
                                Severity::Error,
                            } );
                        }

                        int depth = 1;
                        size_t j = open_paren + 1;
                        bool in_string = false;
                        char string_char = 0;

                        while ( j < line.size() && depth > 0 )
                        {
                            char c = line[j];

                            if ( !in_string && ( c == '"' || c == '\'' ) )
                            {
                                in_string = true;
                                string_char = c;
                            }
                            else if ( in_string && c == string_char && ( j == 0 || line[j - 1] != '\\' ) )
                            {
                                in_string = false;
                            }
                            else if ( !in_string )
                            {
                                if ( c == '(' )
                                {
                                    ++depth;
                                }
                                else if ( c == ')' )
                                {
                                    --depth;
                                    if ( depth == 0 && j > 0 && line[j - 1] != ' ' &&
                                         line[j - 1] != '\t' )
                                    {
                                        violations.push_back( LintViolation {
                                            std::string { name() },
                                            "Missing space before ')' in '" + macro + "'",
                                            line_number,
                                            static_cast<int>( j + 1 ),
                                            Severity::Error,
                                        } );
                                    }
                                }
                            }
                            ++j;
                        }

                        pos = open_paren + 1;
                    }
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
