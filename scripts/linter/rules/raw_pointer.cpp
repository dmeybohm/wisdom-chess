#include <cctype>
#include <optional>
#include <unordered_set>

#include "../linter.hpp"

namespace wisdom_linter
{
namespace
{
    auto isQtType (const std::string& name) -> bool
    {
        return name.size() > 1 && name[0] == 'Q'
            && std::isupper (static_cast<unsigned char> (name[1]));
    }

    // Words that can come just before a '*' that dereferences, rather than
    // one that declares a pointer.
    auto isExpressionKeyword (const std::string& word) -> bool
    {
        static const std::unordered_set<std::string> keywords {
            "return", "delete", "throw", "case", "else", "do", "sizeof", "alignof",
            "co_return", "co_yield", "co_await", "operator", "new",
        };
        return keywords.count (word) > 0;
    }

    // Lower-case type names that global.hpp brings into the wisdom namespace.
    auto globalTypeAliases() -> const std::unordered_set<std::string>&
    {
        static const std::unordered_set<std::string> aliases {
            "string", "string_view", "vector", "array", "optional", "pair", "span",
            "unique_ptr", "shared_ptr", "czstring", "zstring",
        };
        return aliases;
    }

    // Names the file introduces with "using X = ..." or "using ns::X;".
    auto localTypeAliases (const std::vector<Token>& code) -> std::unordered_set<std::string>
    {
        std::unordered_set<std::string> aliases;
        for (size_t i = 0; i + 1 < code.size(); ++i)
        {
            if (!isIdentifier (code[i]) || code[i].text != "using" || code[i + 1].text == "namespace")
            {
                continue;
            }

            if (isIdentifier (code[i + 1]) && i + 2 < code.size() && isPunctuator (code[i + 2], "="))
            {
                aliases.insert (code[i + 1].text);
                continue;
            }

            std::string last_name;
            size_t j = i + 1;
            while (j < code.size() && (isIdentifier (code[j]) || isPunctuator (code[j], "::")))
            {
                if (isIdentifier (code[j]))
                {
                    last_name = code[j].text;
                }
                ++j;
            }
            if (j < code.size() && isPunctuator (code[j], ";") && !last_name.empty())
            {
                aliases.insert (last_name);
            }
        }
        return aliases;
    }

    // Tokens that can appear in a template argument list. Anything else, such
    // as "&&" or ";", means a '<' before it compares rather than opens one.
    auto isTemplateArgumentToken (const Token& token) -> bool
    {
        static const std::unordered_set<std::string> punctuators {
            "::", ",", "*", "&", "<", ">", ">>", "...", "(", ")", "[", "]",
        };
        return isIdentifier (token) || token.kind == TokenKind::Number
            || (token.kind == TokenKind::Punctuator && punctuators.count (token.text) > 0);
    }

    struct PointeeType
    {
        // As written, with template arguments and qualifiers: "vector<Board>".
        std::string text;

        // Without them: "vector".
        std::string name;

        bool qualified_or_template;

        // Index of its first token.
        size_t start;
    };

    // The type that ends at code[last]: an identifier, possibly qualified,
    // with its template argument list when code[last] closes one.
    auto pointeeEndingAt (const std::vector<Token>& code, size_t last) -> std::optional<PointeeType>
    {
        size_t name_index = last;
        bool is_template = false;
        if (isPunctuator (code[last], ">") || isPunctuator (code[last], ">>"))
        {
            int depth = 0;
            size_t j = last + 1;
            while (j > 0)
            {
                --j;
                if (isPunctuator (code[j], ">"))
                {
                    depth += 1;
                }
                else if (isPunctuator (code[j], ">>"))
                {
                    depth += 2;
                }
                else if (isPunctuator (code[j], "<"))
                {
                    if (--depth == 0)
                    {
                        break;
                    }
                }
                else if (!isTemplateArgumentToken (code[j]))
                {
                    return std::nullopt;
                }
            }
            if (depth != 0 || j == 0)
            {
                return std::nullopt;
            }
            name_index = j - 1;
            is_template = true;
        }

        if (!isIdentifier (code[name_index]))
        {
            return std::nullopt;
        }

        size_t start = name_index;
        while (start >= 2 && isPunctuator (code[start - 1], "::") && isIdentifier (code[start - 2]))
        {
            start -= 2;
        }
        if (start >= 1 && isPunctuator (code[start - 1], "::"))
        {
            --start;
        }

        std::string text;
        for (size_t k = start; k <= last; ++k)
        {
            text += code[k].text;
            if (isPunctuator (code[k], ","))
            {
                text += ' ';
            }
        }
        return PointeeType { text, code[name_index].text, is_template || start != name_index, start };
    }

