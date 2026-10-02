class Error : public std::exception
{
public:
    [[nodiscard]] gsl::czstring what() const noexcept override
    {
        return "error";
    }

    std::vector<int> getCodes() const;

    Error::Kind getKind() const
    {
        return my_kind;
    }

    [[nodiscard]] auto message() const -> std::string;
};

std::string_view describe (int code);

std::vector<int>::iterator findValue();

std::map<std::string, std::vector<int>>::const_iterator findEntry (int code);

Holder<int>::Value getHeld();

Error::Error (int code)
    : my_code { code }
{
}

Error::~Error()
{
}

auto Error::run (int code) -> void
{
    std::sort (my_codes.begin(), my_codes.end());
    wisdom::detail::report (code);
    std::vector<int>::iterator found;
    if (std::find (my_codes.begin(), my_codes.end(), code) == my_codes.end())
    {
        throw std::runtime_error ("missing");
    }
}

auto Error::operator== (const Error& other) const -> bool
{
    return std::equal (my_codes.begin(), my_codes.end(), other.my_codes.begin());
}
