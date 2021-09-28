#include "uuid.hpp"

#include <random>

namespace wisdom
{
    using std::optional;
    using std::uniform_int_distribution;
    using std::default_random_engine;
    using std::nullopt;

    static int64_t generate()
    {
        static optional<default_random_engine> storage = nullopt;
        static uniform_int_distribution<int64_t> distribution(0, 0x7fffFFFFffffFFFF);

        if (!storage.has_value ())
        {
            unsigned int seed;
            time_t now = time (nullptr);
            memcpy (&seed, &now, std::min (sizeof (time_t), sizeof (seed)));
            storage = default_random_engine { seed };
        }

        return distribution(*storage);
    }

    Uuid::Uuid()
    {
        my_rand64 = generate ();
    }

    [[nodiscard]] std::string Uuid::to_string () const
    {
        return std::to_string (my_rand64);
    }
}