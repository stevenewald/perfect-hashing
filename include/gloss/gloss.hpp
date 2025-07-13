#pragma once

#include "gloss/random.hpp"

#include <fmt/core.h>

#include <cassert>

#include <algorithm>
#include <string>

namespace gloss {
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

template <const auto& Table>
requires PairRange<decltype(Table)>
struct lookup_lut {
    using key_type = std::ranges::range_value_t<decltype(Table)>::first_type;
    using mapped_type = std::ranges::range_value_t<decltype(Table)>::second_type;

    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    // TODO: actually make conditional
    using value_type = std::conditional_t<sizeof(key_type) <= sizeof(u32), u64, u64>;

    consteval explicit lookup_lut(std::uint32_t max_attempts = 100'000)
    {
        random::pcg rand_pcg{};

        auto attempt_find_perfect_hash = [&]() {
            magic_ = rand_pcg();
            for (const auto& [key, value] : Table) {
                u32 shift = u32((key * magic_) >> SHIFT);
                if (shift >= sizeof(value_type) * __CHAR_BIT__) {
                    lut_ = {};
                    return;
                }

                lut_ |= value_type(value) << shift;
            }

            for (const auto& [key, value] : Table) {
                if ((lut_ >> (value_type(key) * magic_ >> SHIFT) & MASK) != value) {
                    lut_ = {};
                    return;
                }
            }
        };

        while (lut_ == 0 && max_attempts-- > 0) {
            attempt_find_perfect_hash();
        }
    }

    constexpr mapped_type
    operator()(const key_type& search_key)
    {
        return static_cast<mapped_type>(
            (lut_ >> ((search_key * magic_) >> SHIFT)) & MASK
        );
    }

private:
    static constexpr u32 MAX_BITS = []() {
        mapped_type max = 0;
        for (const auto& pair : Table) {
            max = std::max(pair.second, max);
        }
        // std::countl_zero counts differently. use builtin
        return __builtin_clz(max);
    }();

    static constexpr value_type NBITS = (sizeof(u32) * __CHAR_BIT__) - MAX_BITS;
    static constexpr value_type MASK = (1uz << NBITS) - 1uz;
    static constexpr value_type SHIFT = (sizeof(u32) * __CHAR_BIT__) - NBITS;
    static_assert(
        NBITS * Table.size() <= (sizeof(value_type) * __CHAR_BIT__),
        "Values can fit in LUT"
    );

    value_type magic_{};
    value_type lut_{};
};

template <const auto& Table>
auto
lookup(const auto& search_key)
{
    lookup_lut<Table> table{};
    return table(search_key);
}
} // namespace gloss
