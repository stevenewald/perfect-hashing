#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#if __has_include(<experimental/simd>)
#  include <experimental/simd>
#endif

#include <algorithm>
#include <array>
#include <bit>
#include <iostream>
#include <ranges>
#include <string>

namespace gloss {

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
#if defined(__SIZEOF_INT128__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wpedantic"
using i128 = __int128;
using u128 = unsigned __int128;
#  pragma GCC diagnostic pop
#endif

namespace random {
// https://www.pcg-random.org
struct pcg {
    [[nodiscard]] constexpr auto
    operator()() noexcept -> std::uint32_t
    {
        std::uint64_t old_state = state_;
        state_ = old_state * 6364136223846793005u + increment_;
        auto xor_shifted =
            static_cast<std::uint32_t>(((old_state >> 18u) ^ old_state) >> 27u);
        auto rot = static_cast<std::uint32_t>(old_state >> 59u);
        return (xor_shifted >> rot) | (xor_shifted << ((32u - rot) & 31u));
    }

private:
    std::uint64_t increment_{1442695040888963407u};
    std::uint64_t state_{5573589319906701683u + increment_};
};
} // namespace random

template <typename To, typename From>
constexpr To
to(const From& data)
{
    constexpr bool STRING_TO_INTEGRAL = (requires { data.size(); });
    constexpr bool CHAR_ARRAY_TO_INTEGRAL = (requires(u32 n) { data[n]; });
    if consteval {
        if constexpr (STRING_TO_INTEGRAL) {
            To tmp{};
            for (std::size_t i = 0; i < sizeof(To) && i < data.size(); ++i) {
                tmp |= static_cast<To>(static_cast<To>(data[i]) << (i * __CHAR_BIT__));
            }
            return tmp;
        }
        else if constexpr (CHAR_ARRAY_TO_INTEGRAL) {
            To tmp{};
            for (std::size_t i = 0; i < sizeof(To) && data[i] != '\0'; ++i) {
                tmp |= static_cast<To>(static_cast<To>(data[i]) << (i * __CHAR_BIT__));
            }
            return tmp;
        }
        else {
            return static_cast<To>(data);
        }
    }
    else {
        if constexpr (STRING_TO_INTEGRAL) {
            To tmp{};
            __builtin_memcpy(
                &tmp, data.data(), data.size() < sizeof(To) ? data.size() : sizeof(To)
            );
            const auto index = static_cast<unsigned int>(data.size() * __CHAR_BIT__);
#ifdef __BMI2__
            if constexpr (sizeof(To) <= sizeof(u32)) {
                return static_cast<To>(__builtin_ia32_bzhi_si(tmp, index));
            }
            else if constexpr (sizeof(To) <= sizeof(u64)) {
                return __builtin_ia32_bzhi_di(tmp, index);
            }
            else {
#endif
                constexpr To SIZE = sizeof(To) * __CHAR_BIT__;
                return index >= SIZE
                           ? tmp
                           : static_cast<To>(tmp & ((To(1) << index) - To(1)));
#ifdef __BMI2__
            }
#endif
        }
        else if constexpr (CHAR_ARRAY_TO_INTEGRAL) {
            To tmp{};
            auto str_len = std::strlen(data);
            __builtin_memcpy(&tmp, data, str_len < sizeof(To) ? str_len : sizeof(To));
            const auto index = str_len * __CHAR_BIT__;
            constexpr auto SIZE = sizeof(To) * __CHAR_BIT__;
            return index >= SIZE ? tmp
                                 : static_cast<To>(tmp & ((To(1) << index) - To(1)));
        }
        else {
            return static_cast<To>(data);
        }
    }
}

// TODO: better name
template <std::size_t MaxSize>
consteval auto
get_type()
{
    if constexpr (MaxSize <= sizeof(u8)) {
        return u8{};
    }
    else if constexpr (MaxSize <= sizeof(u16)) {
        return u16{};
    }
    else if constexpr (MaxSize <= sizeof(u32)) {
        return u32{};
    }
    else if constexpr (MaxSize <= sizeof(u64)) {
        return u64{};
    }
#if defined(__SIZEOF_INT128__)
    else if constexpr (MaxSize <= sizeof(u128)) {
        return u128{};
    }
#endif
}

template <const auto& Table>
struct entries {
    using pair_type = std::ranges::range_value_t<decltype(Table)>;
    static constexpr auto SIZE = Table.size();

