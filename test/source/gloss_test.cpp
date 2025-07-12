#include "gloss/gloss.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Name is gloss", "[library]")
{
    REQUIRE(name() == "gloss");
}

TEST_CASE("Map int to int", "[library]")
{
    static constexpr auto TEST = std::array{
        std::pair{1, -1},
        std::pair{2, -2}
    };
    REQUIRE(lookup<TEST>(1) == -1);
    REQUIRE(lookup<TEST>(2) == -2);
}
