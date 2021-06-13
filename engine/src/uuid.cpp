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

        sprintf(
                buf,
                "'%08x-%04x-%04x-%04x-%08x%04x'",
                rand(),
                rand(),
                rand(),
                rand(),
                rand(),
                rand()
        );

        my_buf = buf;
    }

    std::string Uuid::to_string () const
    {
        return my_buf;
    }

}