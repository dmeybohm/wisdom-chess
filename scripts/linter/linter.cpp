#include "linter.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

namespace wisdom_linter
{

auto getAllRules() -> std::vector<std::shared_ptr<Rule>>
{
    return registerAllRules();
}

Linter::Linter( const LinterConfig& config )
    : my_config { config }
    , my_rules { registerAllRules() }
{
}

auto Linter::getEnabledRules() const -> std::vector<std::shared_ptr<Rule>>
{
    std::vector<std::shared_ptr<Rule>> enabled;
    for ( const auto& rule : my_rules )
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

    for ( const auto& rule : enabled_rules )
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

} // namespace wisdom_linter
