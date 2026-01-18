#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace wisdom_linter
{

enum class Severity
{
    Error,
    Warning,
    Off
};

struct LintViolation
{
    std::string rule;
    std::string message;
    int line;
    int column;
    Severity severity;
};

struct LintContext
{
    std::filesystem::path filename;
    std::vector<std::string> lines;
    std::string content;
};

struct LintResult
{
    std::filesystem::path filename;
    std::vector<LintViolation> violations;
};

class Rule
{
public:
    virtual ~Rule() = default;
    [[nodiscard]] virtual auto name() const -> std::string_view = 0;
    [[nodiscard]] virtual auto description() const -> std::string_view = 0;
    [[nodiscard]] virtual auto check( const LintContext& context ) const
        -> std::vector<LintViolation> = 0;
};

enum class OutputFormat
{
    Stylish,
    Compact,
    Json,
    Simple
};

struct LinterConfig
{
    std::unordered_map<std::string, Severity> rules;
    std::vector<std::string> ignore;
};

class Linter
{
public:
    explicit Linter( const LinterConfig& config );

    [[nodiscard]] auto lintFiles( const std::vector<std::string>& patterns ) const
        -> std::vector<LintResult>;
    [[nodiscard]] auto lintFile( const std::filesystem::path& filename ) const -> LintResult;

private:
    [[nodiscard]] auto resolveFiles( const std::vector<std::string>& patterns ) const
        -> std::vector<std::filesystem::path>;
    [[nodiscard]] static auto isCppFile( const std::filesystem::path& path ) -> bool;
    [[nodiscard]] auto getEnabledRules() const -> std::vector<std::shared_ptr<Rule>>;

    LinterConfig my_config;
    std::vector<std::shared_ptr<Rule>> my_rules;
};

[[nodiscard]] auto formatResults( const std::vector<LintResult>& results, OutputFormat format )
    -> std::string;

[[nodiscard]] auto parseOutputFormat( std::string_view format_str ) -> std::optional<OutputFormat>;

[[nodiscard]] auto getDefaultConfig() -> LinterConfig;

[[nodiscard]] auto getAllRules() -> std::vector<std::shared_ptr<Rule>>;

[[nodiscard]] auto registerAllRules() -> std::vector<std::shared_ptr<Rule>>;

} // namespace wisdom_linter
