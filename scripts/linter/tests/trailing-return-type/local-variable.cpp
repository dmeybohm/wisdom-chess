namespace sample
{
    auto countBuckets (size_t table_size) -> size_t
    {
        std::vector<size_t> bucket_counts (table_size, 0);
        std::istringstream stream (line);
        Buckets buckets (table_size);

        struct Local
        {
            std::string name() const
            {
                return "local";
            }
        };

        auto sum = [&bucket_counts] (size_t index)
        {
            std::vector<size_t> copy (bucket_counts);
            return copy[index];
        };
        return sum (0);
    }

    template <class Type>
    class Holder
    {
    public:
        Holder (Type value)
            : my_value { value }
        {
            std::vector<Type> values (1, value);
        }

        std::string describe (int code);

    private:
        Type my_value {};
    };
}

extern "C"
{
    std::size_t countAll (int code);
}
