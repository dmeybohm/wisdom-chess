#include "../linter.hpp"

#include <cctype>
#include <unordered_set>

namespace wisdom_linter
{
namespace
{
    auto isIdentifierChar (char c) -> bool
    {
        return std::isalnum (static_cast<unsigned char> (c)) || c == '_';
    }

    // Blank the contents of string and character literals, so that a '*'
    // inside one is not taken for a pointer. A quote after a digit is a
    // digit separator.
    auto blankLiterals (std::string line) -> std::string
    {
        char quote = 0;
        for (size_t i = 0; i < line.size(); ++i)
        {
            char c = line[i];
            if (quote == 0)
            {
                bool digit_separator = c == '\'' && i > 0
                    && std::isxdigit (static_cast<unsigned char> (line[i - 1]));
                if ((c == '"' || c == '\'') && !digit_separator)
                {
                    quote = c;
                }
                continue;
            }

            if (c == quote)
            {
                quote = 0;
                continue;
            }

            if (c == '\\' && i + 1 < line.size())
            {
                line[i] = ' ';
                ++i;
            }
            line[i] = ' ';
        }
        return line;
    }

    // Where the type before a '*' starts: an identifier, possibly qualified,
    // and its template argument list if one ends at the '*'. Returns star
    // when there is no such type.
    auto pointeeStart (const std::string& line, size_t star) -> size_t
    {
        size_t end = star;
        if (line[end - 1] == '>')
        {
            int depth = 0;
            size_t j = end;
            while (j > 0)
            {
                --j;
                if (line[j] == '>')
                {
                    ++depth;
                }
                else if (line[j] == '<' && --depth == 0)
                {
                    break;
                }
            }
            if (depth != 0)
            {
                return star;
            }
            end = j;
        }

        size_t start = end;
        while (start > 0 && (isIdentifierChar (line[start - 1]) || line[start - 1] == ':'))
        {
            --start;
        }
        return start == end ? star : start;
    }

    // The unqualified name of a type, without template arguments.
    auto lastComponent (const std::string& type) -> std::string
    {
        auto name = type.substr (0, type.find ( '<' ));
        size_t colon = name.rfind ( ':' );
        return colon == std::string::npos ? name : name.substr (colon + 1);
    }

    auto isQtType (const std::string& name) -> bool
    {
        return name.size() > 1 && name[0] == 'Q'
            && std::isupper (static_cast<unsigned char> (name[1]));
    }

