#include <cctype>
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

    // The directive with its comments and line continuations replaced by
    // spaces, so that what remains is the directive's own text.
    auto withoutComments (const std::string& directive) -> std::string
    {
        std::string text;
        for (size_t i = 0; i < directive.size(); ++i)
        {
            if (directive.compare (i, 2, "/*") == 0)
            {
                auto end = directive.find ("*/", i + 2);
                i = end == std::string::npos ? directive.size() : end + 1;
                text += ' ';
            }
            else if (directive.compare (i, 2, "//") == 0)
            {
                break;
            }
            else if (directive.compare (i, 2, "\\\n") == 0)
            {
                ++i;
                text += ' ';
            }
            else
            {
                text += directive[i];
            }
        }
        return text;
    }

    auto skipSpace (const std::string& text, size_t pos) -> size_t
    {
        while (pos < text.size() && std::isspace (static_cast<unsigned char> (text[pos])))
        {
            ++pos;
        }
        return pos;
    }

    // The operand of an #include directive, as written between its quotes
    // or angle brackets, or empty when the directive is not an include or
    // names its header through a macro.
    auto includeOperand (const std::string& directive) -> std::string
    {
        auto text = withoutComments (directive);
        size_t pos = skipSpace (text, 0);
        if (pos >= text.size() || text[pos] != '#')
        {
            return {};
        }
        pos = skipSpace (text, pos + 1);
        static const std::string keyword = "include";
        if (text.compare (pos, keyword.size(), keyword) != 0)
        {
            return {};
        }
        pos += keyword.size();
        if (pos < text.size() && (std::isalnum (static_cast<unsigned char> (text[pos])) || text[pos] == '_'))
        {
            return {};
        }
        pos = skipSpace (text, pos);
        if (pos >= text.size() || (text[pos] != '"' && text[pos] != '<'))
        {
            return {};
        }
        char closer = text[pos] == '"' ? '"' : '>';
        auto end = text.find (closer, pos + 1);
        if (end == std::string::npos)
        {
            return {};
        }
        return text.substr (pos + 1, end - pos - 1);
    }

    // The file name in an include of "wisdom-chess/engine/<file>", or empty.
    auto includedEngineHeader (const std::string& directive) -> std::string
    {
        static const std::string prefix = "wisdom-chess/engine/";
        auto operand = includeOperand (directive);
        if (operand.compare (0, prefix.size(), prefix) != 0)
        {
            return {};
        }
        return operand.substr (prefix.size());
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
