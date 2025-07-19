#include "gloss.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>

#include <array>

// TODO: clean these up. They're testing implementation details, which isn't ideal. Find
// cleaner way to do this

using gloss::lookup;
using gloss::LookupMethod;

// Small value tests

TEST_CASE("Map int to int", "[library]")
{
    static constexpr auto TEST = std::array{
        std::pair<uint32_t, uint8_t>{7, 8},
         std::pair<uint32_t, uint8_t>{5, 6}
    };
    static_assert(lookup<TEST, LookupMethod::word>(5u) == 6);
    static_assert(lookup<TEST, LookupMethod::word>(7u) == 8);

    static_assert(lookup<TEST, LookupMethod::array>(5u) == 6);
    static_assert(lookup<TEST, LookupMethod::array>(7u) == 8);
}

TEST_CASE("Map enum to int", "[library]")
{
    enum class TestEnum : uint8_t { first = 5, second = 2 };

    static constexpr auto TEST = std::array{
        std::pair<TestEnum, uint8_t>{TestEnum::first,  1},
        std::pair<TestEnum, uint8_t>{TestEnum::second, 2}
    };
    static_assert(lookup<TEST, LookupMethod::word>(TestEnum::first) == 1);
    static_assert(lookup<TEST, LookupMethod::word>(TestEnum::second) == 2);

    static_assert(lookup<TEST, LookupMethod::array>(TestEnum::first) == 1);
    static_assert(lookup<TEST, LookupMethod::array>(TestEnum::second) == 2);
}

TEST_CASE("Map int to enum", "[library]")
{
    enum class TestEnum : uint8_t { first = 5, second = 2 };

    static constexpr auto TEST = std::array{
        std::pair<uint8_t, TestEnum>{1, TestEnum::first },
        std::pair<uint8_t, TestEnum>{2, TestEnum::second}
    };
    static_assert(lookup<TEST, LookupMethod::word>(1) == TestEnum::first);
    static_assert(lookup<TEST, LookupMethod::word>(2) == TestEnum::second);

    static_assert(lookup<TEST, LookupMethod::array>(1) == TestEnum::first);
    static_assert(lookup<TEST, LookupMethod::array>(2) == TestEnum::second);
}

TEST_CASE("Map string to enum", "[library]")
{
    enum class TestEnum : uint8_t { first = 5, second = 2 };

    static constexpr auto TEST = std::array{
        std::pair<std::string_view, TestEnum>{"one", TestEnum::first },
        std::pair<std::string_view, TestEnum>{"two", TestEnum::second}
    };
    static_assert(lookup<TEST, LookupMethod::word>("one") == TestEnum::first);
    static_assert(lookup<TEST, LookupMethod::word>("two") == TestEnum::second);

    static_assert(lookup<TEST, LookupMethod::array>("one") == TestEnum::first);
    static_assert(lookup<TEST, LookupMethod::array>("two") == TestEnum::second);
}

TEST_CASE("Map int to int 2", "[library]")
{
    static constexpr auto TEST = std::array{
        std::pair<uint32_t, uint32_t>{5, 1},
         std::pair<uint32_t, uint32_t>{4, 2},
        std::pair<uint32_t, uint32_t>{3, 3},
         std::pair<uint32_t, uint32_t>{2, 4},
        std::pair<uint32_t, uint32_t>{1, 5}
    };

    static_assert(gloss::lookup<TEST, LookupMethod::array>(5) == 1);
    static_assert(gloss::lookup<TEST, LookupMethod::array>(4) == 2);
    static_assert(gloss::lookup<TEST, LookupMethod::array>(3) == 3);
    static_assert(gloss::lookup<TEST, LookupMethod::array>(2) == 4);
    static_assert(gloss::lookup<TEST, LookupMethod::array>(1) == 5);
}

