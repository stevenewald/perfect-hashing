#include "gloss/gloss.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Name is gloss", "[library]")
{
    REQUIRE(gloss::name() == "gloss");
}

TEST_CASE("Map int to int", "[library]")
{
    static constexpr auto TEST = std::array{
        std::pair<uint32_t, uint32_t>{7, 8},
         std::pair<uint32_t, uint32_t>{5, 6}
    };
    REQUIRE(gloss::lookup<TEST>(5u) == 6);
    REQUIRE(gloss::lookup<TEST>(7u) == 8);
}

TEST_CASE("Map many ints to int", "[library]")
{
    static constexpr auto TEST = std::array{
        std::pair<uint32_t, uint32_t>{1,  11},
         std::pair<uint32_t, uint32_t>{2,  12},
        std::pair<uint32_t, uint32_t>{3,  13},
         std::pair<uint32_t, uint32_t>{4,  14},
        std::pair<uint32_t, uint32_t>{5,  15},
         std::pair<uint32_t, uint32_t>{6,  16},
        std::pair<uint32_t, uint32_t>{7,  17},
         std::pair<uint32_t, uint32_t>{8,  18},
        std::pair<uint32_t, uint32_t>{9,  19},
         std::pair<uint32_t, uint32_t>{10, 20},
        std::pair<uint32_t, uint32_t>{11, 20},
         std::pair<uint32_t, uint32_t>{12, 20}
    };
    for (std::uint32_t i = 1; i <= 5; ++i) {
        REQUIRE(gloss::lookup<TEST>(i) == i + 10);
    }
}
