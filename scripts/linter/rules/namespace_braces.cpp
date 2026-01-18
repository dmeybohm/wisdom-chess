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

        [[nodiscard]] auto check( const LintContext& context ) const
            -> std::vector<LintViolation> override
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
    };
} // namespace

auto createNamespaceBracesRule() -> std::shared_ptr<Rule>
{
    return std::make_shared<NamespaceBracesRule>();
}

} // namespace wisdom_linter
