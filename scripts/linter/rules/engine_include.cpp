#include <algorithm>
#include <unordered_set>

#include "../linter.hpp"

namespace wisdom_linter
{
namespace
{
    // The engine headers that are a frontend's interface: the game, what it
    // reports, and the value types the frontend names in its own API.
    auto frontendEngineHeaders() -> const std::unordered_set<std::string>&
    {
        static const std::unordered_set<std::string> headers {
            "global.hpp",
            "game.hpp",
            "game_status.hpp",
            "game_file.hpp",
            "coord.hpp",
            "move.hpp",
            "move_list.hpp",
            "piece.hpp",
            "move_timer.hpp",
            "transposition_table.hpp",
        };
        return headers;
    }

    // A frontend source: under a "ui" directory, and not in its tests, which
    // may look at the engine's state directly.
    auto isFrontendSource (const std::filesystem::path& filename) -> bool
    {
        bool in_ui = false;
        for (const auto& part : filename)
        {
            if (part == "ui")
            {
                in_ui = true;
            }
            else if (in_ui && part == "test")
            {
                return false;
            }
        }
        return in_ui;
    }

    // The file name in '#include "wisdom-chess/engine/<file>"', or empty.
    auto includedEngineHeader (const std::string& directive) -> std::string
    {
        static const std::string prefix = "\"wisdom-chess/engine/";
        if (directive.compare (0, 8, "#include") != 0 && directive.compare (0, 9, "# include") != 0)
        {
            return {};
        }
        auto start = directive.find (prefix);
        if (start == std::string::npos)
        {
            return {};
        }
        start += prefix.size();
        auto end = directive.find ('"', start);
        if (end == std::string::npos)
        {
            return {};
        }
        return directive.substr (start, end - start);
    }

    class EngineIncludeRule : public Rule
    {
    public:
        [[nodiscard]] auto name() const -> std::string_view override
        {
            return "engine-include";
        }

        [[nodiscard]] auto description() const -> std::string_view override
        {
            return "A frontend includes only the engine headers that are its interface";
        }

        [[nodiscard]] auto check (const LintContext& context) const
            -> std::vector<LintViolation> override
        {
            std::vector<LintViolation> violations;
            if (!isFrontendSource (context.filename))
            {
                return violations;
            }

            for (const auto& token : context.tokens)
            {
                if (token.kind != TokenKind::Preprocessor)
                {
                    continue;
                }
                auto header = includedEngineHeader (token.text);
                if (header.empty() || frontendEngineHeaders().count (header) > 0)
                {
                    continue;
                }
                violations.push_back (LintViolation {
                    std::string { name() },
                    "Frontend includes engine header '" + header
                        + "': ask the Game for what it needs, or add the header to "
                          "the rule's list",
                    token.line,
                    token.column,
                    Severity::Error,
                });
            }

            return violations;
        }
    };
} // namespace

auto createEngineIncludeRule() -> std::shared_ptr<Rule>
{
    return std::make_shared<EngineIncludeRule>();
}

} // namespace wisdom_linter
