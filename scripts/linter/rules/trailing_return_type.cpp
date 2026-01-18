#include "../linter.hpp"

#include <cctype>

namespace wisdom_linter
{
namespace
{
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
            -> std::vector<LintViolation> override
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
    };
} // namespace

auto createTrailingReturnTypeRule() -> std::shared_ptr<Rule>
{
    return std::make_shared<TrailingReturnTypeRule>();
}

} // namespace wisdom_linter