    // Whether a type reads as one by the project's naming: PascalCase, a
    // built-in or *_t type, a known alias, or anything with template
    // arguments or a namespace. Variables are snake_case and constants
    // Capitalized_Snake, so a '*' between two of those multiplies.
    auto looksLikeType (const PointeeType& type, const std::unordered_set<std::string>& aliases)
        -> bool
    {
        static const std::unordered_set<std::string> builtins {
            "void", "bool", "char", "short", "int", "long", "float", "double",
            "signed", "unsigned", "wchar_t", "char8_t", "char16_t", "char32_t",
        };
        const auto& name = type.name;
        return type.qualified_or_template
            || builtins.count (name) > 0 || globalTypeAliases().count (name) > 0
            || aliases.count (name) > 0
            || (name.size() > 2 && name.compare (name.size() - 2, 2, "_t") == 0)
            || (std::isupper (static_cast<unsigned char> (name[0]))
                && name.find ( '_' ) == std::string::npos);
    }

    // Whether the '*' at code[star] declares a pointer to the type before it,
    // rather than multiplying.
    auto declaresPointer (const std::vector<Token>& code, size_t star, const PointeeType& type,
                          const std::unordered_set<std::string>& aliases)
        -> bool
    {
        bool attached_left = !code[star].spaced_before;
        bool follows_arrow = type.start > 0 && isPunctuator (code[type.start - 1], "->");

        // A '*' that ends the source, or its line before a name, is a
        // trailing return type or an expression that goes on.
        if (star + 1 >= code.size())
        {
            return attached_left || follows_arrow;
        }

        const auto& next = code[star + 1];
        bool attached_right = !next.spaced_before;
        if (next.kind == TokenKind::Punctuator)
        {
            if (next.text == "*" || next.text == "&" || next.text == "&&")
            {
                return attached_right;
            }
            static const std::unordered_set<std::string> type_enders {
                ",", ")", ">", ">>", ";", "=", "{", "[", "...",
            };
            return type_enders.count (next.text) > 0;
        }
        if (!isIdentifier (next))
        {
            return false;
        }
        if (next.line > code[star].line)
        {
            return attached_left || follows_arrow;
        }

        // A declaration is spelled "T* name" in this project, or "T *name"
        // in C style. Spaced on both sides or neither, it may multiply.
        if (attached_left != attached_right)
        {
            return true;
        }
        return looksLikeType (type, aliases);
    }

    class RawPointerRule : public Rule
    {
    public:
        [[nodiscard]] auto name() const -> std::string_view override
        {
            return "raw-pointer";
        }

        [[nodiscard]] auto description() const -> std::string_view override
        {
            return "Non-owning pointers should be nonnull<T> or nullable<T>, and C strings "
                   "czstring or zstring";
        }

        [[nodiscard]] auto check (const LintContext& context) const
            -> std::vector<LintViolation> override
        {
            std::vector<LintViolation> violations;
            auto code = codeTokens (context.tokens);
            auto aliases = localTypeAliases (code);

            for (size_t star = 1; star < code.size(); ++star)
            {
                if (!isPunctuator (code[star], "*"))
                {
                    continue;
                }

                // In "T const*", the type is the token before the qualifier.
                size_t last = star - 1;
                if (isIdentifier (code[last]) && (code[last].text == "const" || code[last].text == "volatile"))
                {
                    if (last == 0)
                    {
                        continue;
                    }
                    --last;
                }
                if (!isIdentifier (code[last]) && !isPunctuator (code[last], ">")
                    && !isPunctuator (code[last], ">>"))
                {
                    continue;
                }

                auto type = pointeeEndingAt (code, last);
                if (!type || type->name == "auto" || isExpressionKeyword (type->name)
                    || isQtType (type->name) || !declaresPointer (code, star, *type, aliases))
                {
                    continue;
                }

                std::string message = type->name == "char"
                    ? "C string: use czstring or zstring, or span or string_view for a buffer"
                    : "Raw pointer to '" + type->text + "': use nonnull<" + type->text
                        + "> or nullable<" + type->text
                        + ">, or mark an interop pointer lint-allow(raw-pointer)";

                violations.push_back (LintViolation {
                    std::string { name() },
                    std::move (message),
                    code[star].line,
                    code[star].column,
                    Severity::Error,
                });
            }

            return violations;
        }
    };
} // namespace

auto createRawPointerRule() -> std::shared_ptr<Rule>
{
    return std::make_shared<RawPointerRule>();
}

} // namespace wisdom_linter
