#pragma once

#include <cassert>
#include <cstdint>

#include <algorithm>

namespace gloss {

namespace random {
// https://www.pcg-random.org
struct pcg {
    [[nodiscard]] constexpr auto
    operator()() noexcept -> std::uint32_t
    {
        std::uint64_t old_state = state;
        state = old_state * 6364136223846793005u + increment;
        auto xor_shifted =
            static_cast<std::uint32_t>(((old_state >> 18u) ^ old_state) >> 27u);
        auto rot = static_cast<std::uint32_t>(old_state >> 59u);
        return (xor_shifted >> rot) | (xor_shifted << ((-rot) & 31u));
    }

    std::uint64_t increment{1442695040888963407u};
    std::uint64_t state{5573589319906701683u + increment};
};
} // namespace random

template <typename P>
concept Pair = requires {
    typename P::first_type;
    typename P::second_type;
};

template <typename R>
concept PairRange =
    std::ranges::forward_range<R> && Pair<std::ranges::range_value_t<R>>;

template <const auto& Table, typename ValueType>
requires PairRange<decltype(Table)>
struct lookup_lut {
    using key_type = std::ranges::range_value_t<decltype(Table)>::first_type;
    using mapped_type = std::ranges::range_value_t<decltype(Table)>::second_type;

    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    static constexpr ValueType MAX_BITS = []() {
        mapped_type max = 0;
        for (const auto& pair : Table) {
            max = std::max(pair.second, max);
        }
        // std::countl_zero counts differently, so use builtin
        return __builtin_clz(max);
    }();

    using value_type = u64;

    consteval explicit lookup_lut(std::uint32_t max_attempts = 100'000) noexcept
    {
        random::pcg rand_pcg{};

        // Values won't fit in the LUT
        if (NBITS * Table.size() > (sizeof(ValueType) * __CHAR_BIT__))
            return;

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

    constexpr explicit
    operator bool() const noexcept
    {
        return MASK and SHIFT and magic_ and lut_;
    }

    constexpr mapped_type
    operator()(const key_type& search_key) const noexcept
    {
        return static_cast<mapped_type>(
            (lut_ >> ((search_key * magic_) >> SHIFT)) & MASK
        );
    }

private:
    static constexpr ValueType NBITS = (sizeof(u32) * __CHAR_BIT__) - MAX_BITS;
    static constexpr ValueType MASK = (1uz << NBITS) - 1uz;
    static constexpr ValueType SHIFT = (sizeof(u32) * __CHAR_BIT__) - NBITS;

    value_type magic_{};
    value_type lut_{};
};

template <const auto& Table>
auto
lookup(const auto& search_key)
{
    // first, try with 32 bits. If that fails, fallback to 64
    if constexpr (constexpr lookup_lut<Table, std::uint32_t> TABLE{}; TABLE) {
        return TABLE(search_key);
    }
    else {
        return lookup_lut<Table, std::uint64_t>{}(search_key);
    }
}
} // namespace gloss
