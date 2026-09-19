#include "../linter.hpp"

namespace wisdom_linter
{
namespace
{
    class NoTabsRule : public Rule
    {
    public:
        [[nodiscard]] auto name() const -> std::string_view override
        {
            return "no-tabs";
        }

        [[nodiscard]] auto description() const -> std::string_view override
        {
            return "Source files should not contain tab characters";
        }

        [[nodiscard]] auto check( const LintContext& context ) const
            -> std::vector<LintViolation> override
        {
            std::vector<LintViolation> violations;

            for ( size_t i = 0; i < context.lines.size(); ++i )
            {
                size_t tab_pos = context.lines[i].find( '\t' );
                if ( tab_pos == std::string::npos )
                {
                    continue;
                }

                violations.push_back( LintViolation {
                    std::string { name() },
                    "Tab character found; use spaces",
                    static_cast<int>( i + 1 ),
                    static_cast<int>( tab_pos + 1 ),
                    Severity::Error,
                } );
            }

            return violations;
        }
    };
} // namespace

auto createNoTabsRule() -> std::shared_ptr<Rule>
{
    return std::make_shared<NoTabsRule>();
}

} // namespace wisdom_linter