    // Support string_view, const char*, and integral keys
    using key_type = decltype([]() {
        if constexpr (requires { Table[0].first.size(); }) {
            static constexpr auto MAX_SIZE = []() {
                std::size_t max{};
                for (std::size_t i = 0; i < SIZE; ++i) {
                    max = std::max(max, Table[i].first.size());
                }
                return max;
            }();

            return get_type<MAX_SIZE>();
        }
        else if constexpr (std::is_same_v<
                               typename pair_type::first_type, const char*>) {
            static constexpr auto MAX_SIZE = []() {
                std::size_t max{};
                for (std::size_t i = 0; i < SIZE; ++i) {
                    const char* tmp = Table[i].first;
                    std::size_t len{};
                    while (*tmp != '\0') {
                        ++tmp;
                        ++len;
                    }
                    max = std::max(max, len);
                }
                return max;
            }();

            return get_type<MAX_SIZE>();
        }
        else {
            return typename pair_type::first_type{};
        }
    }());

    using mapped_type = pair_type::second_type;

    static constexpr auto MAPPINGS = []() {
        std::array<std::pair<key_type, mapped_type>, SIZE> entries;
        for (std::size_t i = 0; i < SIZE; ++i) {
            entries[i] = std::pair<key_type, mapped_type>{
                to<key_type>(Table[i].first), to<mapped_type>(Table[i].second)
            };
        }
        return entries;
    }();
};

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
struct lookup_magic_lut {
    using key_type = entries<Table>::key_type;
    using mapped_type = entries<Table>::mapped_type;
    using result_type = std::ranges::range_value_t<decltype(Table)>::second_type;

