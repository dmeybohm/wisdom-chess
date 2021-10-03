#include "uuid.hpp"

#include <random>
#include <cstring>

namespace wisdom
{
    using std::uniform_int_distribution;
    using Rng = std::mt19937_64;

    static int64_t generate()
    {
        static optional<Rng> storage = nullopt;
        static uniform_int_distribution<int64_t> distribution(0, 0x7FFFffffFFFFffffLL);

        if (!storage.has_value ())
        {
            unsigned int seed;
            // todo use chrono
            time_t now = time (nullptr);
            std::memcpy (&seed, &now, std::min (sizeof (time_t), sizeof (seed)));
            storage = Rng { seed };
        }

        return distribution (*storage);
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
