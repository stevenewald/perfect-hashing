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

TEST_CASE("Map string to int", "[library]")
{
    static constexpr auto TEST = std::array{
        std::pair<std::string_view, uint8_t>{"hi",  8},
        std::pair<std::string_view, uint8_t>{"bye", 6}
    };
    REQUIRE(lookup<TEST, LookupMethod::word>("hi") == 8);
    REQUIRE(lookup<TEST, LookupMethod::word>("bye") == 6);
}

#ifndef TARGET_OS_X
TEST_CASE("Map int to string", "[library]")
{
    static constexpr auto TEST = std::array{
        std::pair<uint8_t, std::string>{8, "h"},
         std::pair<uint8_t, std::string>{6, "b"}
    };
    REQUIRE(lookup<TEST, LookupMethod::array>(8) == "h");
    REQUIRE(lookup<TEST, LookupMethod::word>(6) == "b");
}

TEST_CASE("Map int to long string", "[library]")
{
    static constexpr auto TEST = std::array{
        std::pair<uint8_t, std::string>{8, "hello!"},
        std::pair<uint8_t, std::string>{6, "bye!"  }
    };
    REQUIRE(lookup<TEST, LookupMethod::array>(8) == "hello!");
    REQUIRE(lookup<TEST, LookupMethod::array>(6) == "bye!");
}
#endif

TEST_CASE("Map const char* to int", "[library]")
{
    static constexpr auto TEST = std::array{
        std::pair<const char*, uint8_t>{"hi",  8},
        std::pair<const char*, uint8_t>{"bye", 6}
    };
    REQUIRE(lookup<TEST, LookupMethod::word>("hi") == 8);
    REQUIRE(lookup<TEST, LookupMethod::word>("bye") == 6);
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

TEST_CASE("to<T>(const char*)", "[library]")
{
    const char* str = "h";
    REQUIRE(gloss::to<uint64_t>(str) == uint8_t{'h'});
    REQUIRE(gloss::to<uint8_t>(str) == uint8_t{'h'});
    str = "hi!";
    REQUIRE(
        gloss::to<uint64_t>(str)
        == (uint32_t{'!'} << 16u | uint32_t{'i'} << 8u | uint32_t{'h'})
    );
    REQUIRE(gloss::to<uint16_t>(str) == (uint32_t{'i'} << 8u | uint32_t{'h'}));
    REQUIRE(gloss::to<uint8_t>(str) == uint32_t{'h'});
}

TEST_CASE("to<T>(std::string)", "[library]")
{
    std::string str{"h"};
    REQUIRE(gloss::to<uint64_t>(str) == uint8_t{'h'});
    REQUIRE(gloss::to<uint8_t>(str) == uint8_t{'h'});
    str = "hi!";
    REQUIRE(
        gloss::to<uint64_t>(str)
        == (uint32_t{'!'} << 16u | uint32_t{'i'} << 8u | uint32_t{'h'})
    );
    REQUIRE(gloss::to<uint16_t>(str) == (uint32_t{'i'} << 8u | uint32_t{'h'}));
    REQUIRE(gloss::to<uint8_t>(str) == uint32_t{'h'});
}

TEST_CASE("to<T>(std::string_view)", "[library]")
{
    constexpr std::string_view STR{"h"};
    static_assert(gloss::to<uint64_t>(STR) == uint8_t{'h'});
    static_assert(gloss::to<uint8_t>(STR) == uint8_t{'h'});
    constexpr std::string_view STR2 = "hi!";
    static_assert(
        gloss::to<uint64_t>(STR2)
        == (uint32_t{'!'} << 16u | uint32_t{'i'} << 8u | uint32_t{'h'})
    );
    static_assert(gloss::to<uint16_t>(STR2) == (uint32_t{'i'} << 8u | uint32_t{'h'}));
    static_assert(gloss::to<uint8_t>(STR2) == uint32_t{'h'});
}
