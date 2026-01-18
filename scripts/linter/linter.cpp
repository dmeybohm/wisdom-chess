#include "linter.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

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

        [[nodiscard]] auto check( const LintContext& context ) const
            -> std::vector<LintViolation> override;
    };

    class TrailingReturnTypeRule : public Rule
    {
    public:
        [[nodiscard]] auto name() const -> std::string_view override
        {
            return "trailing-return-type";
        }

        [[nodiscard]] auto description() const -> std::string_view override
        {
            return "Functions should use trailing return type syntax: auto func() -> Type";
        }

        [[nodiscard]] auto check( const LintContext& context ) const
            -> std::vector<LintViolation> override;
    };

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
            -> std::vector<LintViolation> override;
    };

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
            -> std::vector<LintViolation> override;
    };

    NamespaceBracesRule g_namespace_braces_rule;
    TrailingReturnTypeRule g_trailing_return_type_rule;
    TestMacroSpacingRule g_test_macro_spacing_rule;
    FunctionCallSpacingRule g_function_call_spacing_rule;

    std::vector<const Rule*> g_all_rules = {
        &g_namespace_braces_rule,
        &g_trailing_return_type_rule,
        &g_test_macro_spacing_rule,
        &g_function_call_spacing_rule,
    };
} // namespace

auto getAllRules() -> std::vector<const Rule*>
{
    return g_all_rules;
}

Linter::Linter( const LinterConfig& config )
    : my_config { config }
{
}

auto Linter::getEnabledRules() const -> std::vector<const Rule*>
{
    std::vector<const Rule*> enabled;
    for ( const auto* rule : g_all_rules )
    {
        auto it = my_config.rules.find( std::string { rule->name() } );
        if ( it != my_config.rules.end() && it->second != Severity::Off )
        {
            enabled.push_back( rule );
        }
    }
    return enabled;
}

auto Linter::isCppFile( const std::filesystem::path& path ) -> bool
{
    auto ext = path.extension().string();
    std::transform( ext.begin(), ext.end(), ext.begin(), ::tolower );
    return ext == ".cpp" || ext == ".hpp" || ext == ".h";
}

auto Linter::resolveFiles( const std::vector<std::string>& patterns ) const
    -> std::vector<std::filesystem::path>
{
    std::vector<std::filesystem::path> files;

    for ( const auto& pattern : patterns )
    {
        std::filesystem::path path { pattern };
        if ( std::filesystem::is_regular_file( path ) )
        {
            if ( isCppFile( path ) )
            {
                files.push_back( path );
            }
        }
        else if ( std::filesystem::is_directory( path ) )
        {
            for ( const auto& entry :
                  std::filesystem::recursive_directory_iterator { path } )
            {
                if ( entry.is_regular_file() && isCppFile( entry.path() ) )
                {
                    files.push_back( entry.path() );
                }
            }
        }
    }

    std::sort( files.begin(), files.end() );
    files.erase( std::unique( files.begin(), files.end() ), files.end() );
    return files;
}

auto Linter::lintFiles( const std::vector<std::string>& patterns ) const
    -> std::vector<LintResult>
{
    auto files = resolveFiles( patterns );
    std::vector<LintResult> results;
    results.reserve( files.size() );

    for ( const auto& file : files )
    {
        results.push_back( lintFile( file ) );
    }

    return results;
}