TEST_CASE("Map string to int", "[library]")
{
    static constexpr auto TEST = std::array{
        std::pair<std::string, uint8_t>{"hi",  8},
        std::pair<std::string, uint8_t>{"bye", 6}
    };

    static_assert(lookup<TEST, LookupMethod::word>("hi") == 8);
    static_assert(lookup<TEST, LookupMethod::word>("bye") == 6);

    static_assert(lookup<TEST, LookupMethod::array>("hi") == 8);
    static_assert(lookup<TEST, LookupMethod::array>("bye") == 6);
}

TEST_CASE("Map const char* to int", "[library]")
{
    static constexpr auto TEST = std::array{
        std::pair<const char*, uint8_t>{"hi",  8},
        std::pair<const char*, uint8_t>{"bye", 6}
    };

    static_assert(lookup<TEST, LookupMethod::word>("hi") == 8);
    static_assert(lookup<TEST, LookupMethod::word>("bye") == 6);

    static_assert(lookup<TEST, LookupMethod::array>("hi") == 8);
    static_assert(lookup<TEST, LookupMethod::array>("bye") == 6);
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
    for (std::uint32_t i = 1; i <= 12; ++i) {
        REQUIRE(gloss::lookup<TEST, LookupMethod::array>(i) == i + 10);
    }
}

// Array tests

TEST_CASE("Map long string to int", "[library]")
{
    static constexpr auto TEST = std::array{
        std::pair<std::string, uint8_t>{"hello!",   8},
        std::pair<std::string, uint8_t>{"bye!",     6},
        std::pair<std::string, uint8_t>{"gutentag", 3}
    };

    static_assert(lookup<TEST, LookupMethod::array>("hello!") == 8);
    static_assert(lookup<TEST, LookupMethod::array>("bye!") == 6);
    static_assert(lookup<TEST, LookupMethod::array>("gutentag") == 3);
}

TEST_CASE("Map very long string to int", "[library]")
{
    static constexpr auto TEST = std::array{
        std::pair<std::string, uint8_t>{"I need int128!",  1},
        std::pair<std::string, uint8_t>{"this is another", 2},
        std::pair<std::string, uint8_t>{"gutentag",        3}
    };

    static_assert(lookup<TEST, LookupMethod::array>("I need int128!") == 1);
    static_assert(lookup<TEST, LookupMethod::array>("this is another") == 2);
    static_assert(lookup<TEST, LookupMethod::array>("gutentag") == 3);
}

TEST_CASE("Many int to int mappings", "[library]")
{
    static constexpr std::size_t SIZE = 64;
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

// to<T> tests

TEST_CASE("to<T>(const char*)", "[library]")
{
    constexpr const char* STR = "h";
    static_assert(gloss::to<uint64_t>(STR) == uint8_t{'h'});
    static_assert(gloss::to<uint8_t>(STR) == uint8_t{'h'});
    constexpr const char* STR2 = "hi!";
    static_assert(
        gloss::to<uint64_t>(STR2)
        == (uint32_t{'!'} << 16u | uint32_t{'i'} << 8u | uint32_t{'h'})
    );
    static_assert(gloss::to<uint16_t>(STR2) == (uint32_t{'i'} << 8u | uint32_t{'h'}));
    static_assert(gloss::to<uint8_t>(STR2) == uint32_t{'h'});
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

TEST_CASE("pext", "[library]")
{
    constexpr unsigned int MASK = 0b101u;
    static_assert(gloss::pext(0b010u, MASK) == 0b000u);
    static_assert(gloss::pext(0b110u, MASK) == 0b010u);
}

TEST_CASE("Mask uniqueness", "[library]")
{
    static constexpr auto TEST1 = std::array{
        std::pair{0b11, 0},
        std::pair{0b01, 0}
    };
    static_assert(gloss::find_mask<TEST1>() == 0b10);

    static constexpr auto TEST2 = std::array{
        std::pair{0b101, 0},
        std::pair{0b111, 0}
    };
    static_assert(gloss::find_mask<TEST2>() == 0b010);

    static constexpr auto TEST3 = std::array{
        std::pair{0b101, 0},
        std::pair{0b110, 0},
        std::pair{0b111, 0}
    };
    static_assert(gloss::find_mask<TEST3>() == 0b011);
}
