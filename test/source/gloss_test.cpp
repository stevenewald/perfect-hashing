#include "gloss/gloss.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Name is gloss", "[library]")
{
    REQUIRE(name() == "gloss");
}

TEST_CASE("Map int to int", "[library]")
{
    static constexpr auto TEST = std::array{
        std::pair<uint32_t, uint8_t>{7,     8  },
         std::pair<uint32_t, uint8_t>{5,     6  },
        std::pair<uint32_t, uint8_t>{3,     4  },
         std::pair<uint32_t, uint8_t>{12307, 122}
    };
    REQUIRE(lookup<TEST>(5) == 6);
    REQUIRE(lookup<TEST>(7) == 8);
    REQUIRE(lookup<TEST>(3) == 4);
    REQUIRE(lookup<TEST>(12307) == 122);
}
