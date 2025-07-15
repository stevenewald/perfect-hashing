#include "gloss/gloss.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>

#include <array>

// TODO: clean these up. They're testing implementation details, which isn't ideal. Find
// cleaner way to do this

using gloss::lookup;
using gloss::LookupMethod;

TEST_CASE("Map int to int", "[library]")
{
    static constexpr auto TEST = std::array{
        std::pair<uint32_t, uint8_t>{7, 8},
         std::pair<uint32_t, uint8_t>{5, 6}
    };
    REQUIRE(lookup<TEST, LookupMethod::word>(5u) == 6);
    REQUIRE(lookup<TEST, LookupMethod::word>(7u) == 8);
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
        std::pair<uint32_t, uint32_t>{11, 21},
         std::pair<uint32_t, uint32_t>{12, 22},
    };
    for (std::uint32_t i = 1; i <= 12; ++i) {
        REQUIRE(gloss::lookup<TEST, LookupMethod::word>(i) == i + 10);
    }
}

TEST_CASE("Array LUT", "[library]")
{
    static constexpr std::size_t SIZE = 128;
    static constexpr std::array<std::pair<uint32_t, uint32_t>, SIZE> TEST = []() {
        std::array<std::pair<uint32_t, uint32_t>, SIZE> arr;
        for (std::size_t i = 0; i < arr.size(); ++i) {
            arr[i] = {i, i + 13};
        }
        return arr;
    }();
    for (std::uint32_t i = 0; i < SIZE; ++i) {
        REQUIRE(gloss::lookup<TEST, LookupMethod::array>(i) == i + 13);
    }
}
