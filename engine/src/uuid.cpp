#include "uuid.hpp"

namespace wisdom
{
    Uuid::Uuid()
    {
        static bool seeded = false;
        if (!seeded) {
            srand(time(NULL));
            seeded = true;
        }

        int ints[] = {
                rand(),
                rand(),
        };
        my_rand64 = static_cast<int64_t>(ints[0]) << 31 | static_cast<int64_t>(ints[1]);
    }

    [[nodiscard]] std::string Uuid::to_string () const
    {
        return std::to_string (my_rand64);
    }
}

#ifdef UUID_TEST
#include <iostream>

int main()
{
    for (int i = 0; i < 1000; i++)
    {
        wisdom::Uuid uuid;
        std::cout << uuid.to_string() << '\n';
    }
}
#endif