    consteval explicit lookup_magic_lut(std::uint32_t max_attempts = 10'000) noexcept
    {
        random::pcg rand_pcg{};

        // Values won't fit in the LUT
        if (NBITS * Table.size() > (sizeof(ValueType) * __CHAR_BIT__))
            return;

        auto attempt_find_perfect_hash = [&]() {
            magic_ = rand_pcg();
            for (const auto& [key, value] : entries<Table>::MAPPINGS) {
                u32 shift = u32((key * magic_) >> SHIFT);
                if (shift >= sizeof(ValueType) * __CHAR_BIT__) {
                    lut_ = {};
                    return;
                }

                lut_ |= ValueType(value) << shift;
            }

            for (const auto& [key, value] : entries<Table>::MAPPINGS) {
                if ((lut_ >> (ValueType(key) * magic_ >> SHIFT) & MASK) != value) {
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

    constexpr result_type
    operator()(const auto& search_key) const noexcept
    {
        return to<result_type>(
            (lut_ >> ((to<key_type>(search_key) * magic_) >> SHIFT)) & MASK
        );
    }

private:
    static constexpr ValueType MAX_BITS = []() {
        u32 max = 0;
        for (const auto& pair : entries<Table>::MAPPINGS) {
            max = std::max(static_cast<u32>(pair.second), max);
        }
        // std::countl_zero counts differently, so use builtin
        return std::countl_zero(max);
    }();
    static constexpr ValueType NBITS = (sizeof(u32) * __CHAR_BIT__) - MAX_BITS;
    static constexpr ValueType MASK = (1uz << NBITS) - 1uz;
    static constexpr ValueType SHIFT = (sizeof(u32) * __CHAR_BIT__) - NBITS;

    ValueType magic_{};
    ValueType lut_{};
};

template <typename T, T Size = sizeof(T) * __CHAR_BIT__>
constexpr T
pext(const T& value, T mask)
{
    auto manual_pext = [&]() {
        T tmp{};
        auto dst_pos = 0u;
        for (std::size_t i = 0; i < Size; ++i) {
            if (mask & 1)
                tmp |= ((value >> i) & 1) << dst_pos++;
            mask >>= 1;
        }
        return tmp;
    };

    if consteval {
        return manual_pext();
    }
#ifdef __BMI2__
    if constexpr (requires { u32{mask}; }) {
        return __builtin_ia32_pext_si(value, mask);
    }
    if constexpr (requires { u64{mask}; }) {
        return static_cast<T>(__builtin_ia32_pext_di(value, mask));
    }

#endif
    return manual_pext();
}

template <const auto& Table>
consteval auto
find_mask() -> entries<Table>::key_type
{
    using key_type = entries<Table>::key_type;
    static constexpr auto MAPPINGS = entries<Table>::MAPPINGS;

    constexpr std::size_t SIZE = Table.size();
    constexpr int NUM_BITS = sizeof(key_type) * __CHAR_BIT__;
    key_type cur_mask = (key_type{}) - 1;

    for (int bit = NUM_BITS - 1; bit >= 0; --bit) {
        key_type test_mask = cur_mask & ~(key_type(1) << bit);

        bool unique = true;
        for (std::size_t i = 0; i < SIZE && unique; ++i) {
            for (std::size_t j = i + 1; j < SIZE; ++j) {
                if (((MAPPINGS[i].first & test_mask) == (MAPPINGS[j].first & test_mask)
                    )) {
                    unique = false;
                    break;
                }
            }
        }
        if (unique) {
            cur_mask = test_mask;
        }
    }
    return cur_mask;
}

template <const auto& Table>
requires PairRange<decltype(Table)>
struct lookup_pext {
    using value_type = u64;

    using key_type = entries<Table>::key_type;
    using mapped_type = entries<Table>::mapped_type;
    using result_type = std::ranges::range_value_t<decltype(Table)>::second_type;

    constexpr result_type
    operator()(const auto& search_key) const noexcept
    {
        return TABLE[static_cast<std::size_t>(pext(to<key_type>(search_key), MASK))];
    }

private:
    static constexpr key_type MASK = find_mask<Table>();
    static constexpr key_type SIZE = []() {
        key_type max{};
        for (const auto& [key, _] : entries<Table>::MAPPINGS) {
            max = std::max(max, pext(to<key_type>(key), MASK));
        }
        return max + 1u;
    }();

    static constexpr std::array<mapped_type, SIZE> TABLE = []() {
        std::array<mapped_type, SIZE> table{};
        for (const auto& [key, value] : entries<Table>::MAPPINGS) {
            table[static_cast<std::size_t>(pext(to<key_type>(key), MASK))] =
                to<mapped_type>(value);
        }
        return table;
    }();
};

template <const auto& Table>
requires PairRange<decltype(Table)>
struct lookup_magic_array {
    using value_type = u64;

    using key_type = entries<Table>::key_type;
    using mapped_type = entries<Table>::mapped_type;
    using result_type = std::ranges::range_value_t<decltype(Table)>::second_type;

    consteval explicit lookup_magic_array(std::uint32_t max_attempts = 10'000) noexcept
    {
        random::pcg rand_pcg{};

        auto attempt_find_perfect_hash = [&]() {
            magic_ = rand_pcg();
            for (const auto& [key, value] : entries<Table>::MAPPINGS) {
                u32 shift = u32(((key * magic_) >> SHIFT) & SIZE_MASK);
                if (shift >= SIZE) {
                    table_ = {};
                    magic_ = {};
                    return;
                }

                table_[shift] = to<mapped_type>(value);
            }

            for (const auto& [key, value] : entries<Table>::MAPPINGS) {
                if (table_[(to<key_type>(key) * magic_ >> SHIFT) & SIZE_MASK]
                    != to<mapped_type>(value)) {
                    table_ = {};
                    magic_ = {};
                    return;
                }
            }
        };

        while (magic_ == 0 && max_attempts-- > 0) {
            attempt_find_perfect_hash();
        }
    }

    constexpr explicit
    operator bool() const noexcept
    {
        return magic_ != 0;
    }

    constexpr result_type
    operator()(const auto& search_key) const noexcept
    {
        return to<result_type>(
            table_[((to<key_type>(search_key) * magic_ >> SHIFT)) & SIZE_MASK]
        );
    }

private:
    static constexpr std::size_t SIZE = Table.size();
    static constexpr value_type MAX_BITS = []() {
        u32 max = 0;
        for (const auto& pair : entries<Table>::MAPPINGS) {
            max = std::max(static_cast<u32>(pair.second), max);
        }
        // std::countl_zero counts differently, so use builtin
        return std::countl_zero(max);
    }();
    static constexpr value_type NBITS = (sizeof(u32) * __CHAR_BIT__) - MAX_BITS;
    static constexpr value_type SHIFT = (sizeof(u32) * __CHAR_BIT__) - NBITS;
    static constexpr u64 SIZE_MASK =
        (1u << static_cast<unsigned>(std::bit_width(SIZE - 1))) - 1u;

    value_type magic_{};
    std::array<value_type, SIZE> table_{};
};

enum class LookupMethod : std::uint8_t { word, array, any };

template <const auto& Table, LookupMethod Method = LookupMethod::any>
constexpr auto
lookup(const auto& search_key)
{
    if constexpr (Method != LookupMethod::array) {
        if constexpr (constexpr lookup_magic_lut<Table, u32> TABLE32{}; TABLE32) {
            return TABLE32(search_key);
        }
        else if constexpr (constexpr lookup_magic_lut<Table, u64> TABLE64{}; TABLE64) {
            return TABLE64(search_key);
        }
    }
#ifdef __BMI2__
    if constexpr (Method != LookupMethod::word) {
        return lookup_pext<Table>{}(search_key);
    }
#else
    if constexpr (Method != LookupMethod::word) {
        if constexpr (constexpr lookup_magic_array<Table> TABLE_ARRAY{}; TABLE_ARRAY) {
            return TABLE_ARRAY(search_key);
        }
    }
#endif
}

} // namespace gloss
