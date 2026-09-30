#include <cctype>
#include <optional>
#include <unordered_set>

#include "../linter.hpp"

namespace wisdom_linter
{
namespace
{
    // The index past the template argument list that opens at code[open],
    // or open when it is not closed.
    auto skipTemplateArguments (const std::vector<Token>& code, size_t open) -> size_t
    {
        int depth = 0;
        for (size_t i = open; i < code.size(); ++i)
        {
            if (isPunctuator (code[i], "<"))
            {
                ++depth;
            }
            else if (isPunctuator (code[i], ">"))
            {
                --depth;
            }
            else if (isPunctuator (code[i], ">>"))
            {
                depth -= 2;
            }
            if (depth <= 0)
            {
                return i + 1;
            }
        }
        return open;
    }

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

        [[nodiscard]] auto check (const LintContext& context) const
            -> std::vector<LintViolation> override
        {
            std::vector<LintViolation> violations;
            auto code = codeTokens (context.tokens);

            for (size_t line_start = 0; line_start < code.size(); )
            {
                size_t line_end = line_start + 1;
                while (line_end < code.size() && code[line_end].line == code[line_start].line)
                {
                    ++line_end;
                }

                auto violation = checkLine (context, code, line_start, line_end);
                if (violation)
                {
                    violations.push_back (std::move (*violation));
                }
                line_start = line_end;
            }

            return violations;
        }

    private:
        // A declaration of a function with a leading return type, on the line
        // of tokens code[start, end).
        [[nodiscard]] auto checkLine (const LintContext& context, const std::vector<Token>& code,
                                      size_t start, size_t end) const
            -> std::optional<LintViolation>
        {
            static const std::unordered_set<std::string> skip_starts {
                "class", "struct", "enum", "union", "namespace", "using", "typedef", "template",
                "return", "auto",
            };
            static const std::unordered_set<std::string> control_statements {
                "if", "for", "while", "switch", "catch",
            };
            static const std::unordered_set<std::string> specifiers {
                "static", "inline", "virtual", "explicit", "constexpr", "consteval", "friend",
                "extern",
            };
            static const std::unordered_set<std::string> common_types {
                "char", "short", "long", "float", "double", "size_t", "int8_t", "int16_t",
                "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t", "string",
                "wstring",
            };

            const auto& first = code[start];
            if ((isIdentifier (first) && skip_starts.count (first.text) > 0) || isPunctuator (first, "*")
                || (isPunctuator (first, "[") && start + 1 < end && isPunctuator (code[start + 1], "[")))
            {
                return std::nullopt;
            }

            bool has_body_or_end = false;
            for (size_t i = start; i < end; ++i)
            {
                bool followed_by_paren = i + 1 < end && isPunctuator (code[i + 1], "(");
                bool lambda = isPunctuator (code[i], "=") && i + 1 < end && isPunctuator (code[i + 1], "[");
                if ((isIdentifier (code[i]) && control_statements.count (code[i].text) > 0 && followed_by_paren)
                    || lambda || isPunctuator (code[i], "->"))
                {
                    return std::nullopt;
                }
                has_body_or_end = has_body_or_end || isPunctuator (code[i], "{") || isPunctuator (code[i], ";");
            }

            size_t type_start = start;
            while (type_start < end && isIdentifier (code[type_start])
                   && specifiers.count (code[type_start].text) > 0)
            {
                ++type_start;
            }
            if (type_start >= end || !isIdentifier (code[type_start]))
            {
                return std::nullopt;
            }

            // The return type: one of the common types, possibly a pointer or
            // reference, or a PascalCase type with any template arguments.
            size_t type_end = type_start + 1;
            size_t name_index = type_end;
            const auto& type_name = code[type_start].text;
            if (common_types.count (type_name) > 0)
            {
                if (type_end < end && isPunctuator (code[type_end], "::"))
                {
                    return std::nullopt;
                }
                while (name_index < end && (isPunctuator (code[name_index], "*")
                       || isPunctuator (code[name_index], "&") || isPunctuator (code[name_index], "&&")))
                {
                    ++name_index;
                }
            }
            else if (std::isupper (static_cast<unsigned char> (type_name[0])))
            {
                if (type_end < end && isPunctuator (code[type_end], "<"))
                {
                    type_end = skipTemplateArguments (code, type_end);
                    if (type_end == type_start + 1 || type_end > end)
                    {
                        return std::nullopt;
                    }
                }
                name_index = type_end;
                if (name_index >= end || !code[name_index].spaced_before)
                {
                    return std::nullopt;
                }
            }
            else
            {
                return std::nullopt;
            }

            if (name_index + 1 >= end || !isIdentifier (code[name_index])
                || !isPunctuator (code[name_index + 1], "("))
            {
                return std::nullopt;
            }

            const auto& func_name = code[name_index].text;
            const auto& line = context.lines[static_cast<size_t> (code[start].line - 1)];
            const auto& last_type_token = code[type_end - 1];
            auto type_begin = static_cast<size_t> (code[type_start].column - 1);
            auto type_finish = static_cast<size_t> (last_type_token.column - 1) + last_type_token.text.size();
            std::string return_type = line.substr (type_begin, type_finish - type_begin);

            if (func_name == "main" || return_type == func_name || func_name == "TEST_CASE"
                || func_name == "SUBCASE" || func_name == "SECTION")
            {
                return std::nullopt;
            }

            bool next_line_opens = end < code.size() && code[end].line == code[start].line + 1
                && isPunctuator (code[end], "{");
            if (!has_body_or_end && !next_line_opens)
            {
                return std::nullopt;
            }

            return LintViolation {
                std::string { name() },
                "Function '" + func_name + "' should use trailing return type: auto " + func_name
                    + "(...) -> " + return_type,
                code[start].line,
                1,
                Severity::Warning,
            };
        }
    };
} // namespace

auto createTrailingReturnTypeRule() -> std::shared_ptr<Rule>
{
    return std::make_shared<TrailingReturnTypeRule>();
}

} // namespace wisdom_linter
