#include "../linter.hpp"

#include <cctype>
#include <unordered_set>

namespace wisdom_linter
{
namespace
{
    auto isPreprocessorOrComment( std::string_view line ) -> bool
    {
        size_t start = line.find_first_not_of( " \t" );
        if ( start == std::string_view::npos )
        {
            return false;
        }
        std::string_view trimmed = line.substr( start );
        return trimmed.starts_with( "#" ) || trimmed.starts_with( "//" ) ||
               trimmed.starts_with( "/*" ) || trimmed.starts_with( "*" );
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

        [[nodiscard]] auto check( const LintContext& context ) const
            -> std::vector<LintViolation> override
        {
            std::vector<LintViolation> violations;

            static const std::unordered_set<std::string> exception_keywords = {
                "if", "for", "while", "switch", "catch", "sizeof", "alignof", "decltype",
                "typeid", "noexcept", "void", "return", "throw", "co_return", "co_yield",
                "co_await", "defined", "NOLINT", "Expects", "Ensures", "assert", "static_assert",
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
                "Q_OBJECT", "Q_PROPERTY", "Q_SIGNAL",
                "Q_SLOT", "Q_EMIT", "Q_INVOKABLE", "Q_DECLARE_METATYPE", "Q_ENUM", "Q_FLAG",
                "Q_NAMESPACE", "emit", "signals", "slots",
                "EM_ASM", "EM_ASM_PTR", "EM_ASM_INT", "EM_ASM_DOUBLE", "EM_ASM_ARGS",
                "lengthBytesUTF8", "stringToUTF8", "UTF8ToString", "_malloc", "_free",
            };

            auto isInsideString = []( const std::string& line, size_t position ) -> bool {
                bool in_string = false;
                char string_char = 0;

                for ( size_t i = 0; i < position; ++i )
                {
                    char c = line[i];
                    char prev = ( i > 0 ) ? line[i - 1] : 0;

                    if ( !in_string && ( c == '"' || c == '\'' ) )
                    {
                        in_string = true;
                        string_char = c;
                    }
                    else if ( in_string && c == string_char && prev != '\\' )
                    {
                        in_string = false;
                    }
                }
                return in_string;
            };

            auto findClosingParen = []( const std::string& line, size_t open_paren ) -> int {
                int depth = 1;
                size_t i = open_paren + 1;
                bool in_string = false;
                char string_char = 0;

                while ( i < line.size() && depth > 0 )
                {
                    char c = line[i];
                    char prev = ( i > 0 ) ? line[i - 1] : 0;

                    if ( !in_string && ( c == '"' || c == '\'' ) )
                    {
                        in_string = true;
                        string_char = c;
                    }
                    else if ( in_string && c == string_char && prev != '\\' )
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
                            if ( depth == 0 )
                            {
                                return static_cast<int>( i );
                            }
                        }
                    }
                    ++i;
                }
                return -1;
            };

            auto hasArguments = [&]( size_t line_idx, size_t open_paren, int close_paren ) -> bool {
                const auto& line = context.lines[line_idx];

                if ( close_paren >= 0 )
                {
                    for ( size_t i = open_paren + 1; i < static_cast<size_t>( close_paren ); ++i )
                    {
                        if ( line[i] != ' ' && line[i] != '\t' )
                        {
                            return true;
                        }
                    }
                    return false;
                }

                for ( size_t i = open_paren + 1; i < line.size(); ++i )
                {
                    if ( line[i] != ' ' && line[i] != '\t' )
                    {
                        return true;
                    }
                }

                if ( line_idx + 1 < context.lines.size() )
                {
                    const auto& next_line = context.lines[line_idx + 1];
                    size_t start = next_line.find_first_not_of( " \t" );
                    if ( start != std::string::npos && next_line[start] != ')' )
                    {
                        return true;
                    }
                }

                return false;
            };

            auto isMultiLineCall = [&]( const std::string& line, size_t open_paren,
                                        int close_paren ) -> bool {
                if ( close_paren >= 0 )
                {
                    return false;
                }
                for ( size_t i = open_paren + 1; i < line.size(); ++i )
                {
                    if ( line[i] != ' ' && line[i] != '\t' )
                    {
                        return false;
                    }
                }
                return true;
            };

            for ( size_t i = 0; i < context.lines.size(); ++i )
            {
                const auto& line = context.lines[i];
                int line_number = static_cast<int>( i + 1 );

                if ( isPreprocessorOrComment( line ) )
                {
                    continue;
                }

                for ( size_t pos = 0; pos < line.size(); ++pos )
                {
                    if ( !std::isalpha( line[pos] ) && line[pos] != '_' )
                    {
                        continue;
                    }

                    if ( pos > 0 && ( std::isalnum( line[pos - 1] ) || line[pos - 1] == '_' ) )
                    {
                        continue;
                    }

                    size_t name_start = pos;
                    size_t name_end = pos;
                    while ( name_end < line.size() &&
                            ( std::isalnum( line[name_end] ) || line[name_end] == '_' ) )
                    {
                        ++name_end;
                    }

                    std::string func_name = line.substr( name_start, name_end - name_start );

                    if ( exception_keywords.count( func_name ) )
                    {
                        pos = name_end - 1;
                        continue;
                    }

                    if ( isInsideString( line, name_start ) )
                    {
                        pos = name_end - 1;
                        continue;
                    }

                    size_t after_name = name_end;
                    bool has_space = false;
                    while ( after_name < line.size() &&
                            ( line[after_name] == ' ' || line[after_name] == '\t' ) )
                    {
                        has_space = true;
                        ++after_name;
                    }

                    if ( after_name >= line.size() || line[after_name] != '(' )
                    {
                        pos = name_end - 1;
                        continue;
                    }

                    size_t open_paren = after_name;
                    int close_paren = findClosingParen( line, open_paren );

                    if ( isMultiLineCall( line, open_paren, close_paren ) )
                    {
                        pos = name_end - 1;
                        continue;
                    }

                    bool has_args = hasArguments( i, open_paren, close_paren );

                    if ( !has_space && has_args )
                    {
                        violations.push_back( LintViolation {
                            std::string { name() },
                            "Missing space before '(' in '" + func_name + "' (has arguments)",
                            line_number,
                            static_cast<int>( open_paren + 1 ),
                            Severity::Error,
                        } );
                    }
                    else if ( has_space && !has_args )
                    {
                        violations.push_back( LintViolation {
                            std::string { name() },
                            "Unnecessary space before '(' in '" + func_name + "' (no arguments)",
                            line_number,
                            static_cast<int>( open_paren + 1 ),
                            Severity::Error,
                        } );
                    }

                    pos = name_end - 1;
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