auto Linter::lintFile( const std::filesystem::path& filename ) const -> LintResult
{
    std::ifstream file { filename };
    if ( !file )
    {
        return LintResult { filename, {} };
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    std::vector<std::string> lines;
    std::istringstream stream { content };
    std::string line;
    while ( std::getline( stream, line ) )
    {
        lines.push_back( line );
    }

    LintContext context { filename, std::move( lines ), std::move( content ) };

    std::vector<LintViolation> violations;
    auto enabled_rules = getEnabledRules();

    for ( const auto* rule : enabled_rules )
    {
        auto rule_violations = rule->check( context );
        auto it = my_config.rules.find( std::string { rule->name() } );
        Severity severity = ( it != my_config.rules.end() ) ? it->second : Severity::Error;

        for ( auto& violation : rule_violations )
        {
            violation.severity = severity;
            violations.push_back( std::move( violation ) );
        }
    }

    std::sort( violations.begin(), violations.end(),
               []( const LintViolation& a, const LintViolation& b ) {
                   return a.line < b.line;
               } );

    return LintResult { filename, std::move( violations ) };
}

auto formatSimple( const std::vector<LintResult>& results ) -> std::string
{
    std::ostringstream output;
    bool first = true;

    for ( const auto& result : results )
    {
        for ( const auto& violation : result.violations )
        {
            if ( !first )
            {
                output << '\n';
            }
            first = false;
            output << violation.line << ':' << violation.column << ':' << violation.rule
                   << ':' << violation.message;
        }
    }

    return output.str();
}

auto formatCompact( const std::vector<LintResult>& results ) -> std::string
{
    std::ostringstream output;

    for ( const auto& result : results )
    {
        for ( const auto& violation : result.violations )
        {
            output << result.filename.string() << ':' << violation.line << ':'
                   << violation.column << ": "
                   << ( violation.severity == Severity::Error ? "error" : "warning" ) << ": "
                   << violation.message << " [" << violation.rule << "]\n";
        }
    }

    return output.str();
}

auto formatStylish( const std::vector<LintResult>& results ) -> std::string
{
    std::ostringstream output;
    int total_errors = 0;
    int total_warnings = 0;

    for ( const auto& result : results )
    {
        if ( result.violations.empty() )
        {
            continue;
        }

        output << '\n' << result.filename.string() << '\n';

        for ( const auto& violation : result.violations )
        {
            std::string location =
                std::to_string( violation.line ) + ":" + std::to_string( violation.column );

            output << "  " << location;
            for ( size_t i = location.size(); i < 8; ++i )
            {
                output << ' ';
            }

            std::string severity_str =
                ( violation.severity == Severity::Error ) ? "error" : "warning";
            output << ' ' << severity_str;
            for ( size_t i = severity_str.size(); i < 8; ++i )
            {
                output << ' ';
            }

            output << ' ' << violation.message << "  " << violation.rule << '\n';

            if ( violation.severity == Severity::Error )
            {
                ++total_errors;
            }
            else
            {
                ++total_warnings;
            }
        }
    }

    if ( total_errors > 0 || total_warnings > 0 )
    {
        output << '\n' << "  ";
        if ( total_errors > 0 )
        {
            output << total_errors << " error" << ( total_errors == 1 ? "" : "s" );
            if ( total_warnings > 0 )
            {
                output << ", ";
            }
        }
        if ( total_warnings > 0 )
        {
            output << total_warnings << " warning" << ( total_warnings == 1 ? "" : "s" );
        }
        output << '\n';
    }

    return output.str();
}

auto formatJson( const std::vector<LintResult>& results ) -> std::string
{
    std::ostringstream output;
    output << "[\n";

    bool first_result = true;
    for ( const auto& result : results )
    {
        if ( !first_result )
        {
            output << ",\n";
        }
        first_result = false;

        output << "  {\n";
        output << "    \"filename\": \"" << result.filename.string() << "\",\n";
        output << "    \"violations\": [\n";

        bool first_violation = true;
        for ( const auto& violation : result.violations )
        {
            if ( !first_violation )
            {
                output << ",\n";
            }
            first_violation = false;

            output << "      {\n";
            output << "        \"rule\": \"" << violation.rule << "\",\n";
            output << "        \"message\": \"" << violation.message << "\",\n";
            output << "        \"line\": " << violation.line << ",\n";
            output << "        \"column\": " << violation.column << ",\n";
            output << "        \"severity\": \""
                   << ( violation.severity == Severity::Error ? "error" : "warning" )
                   << "\"\n";
            output << "      }";
        }

        output << "\n    ]\n";
        output << "  }";
    }

    output << "\n]\n";
    return output.str();
}

auto formatResults( const std::vector<LintResult>& results, OutputFormat format ) -> std::string
{
    switch ( format )
    {
    case OutputFormat::Simple:
        return formatSimple( results );
    case OutputFormat::Compact:
        return formatCompact( results );
    case OutputFormat::Json:
        return formatJson( results );
    case OutputFormat::Stylish:
    default:
        return formatStylish( results );
    }
}

auto parseOutputFormat( std::string_view format_str ) -> std::optional<OutputFormat>
{
    if ( format_str == "simple" )
    {
        return OutputFormat::Simple;
    }
    if ( format_str == "compact" )
    {
        return OutputFormat::Compact;
    }
    if ( format_str == "json" )
    {
        return OutputFormat::Json;
    }
    if ( format_str == "stylish" )
    {
        return OutputFormat::Stylish;
    }
    return std::nullopt;
}

auto getDefaultConfig() -> LinterConfig
{
    return LinterConfig {
        {
            { "function-call-spacing", Severity::Error },
            { "test-macro-spacing", Severity::Error },
            { "trailing-return-type", Severity::Warning },
            { "namespace-braces", Severity::Error },
        },
        { "**/build/**", "**/node_modules/**", "**/.git/**" },
    };
}

// Rule implementations

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

    auto NamespaceBracesRule::check( const LintContext& context ) const
        -> std::vector<LintViolation>
    {
        std::vector<LintViolation> violations;

        for ( size_t i = 0; i < context.lines.size(); ++i )
        {
            const auto& line = context.lines[i];
            int line_number = static_cast<int>( i + 1 );

            size_t ns_pos = line.find( "namespace" );
            if ( ns_pos == std::string::npos )
            {
                continue;
            }

            size_t start = line.find_first_not_of( " \t" );
            if ( start != ns_pos )
            {
                continue;
            }

            size_t after_ns = ns_pos + 9;
            size_t brace_pos = line.find( '{', after_ns );
            if ( brace_pos == std::string::npos )
            {
                continue;
            }

            std::string namespace_name = "(anonymous)";
            size_t name_start = line.find_first_not_of( " \t", after_ns );
            if ( name_start != std::string::npos && name_start < brace_pos )
            {
                size_t name_end = line.find_first_of( " \t:{", name_start );
                if ( name_end != std::string::npos && name_end > name_start )
                {
                    namespace_name = line.substr( name_start, name_end - name_start );
                }
            }

            violations.push_back( LintViolation {
                std::string { name() },
                "Namespace '" + namespace_name + "' opening brace should be on the next line",
                line_number,
                static_cast<int>( brace_pos + 1 ),
                Severity::Error,
            } );
        }

        return violations;
    }

    auto TrailingReturnTypeRule::check( const LintContext& context ) const
        -> std::vector<LintViolation>
    {
        std::vector<LintViolation> violations;

        static const std::vector<std::string> common_types = { "char", "short", "long", "float",
                                                               "double", "size_t", "int8_t",
                                                               "int16_t", "int32_t", "int64_t",
                                                               "uint8_t", "uint16_t", "uint32_t",
                                                               "uint64_t", "string", "wstring" };

        static const std::vector<std::string> skip_starts = {
            "class ", "struct ", "enum ", "union ", "namespace ",
            "using ", "typedef ", "template<", "template <",
            "#", "//", "/*", "*", "return ", "[[",
        };

        static const std::vector<std::string> specifiers = {
            "static", "inline", "virtual", "explicit", "constexpr", "consteval", "friend", "extern",
        };

        for ( size_t i = 0; i < context.lines.size(); ++i )
        {
            const auto& line = context.lines[i];
            int line_number = static_cast<int>( i + 1 );

            size_t start = line.find_first_not_of( " \t" );
            if ( start == std::string_view::npos )
            {
                continue;
            }
            std::string_view trimmed = std::string_view { line }.substr( start );

            bool skip = false;
            for ( const auto& prefix : skip_starts )
            {
                if ( trimmed.starts_with( prefix ) )
                {
                    skip = true;
                    break;
                }
            }
            if ( skip )
            {
                continue;
            }

            if ( trimmed.starts_with( "auto " ) )
            {
                continue;
            }

            if ( trimmed.find( "if(" ) != std::string_view::npos ||
                 trimmed.find( "if (" ) != std::string_view::npos ||
                 trimmed.find( "for(" ) != std::string_view::npos ||
                 trimmed.find( "for (" ) != std::string_view::npos ||
                 trimmed.find( "while(" ) != std::string_view::npos ||
                 trimmed.find( "while (" ) != std::string_view::npos ||
                 trimmed.find( "switch(" ) != std::string_view::npos ||
                 trimmed.find( "switch (" ) != std::string_view::npos ||
                 trimmed.find( "catch(" ) != std::string_view::npos ||
                 trimmed.find( "catch (" ) != std::string_view::npos )
            {
                continue;
            }

            if ( line.find( "= [" ) != std::string::npos )
            {
                continue;
            }

            std::string_view current = trimmed;
            for ( const auto& spec : specifiers )
            {
                if ( current.starts_with( spec ) &&
                     current.size() > spec.size() &&
                     ( current[spec.size()] == ' ' || current[spec.size()] == '\t' ) )
                {
                    size_t after_spec = current.find_first_not_of( " \t", spec.size() );
                    if ( after_spec != std::string_view::npos )
                    {
                        current = current.substr( after_spec );
                    }
                }
            }

            std::string return_type;
            std::string func_name;

            for ( const auto& type : common_types )
            {
                if ( current.starts_with( type ) &&
                     ( current.size() == type.size() ||
                       current[type.size()] == ' ' ||
                       current[type.size()] == '*' ||
                       current[type.size()] == '&' ||
                       current[type.size()] == '\t' ) )
                {
                    return_type = type;
                    size_t after_type = type.size();

                    while ( after_type < current.size() &&
                            ( current[after_type] == ' ' ||
                              current[after_type] == '\t' ||
                              current[after_type] == '*' ||
                              current[after_type] == '&' ) )
                    {
                        ++after_type;
                    }

                    size_t name_end = after_type;
                    while ( name_end < current.size() &&
                            ( std::isalnum( current[name_end] ) || current[name_end] == '_' ) )
                    {
                        ++name_end;
                    }

                    if ( name_end > after_type )
                    {
                        func_name = std::string { current.substr( after_type, name_end - after_type ) };

                        size_t paren_pos = current.find( '(', name_end );
                        if ( paren_pos != std::string_view::npos )
                        {
                            bool only_whitespace = true;
                            for ( size_t j = name_end; j < paren_pos; ++j )
                            {
                                if ( current[j] != ' ' && current[j] != '\t' )
                                {
                                    only_whitespace = false;
                                    break;
                                }
                            }
                            if ( only_whitespace )
                            {
                                break;
                            }
                        }
                    }
                    func_name.clear();
                    return_type.clear();
                }
            }

            if ( return_type.empty() )
            {
                if ( std::isupper( current[0] ) )
                {
                    size_t type_end = 0;
                    while ( type_end < current.size() &&
                            ( std::isalnum( current[type_end] ) || current[type_end] == '_' ) )
                    {
                        ++type_end;
                    }

                    if ( type_end > 0 && type_end < current.size() )
                    {
                        if ( current[type_end] == '<' )
                        {
                            int depth = 1;
                            size_t j = type_end + 1;
                            while ( j < current.size() && depth > 0 )
                            {
                                if ( current[j] == '<' )
                                {
                                    ++depth;
                                }
                                else if ( current[j] == '>' )
                                {
                                    --depth;
                                }
                                ++j;
                            }
                            type_end = j;
                        }

                        if ( type_end < current.size() &&
                             ( current[type_end] == ' ' || current[type_end] == '\t' ) )
                        {
                            return_type = std::string { current.substr( 0, type_end ) };

                            size_t after_type = type_end;
                            while ( after_type < current.size() &&
                                    ( current[after_type] == ' ' || current[after_type] == '\t' ) )
                            {
                                ++after_type;
                            }

                            size_t name_end = after_type;
                            while ( name_end < current.size() &&
                                    ( std::isalnum( current[name_end] ) ||
                                      current[name_end] == '_' ) )
                            {
                                ++name_end;
                            }

                            if ( name_end > after_type )
                            {
                                func_name =
                                    std::string { current.substr( after_type, name_end - after_type ) };

                                size_t paren_pos = current.find( '(', name_end );
                                if ( paren_pos == std::string_view::npos )
                                {
                                    func_name.clear();
                                    return_type.clear();
                                }
                                else
                                {
                                    bool only_whitespace = true;
                                    for ( size_t j = name_end; j < paren_pos; ++j )
                                    {
                                        if ( current[j] != ' ' && current[j] != '\t' )
                                        {
                                            only_whitespace = false;
                                            break;
                                        }
                                    }
                                    if ( !only_whitespace )
                                    {
                                        func_name.clear();
                                        return_type.clear();
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if ( !func_name.empty() && !return_type.empty() )
            {
                if ( func_name == "main" )
                {
                    continue;
                }

                if ( return_type == func_name )
                {
                    continue;
                }

                if ( func_name == "TEST_CASE" || func_name == "SUBCASE" || func_name == "SECTION" )
                {
                    continue;
                }

                if ( line.find( "->" ) != std::string::npos )
                {
                    continue;
                }

                bool is_definition = false;
                if ( line.find( '{' ) != std::string::npos )
                {
                    is_definition = true;
                }
                else if ( line.find( ';' ) != std::string::npos )
                {
                    is_definition = true;
                }
                else if ( i + 1 < context.lines.size() )
                {
                    size_t next_start = context.lines[i + 1].find_first_not_of( " \t" );
                    if ( next_start != std::string::npos &&
                         context.lines[i + 1][next_start] == '{' )
                    {
                        is_definition = true;
                    }
                }

                if ( is_definition )
                {
                    violations.push_back( LintViolation {
                        std::string { name() },
                        "Function '" + func_name + "' should use trailing return type: auto " +
                            func_name + "(...) -> " + return_type,
                        line_number,
                        1,
                        Severity::Warning,
                    } );
                }
            }
        }

        return violations;
    }

    auto TestMacroSpacingRule::check( const LintContext& context ) const
        -> std::vector<LintViolation>
    {
        std::vector<LintViolation> violations;

        static const std::vector<std::string> test_macros = { "TEST_CASE", "SUBCASE", "CHECK",
                                                              "CHECK_FALSE", "REQUIRE",
                                                              "REQUIRE_FALSE", "WARN",
                                                              "WARN_FALSE", "INFO", "CAPTURE" };

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
                    if ( after_macro >= line.size() || line[after_macro] != '(' )
                    {
                        ++pos;
                        continue;
                    }

                    size_t open_paren = after_macro;
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

    auto FunctionCallSpacingRule::check( const LintContext& context ) const
        -> std::vector<LintViolation>
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
            "INFO", "CAPTURE", "GENERATE", "SECTION", "Q_OBJECT", "Q_PROPERTY", "Q_SIGNAL",
            "Q_SLOT", "Q_EMIT", "Q_INVOKABLE", "Q_DECLARE_METATYPE", "Q_ENUM", "Q_FLAG",
            "Q_NAMESPACE", "emit", "signals", "slots",
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

} // namespace

} // namespace wisdom_linter
