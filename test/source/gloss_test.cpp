#include "gloss/gloss.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Name is gloss", "[library]")
{
    REQUIRE(name() == "gloss");
}
