#include "uuid.hpp"

namespace wisdom
{
    Uuid::Uuid()
    {
        char buf[48];

        static bool seeded = false;
        if (!seeded) {
            srand(time(NULL));
            seeded = true;
        }

        int ints[] = {
                rand(),
                rand(),
                rand(),
                rand()
        };
        sprintf(
                buf,
                "'%08x-%04x-%04x-%04x-%08x%04x'",
                ints[0] & 0xffffffff,
                ints[1] & 0xffff,
                (ints[1] >> 16) & 0xffff,
                ints[2] & 0xffff,
                ints[3],
                (ints[2] >> 16) & 0xffff
        );

        my_buf = buf;
    }

    std::string Uuid::to_string () const
    {
        return my_buf;
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
