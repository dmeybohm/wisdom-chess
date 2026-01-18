#include "linter.hpp"

#include <cstring>
#include <iostream>

using namespace wisdom_linter;

namespace
{
    void printUsage( const char* program_name )
    {
        std::cerr << "Usage: " << program_name << " [options] <files...>\n\n"
                  << "Options:\n"
                  << "  -f, --format <format>  Output format: stylish, compact, json, simple (default: stylish)\n"
                  << "  --rules <rules>        Comma-separated list of rules to run\n"
                  << "  --list-rules           List all available rules\n"
                  << "  -h, --help             Show this help message\n"
                  << "  -v, --version          Show version\n";
    }

    void printVersion()
    {
        std::cout << "wisdom-linter 1.0.0\n";
    }

    void printRules()
    {
        std::cout << "Available rules:\n\n";
        for ( const auto* rule : getAllRules() )
        {
            std::cout << "  " << rule->name() << "\n";
            std::cout << "    " << rule->description() << "\n\n";
        }
    }
} // namespace

auto main( int argc, char* argv[] ) -> int
{
    OutputFormat format = OutputFormat::Stylish;
    std::vector<std::string> files;
    std::vector<std::string> selected_rules;
    bool list_rules = false;

    for ( int i = 1; i < argc; ++i )
    {
        std::string arg = argv[i];

        if ( arg == "-h" || arg == "--help" )
        {
            printUsage( argv[0] );
            return 0;
        }
        else if ( arg == "-v" || arg == "--version" )
        {
            printVersion();
            return 0;
        }
        else if ( arg == "--list-rules" )
        {
            list_rules = true;
        }
        else if ( arg == "-f" || arg == "--format" )
        {
            if ( i + 1 >= argc )
            {
                std::cerr << "Error: " << arg << " requires an argument\n";
                return 1;
            }
            ++i;
            auto parsed_format = parseOutputFormat( argv[i] );
            if ( !parsed_format )
            {
                std::cerr << "Error: Unknown format: " << argv[i] << "\n";
                return 1;
            }
            format = *parsed_format;
        }
        else if ( arg.starts_with( "--format=" ) )
        {
            std::string format_str = arg.substr( 9 );
            auto parsed_format = parseOutputFormat( format_str );
            if ( !parsed_format )
            {
                std::cerr << "Error: Unknown format: " << format_str << "\n";
                return 1;
            }
            format = *parsed_format;
        }
        else if ( arg == "--rules" )
        {
            if ( i + 1 >= argc )
            {
                std::cerr << "Error: --rules requires an argument\n";
                return 1;
            }
            ++i;
            std::string rules_arg = argv[i];
            size_t pos = 0;
            while ( pos < rules_arg.size() )
            {
                size_t comma = rules_arg.find( ',', pos );
                if ( comma == std::string::npos )
                {
                    comma = rules_arg.size();
                }
                std::string rule = rules_arg.substr( pos, comma - pos );
                size_t start = rule.find_first_not_of( " \t" );
                size_t end = rule.find_last_not_of( " \t" );
                if ( start != std::string::npos && end != std::string::npos )
                {
                    selected_rules.push_back( rule.substr( start, end - start + 1 ) );
                }
                pos = comma + 1;
            }
        }
        else if ( arg.starts_with( "--rules=" ) )
        {
            std::string rules_arg = arg.substr( 8 );
            size_t pos = 0;
            while ( pos < rules_arg.size() )
            {
                size_t comma = rules_arg.find( ',', pos );
                if ( comma == std::string::npos )
                {
                    comma = rules_arg.size();
                }
                std::string rule = rules_arg.substr( pos, comma - pos );
                size_t start = rule.find_first_not_of( " \t" );
                size_t end = rule.find_last_not_of( " \t" );
                if ( start != std::string::npos && end != std::string::npos )
                {
                    selected_rules.push_back( rule.substr( start, end - start + 1 ) );
                }
                pos = comma + 1;
            }
        }
        else if ( arg.starts_with( "-" ) )
        {
            std::cerr << "Error: Unknown option: " << arg << "\n";
            return 1;
        }
        else
        {
            files.push_back( arg );
        }
    }

    if ( list_rules )
    {
        printRules();
        return 0;
    }

    if ( files.empty() )
    {
        std::cerr << "Error: No file patterns specified\n";
        printUsage( argv[0] );
        return 1;
    }

    auto config = getDefaultConfig();

    if ( !selected_rules.empty() )
    {
        std::unordered_set<std::string> selected_set { selected_rules.begin(),
                                                       selected_rules.end() };
        for ( auto& [rule_name, severity] : config.rules )
        {
            if ( selected_set.find( rule_name ) == selected_set.end() )
            {
                severity = Severity::Off;
            }
        }
    }

    Linter linter { config };
    auto results = linter.lintFiles( files );
    std::string output = formatResults( results, format );

    if ( !output.empty() )
    {
        std::cout << output;
        if ( format != OutputFormat::Simple && output.back() != '\n' )
        {
            std::cout << '\n';
        }
    }

    int total_errors = 0;
    for ( const auto& result : results )
    {
        for ( const auto& violation : result.violations )
        {
            if ( violation.severity == Severity::Error )
            {
                ++total_errors;
            }
        }
    }

    return total_errors > 0 ? 1 : 0;
}
