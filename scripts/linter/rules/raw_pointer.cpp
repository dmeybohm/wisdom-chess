#include "../linter.hpp"

#include <cctype>

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

    // Whether what follows a '*' can end a declared type: a name, the end of
    // a parameter or template argument list, another '*' or '&', or the end
    // of the line (a trailing return type).
    auto endsDeclaredType (const std::string& line, size_t star) -> bool
    {
        if (star + 1 < line.size() && line[star + 1] == '=' )
        {
            return false;
        }

        size_t next = line.find_first_not_of ( ' ', star + 1);
        if (next == std::string::npos)
        {
            return true;
        }

        char c = line[next];
        return isIdentifierChar (c) || c == ',' || c == ')' || c == '>' || c == '&'
            || c == '*' || c == ';' || c == '=' || c == '{' || c == '[';
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
                    if (star == 0 || !(isIdentifierChar (line[star - 1]) || line[star - 1] == '>'))
                    {
                        continue;
                    }

                    size_t start = pointeeStart (line, star);
                    auto type = line.substr (start, star - start);
                    auto base_name = lastComponent (type);
                    if (base_name.empty() || std::isdigit (static_cast<unsigned char> (base_name[0]))
                        || base_name == "operator" || base_name == "auto" || isQtType (base_name)
                        || !endsDeclaredType (line, star))
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
