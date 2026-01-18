#include "../linter.hpp"

namespace wisdom_linter
{

extern auto createNamespaceBracesRule() -> std::shared_ptr<Rule>;
extern auto createTrailingReturnTypeRule() -> std::shared_ptr<Rule>;
extern auto createTestMacroSpacingRule() -> std::shared_ptr<Rule>;
extern auto createFunctionCallSpacingRule() -> std::shared_ptr<Rule>;

auto registerAllRules() -> std::vector<std::shared_ptr<Rule>>
{
    return {
        createNamespaceBracesRule(),
        createTrailingReturnTypeRule(),
        createTestMacroSpacingRule(),
        createFunctionCallSpacingRule(),
    };
}

} // namespace wisdom_linter