    auto isIdentifierStart (char c) -> bool
    {
        return std::isalpha (static_cast<unsigned char> (c)) || c == '_';
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
    const std::unordered_set<std::string>& globalTypeAliases()
    {
        static const std::unordered_set<std::string> aliases {
            "string", "string_view", "vector", "array", "optional", "pair", "span",
            "unique_ptr", "shared_ptr", "czstring", "zstring",
        };
        return aliases;
    }

    // Names a file introduces with "using X = ..." or "using ns::X;".
    auto localTypeAliases (const std::vector<std::string>& lines)
        -> std::unordered_set<std::string>
    {
        std::unordered_set<std::string> aliases;
        for (const auto& line : lines)
        {
            size_t using_pos = line.find ( "using " );
            if (using_pos == std::string::npos
                || (using_pos > 0 && isIdentifierChar (line[using_pos - 1]))
                || line.find ( "namespace", using_pos) != std::string::npos)
            {
                continue;
            }

            // The name can be on a later line, after a comment; that alias
            // is missed rather than guessed at.
            size_t name_start = line.find_first_not_of ( ' ', using_pos + 6);
            if (name_start == std::string::npos)
            {
                continue;
            }
            size_t name_end = name_start;
            while (name_end < line.size() && (isIdentifierChar (line[name_end]) || line[name_end] == ':'))
            {
                ++name_end;
            }
            auto name = line.substr (name_start, name_end - name_start);
            if (name.empty())
            {
                continue;
            }
            size_t colon = name.rfind ( ':' );
            aliases.insert (colon == std::string::npos ? name : name.substr (colon + 1));
        }
        return aliases;
    }

    // Whether a name reads as a type by the project's naming: PascalCase,
    // a built-in or *_t type, a known alias, or anything with template
    // arguments or a namespace. Variables are snake_case and constants
    // Capitalized_Snake, so a '*' between two of those multiplies.
    auto looksLikeType (const std::string& type, const std::unordered_set<std::string>& aliases)
        -> bool
    {
        static const std::unordered_set<std::string> builtins {
            "void", "bool", "char", "short", "int", "long", "float", "double",
            "signed", "unsigned", "wchar_t", "char8_t", "char16_t", "char32_t",
            "const", "volatile",
        };
        if (type.find ( '<' ) != std::string::npos || type.find ( "::" ) != std::string::npos)
        {
            return true;
        }

        auto name = lastComponent (type);
        return builtins.count (name) > 0 || globalTypeAliases().count (name) > 0
            || aliases.count (name) > 0
            || (name.size() > 2 && name.compare (name.size() - 2, 2, "_t") == 0)
            || (std::isupper (static_cast<unsigned char> (name[0]))
                && name.find ( '_' ) == std::string::npos);
    }

    // Whether a trailing return type's "->" comes just before position.
    auto followsArrow (const std::string& line, size_t position) -> bool
    {
        size_t end = line.find_last_not_of ( ' ', position == 0 ? 0 : position - 1);
        return end != std::string::npos && end >= 1 && line[end] == '>' && line[end - 1] == '-';
    }

    // Whether the '*' at star, after the type that spans [type_start,
    // type_end), declares a pointer rather than multiplying.
    auto declaresPointer (const std::string& line, size_t type_start, size_t type_end, size_t star,
                          const std::string& type, const std::unordered_set<std::string>& aliases)
        -> bool
    {
        bool attached_left = type_end == star;
        if (star + 1 < line.size() && line[star + 1] == '=' )
        {
            return false;
        }

        // At the end of a line: a trailing return type, or an expression
        // that continues on the next line.
        size_t next = line.find_first_not_of ( ' ', star + 1);
        if (next == std::string::npos)
        {
            return attached_left || followsArrow (line, type_start);
        }

        char c = line[next];
        bool attached_right = next == star + 1;
        if (c == '*' || c == '&')
        {
            return attached_right;
        }
        if (c == ',' || c == ')' || c == '>' || c == ';' || c == '=' || c == '{' || c == '[')
        {
            return true;
        }
        if (!isIdentifierStart (c))
        {
            return false;
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
            auto code_lines = stripComments (context.lines);
            auto aliases = localTypeAliases (code_lines);

            for (size_t i = 0; i < code_lines.size(); ++i)
            {
                auto line = blankLiterals (code_lines[i]);
                size_t first = line.find_first_not_of ( ' ' );
                if (first == std::string::npos || line[first] == '#' )
                {
                    continue;
                }

                for (size_t star = line.find ( '*' ); star != std::string::npos;
                     star = line.find ( '*', star + 1))
                {
                    size_t type_end = star;
                    while (type_end > 0 && line[type_end - 1] == ' ' )
                    {
                        --type_end;
                    }
                    if (type_end == 0
                        || !(isIdentifierChar (line[type_end - 1]) || line[type_end - 1] == '>'))
                    {
                        continue;
                    }

                    size_t start = pointeeStart (line, type_end);
                    auto type = line.substr (start, type_end - start);
                    auto base_name = lastComponent (type);
                    if (base_name == "const" || base_name == "volatile")
                    {
                        size_t before = start;
                        while (before > 0 && line[before - 1] == ' ' )
                        {
                            --before;
                        }
                        size_t qualified_start = before > 0 ? pointeeStart (line, before) : before;
                        if (qualified_start < before)
                        {
                            type = line.substr (qualified_start, before - qualified_start);
                            base_name = lastComponent (type);
                        }
                    }

                    if (base_name.empty() || !isIdentifierStart (base_name[0])
                        || base_name == "auto" || isExpressionKeyword (base_name)
                        || isQtType (base_name)
                        || !declaresPointer (line, start, type_end, star, type, aliases))
                    {
                        continue;
                    }

                    std::string message = base_name == "char"
                        ? "C string: use czstring or zstring, or span or string_view for a buffer"
                        : "Raw pointer to '" + type + "': use nonnull<" + type + "> or nullable<"
                            + type + ">, or mark an interop pointer lint-allow(raw-pointer)";

                    violations.push_back (LintViolation {
                        std::string { name() },
                        std::move (message),
                        static_cast<int> (i + 1),
                        static_cast<int> (star + 1),
                        Severity::Error,
                    });
                }
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
