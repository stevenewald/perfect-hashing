#pragma once

#include "gloss/random.hpp"

#include <fmt/core.h>

#include <cassert>

#include <algorithm>
#include <array>
#include <bit>
#include <functional>
#include <source_location>
#include <string>
#include <utility>

inline auto
name() -> std::string
{
    return fmt::format("{}", "gloss");
}

template <typename P>
concept Pair = requires {
    typename P::first_type;
    typename P::second_type;
};

template <typename R>
concept PairRange =
    std::ranges::forward_range<R> && Pair<std::ranges::range_value_t<R>>;

// for shorthands
template <const auto& Table>
requires PairRange<decltype(Table)>
struct lookup_lut {
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    using first_type = std::ranges::range_value_t<decltype(Table)>::first_type;
    using second_type = std::ranges::range_value_t<decltype(Table)>::second_type;

    struct vals {
        static constexpr u32 max_bits = []() {
            second_type max = 0;
            for (const auto& pair : Table) {
                max = std::max(pair.second, max);
            }
            return __builtin_clz(max);
        }();
        u32 magic{};
        static constexpr u32 nbits = (sizeof(u32) * __CHAR_BIT__) - max_bits;
        static constexpr u32 mask = (1u << nbits) - 1u;
        static constexpr u32 shift = (sizeof(u32) * __CHAR_BIT__) - nbits;
        u32 lut = {};
    };

    constexpr second_type
    operator()(const first_type& search_key)
    {
        return (values.lut >> ((search_key * values.magic) >> values.shift))
               & values.mask;
    }

    static constexpr u64 MAX_ATTEMPTS = 100'000;
    static constexpr vals values = []() {
        rndom::pcg rand_pcg{};
        u64 max_attempts = MAX_ATTEMPTS;
        vals v = {};
        static_assert(v.nbits * Table.size() <= (sizeof(u32) * __CHAR_BIT__));
        while (max_attempts-- > 0) {
            v.magic = rand_pcg();
            for (const auto& pair : Table) {
                u32 s2 = ((pair.first * v.magic) >> v.shift);
                if (s2 >= sizeof(u32) * __CHAR_BIT__) {
                    v.lut = {};
                    break;
                }

                v.lut |= pair.second << s2;
            }
            if (v.lut == 0)
                continue;

            for (const auto& pair : Table) {
                if ((v.lut >> (pair.first * v.magic >> v.shift) & v.mask)
                    != pair.second) {
                    v.lut = {};
                    break;
                }
            }
            if (v.lut != 0)
                return v;
        }
    }();
};

template <const auto& Table>
auto
lookup(const auto& search_key)
{
    lookup_lut<Table> table{};
    return table(search_key);
}
