#include "tests.hpp"
#include "uuid.hpp"

using wisdom::Uuid;

TEST_CASE("Random uuids can be generated")
{
    Uuid uuidOne;
    Uuid uuidTwo;

    bool equals = uuidOne == uuidTwo;
    REQUIRE( !equals );
}